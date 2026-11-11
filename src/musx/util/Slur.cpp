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
#include "musx/util/Slur.h"

namespace musx {
namespace util {
namespace {
using SlurContourDirection = dom::CurveContourDirection;
void validateSlurShape(const dom::others::SmartShape& shape)
{
    MUSX_ASSERT_IF(!shape.calcIsSlur())
    {
        throw std::invalid_argument("Slur requires a slur shape.");
    }
}
SlurContourDirection slurContourFromStemDirection(Stem::Direction direction)
{
    return direction == Stem::Direction::Up ? SlurContourDirection::Up : SlurContourDirection::Down;
}
bool qualifiesForSlurDirectionOverride(const dom::EntryInfoPtr& entry)
{
    const auto raw = entry->getEntry();
    if (raw->graceNote) {
        return false;
    }
    const auto staff = entry.createCurrentStaff();
    MUSX_ASSERT_IF(!staff)
    {
        throw dom::integrity_error("Slur staff is unavailable.");
    }
    const auto layer = entry.getFrame()->getLayerAttributes();
    return entry.calcEntryStemSettings().first || raw->voice2 || raw->v2Launch ||
           (layer && layer->freezeLayer && entry.calcIfLayerSettingsApply()) ||
           (staff->altNotation ==
                dom::others::Staff::AlternateNotation::Rhythmic &&
            staff->altLayer == entry.getFrame()->getLayerIndex() &&
            !entry.calcDisplaysAsRest());
}
} // namespace

Slur::DirectionInfo
Slur::calcEffectiveDirection(const dom::others::SmartShape& shape,
                             const Options& options)
{
    validateSlurShape(shape);
    DirectionInfo result;
    result.direction = shape.calcDefinitionContourDirection();
    if (result.direction != SlurContourDirection::Unspecified) {
        result.reason = Reason::Definition;
        return result;
    }
    return calcDefaultDirection(shape, options);
}

Slur::DirectionInfo
Slur::calcDefaultDirection(const dom::others::SmartShape& shape,
                           const Options& options)
{
    validateSlurShape(shape);
    DirectionInfo result;
    auto fail = [&](Resolution resolution) {
        result.resolution = resolution;
        return result;
    };
    auto choose = [&](SlurContourDirection direction, Reason reason,
                      const dom::EntryInfoPtr& source) {
        result.direction = direction;
        result.reason = reason;
        result.sourceEntry = source;
        return result;
    };
    if (!shape.entryBased) {
        return fail(Resolution::AutomaticBeatAttached);
    }
    if (!shape.startTermSeg || !shape.endTermSeg ||
        !shape.startTermSeg->endPoint || !shape.endTermSeg->endPoint) {
        return fail(Resolution::MissingEndpoint);
    }
    if (!shape.startTermSeg->endPoint->entryNumber ||
        !shape.endTermSeg->endPoint->entryNumber) {
        return fail(Resolution::MissingEndpoint);
    }
    const auto start = shape.startTermSeg->endPoint->calcAssociatedEntry();
    const auto end = shape.endTermSeg->endPoint->calcAssociatedEntry();
    if (!start || !end) {
        return fail(Resolution::MissingEndpoint);
    }
    if (shape.calcIsBackwards()) {
        return fail(Resolution::InvalidSpan);
    }
    const bool sameVoice =
        start.getStaff() == end.getStaff() &&
        start.getFrame()->getLayerIndex() == end.getFrame()->getLayerIndex() &&
        start->getEntry()->voice2 == end->getEntry()->voice2;
    std::vector<dom::EntryInfoPtr> entries;
    if (!sameVoice) {
        entries = {start, end};
    } else {
        bool started = false;
        bool ended = false;
        const int voice = start.getVoice();
        for (auto measure = start.getMeasure(); measure <= end.getMeasure();
             ++measure) {
            const dom::details::GFrameHoldContext context(shape.getDocument(),
                                                          shape.getRequestedPartId(),
                                                          start.getStaff(), measure);
            if (!context) {
                continue;
            }
            const auto frame =
                context.createEntryFrame(start.getFrame()->getLayerIndex());
            if (!frame) {
                continue;
            }
            for (auto it = frame->getFirstInterpretedIterator(voice, false);
                 it.getEntryInfo(); it = it.getNext()) {
                const auto entry = it.getEntryInfo();
                if (entry.isSameEntry(start)) {
                    started = true;
                }
                if (started && !it.getEffectiveHidden()) {
                    entries.push_back(entry);
                }
                if (entry.isSameEntry(end)) {
                    ended = true;
                    break;
                }
            }
            if (ended) {
                break;
            }
        }
        if (!started || !ended) {
            return fail(Resolution::InvalidSpan);
        }
    }
    if (!sameVoice) {
        entries.erase(
            std::remove_if(entries.begin(), entries.end(),
                           [](const auto& e) {
                               for (auto it =
                                        e.getFrame()->getFirstInterpretedIterator(
                                            e.getVoice(), false);
                                    it.getEntryInfo(); it = it.getNext()) {
                                   if (it.getEntryInfo().isSameEntry(e)) {
                                       return it.getEffectiveHidden();
                                   }
                               }
                               return true;
                           }),
            entries.end());
    }
    if (entries.empty()) {
        return fail(Resolution::NoEligibleEntries);
    }
    for (const auto& entry : entries) {
        if (entry.getFrame()->getContext()->mirrorFrame) {
            return fail(Resolution::UnsupportedLegacyContext);
        }
    }
    const auto startStaff = start.createCurrentStaff();
    const auto endStaff = end.createCurrentStaff();
    MUSX_ASSERT_IF(!startStaff || !endStaff)
    {
        throw dom::integrity_error("Slur endpoint staff is unavailable.");
    }
    if (startStaff->notationStyle ==
            dom::others::Staff::NotationStyle::Tablature &&
        endStaff->notationStyle == dom::others::Staff::NotationStyle::Tablature &&
        startStaff->hideStems == endStaff->hideStems &&
        startStaff->stemDirection == endStaff->stemDirection) {
        if (startStaff->hideStems ||
            startStaff->stemDirection == dom::StemDirection::AlwaysDown) {
            return choose(SlurContourDirection::Up, Reason::Tablature, start);
        }
        if (startStaff->stemDirection == dom::StemDirection::AlwaysUp) {
            return choose(SlurContourDirection::Down, Reason::Tablature, start);
        }
    }
    dom::EntryInfoPtr forced;
    dom::EntryInfoPtr firstMain;
    for (const auto& entry : entries) {
        if (!forced && qualifiesForSlurDirectionOverride(entry)) {
            forced = entry;
        }
        if (!firstMain && !entry->getEntry()->graceNote) {
            firstMain = entry;
        }
    }
    Stem::Options stemOptions;
    stemOptions.staffOriginOffsetResolver = options.staffOriginOffsetResolver;
    stemOptions.geometryResolver = options.geometryResolver;
    auto stem = [&](const dom::EntryInfoPtr& entry) {
        auto info = Stem::calcEffectiveDirection(entry, stemOptions);
        if (info.geometrySource == GeometrySource::Caller ||
            result.geometrySource == GeometrySource::NotRequired) {
            result.geometrySource = info.geometrySource;
        }
        return info;
    };
    auto failure = [&](const Stem::DirectionInfo& info) {
        return fail(info.resolution == Stem::Resolution::UnsupportedLegacyContext
                        ? Resolution::UnsupportedLegacyContext
                    : info.resolution == Stem::Resolution::NoEligiblePositions
                        ? Resolution::NoEligibleEntries
                        : Resolution::MissingGeometry);
    };
    const bool graceSpan = sameVoice && start->getEntry()->graceNote &&
                           firstMain && firstMain.isSameEntry(end);
    bool graceSameStems = false;
    std::optional<Stem::Direction> graceDirection;
    if (graceSpan) {
        const auto first = stem(start);
        const auto last = stem(end);
        if (!first.primaryDirection) {
            return failure(first);
        }
        if (!last.primaryDirection) {
            return failure(last);
        }
        graceSameStems = first.primaryDirection == last.primaryDirection;
        graceDirection = first.primaryDirection;
        size_t count = 0;
        const auto voicing = end.getFrame()->getContext().getPolicyPartVoicing();
        for (size_t i = 0; i < end->getEntry()->notes.size(); ++i) {
            if (!voicing || voicing->calcShowsNote(dom::NoteInfoPtr(end, i))) {
                ++count;
            }
        }
        constexpr size_t chordSize = 2;
        if (*first.primaryDirection == Stem::Direction::Up &&
            *last.primaryDirection == Stem::Direction::Down && count < chordSize) {
            return choose(SlurContourDirection::Down, Reason::GraceException, start);
        }
    }
    if (forced) {
        const auto info = stem(forced);
        if (!info.primaryDirection) {
            return failure(info);
        }
        return choose(slurContourFromStemDirection(*info.primaryDirection),
                      graceSameStems && graceDirection == info.primaryDirection
                          ? Reason::GraceException
                          : Reason::ContextualOverride,
                      forced);
    }
    std::optional<Stem::DirectionInfo> unresolved;
    bool up = false;
    bool down = false;
    for (const auto& entry : entries) {
        const auto info = stem(entry);
        if (!info.primaryDirection) {
            if (!unresolved) {
                unresolved = info;
            }
            continue;
        }
        up |= *info.primaryDirection == Stem::Direction::Up;
        down |= *info.primaryDirection == Stem::Direction::Down;
        if (entry->getEntry()->splitStem || entry->getEntry()->doubleStem) {
            up = down = true;
        }
    }
    if (unresolved && !(up && down)) {
        return failure(*unresolved);
    }
    return choose(up && !down ? SlurContourDirection::Down : SlurContourDirection::Up,
                  up && down ? Reason::MixedStems : Reason::UniformStems, start);
}

} // namespace util
} // namespace musx
