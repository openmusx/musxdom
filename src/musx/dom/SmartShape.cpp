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
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <string>

#include "musx/musx.h"

namespace musx {
namespace dom {

namespace {

template <typename T, typename Parent>
std::shared_ptr<T> copyContained(
    const std::shared_ptr<T>& source, const std::shared_ptr<Parent>& parent)
{
    if (!source) {
        return nullptr;
    }
    auto result = std::make_shared<T>(parent);
    *result = *source;
    return result;
}

} // namespace

std::shared_ptr<others::SmartShape::TerminationSeg> PartContextRebinder<others::SmartShape>::copyTerminationSeg(
    const std::shared_ptr<others::SmartShape::TerminationSeg>& source,
    const std::shared_ptr<others::SmartShape>& parent)
{
    auto result = std::make_shared<others::SmartShape::TerminationSeg>(parent);
    if (source) {
        *result = *source;
        result->endPoint = copyContained(source->endPoint, parent);
        result->endPointAdj = copyContained(source->endPointAdj, parent);
        result->ctlPtAdj = copyContained(source->ctlPtAdj, parent);
        result->breakAdj = copyContained(source->breakAdj, parent);
    }
    return result;
}

void PartContextRebinder<others::SmartShape>::rebind(const std::shared_ptr<others::SmartShape>& shape)
{
    shape->startTermSeg = copyTerminationSeg(shape->startTermSeg, shape);
    shape->endTermSeg = copyTerminationSeg(shape->endTermSeg, shape);
    shape->fullCtlPtAdj = copyContained(shape->fullCtlPtAdj, shape);
}

void PartContextRebinder<details::CenterShape>::rebind(const std::shared_ptr<details::CenterShape>& shape)
{
    shape->startBreakAdj = copyContained(shape->startBreakAdj, shape);
    shape->endBreakAdj = copyContained(shape->endBreakAdj, shape);
    shape->ctlPtAdj = copyContained(shape->ctlPtAdj, shape);
}

// ********************
// ***** EndPoint *****
// ********************

EntryInfoPtr smartshape::EndPoint::calcAssociatedEntry(bool findExact) const
{
    const auto doc = getDocument();
    auto shapeParent = getParent<others::SmartShape>();
    MUSX_ASSERT_IF(!shapeParent) {
        throw std::logic_error("Unknown parent type for SmartShape::EndPoint.");
    }
    const Cmper forPartId = shapeParent->getRequestedPartId();

    EntryInfoPtr result;
    if (entryNumber != 0) {
        result = EntryInfoPtr::fromEntryNumber(doc, forPartId, entryNumber);
        if (!result) {
            MUSX_INTEGRITY_ERROR("SmartShape at Staff " + std::to_string(staffId) + " Measure " + std::to_string(measId)
                + " contains endpoint with invalid entry number " + std::to_string(entryNumber));
        }
        return result;
    }

    auto gfHold = details::GFrameHoldContext(doc, forPartId, staffId, measId);
    if (!gfHold) {
        return {};
    }

    const auto pos = util::Fraction::fromEdu(eduPosition);
    result = gfHold.calcNearestEntry(pos, findExact);
    if (!result || findExact) {
        return result;
    }

    const auto measure = doc->getOthers()->get<others::Measure>(forPartId, result.getMeasure());
    MUSX_ASSERT_IF(!measure) {
        // measure should exist, but there is nothing to do here: trap it in debug mode
        return {};
    }
    const auto beatVal = measure->createTimeSignature(staffId)->calcBeatValueAt(eduPosition);

    constexpr auto BEAT_THRESHOLD_MULTIPLIER = util::Fraction(1, 4); // modify this if too many false positives or negatives
    const auto beatThreshold = beatVal * BEAT_THRESHOLD_MULTIPLIER;
    using std::abs;
    if (abs(result->elapsedDuration - pos) > beatThreshold) {
        return {};
    }

    return result;
}

MusxInstance<others::SmartShapeMeasureAssign> smartshape::EndPoint::getMeasureAssignment() const
{
    auto shapeParent = getParent<others::SmartShape>();
    MUSX_ASSERT_IF(!shapeParent) {
        throw std::logic_error("Unknown parent type for SmartShape::EndPoint.");
    }
    if (auto measure = getDocument()->getOthers()->get<others::Measure>(shapeParent->getRequestedPartId(), measId)) {
        if (measure->hasSmartShape) {
            auto assigns = getDocument()->getOthers()->getArray<others::SmartShapeMeasureAssign>(shapeParent->getRequestedPartId(), measId);
            for (const auto& assign : assigns) {
                if (assign->shapeNum == shapeParent->getCmper()) {
                    return assign;
                }
            }
        }
    }
    return nullptr;
}

MusxInstance<details::SmartShapeEntryAssign> smartshape::EndPoint::getEntryAssignment() const
{
    auto shapeParent = getParent<others::SmartShape>();
    MUSX_ASSERT_IF(!shapeParent) {
        throw std::logic_error("Unknown parent type for SmartShape::EndPoint.");
    }
    if (entryNumber != 0) {
        Cmper shapeId = shapeParent->getCmper();
        if (auto entry = getDocument()->getEntries()->get(entryNumber)) {
            if (entry->smartShapeDetail) {
                auto assigns = getDocument()->getDetails()->getArray<details::SmartShapeEntryAssign>(shapeParent->getRequestedPartId(), entryNumber);
                for (const auto& assign : assigns) {
                    if (assign->shapeNum == shapeId) {
                        return assign;
                    }
                }
            }
        }
    }
    return nullptr;
}

bool smartshape::EndPoint::calcIsAssigned() const
{
    if (!getMeasureAssignment()) {
        return false;
    }
    if (entryNumber != 0 && !getEntryAssignment()) {
        return false;
    }
    return true;
}

MusxInstance<others::StaffComposite> smartshape::EndPoint::createCurrentStaff() const
{
    auto shapeParent = getParent<others::SmartShape>();
    MUSX_ASSERT_IF(!shapeParent) {
        throw std::logic_error("Unknown parent type for SmartShape::EndPoint.");
    }
    return others::StaffComposite::createCurrent(getDocument(), shapeParent->getRequestedPartId(),
        staffId, measId, calcPosition().calcEduDuration());
}

bool smartshape::EndPoint::calcIsValid() const
{
    if (entryNumber) {
        const auto entryInfo = calcAssociatedEntry();
        if (!entryInfo) return false;
        if (entryInfo.getMeasure() != measId || entryInfo.getStaff() != staffId) {
            return false;
        }
    }
    return getDocument()->getOthers()->get<others::Measure>(SCORE_PARTID, measId) != nullptr;
}

util::Fraction smartshape::EndPoint::calcPosition() const
{
    if (!entryNumber) {
        return util::Fraction::fromEdu(eduPosition);
    }
    if (const auto entryInfo = calcAssociatedEntry()) {
        return entryInfo->elapsedDuration;
    }
    return 0;
}

util::Fraction smartshape::EndPoint::calcGlobalPosition() const
{
    auto shapeParent = getParent<others::SmartShape>();
    MUSX_ASSERT_IF(!shapeParent) {
        throw std::logic_error("Unknown parent type for SmartShape::EndPoint.");
    }
    if (!entryNumber) {
        const auto rawPosition = util::Fraction::fromEdu(eduPosition);
        if (auto meas = getDocument()->getOthers()->get<others::Measure>(shapeParent->getRequestedPartId(), measId)) {
            return rawPosition * meas->calcTimeStretch(staffId);
        }
        return rawPosition;
    }
    if (auto entryInfo = calcAssociatedEntry()) {
        return entryInfo.calcGlobalElapsedDuration();
    }
    return 0;
}

int smartshape::EndPoint::compareMetricPosition(const EndPoint& other) const
{
    if (this->entryNumber != 0 && other.entryNumber != 0 && this->entryNumber == other.entryNumber) {
        MUSX_ASSERT_IF(measId != other.measId) {
            // nothing to do, but trap it in debug mode.
        }
        return 0;
    }
    if (measId != other.measId) {
        return (measId < other.measId) ? -1 : +1;
    }
    const auto thisPosition = this->calcPosition();
    const auto otherPosition = other.calcPosition();
    if (thisPosition != otherPosition) {
        return (thisPosition < otherPosition) ? -1 : +1;
    }
    return 0;
}

// ******************************
// ***** EndPointAdjustment *****
// ******************************

bool smartshape::EndPointAdjustment::calcHasVerticalEquivalentConnection(const smartshape::EndPointAdjustment& other) const
{
    using E = EntryConnectionType;

    const auto verticalGroup = [](E v) -> int {
        switch (v) {
        // System: always comparable with each other
        case E::SystemLeft:
        case E::SystemRight:
            return 100;

        // Head rectangle
        case E::HeadLeftTop:
        case E::HeadRightTop:
            return 200;
        case E::HeadLeftBottom:
        case E::HeadRightBottom:
            return 201;

        // Stem rectangle
        case E::StemLeftTop:
        case E::StemRightTop:
            return 210;
        case E::StemLeftBottom:
        case E::StemRightBottom:
            return 211;

        // Notehead
        case E::NoteLeftTop:
        case E::NoteRightTop:
            return 220;
        case E::NoteLeftBottom:
        case E::NoteRightBottom:
            return 221;
        case E::NoteLeftCenter:
        case E::NoteRightCenter:
            return 222;

        // Lyrics (obvious groupings)
        case E::LyricLeftCenter:
        case E::LyricRightCenter:
            return 230;

        case E::HeadRightLyricBaseline:
        case E::DotRightLyricBaseline:
        case E::DurationLyricBaseline:
            return 231;

        case E::LyricRightBottom:
            return 232;
        }

        return -1; // should be unreachable if enum is complete
    };

    return verticalGroup(contextEntCnct) == verticalGroup(other.contextEntCnct);
}

// **********************
// ***** SmartShape *****
// **********************

bool others::SmartShape::calcAppliesTo(const EntryInfoPtr& entryInfo) const
{
    auto entry = entryInfo->getEntry();
    if (entryBased) {
        if (entry->getEntryNumber() == startTermSeg->endPoint->entryNumber) return true;
        if (entry->getEntryNumber() == endTermSeg->endPoint->entryNumber) return true;
    }
    const StaffCmper entryStaffId = entryInfo.getStaff();
    const MeasCmper entryMeasureId = entryInfo.getMeasure();
    if (entryStaffId != startTermSeg->endPoint->staffId && entryStaffId != endTermSeg->endPoint->staffId) {
        return false;
    }
    if (entryMeasureId < startTermSeg->endPoint->measId || entryMeasureId > endTermSeg->endPoint->measId) {
        return false;
    }
    const auto entryPos = entryInfo->elapsedDuration;
    auto range = createMusicRange();
    if (auto meas = entry->getDocument()->getOthers()->get<others::Measure>(entryInfo.getFrame()->getRequestedPartId(), entryInfo.getMeasure())) {
        if (meas->hasSmartShape) {
            auto shapeAssigns = entry->getDocument()->getOthers()->getArray<others::SmartShapeMeasureAssign>(entryInfo.getFrame()->getRequestedPartId(), entryInfo.getMeasure());
            for (const auto& asgn : shapeAssigns) {
                if (asgn->shapeNum == getCmper()) {
                    if (range.contains(entryMeasureId, entryPos)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

MusicRange others::SmartShape::createMusicRange() const
{
    return MusicRange(getDocument(), startTermSeg->endPoint->measId, startTermSeg->endPoint->calcPosition(), endTermSeg->endPoint->measId, endTermSeg->endPoint->calcPosition());
}

MusicRange others::SmartShape::createGlobalMusicRange() const
{
    return MusicRange(getDocument(), startTermSeg->endPoint->measId, startTermSeg->endPoint->calcGlobalPosition(), endTermSeg->endPoint->measId, endTermSeg->endPoint->calcGlobalPosition());
}

bool others::SmartShape::iterateEntries(std::function<bool(const EntryInfoPtr&)> iterator, DeferredReference<MusxInstanceList<others::StaffUsed>> staffList) const
{
    if (!staffList) {
        staffList.emplace(getDocument()->getScrollViewStaves(getRequestedPartId()));
    }
    auto startIndex = staffList->getIndexForStaff(startTermSeg->endPoint->staffId);
    auto endIndex = staffList->getIndexForStaff(endTermSeg->endPoint->staffId);
    MUSX_ASSERT_IF(!startIndex || !endIndex) {
        throw std::logic_error("Smart shape spans staves that do not exist on the supplied staff list.");
    }
    return staffList->iterateEntries(*startIndex, *endIndex, createGlobalMusicRange(), iterator);
}

NoteInfoPtr others::SmartShape::calcEndpointNote(
    const std::shared_ptr<TerminationSeg>& termSeg, NoteNumber noteId) const
{
    MUSX_ASSERT_IF(!termSeg) {
        throw std::logic_error("SmartShape termination segment is null.");
    }
    if (!entryBased) {
        return NoteInfoPtr();
    }
    // calcAssociatedEntry's fuzzy search is deliberately not wanted here: entryBased guarantees the
    // endpoint carries an entry number, so the lookup below is exact.
    const auto entry = termSeg->endPoint->calcAssociatedEntry();
    if (!entry) {
        return NoteInfoPtr();
    }
    return entry.findNoteId(noteId);
}

NoteInfoPtr others::SmartShape::calcStartNote() const
{
    return calcEndpointNote(startTermSeg, startNoteId);
}

NoteInfoPtr others::SmartShape::calcEndNote() const
{
    return calcEndpointNote(endTermSeg, endNoteId);
}

NoteInfoPtr others::SmartShape::calcArpeggiatedTieToNote(const EntryInfoPtr& forStartEntry) const
{
    return musx::util::calcSmartShapeArpeggiatedTieToNote(*this, forStartEntry);
}

bool others::SmartShape::calcIsValid() const
{
    return startTermSeg->endPoint->calcIsValid() && endTermSeg->endPoint->calcIsValid();
}

bool others::SmartShape::calcIsSlur() const
{
    using ST = ShapeType;
    switch (shapeType) {
    default:
        break;
    case ST::SlurAuto:
    case ST::SlurUp:
    case ST::SlurDown:
    case ST::DashSlurAuto:
    case ST::DashSlurUp:
    case ST::DashSlurDown:
    case ST::DashContourSlurAuto:
    case ST::DashContourSlurUp:
    case ST::DashContourSlurDown:
        return true;
    }
    return false;
}

bool others::SmartShape::calcIsLyricShape() const
{
    return shapeType == ShapeType::Hyphen || shapeType == ShapeType::WordExtension;
}

CurveContourDirection others::SmartShape::calcContourDirection() const
{
    using ST = ShapeType;
    switch (shapeType) {
    case ST::SlurUp:
    case ST::DashSlurUp:
    case ST::DashContourSlurUp:
        return CurveContourDirection::Up;

    case ST::SlurDown:
    case ST::DashSlurDown:
    case ST::DashContourSlurDown:
        return CurveContourDirection::Down;

    case ST::SlurAuto:
    case ST::DashContourSlurAuto:
    case ST::DashSlurAuto:
        if (entryBased) {
            if (const auto startEntryInfoPtr = startTermSeg->endPoint->calcAssociatedEntry()) {
                auto [freezeStem, upStem] = startEntryInfoPtr.calcEntryStemSettings();
                if (freezeStem) {
                    // Finale freezes slurs in the direction of a frozen stem on the launch entry.
                    return upStem ? CurveContourDirection::Up : CurveContourDirection::Down;
                }
            }
        }
        break;

    default:
        break;
    }

    switch (direction) {
    case ShapeDirection::Over:
        return CurveContourDirection::Up;
    case ShapeDirection::Under:
        return CurveContourDirection::Down;
    case ShapeDirection::Automatic:
        break;
    }

    return CurveContourDirection::Unspecified;
}

VerticalPlacement others::SmartShape::calcVerticalPlacementForBeatAttached() const
{
    if (entryBased) {
        return VerticalPlacement::NotApplicable;
    }

    /// @note Beat-attached smart shapes are positioned relative to the top staff line,
    /// not the reference line. That means no staff is necessary for the Above calculation.

    if (startTermSeg->endPointAdj->calcVertOffset() >= 0 && endTermSeg->endPointAdj->calcVertOffset() >= 0) {
        return VerticalPlacement::Above;
    }

    auto isBelow = [&](const TerminationSeg& termSeg) -> bool {
        bool result = false;
        if (const auto staff = termSeg.endPoint->createCurrentStaff()) {
            // Offsets are relative to the top staff line, so the bottom staff line
            // is at a negative offset from it.
            const Evpu bottomOffset = staff->calcBottomLineEvpu() - staff->calcTopLineEvpu();
            result = (termSeg.endPointAdj->calcVertOffset() < bottomOffset);
        } else {
            /// @todo force top-level function to return NotApplicable from here.
            MUSX_INTEGRITY_ERROR("SmartShape with cmper " + std::to_string(getCmper())
                + " is missing staff cmper " + std::to_string(termSeg.endPoint->staffId) + " at endpoint.");
        }
        return result;
    };

    if (isBelow(*startTermSeg) && isBelow(*endTermSeg)) {
        return VerticalPlacement::Below;
    }

    return VerticalPlacement::Float;
}

bool others::SmartShape::calcIsDashed() const
{
    using ST = ShapeType;
    switch (shapeType) {
    case ST::DashLineUp:
    case ST::DashLineDown:
    case ST::DashSlurDown:
    case ST::DashSlurUp:
    case ST::DashLine:
    case ST::DashSlurAuto:
    case ST::DashLineDownBoth:
    case ST::DashLineUpBoth:
    case ST::DashLineUpLeft:
    case ST::DashLineDownLeft:
    case ST::DashLineUpDown:
    case ST::DashLineDownUp:
    case ST::DashContourSlurDown:
    case ST::DashContourSlurUp:
    case ST::DashContourSlurAuto:
        return true;
    default:
        return false;
    }
}

bool others::SmartShape::calcIsPseudoTie(utils::PseudoTieMode mode, const EntryInfoPtr& forStartEntry) const
{
    return musx::util::calcIsSmartShapePseudoTie(*this, mode, forStartEntry);
}

// ********************************
// ***** SmartShapeCustomLine *****
// ********************************

util::EnigmaParsingContext others::SmartShapeCustomLine::getRawTextCtx(Cmper forPartId, Cmper rawTextId) const
{
    if (auto rawText = getDocument()->getTexts()->get<texts::SmartShapeText>(rawTextId)) {
        return rawText->getRawTextCtx(rawText, forPartId);
    }
    return {};
}

std::optional<Cmper> others::importSmartShapeCustomLineInto(const DocumentPtr& target,
    const MusxInstance<SmartShapeCustomLine>& source,
    const ImportObjectCallback& onImported)
{
    MUSX_ASSERT_IF(!target) {
        throw std::invalid_argument(
            "importSmartShapeCustomLineInto received a null target document");
    }
    MUSX_ASSERT_IF(!source) {
        throw std::invalid_argument(
            "importSmartShapeCustomLineInto received a null custom line");
    }

    const auto newLineId =
        target->getOthers()->nextFreeCmper<SmartShapeCustomLine>(SCORE_PARTID);
    if (!newLineId) {
        return std::nullopt;
    }
    const auto sourceDocument = source->getDocument();

    std::optional<Cmper> importedFontId;
    if (source->charParams) {
        if (!source->charParams->font) {
            return std::nullopt;
        }
        importedFontId = source->charParams->font->fontId;
        if (const auto sourceFont = sourceDocument->getOthers()->get<FontDefinition>(
                SCORE_PARTID, *importedFontId)) {
            importedFontId = importFontDefinitionInto(target, sourceFont, onImported);
            if (!importedFontId) {
                return std::nullopt;
            }
        } else if (*importedFontId != 0) {
            return std::nullopt;
        }
    }

    std::map<Cmper, Cmper> importedShapes;
    const auto importArrow = [&](SmartShapeCustomLine::LineCapType type, Cmper sourceId)
        -> std::optional<Cmper> {
        if (type != SmartShapeCustomLine::LineCapType::ArrowheadCustom || sourceId == 0) {
            return sourceId;
        }
        if (const auto found = importedShapes.find(sourceId); found != importedShapes.end()) {
            return found->second;
        }
        const auto sourceShape =
            sourceDocument->getOthers()->get<ShapeDef>(SCORE_PARTID, sourceId);
        if (!sourceShape) {
            return std::nullopt;
        }
        const auto imported = importShapeDefInto(target, sourceShape, onImported);
        if (imported) {
            importedShapes.emplace(sourceId, *imported);
        }
        return imported;
    };
    const auto startArrowId =
        importArrow(source->lineCapStartType, source->lineCapStartArrowId);
    const auto endArrowId =
        importArrow(source->lineCapEndType, source->lineCapEndArrowId);
    if (!startArrowId || !endArrowId) {
        return std::nullopt;
    }

    std::map<Cmper, Cmper> importedTexts;
    const auto importText = [&](Cmper sourceId) -> std::optional<Cmper> {
        if (sourceId == 0) {
            return Cmper(0);
        }
        if (const auto found = importedTexts.find(sourceId); found != importedTexts.end()) {
            return found->second;
        }
        const auto sourceText = sourceDocument->getTexts()->get<texts::SmartShapeText>(sourceId);
        if (!sourceText) {
            return std::nullopt;
        }
        const auto imported = texts::importTextInto(target, sourceText, onImported);
        if (imported) {
            importedTexts.emplace(sourceId, *imported);
        }
        return imported;
    };

    auto result = std::make_shared<SmartShapeCustomLine>(
        target, SCORE_PARTID, EnigmaBase::ShareMode::All, *newLineId);
    *result = *source;
    if (source->charParams) {
        result->charParams = std::make_shared<SmartShapeCustomLine::CharParams>(result);
        *result->charParams = *source->charParams;
        result->charParams->font = std::make_shared<FontInfo>(
            target, source->charParams->font->getSizeIsPercent());
        *result->charParams->font = *source->charParams->font;
        result->charParams->font->fontId = *importedFontId;
    }
    if (source->solidParams) {
        result->solidParams = std::make_shared<SmartShapeCustomLine::SolidParams>(result);
        *result->solidParams = *source->solidParams;
    }
    if (source->dashedParams) {
        result->dashedParams = std::make_shared<SmartShapeCustomLine::DashedParams>(result);
        *result->dashedParams = *source->dashedParams;
    }
    result->lineCapStartArrowId = *startArrowId;
    result->lineCapEndArrowId = *endArrowId;

    constexpr std::array rawTextMembers{
        &SmartShapeCustomLine::leftStartRawTextId,
        &SmartShapeCustomLine::leftContRawTextId,
        &SmartShapeCustomLine::rightEndRawTextId,
        &SmartShapeCustomLine::centerFullRawTextId,
        &SmartShapeCustomLine::centerAbbrRawTextId,
    };
    for (const auto member : rawTextMembers) {
        const auto importedId = importText(source.get()->*member);
        if (!importedId) {
            return std::nullopt;
        }
        result.get()->*member = *importedId;
    }

    target->getOthers()->add(SmartShapeCustomLine::XmlNodeName, result);
    if (onImported) {
        onImported(*result);
    }
    return *newLineId;
}

} // namespace dom
} // namespace musx
