/*
 * Copyright (C) 2025, Robert Patterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "musx/musx.h"

namespace musx {
namespace util {
namespace {

void validateStemEntry(const dom::EntryInfoPtr& entry)
{
    MUSX_ASSERT_IF(!entry) { throw std::invalid_argument("Stem requires an entry."); }
}

Stem::Direction stemDirectionFromUpFlag(bool up)
{
    return up ? Stem::Direction::Up : Stem::Direction::Down;
}

bool hasUnsupportedStemContext(const dom::EntryInfoPtr& entry)
{
    return entry.getFrame()->getContext()->mirrorFrame;
}

Stem::DirectionInfo calcLocalStemOverride(const dom::EntryInfoPtr& entry, bool special)
{
    Stem::DirectionInfo result;
    const auto staff = entry.createCurrentStaff();
    MUSX_ASSERT_IF(!staff) { throw dom::integrity_error("Stem staff is unavailable."); }
    const auto [frozen, up] = entry.calcEntryStemSettings();
    const auto layer = entry.getFrame()->getLayerAttributes();
    std::optional<bool> value;
    if (staff->altNotation == dom::others::Staff::AlternateNotation::Rhythmic
        && staff->altLayer == entry.getFrame()->getLayerIndex() && !entry.calcDisplaysAsRest()) {
        value = staff->altRhythmStemsUp;
        result.reason = Stem::Reason::Rhythmic;
    } else if (frozen) {
        value = up;
        result.reason = Stem::Reason::EntryFreeze;
    } else if (layer && layer->freezeLayer && entry.calcIfLayerSettingsApply()) {
        value = layer->freezeStemsUp;
        result.reason = Stem::Reason::LayerFreeze;
    } else if (staff->stemDirection != dom::StemDirection::Default) {
        value = staff->stemDirection == dom::StemDirection::AlwaysUp;
        result.reason = Stem::Reason::Staff;
    } else {
        const auto raw = entry->getEntry();
        if (raw->v2Launch || raw->voice2 || raw->graceNote || (special && (raw->splitStem || raw->doubleStem))) {
            value = up;
            result.reason = Stem::Reason::Stored;
        }
    }
    if (value) {
        result.primaryDirection = stemDirectionFromUpFlag(*value);
        result.sourceEntry = entry;
    }
    return result;
}

bool isStemNoteShown(const dom::EntryInfoPtr& entry, size_t index)
{
    const auto voicing = entry.getFrame()->getContext().getPolicyPartVoicing();
    return !voicing || voicing->calcShowsNote(dom::NoteInfoPtr(entry, index));
}

void assignStemDirections(Stem::DirectionInfo& result, const dom::EntryInfoPtr& entry)
{
    const auto raw = entry->getEntry();
    result.assignments.assign(raw->notes.size(), Stem::Assignment::None);
    if (!result.primaryDirection || entry.calcDisplaysAsRest()) {
        return;
    }
    bool anySplit = false;
    for (const auto& note : raw->notes) anySplit = anySplit || note->upSplitStem;
    for (size_t index = 0; index < raw->notes.size(); ++index) {
        if (!isStemNoteShown(entry, index)) {
            continue;
        }
        auto& assignment = result.assignments[index];
        if (raw->splitStem && anySplit) {
            assignment = raw->notes[index]->upSplitStem ? Stem::Assignment::Up : Stem::Assignment::Down;
        } else if (raw->doubleStem || raw->splitStem) {
            assignment = Stem::Assignment::Both;
        } else {
            assignment = *result.primaryDirection == Stem::Direction::Up ? Stem::Assignment::Up : Stem::Assignment::Down;
        }
    }
}

Stem::Resolution calcStemGeometryResolution(const EntryGeometry::Positions& geometry)
{
    switch (geometry.resolution) {
    case EntryGeometry::Resolution::MissingGeometry:
        return Stem::Resolution::MissingGeometry;
    case EntryGeometry::Resolution::UnsupportedLegacyContext:
        return Stem::Resolution::UnsupportedLegacyContext;
    case EntryGeometry::Resolution::Resolved:
        break;
    }
    for (const auto& note : geometry.notes) {
        if (note && !note->verticalPosition) {
            return Stem::Resolution::MissingGeometry;
        }
    }
    if (geometry.restStaffPosition && !geometry.restVerticalPosition) {
        return Stem::Resolution::MissingGeometry;
    }
    return Stem::Resolution::Resolved;
}

Stem::Resolution resolveStemGeometry(const dom::EntryInfoPtr& entry, const Stem::Options& options,
    Stem::Geometry& output, Stem::DirectionInfo& result)
{
    if (options.geometryResolver) {
        auto supplied = options.geometryResolver(entry);
        if (supplied.decision == StaffOriginOffsetResolverDecision::Unavailable) {
            result.geometrySource = Stem::GeometrySource::Caller;
            return Stem::Resolution::MissingGeometry;
        }
        if (supplied.decision == StaffOriginOffsetResolverDecision::Offset) {
            output = std::move(supplied.geometry);
            MUSX_ASSERT_IF(output.entryGeometry.notes.size() != entry->getEntry()->notes.size()) {
                throw std::invalid_argument("Stem geometry must preserve original note indices.");
            }
            result.geometrySource = Stem::GeometrySource::Caller;
            return calcStemGeometryResolution(output.entryGeometry);
        }
    }
    output.entryGeometry = EntryGeometry::calc(entry, options.staffOriginOffsetResolver);
    const auto& geometry = output.entryGeometry;
    if (geometry.source != Stem::GeometrySource::NotRequired
        && (result.geometrySource != Stem::GeometrySource::Caller || geometry.source == Stem::GeometrySource::Caller)) {
        result.geometrySource = geometry.source;
    }
    const auto resolution = calcStemGeometryResolution(geometry);
    if (resolution != Stem::Resolution::Resolved) {
        return resolution;
    }
    const auto staff = entry.createCurrentStaff();
    MUSX_ASSERT_IF(!staff) {
        throw dom::integrity_error("Stem staff is unavailable.");
    }
    output.reversal = int64_t(staff->stemReversal) * geometry.sourceStaffPositionSize;
    const auto stemOptions = entry.getFrame()->getDocument()->getOptions()->get<dom::options::StemOptions>();
    MUSX_ASSERT_IF(!stemOptions) {
        throw dom::integrity_error("Stem options are unavailable.");
    }
    output.reverseAdjustment = stemOptions->revStemAdj;
    return Stem::Resolution::Resolved;
}
} // namespace

Stem::DirectionInfo Stem::calcForcedDirection(const dom::EntryInfoPtr& entry, const Options&)
{
    validateStemEntry(entry);
    const auto start = entry.findBeamStartOrCurrent();
    for (auto next = start; next; next = next.getNextInBeamGroup()) {
        if (hasUnsupportedStemContext(next)) {
            DirectionInfo result;
            result.resolution = Resolution::UnsupportedLegacyContext;
            return result;
        }
        auto result = calcLocalStemOverride(next, entry.calcUnbeamed());
        if (result.primaryDirection) {
            return result;
        }
    }
    return {};
}

Stem::DirectionInfo Stem::calcDefaultDirection(const dom::EntryInfoPtr& entry, const Options& options)
{
    validateStemEntry(entry);
    DirectionInfo result;
    const auto start = entry.findBeamStartOrCurrent();
    std::optional<int64_t> high, low, reversal;
    int64_t above = 0, below = 0;
    const auto beamOptions = entry.getFrame()->getDocument()->getOptions()->get<dom::options::BeamOptions>();
    const auto stemOptions = entry.getFrame()->getDocument()->getOptions()->get<dom::options::StemOptions>();
    MUSX_ASSERT_IF(!beamOptions || !stemOptions) { throw dom::integrity_error("Stem calculation requires beam and stem options."); }
    for (auto next = start; next; next = next.getNextInBeamGroup()) {
        if (hasUnsupportedStemContext(next)) { result.resolution = Resolution::UnsupportedLegacyContext; return result; }
        const auto raw = next->getEntry();
        Geometry positions;
        result.resolution = resolveStemGeometry(next, options, positions, result);
        if (result.resolution != Resolution::Resolved) {
            return result;
        }
        if (!reversal) {
            reversal = positions.reversal;
        }
        if (raw->graceNote || raw->isHidden) {
            continue;
        }
        std::vector<int64_t> all, up, down;
        for (size_t index = 0; index < positions.entryGeometry.notes.size(); ++index) {
            if (!positions.entryGeometry.notes[index] || !isStemNoteShown(next, index)) {
                continue;
            }
            const auto y = *positions.entryGeometry.notes[index]->verticalPosition;
            all.push_back(y);
            (raw->notes[index]->upSplitStem ? up : down).push_back(y);
        }
        const bool rest = next.calcDisplaysAsRest();
        if (rest && raw->isNote) {
            continue; // A chord excluded by part voicing has no directional position.
        }
        if (rest) {
            all.clear();
            if (positions.entryGeometry.restVerticalPosition) {
                all.push_back(*positions.entryGeometry.restVerticalPosition);
            }
        }
        if (all.empty()) {
            continue;
        }
        auto upperNotes = raw->splitStem && !up.empty() ? up : all;
        auto lowerNotes = raw->splitStem && !down.empty() ? down : all;
        int64_t upper = *std::max_element(upperNotes.begin(), upperNotes.end());
        int64_t lower = *std::min_element(lowerNotes.begin(), lowerNotes.end());
        if (!stemOptions->noReverseStems) {
            if (raw->reverseUpStem) {
                upper = *std::min_element(upperNotes.begin(), upperNotes.end()) - positions.reverseAdjustment;
            }
            if (raw->reverseDownStem) {
                lower = *std::max_element(lowerNotes.begin(), lowerNotes.end()) + positions.reverseAdjustment;
            }
        }
        for (const auto y : {upper, lower}) {
            above += y > positions.reversal;
            below += y < positions.reversal;
        }
        if (!rest || beamOptions->oldFinaleRestBeams || entry.calcUnbeamed()) {
            high = high ? std::max(*high, upper) : upper;
            low = low ? std::min(*low, lower) : lower;
        }
    }
    if (!high || !low || !reversal) { result.resolution = Resolution::NoEligiblePositions; return result; }
    const auto upperDistance = *high - *reversal;
    const auto lowerDistance = *reversal - *low;
    const bool tie = upperDistance == lowerDistance && !entry.calcUnbeamed();
    result.primaryDirection = stemDirectionFromUpFlag(tie ? below > above : upperDistance < lowerDistance);
    result.reason = tie ? Stem::Reason::BeamTieBreak : Stem::Reason::Extremes;
    result.sourceEntry = start;
    if (!entry.calcUnbeamed()) {
        result.beamDirection = result.primaryDirection;
    }
    assignStemDirections(result, entry);
    return result;
}

Stem::DirectionInfo Stem::calcEffectiveDirection(const dom::EntryInfoPtr& entry, const Options& options)
{
    auto result = calcForcedDirection(entry, options);
    if (result.resolution != Resolution::Resolved) {
        return result;
    }
    if (!result.primaryDirection) {
        result = calcDefaultDirection(entry, options);
    }
    if (!entry.calcUnbeamed()) {
        result.beamDirection = result.primaryDirection;
        const auto raw = entry->getEntry();
        if (raw->splitStem || raw->doubleStem) {
            auto local = calcLocalStemOverride(entry, true);
            result.primaryDirection = local.primaryDirection;
        }
    }
    assignStemDirections(result, entry);
    return result;
}

} // namespace util
} // namespace musx
