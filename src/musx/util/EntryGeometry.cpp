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

#include <cmath>

namespace musx {
namespace util {
namespace {

int64_t calcEntryGeometryStaffPositionSize(dom::Evpu lineSpace)
{
    return std::max<int64_t>(1, (int64_t(lineSpace) + 1) / 2);
}

} // namespace

EntryGeometry::Positions EntryGeometry::calc(const dom::EntryInfoPtr& entry, const StaffOriginOffsetResolver& originResolver)
{
    MUSX_ASSERT_IF(!entry) {
        throw std::invalid_argument("EntryGeometry requires an entry.");
    }
    Positions result;
    const auto frame = entry.getFrame();
    const auto doc = frame->getDocument();
    const auto raw = entry->getEntry();
    result.sourceStaffId = entry.getStaff();
    result.notes.resize(raw->notes.size());
    if (frame->getContext()->mirrorFrame) {
        result.resolution = Resolution::UnsupportedLegacyContext;
        return result;
    }
    const auto staff = entry.createCurrentStaff();
    MUSX_ASSERT_IF(!staff) {
        throw dom::integrity_error("Entry geometry source staff is unavailable.");
    }
    result.sourceStaffPositionSize = calcEntryGeometryStaffPositionSize(staff->lineSpace);
    const auto system = doc->calcSystemFromMeasure(frame->getRequestedPartId(), frame->getMeasure());
    result.sourceStaffScale = system ? system->calcStaffScaling(entry.getStaff()) : Fraction(1);
    if (result.sourceStaffScale <= 0) {
        result.resolution = Resolution::MissingGeometry;
        return result;
    }
    if (entry.calcDisplaysAsRest()) {
        result.restStaffPosition = entry.calcIsZeroNote() ? entry.calcZeroNotePosition()
            : entry.calcTopBottomStaffPositions().first;
        result.restVerticalPosition = int64_t(*result.restStaffPosition) * result.sourceStaffPositionSize;
        return result;
    }
    const auto voicing = frame->getContext().getPolicyPartVoicing();
    for (size_t index = 0; index < raw->notes.size(); ++index) {
        const dom::NoteInfoPtr note(entry, index);
        if (voicing && !voicing->calcShowsNote(note)) {
            continue;
        }
        auto& position = result.notes[index].emplace();
        position.staffId = note.calcStaff();
        const auto target = entry.createCurrentStaff(position.staffId);
        MUSX_ASSERT_IF(!target) {
            throw dom::integrity_error("Entry geometry target staff is unavailable.");
        }
        if (target->notationStyle == dom::others::Staff::NotationStyle::Tablature) {
            const auto tab = doc->getDetails()->getForNote<dom::details::TablatureNoteMods>(note);
            if (!tab || tab->stringNumber <= 0) {
                result.resolution = Resolution::MissingGeometry;
                continue;
            }
            position.staffPosition = 2 * (1 - tab->stringNumber);
        } else {
            position.staffPosition = note.calcNoteProperties().staffPosition;
        }
        double offset = 0;
        Fraction ratio(1);
        if (position.staffId != result.sourceStaffId) {
            StaffOriginOffsetRequest request{result.sourceStaffId, position.staffId, frame->getMeasure(),
                entry->elapsedDuration.calcEduDuration()};
            StaffOriginOffsetResolverResult resolved;
            if (originResolver) {
                resolved = originResolver(doc, frame->getRequestedPartId(), request);
            }
            if (resolved.decision == StaffOriginOffsetResolverDecision::NotHandled) {
                resolved = calcStaffOriginOffsetUsingSystemStaffProxy(doc, frame->getRequestedPartId(), request);
                if (result.source != Source::Caller) {
                    const auto staves = calcSystemStavesOrScrollView(doc, frame->getRequestedPartId(), frame->getMeasure());
                    result.source = !staves.empty() && staves[0]->getCmper() > 0
                        ? Source::SavedSystem : Source::ScrollViewProxy;
                }
            } else {
                result.source = Source::Caller;
            }
            if (resolved.decision != StaffOriginOffsetResolverDecision::Offset || !std::isfinite(resolved.targetOriginOffsetEvpu)) {
                result.resolution = Resolution::MissingGeometry;
                continue;
            }
            offset = -resolved.targetOriginOffsetEvpu / result.sourceStaffScale.toDouble();
            ratio = (system ? system->calcStaffScaling(position.staffId) : Fraction(1)) / result.sourceStaffScale;
            if (ratio <= 0) {
                result.resolution = Resolution::MissingGeometry;
                continue;
            }
        }
        const int64_t scaledNumerator = *position.staffPosition
            * calcEntryGeometryStaffPositionSize(target->lineSpace) * ratio.numerator();
        const int64_t denominator = ratio.denominator();
        // Round to nearest EVPU, with exact halves away from zero.
        const int64_t scaledPosition = scaledNumerator / denominator;
        const int64_t remainder = scaledNumerator % denominator;
        const int64_t roundingAdjustment = std::abs(remainder) * 2 >= denominator
            ? (scaledNumerator < 0 ? -1 : 1) : 0;
        position.verticalPosition = std::llround(offset) + scaledPosition + roundingAdjustment;
    }
    return result;
}

} // namespace util
} // namespace musx
