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

#include <cmath>
#include <string>

#include "musx/musx.h"

namespace musx::util {
namespace {

using dom::KnownSmartShapeType;
using dom::MusxInstance;
using dom::NoteInfoPtr;
using dom::others::SmartShape;
using CL = dom::others::SmartShapeCustomLine;

constexpr dom::Efix RIGHT_HOOK_MAX_LENGTH_EVPU = 48 * dom::Efix(dom::EFIX_PER_EVPU);

static bool isRightHookLine(const dom::Evpu hookLength)
{
    return std::abs(hookLength) > 0
        && std::abs(hookLength) <= RIGHT_HOOK_MAX_LENGTH_EVPU;
}

static bool calcIsPotentialTie(const SmartShape& smartShape, const dom::EntryInfoPtr& forStartEntry)
{
    // 6 EVPU is approximately 1/4 staff space; threshold is applied strictly (|delta| < 6 EVPU).
    constexpr dom::Evpu HORIZONTAL_THRESHOLD = 6;

    switch (smartShape.shapeType) {
    case SmartShape::ShapeType::SlurAuto:
    case SmartShape::ShapeType::SlurDown:
    case SmartShape::ShapeType::SlurUp:
        break;

    default:
        return false;
    }

    if (smartShape.entryBased) {
        // Both ends must be the same entry for entry-attached shapes.
        const auto startEntryNum = smartShape.startTermSeg->endPoint->entryNumber;
        if (startEntryNum != smartShape.endTermSeg->endPoint->entryNumber || startEntryNum != forStartEntry->getEntry()->getEntryNumber()) {
            return false;
        }
    } else {
        if (smartShape.startTermSeg->endPoint->calcStaff() != smartShape.endTermSeg->endPoint->calcStaff()) {
            return false;
        }
        const auto thisStartEntry = smartShape.startTermSeg->endPoint->calcAssociatedEntry();
        if (!forStartEntry.isSameEntry(thisStartEntry)) {
            return false;
        }
    }

    // Both ends must match up with an equivalent entry for beat-attached shapes.
    auto endEntry = smartShape.endTermSeg->endPoint->calcAssociatedEntry();
    if (!endEntry) {
        return false;
    }
    if (forStartEntry.calcDisplaysAsRest() || endEntry.calcDisplaysAsRest()) {
        return false;
    }
    // Allow arpeggiation ties (e.g., C -> CEG): end entry may add pitches, but must contain all start pitches.
    if (!endEntry.calcContainsPitchContent(forStartEntry)) {
        return false;
    }
    // If either endpoint is inactive, we cannot compare vertical positions, so permissively allow them.
    if (smartShape.entryBased) {
        if (!smartShape.startTermSeg->endPointAdj->active || !smartShape.endTermSeg->endPointAdj->active) {
            return true;
        }
    }
    const dom::Evpu vertDiff = smartShape.startTermSeg->endPointAdj->calcVertOffset()
        - smartShape.endTermSeg->endPointAdj->calcVertOffset();
    return std::abs(vertDiff) < HORIZONTAL_THRESHOLD;
}

static bool calcIsPotentialForwardTie(const SmartShape& smartShape, const dom::EntryInfoPtr& forStartEntry)
{
    if (!calcIsPotentialTie(smartShape, forStartEntry)) {
        return false;
    }
    const auto& startEndPoint = *smartShape.startTermSeg->endPoint;
    const auto& endEndPoint = *smartShape.endTermSeg->endPoint;
    const int cmp = startEndPoint.compareMetricPosition(endEndPoint);
    if (cmp) {
        return cmp < 0;
    }
    const auto startOffset = smartShape.startTermSeg->endPointAdj->calcHorzOffset();
    const auto endOffset = smartShape.endTermSeg->endPointAdj->calcHorzOffset();
    return utils::calcIsPseudoForwardTie(startOffset, endOffset);
}

} // namespace

KnownSmartShapeType recognizeSmartShape(const SmartShape& smartShape)
{
    if (smartShape.entryBased) {
        return KnownSmartShapeType::Unrecognized;
    }
    if (smartShape.shapeType != SmartShape::ShapeType::CustomLine || smartShape.lineStyleId == 0) {
        return KnownSmartShapeType::Unrecognized;
    }
    const auto customShape = smartShape.getDocument()->getOthers()->get<CL>(smartShape.getRequestedPartId(), smartShape.lineStyleId);
    if (!customShape) {
        Logger::log(Logger::LogLevel::Warning, "Unable to find custom shape for smart shape " + std::to_string(smartShape.getCmper()));
        return KnownSmartShapeType::Unrecognized;
    }

    if (customShape->lineStyle == CL::LineStyle::Solid
        && customShape->lineCapStartType == CL::LineCapType::Hook
        && customShape->lineCapEndType == CL::LineCapType::Hook) {
        if (music_theory::sign(customShape->lineCapStartHookLength) == music_theory::sign(customShape->lineCapEndHookLength)) {
            if (isRightHookLine(customShape->lineCapStartHookLength) && isRightHookLine(customShape->lineCapEndHookLength)) {
                if (smartShape.startTermSeg->endPoint->compareMetricPosition(*smartShape.endTermSeg->endPoint) != 0) {
                    return KnownSmartShapeType::Unrecognized;
                }
                if (smartShape.startTermSeg->endPointAdj->calcHorzOffset() != smartShape.endTermSeg->endPointAdj->calcHorzOffset()) {
                    return KnownSmartShapeType::Unrecognized;
                }
                const auto scrollView = smartShape.getDocument()->getScrollViewStaves(smartShape.getRequestedPartId());
                const auto startStaffIndex = scrollView.getIndexForStaff(smartShape.startTermSeg->endPoint->calcStaff());
                const auto endStaffIndex = scrollView.getIndexForStaff(smartShape.endTermSeg->endPoint->calcStaff());
                if (!startStaffIndex || !endStaffIndex) {
                    return KnownSmartShapeType::Unrecognized;
                }
                const dom::Evpu sourceDistFromTop = scrollView[*startStaffIndex]->distFromTop;
                const double startPosition = static_cast<double>(sourceDistFromTop - scrollView[*startStaffIndex]->distFromTop
                    - smartShape.startTermSeg->endPointAdj->calcVertOffset());
                const double endPosition = static_cast<double>(sourceDistFromTop - scrollView[*endStaffIndex]->distFromTop
                    - smartShape.endTermSeg->endPointAdj->calcVertOffset());
                const double vertDiff = endPosition - startPosition;
                if (vertDiff < 0 && customShape->lineCapStartHookLength < 0) {
                    return KnownSmartShapeType::VerticalLineRightHooks;
                } else if (vertDiff > 0 && customShape->lineCapStartHookLength > 0) {
                    return KnownSmartShapeType::VerticalLineRightHooks;
                }
            }
        }
    }

    return KnownSmartShapeType::Unrecognized;
}

KnownSmartShapeType recognizeSmartShape(const MusxInstance<SmartShape>& smartShape)
{
    return smartShape ? recognizeSmartShape(*smartShape) : KnownSmartShapeType::Unrecognized;
}

bool calcIsSmartShapePseudoTie(
    const SmartShape& smartShape,
    utils::PseudoTieMode mode,
    const dom::EntryInfoPtr& forStartEntry)
{
    switch (mode) {
    case utils::PseudoTieMode::LaissezVibrer:
        if (!calcIsPotentialForwardTie(smartShape, forStartEntry)) {
            return false;
        }
        break;

    case utils::PseudoTieMode::TieEnd:
        if (!calcIsPotentialTie(smartShape, forStartEntry)) {
            return false;
        }
        {
            const auto startOffset = smartShape.startTermSeg->endPointAdj->calcHorzOffset();
            const auto endOffset = smartShape.endTermSeg->endPointAdj->calcHorzOffset();
            if (!utils::calcIsPseudoBackwardTie(startOffset, endOffset)) {
                return false;
            }
        }
        break;
    }

    return smartShape.startTermSeg->endPoint->compareMetricPosition(*smartShape.endTermSeg->endPoint) == 0;
}

NoteInfoPtr calcSmartShapeArpeggiatedTieToNote(const SmartShape& smartShape, const dom::EntryInfoPtr& forStartEntry)
{
    if (!calcIsPotentialForwardTie(smartShape, forStartEntry)) {
        return {};
    }
    const auto entry = forStartEntry->getEntry();
    MUSX_ASSERT_IF(entry->notes.empty()) {
        throw std::logic_error("calcIsPotentialForwardTie was true but startEntry had no notes.");
    }
    if (entry->notes.size() != 1) {
        return {};
    }
    const auto endEntry = smartShape.endTermSeg->endPoint->calcAssociatedEntry();
    MUSX_ASSERT_IF(!endEntry) {
        throw std::logic_error("calcIsPotentialForwardTie was true but endEntry did not exist.");
    }
    if (forStartEntry.isSameEntry(endEntry)) {
        return {}; // Must be a different entry.
    }

    const auto noteInfoPtr = NoteInfoPtr(forStartEntry, 0);
    if ((noteInfoPtr.calcHasTieStart() || forStartEntry.calcIsImmediatelyFollowedBy(endEntry)) && noteInfoPtr.calcTieTo()) {
        // If Finale already provides a real tie target, this smart shape is not a stand-in.
        return {};
    }

    auto endNote = noteInfoPtr.findEqualPitch(endEntry);
    MUSX_ASSERT_IF(!endNote) {
        throw std::logic_error("calcIsPotentialForwardTie was true but no matching pitch was found in endEntry.");
    }
    return endNote;
}

} // namespace musx::util
