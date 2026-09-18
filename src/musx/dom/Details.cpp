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
#include <string>
#include <vector>
#include <cstdlib>
#include <exception>
#include <type_traits>
#include <unordered_set>

#include "musx/musx.h"

namespace musx {
namespace dom {

void PartContextRebinder<details::MeasureNumberIndividualPositioning>::rebind(
    const std::shared_ptr<details::MeasureNumberIndividualPositioning>& positioning)
{
    if (positioning->enclosure) {
        positioning->enclosure = PartContextCloner::copyWithPartId<others::Enclosure>(
            positioning->enclosure, positioning->getRequestedPartId());
    }
}

// ****************************
// ***** EntryDetailsBase *****
// ****************************

template <typename EDUP, typename EDDOWN, typename EDBASE>
MusxInstance<EDBASE> EntryDetailsBase::getStemDependentDetail(const EntryInfoPtr& entryInfo, StemSelection stemSelection)
{
    static_assert(std::is_base_of_v<EntryDetailsBase, EDBASE>, "EDBASE must be derived from EntryDetailsBase.");
    static_assert(std::is_base_of_v<EDBASE, EDUP>, "EDUP must be derived from EDBASE");
    static_assert(std::is_base_of_v<EDBASE, EDDOWN>, "EDDOWN must be derived from EDBASE");

    const auto frame = entryInfo.getFrame();
    const auto entry = entryInfo->getEntry();

    switch (stemSelection) {
    case StemSelection::UpStem:
        return frame->getDocument()->getDetails()->get<EDUP>(frame->getRequestedPartId(), entry->getEntryNumber());
    case StemSelection::DownStem:
        return frame->getDocument()->getDetails()->get<EDDOWN>(frame->getRequestedPartId(), entry->getEntryNumber());
    case StemSelection::MatchEntry:
        if (entryInfo.calcUpStem()) {
            return frame->getDocument()->getDetails()->get<EDUP>(frame->getRequestedPartId(), entry->getEntryNumber());
        } else {
            return frame->getDocument()->getDetails()->get<EDDOWN>(frame->getRequestedPartId(), entry->getEntryNumber());
        }
    case StemSelection::Any:
        if (auto upStem = frame->getDocument()->getDetails()->get<EDUP>(frame->getRequestedPartId(), entry->getEntryNumber())) {
            return upStem;
        } else {
            return frame->getDocument()->getDetails()->get<EDDOWN>(frame->getRequestedPartId(), entry->getEntryNumber());
        }
    }
    return nullptr;
}

namespace details {

// ******************************
// ***** ArticulationAssign *****
// ******************************

bool details::ArticulationAssign::calcPlacementAbove(const MusxInstance<others::ArticulationDef>& definition,
    const EntryInfoPtr& entryInfo) const
{
    if (overridePlacement) {
        return aboveEntry;
    }

    const bool stemUp = entryInfo.calcUpStem();
    if (!definition->autoVert) {
        return vertOffset >= 0;
    }

    using AD = others::ArticulationDef;
    switch (definition->autoVertMode) {
    case AD::AutoVerticalMode::AboveEntry: {
        const bool stemSideInMultiLayer = definition->isStemSideWhenMultipleLayers
            && entryInfo.getFrame()->getContext()->calcIsMultiLayer();
        return stemSideInMultiLayer ? stemUp : true;
    }
    case AD::AutoVerticalMode::BelowEntry:
        return false;
    case AD::AutoVerticalMode::StemSide:
        return stemUp;
    case AD::AutoVerticalMode::AlwaysNoteheadSide:
        switch (entryInfo.calcStemDirectionForced()) {
        case StemDirection::AlwaysUp:
        case StemDirection::AlwaysDown:
            return stemUp;
        case StemDirection::Default:
            return !stemUp;
        }
        assert(false && "invalid enum value");
        // defensive break should be unreachable
        break;
    case AD::AutoVerticalMode::AutoNoteStem:
        return stemUp;
    case AD::AutoVerticalMode::AlwaysOnStem:
        return stemUp;
    }
    assert(false && "invalid enum value");
    return stemUp; // defensive fallback should be unreachable
}

VerticalPlacement details::ArticulationAssign::calcVerticalPlacement(
    const MusxInstance<others::ArticulationDef>& definition,
    const EntryInfoPtr& entryInfo) const
{
    if (overridePlacement) {
        return aboveEntry ? VerticalPlacement::Above : VerticalPlacement::Below;
    }
    if (!definition->autoVert) {
        return vertOffset >= 0 ? VerticalPlacement::Above : VerticalPlacement::Below;
    }

    using AD = others::ArticulationDef;
    switch (definition->autoVertMode) {
    case AD::AutoVerticalMode::AboveEntry: {
        const bool stemSideInMultiLayer = definition->isStemSideWhenMultipleLayers
            && entryInfo.getFrame()->getContext()->calcIsMultiLayer();
        return stemSideInMultiLayer ? VerticalPlacement::Float : VerticalPlacement::Above;
    }
    case AD::AutoVerticalMode::BelowEntry:
        return VerticalPlacement::Below;
    case AD::AutoVerticalMode::AlwaysNoteheadSide:
    case AD::AutoVerticalMode::AutoNoteStem:
        return VerticalPlacement::Float;
    default:
        return VerticalPlacement::NotApplicable;
    }
}

std::optional<details::ArticulationAssign::SelectedSymbolContext>
details::ArticulationAssign::calcSelectedSymbolContext(const EntryInfoPtr& entryInfo) const
{
    SelectedSymbolContext result;
    if (!entryInfo) {
        return std::nullopt;
    }

    result.definition = getDocument()->getOthers()->get<others::ArticulationDef>(getRequestedPartId(), articDef);
    if (!result.definition) {
        return std::nullopt;
    }

    result.symbol = result.definition->calcSelectedSymbol(calcPlacementAbove(result.definition, entryInfo));
    result.placement = calcVerticalPlacement(result.definition, entryInfo);

    return result;
}

NoteInfoPtr details::ArticulationAssign::calcAssociatedNote(const EntryInfoPtr& entryInfo) const
{
    if (!entryInfo) {
        return NoteInfoPtr();
    }
    const auto entry = entryInfo->getEntry();
    MUSX_ASSERT_IF(entry->getEntryNumber() != getEntryNumber()) {
        throw std::logic_error("ArticulationAssign::calcAssociatedNote called with entry number "
            + std::to_string(entry->getEntryNumber()) + " that does not match the assignment's entry number "
            + std::to_string(getEntryNumber()) + ".");
    }
    if (!entry->isNote || entry->notes.empty()) {
        return NoteInfoPtr();
    }
    const auto definition = getDocument()->getOthers()->get<others::ArticulationDef>(getRequestedPartId(), articDef);
    if (!definition || definition->autoVert) {
        return NoteInfoPtr();
    }

    const auto staffPositionOf = [](const NoteInfoPtr& note) {
        NotePropertiesOptions options;
        options.alwaysUseEntryStaff = true;
        return note.calcNoteProperties(options).staffPosition;
    };

    // A zero offset places the symbol at the staff position of the note furthest from the stem.
    const NoteInfoPtr furthestFromStem(entryInfo, entryInfo.calcUpStem() ? 0 : entry->notes.size() - 1);
    const int targetPosition = staffPositionOf(furthestFromStem)
        + static_cast<int>(std::lround(static_cast<double>(vertOffset) / EVPU_PER_STAFF_POSITION));

    for (size_t noteIndex = 0; noteIndex < entry->notes.size(); ++noteIndex) {
        const NoteInfoPtr note(entryInfo, noteIndex);
        if (staffPositionOf(note) == targetPosition) {
            return note;
        }
    }
    return NoteInfoPtr();
}

std::optional<details::ArticulationAssign::PseudoTieShapeContext>
details::ArticulationAssign::calcPseudoTieShapeContext(const EntryInfoPtr& forStartEntry) const
{
    PseudoTieShapeContext result;
    const auto selectedSymbolContext = calcSelectedSymbolContext(forStartEntry);
    if (!selectedSymbolContext) {
        return std::nullopt;
    }
    const auto doc = getDocument();
    result.definition = selectedSymbolContext->definition;
    result.selectedSymbol = selectedSymbolContext->symbol;
    const auto def = result.definition;

    using AD = others::ArticulationDef;
    if (def->copyMode != AD::CopyMode::None || def->centerOnStem) {
        return std::nullopt;
    }
    if (def->autoVert && def->autoVertMode == AD::AutoVerticalMode::AlwaysOnStem) {
        return std::nullopt;
    }
    if (!result.selectedSymbol.isShape) {
        return std::nullopt;
    }
    if (!result.selectedSymbol.shapeId) {
        return std::nullopt;
    }
    result.info.shape = doc->getOthers()->get<others::ShapeDef>(getRequestedPartId(), result.selectedSymbol.shapeId);
    if (!result.info.shape) {
        return std::nullopt;
    }
    auto knownType = result.info.shape->recognize();
    if (knownType != KnownShapeDefType::SlurTieCurveRight
        && knownType != KnownShapeDefType::SlurTieCurveLeft) {
        return std::nullopt;
    }
    result.info.shapeType = knownType;

    return result;
}

std::optional<utils::PseudoTieShapeInfo> details::ArticulationAssign::calcIsPseudoTie(
    utils::PseudoTieMode mode, const EntryInfoPtr& forStartEntry) const
{
    const auto tieShapeContext = calcPseudoTieShapeContext(forStartEntry);
    if (!tieShapeContext) {
        return std::nullopt;
    }
    const auto& tieShapeInfo = tieShapeContext->info;
    if (mode == utils::PseudoTieMode::TieEnd && tieShapeInfo.shapeType == KnownShapeDefType::SlurTieCurveLeft) {
        return tieShapeInfo;
    }
    if (tieShapeInfo.shapeType != KnownShapeDefType::SlurTieCurveRight) {
        return std::nullopt;
    }

    const auto offset = tieShapeContext->selectedSymbol.xOffset + horzOffset;
    const auto endOffset = offset + tieShapeContext->info.calcWidthOffset();
    switch (mode) {
    case utils::PseudoTieMode::LaissezVibrer:
        if (!utils::calcIsPseudoForwardTie(offset, endOffset)) {
            return std::nullopt;
        }
        break;
    case utils::PseudoTieMode::TieEnd:
        if (!utils::calcIsPseudoBackwardTie(offset, endOffset)) {
            return std::nullopt;
        }
        break;
    }

    return tieShapeInfo;
}

// ***************************
// ***** BeamAlterations *****
// ***************************

template <typename T,
          std::enable_if_t<std::is_base_of_v<BeamAlterations, T>, int>>
void BeamAlterations::calcAllActiveFlags(const DocumentPtr& document)
{
    if (const auto beamOptions = document->getOptions()->get<options::BeamOptions>()) {
        const auto values = document->getDetails()->getArray<T>(SCORE_PARTID);
        for (const auto& value : values) {
            T* mutableValue = const_cast<T*>(value.get());
            mutableValue->m_active = (value->flattenStyle == beamOptions->beamingStyle);
        }
    } else {
        MUSX_INTEGRITY_ERROR("Unable to retrieve beaming options. Active indicators for beam alterations were not set.");
    }
}

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
template void BeamAlterations::calcAllActiveFlags<BeamAlterationsUpStem>(const DocumentPtr&);
template void BeamAlterations::calcAllActiveFlags<BeamAlterationsDownStem>(const DocumentPtr&);
template void BeamAlterations::calcAllActiveFlags<SecondaryBeamAlterationsUpStem>(const DocumentPtr&);
template void BeamAlterations::calcAllActiveFlags<SecondaryBeamAlterationsDownStem>(const DocumentPtr&);
#endif // DOXYGEN_SHOULD_IGNORE_THIS

Efix BeamAlterations::calcEffectiveBeamWidth() const
{
    if (getInci().has_value()) { // secondary beams have incis; primary beams do not
        MusxInstance<BeamAlterations> primary;
        if (dynamic_cast<const SecondaryBeamAlterationsDownStem*>(this)) {
            primary = getDocument()->getDetails()->get<BeamAlterationsDownStem>(getRequestedPartId(), getEntryNumber());
        } else {
            primary = getDocument()->getDetails()->get<BeamAlterationsUpStem>(getRequestedPartId(), getEntryNumber());
        }
        if (primary) {
            return primary->calcEffectiveBeamWidth();
        }
    } else if (m_active) {
        if (beamWidth >= 0) {
            return beamWidth;
        }
    }
    Efix result = 0;
    if (const auto beamOptions = getDocument()->getOptions()->get<options::BeamOptions>()) {
        result = beamOptions->beamWidth;
    } else {
        MUSX_INTEGRITY_ERROR("Unable to retrieve beaming options. Beam width value returned is zero.");
    }
    return result;
}

template <typename SecondaryBeamType>
bool BeamAlterations::calcIsFeatheredBeamImpl(const EntryInfoPtr& entryInfo, Evpu& outLeftY, Evpu& outRightY)
{
    static_assert(std::is_same_v<SecondaryBeamType, SecondaryBeamAlterationsDownStem>
               || std::is_same_v<SecondaryBeamType, SecondaryBeamAlterationsUpStem>,
        "SecondaryBeamType must be a secondary beam type.");
    constexpr bool isDownstem = std::is_same_v<SecondaryBeamType, SecondaryBeamAlterationsDownStem>;
    constexpr int direction = isDownstem ? 1 : -1;

    const auto& frame = entryInfo.getFrame();
    const auto& doc = frame->getDocument();
    const auto& secondaries = doc->getDetails()->getArray<SecondaryBeamType>(
        frame->getRequestedPartId(), entryInfo->getEntry()->getEntryNumber());

    const auto beamOptions = doc->getOptions()->get<options::BeamOptions>();
    if (!beamOptions) {
        MUSX_INTEGRITY_ERROR("Unable to retrieve BeamOptions for determining feathered beaming.");
    }

    const Evpu beamSpacing = beamOptions ? beamOptions->beamSepar : 18;
    const Evpu beamWidth   = beamOptions ? Evpu(std::round(beamOptions->beamWidth / EFIX_PER_EVPU)) : 12;

    Evpu leftY = direction * beamWidth;
    Evpu rightY = leftY;
    Evpu extremeLeft = leftY;
    Evpu extremeRight = rightY;

    for (const auto& sec : secondaries) {
        if (!sec->isActive()) continue;

        const Evpu dyL = sec->leftOffsetY + direction * beamSpacing;
        const Evpu dyR = (sec->leftOffsetY + sec->rightOffsetY) + direction * beamSpacing;

        leftY += dyL;
        rightY += dyR;

        if constexpr (isDownstem) {
            extremeLeft  = (std::max)(extremeLeft, leftY);
            extremeRight = (std::max)(extremeRight, rightY);
        } else {
            extremeLeft  = (std::min)(extremeLeft, leftY);
            extremeRight = (std::min)(extremeRight, rightY);
        }
    }

    const Evpu spanLeft  = direction * extremeLeft;
    const Evpu spanRight = direction * extremeRight;

    if (spanLeft != spanRight) {
        outLeftY = spanLeft;
        outRightY = spanRight;
        return true;
    }

    return false;
}

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
template bool BeamAlterations::calcIsFeatheredBeamImpl<SecondaryBeamAlterationsUpStem>(const EntryInfoPtr& entryInfo, Evpu& outLeftY, Evpu& outRightY);
template bool BeamAlterations::calcIsFeatheredBeamImpl<SecondaryBeamAlterationsDownStem>(const EntryInfoPtr& entryInfo, Evpu& outLeftY, Evpu& outRightY);
#endif // DOXYGEN_SHOULD_IGNORE_THIS

template <typename SecondaryBeamType>
MusxInstanceList<SecondaryBeamType> BeamAlterations::getSecondaryBeamArray(const EntryInfoPtr& entryInfo)
{
    static_assert(std::is_same_v<SecondaryBeamType, SecondaryBeamAlterationsDownStem>
               || std::is_same_v<SecondaryBeamType, SecondaryBeamAlterationsUpStem>,
        "SecondaryBeamType must be a secondary beam type.");

    auto frame = entryInfo.getFrame();
    return frame->getDocument()->getDetails()->getArray<SecondaryBeamType>(frame->getRequestedPartId(), entryInfo->getEntry()->getEntryNumber());
}

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
template MusxInstanceList<SecondaryBeamAlterationsUpStem> BeamAlterations::getSecondaryBeamArray<SecondaryBeamAlterationsUpStem>(const EntryInfoPtr& entryInfo);
template MusxInstanceList<SecondaryBeamAlterationsDownStem> BeamAlterations::getSecondaryBeamArray<SecondaryBeamAlterationsDownStem>(const EntryInfoPtr& entryInfo);
#endif // DOXYGEN_SHOULD_IGNORE_THIS

MusxInstance<BeamAlterations> BeamAlterations::getPrimaryForStem(const EntryInfoPtr& entryInfo, StemSelection stemSelection)
{
    if (entryInfo->getEntry()->stemDetail) {
        return getStemDependentDetail<BeamAlterationsUpStem, BeamAlterationsDownStem, BeamAlterations>(entryInfo, stemSelection);
    }
    return nullptr;
}

MusxInstance<BeamAlterations> BeamAlterations::getSecondaryForStem(const EntryInfoPtr& entryInfo, Edu dura, StemSelection stemSelection)
{
    auto searchArray = [&](auto beamAlts) -> MusxInstance<BeamAlterations> {
        auto it = std::find_if(beamAlts.begin(), beamAlts.end(), [&](const auto& beamAlt) {
            return beamAlt->dura == dura;
        });
        if (it != beamAlts.end()) {
            return *it;
        }
        return nullptr;
    };
    if (entryInfo->getEntry()->stemDetail) {
        switch (stemSelection) {
        case StemSelection::UpStem:
            return searchArray(getSecondaryBeamArray<SecondaryBeamAlterationsUpStem>(entryInfo));
        case StemSelection::DownStem:
            return searchArray(getSecondaryBeamArray<SecondaryBeamAlterationsDownStem>(entryInfo));
        case StemSelection::MatchEntry:
            if (entryInfo.calcUpStem()) {
                return searchArray(getSecondaryBeamArray<SecondaryBeamAlterationsUpStem>(entryInfo));
            } else {
                return searchArray(getSecondaryBeamArray<SecondaryBeamAlterationsDownStem>(entryInfo));
            }
        case StemSelection::Any:
            if (auto upStem = searchArray(getSecondaryBeamArray<SecondaryBeamAlterationsUpStem>(entryInfo))) {
                return upStem;
            } else {
                return searchArray(getSecondaryBeamArray<SecondaryBeamAlterationsDownStem>(entryInfo));
            }
        }
    }
    return {};
}

// *************************
// ***** BeamExtension *****
// *************************

unsigned BeamExtension::calcMaxExtension() const
{
    // Legacy behavior: only the 8th-note beam is extended.
    // Regardless of mask contents, Finale would extend only the first beam.
    if (!extBeyond8th) {
        return 1U;
    }

    if (mask) {
        unsigned maxExtension = 10;      // 1 = 8th, ..., 10 = 4096th
        unsigned workingMask  = mask;
        while ((workingMask & 0x01U) == 0U && maxExtension > 0U) {
            workingMask >>= 1U;
            maxExtension--;
        }
        return maxExtension;             // always 1–10 for nonzero mask
    }

    // Fallback: treat as 8th.
    return 0U;
}

MusxInstance<BeamExtension> BeamExtension::getForStem(const EntryInfoPtr& entryInfo, StemSelection stemSelection) {
    if (entryInfo->getEntry()->beamExt) {
        return getStemDependentDetail<BeamExtensionUpStem, BeamExtensionDownStem, BeamExtension>(entryInfo, stemSelection);
    }
    return nullptr;
}

// ***********************
// ***** ChordAssign *****
// ***********************

Cmper ChordAssign::calcFretboardGroupCmper() const
{
    if (suffixId != 0) {
        return suffixId;
    } else if (rootLowerCase) {
        return 65533; // hard-coded minor triad fretboard group
    }
    return 65534; // hard-coded major triad fretboard group
}

MusxInstanceList<others::ChordSuffixElement> ChordAssign::getChordSuffix() const
{
    return getDocument()->getOthers()->getArray<others::ChordSuffixElement>(getRequestedPartId(), suffixId);
}

MusxInstance<others::FretboardGroup> ChordAssign::getFretboardGroup() const
{
    const Cmper groupCmper = calcFretboardGroupCmper();
    if (!useFretboardFont && groupCmper != 0) {
        return getDocument()->getOthers()->get<others::FretboardGroup>(getRequestedPartId(), groupCmper, fretboardGroupInci);
    }
    return nullptr;
}

MusxInstance<others::FretboardStyle> ChordAssign::getFretboardStyle() const
{
    if (!useFretboardFont && fbStyleId != 0) {
        return getDocument()->getOthers()->get<others::FretboardStyle>(getRequestedPartId(), fbStyleId);
    }
    return nullptr;
}

// **********************
// ***** CustomStem *****
// **********************

bool CustomStem::calcIsHiddenStem() const
{
    if (shapeDef != 0) {
        if (const auto shape = getDocument()->getOthers()->get<others::ShapeDef>(getRequestedPartId(), shapeDef)) {
            return shape->isBlank();
        }
    }
    return true;
}

MusxInstance<CustomStem> CustomStem::getForStem(const EntryInfoPtr& entryInfo, StemSelection stemSelection)
{
    if (entryInfo->getEntry()->stemDetail) {
        return getStemDependentDetail<CustomUpStem, CustomDownStem, CustomStem>(entryInfo, stemSelection);
    }
    return nullptr;
}

// **********************
// ***** GFrameHold *****
// **********************

std::pair<MusxInstance<others::Frame>, Edu> details::GFrameHold::findLayerFrame(LayerIndex layerIndex) const
{
    MusxInstance<others::Frame> layerFrame;
    Edu startEdu = 0;
    if (layerIndex < frames.size() && frames[layerIndex]) {
        auto frameIncis = getDocument()->getOthers()->getArray<others::Frame>(getRequestedPartId(), frames[layerIndex]);
        for (const auto& frame : frameIncis) {
            if (frame->startEntry) {
                if (layerFrame) {
                    MUSX_INTEGRITY_ERROR("More than one entry frame inci with startEntry exists for frame " + std::to_string(frames[layerIndex]));
                }
                layerFrame = frame;
            }
            if (frame->startTime > startEdu) {
                startEdu = frame->startTime;
            }
        }
    }
    return std::make_pair(layerFrame, startEdu);
}

bool GFrameHold::iterateRawEntries(std::function<bool(const MusxInstance<Entry>&, LayerIndex)> iterator) const
{
    for (size_t layerIndex = 0; layerIndex < frames.size(); layerIndex++) {
        auto [frame, startEdu] = findLayerFrame(static_cast<LayerIndex>(layerIndex));
        if (frame) {
            if (!frame->iterateRawEntries([&](const MusxInstance<Entry>& entry) {
                    return iterator(entry, static_cast<LayerIndex>(layerIndex));
                })) {
                return false;
            }
        }
    }
    return true;
}

// ***********************************
// ***** IndependentStaffDetails *****
// ***********************************

MusxInstance<TimeSignature> IndependentStaffDetails::createTimeSignature() const
{
   return MusxInstance<TimeSignature>(new TimeSignature(getDocument(), beats, divBeat, altNumTsig, altDenTsig));
}

MusxInstance<TimeSignature> IndependentStaffDetails::createDisplayTimeSignature() const
{
    if (!hasDispTime) {
        return createTimeSignature();
    }
    return MusxInstance<TimeSignature>(new TimeSignature(getDocument(), dispBeats, dispDivBeat, displayAltNumTsig, displayAltDenTsig,
        displayAbbrvTime ? TimeSignature::Abbreviation::Abbreviated : TimeSignature::Abbreviation::Numeric));
}

// ************************
// ***** LyricsAssign *****
// ************************

MusxInstance<texts::LyricsTextBase> LyricAssignVerse::getLyricText() const
{
    return getDocument()->getTexts()->get<TextType>(lyricNumber);
}

MusxInstance<texts::LyricsTextBase> LyricAssignChorus::getLyricText() const
{
    return getDocument()->getTexts()->get<TextType>(lyricNumber);
}

MusxInstance<texts::LyricsTextBase> LyricAssignSection::getLyricText() const
{
    return getDocument()->getTexts()->get<TextType>(lyricNumber);
}

LyricTextType LyricAssignVerse::getLyricTextType() const
{
    return TextType::lyricTextType;
}

LyricTextType LyricAssignChorus::getLyricTextType() const
{
    return TextType::lyricTextType;
}

LyricTextType LyricAssignSection::getLyricTextType() const
{
    return TextType::lyricTextType;
}

util::EnigmaParsingContext LyricAssign::getRawTextCtx() const
{
    // note that lyrics do not have text inserts. The UI doesn't permit them.
    if (auto rawText = getLyricText()) {
        return rawText->getRawTextCtx(rawText, SCORE_PARTID);
    }
    return {};
}

EntryInfoPtr LyricAssign::calcWordExtensionEndpoint() const
{
    if (!wext) {
        return {};
    }
    const auto doc = getDocument();
    if (auto lyricOptions = doc->getOptions()->get<options::LyricOptions>()) {
        if (!lyricOptions->useSmartWordExtensions) {
            return {};
        }
    }
    auto wordExtShapeAssigns = doc->getDetails()->getArray<SmartShapeEntryAssign>(getRequestedPartId(), getEntryNumber());
    for (const auto& assign : wordExtShapeAssigns) {
        if (auto shape = doc->getOthers()->get<others::SmartShape>(getRequestedPartId(), assign->shapeNum)) {
            if (shape->shapeType == others::SmartShape::ShapeType::WordExtension && shape->startLyricType == getLyricTextType() && shape->startLyricNum == lyricNumber) {
                return shape->endTermSeg->endPoint->calcAssociatedEntry();
            }
        }
    }
    return {};
}

std::optional<std::string> LyricAssign::calcDisplayNumberText() const
{
    bool showNumber = displayVerseNum;
    if (!showNumber && syllable == 1) {
        if (auto lyricOptions = getDocument()->getOptions()->get<options::LyricOptions>()) {
            switch (getLyricTextType()) {
                case LyricTextType::Verse:
                    showNumber = lyricOptions->showAutoNumbersOnVerses;
                    break;
                case LyricTextType::Chorus:
                    showNumber = lyricOptions->showAutoNumbersOnChoruses;
                    break;
                case LyricTextType::Section:
                    showNumber = lyricOptions->showAutoNumbersOnSections;
                    break;
            }
        }
    }
    if (!showNumber) {
        return std::nullopt;
    }
    return std::to_string(lyricNumber) + ".";
}

// *****************************
// ***** MeasureTextAssign *****
// *****************************

MusxInstance<others::TextBlock> MeasureTextAssign::getTextBlock() const
{
    return getDocument()->getOthers()->get<others::TextBlock>(getRequestedPartId(), block);
}

util::EnigmaParsingContext MeasureTextAssign::getRawTextCtx(Cmper forPartId) const
{
    if (auto textBlock = getTextBlock()) {
        if (const auto page = getDocument()->calcPageFromMeasure(forPartId, getCmper2())) {
            return textBlock->getRawTextCtx(forPartId, page->getCmper());
        }
    }
    return {};
}

// **********************
// ***** StaffGroup *****
// **********************

util::EnigmaParsingContext StaffGroup::getFullNameCtx() const
{
    if (auto textBlock = getDocument()->getOthers()->get<others::TextBlock>(getRequestedPartId(), fullNameId)) {
        return textBlock->getRawTextCtx(getRequestedPartId());
    }
    return {};
}

std::string StaffGroup::getFullName(util::EnigmaString::AccidentalStyle accidentalStyle) const
{
    if (auto ctx = getFullNameCtx()) {
        return ctx.getText(true, accidentalStyle);
    }
    return {};
}

util::EnigmaParsingContext StaffGroup::getAbbreviatedNameCtx() const
{
    if (auto textBlock = getDocument()->getOthers()->get<others::TextBlock>(getRequestedPartId(), abbrvNameId)) {
        return textBlock->getRawTextCtx(getRequestedPartId());
    }
    return {};
}

std::string StaffGroup::getAbbreviatedName(util::EnigmaString::AccidentalStyle accidentalStyle) const
{
    if (auto ctx = getAbbreviatedNameCtx()) {
        return ctx.getText(true, accidentalStyle);
    }
    return {};
}

MusxInstance<others::MultiStaffInstrumentGroup> StaffGroup::getMultiStaffInstGroup() const
{
    if (multiStaffGroupId) {
        if (auto retval = getDocument()->getOthers()->get<others::MultiStaffInstrumentGroup>(SCORE_PARTID, multiStaffGroupId)) {
            return retval;
        }
        MUSX_INTEGRITY_ERROR("StaffGroup " + std::to_string(getCmper2()) + " points to non-existent MultiStaffInstrumentGroup " + std::to_string(multiStaffGroupId));
    }
    return nullptr;
}

util::EnigmaParsingContext StaffGroup::getFullInstrumentNameCtx() const
{
    if (auto multiStaffGroup = getMultiStaffInstGroup()) {
        if (auto staff = multiStaffGroup->getFirstStaffInstance()) {
            return staff->getFullInstrumentNameCtx(getRequestedPartId());
        }
    }
    return getFullNameCtx();
}

std::string StaffGroup::getFullInstrumentName(util::EnigmaString::AccidentalStyle accidentalStyle) const
{
    if (auto ctx = getFullInstrumentNameCtx()) {
        return ctx.getText(true, accidentalStyle);
    }
    return {};
}

util::EnigmaParsingContext StaffGroup::getAbbreviatedInstrumentNameCtx() const
{
    if (auto multiStaffGroup = getMultiStaffInstGroup()) {
        if (auto staff = multiStaffGroup->getFirstStaffInstance()) {
            return staff->getAbbreviatedInstrumentNameCtx(getRequestedPartId());
        }
    }
    return getAbbreviatedNameCtx();
}

std::string StaffGroup::getAbbreviatedInstrumentName(util::EnigmaString::AccidentalStyle accidentalStyle) const
{
    if (auto ctx = getAbbreviatedInstrumentNameCtx()) {
        return ctx.getText(true, accidentalStyle);
    }
    return {};
}

// **************************
// ***** StaffGroupInfo *****
// **************************

StaffGroupInfo::StaffGroupInfo(const MusxInstance<StaffGroup>& staffGroup,
    const MusxInstanceList<others::StaffUsed>& inpSysStaves) : group(staffGroup), systemStaves(inpSysStaves)
{
    if (inpSysStaves.empty()) {
        throw std::logic_error("Attempt to create StaffGroupInfo with no system staves (StaffGroup " + std::to_string(staffGroup->getCmper2()) + ")");
    }
    auto it = std::find_if(inpSysStaves.begin(), inpSysStaves.end(), [&](const auto& item) {
        return item->staffId == group->startInst;
    });
    if (it != inpSysStaves.end()) {
        startSlot = std::distance(inpSysStaves.begin(), it);
    }
    it = std::find_if(inpSysStaves.begin(), inpSysStaves.end(), [&](const auto& item) {
        return item->staffId == group->endInst;
    });
    if (it != inpSysStaves.end()) {
        endSlot = std::distance(inpSysStaves.begin(), it);
    }
}

void StaffGroupInfo::iterateStaves(MeasCmper measId, Edu eduPosition, std::function<bool(const MusxInstance<others::StaffComposite>&)> iterator) const
{
    MUSX_ASSERT_IF (!startSlot || !endSlot) {
        throw std::logic_error("StaffGroupInfo::iterateStaves called with invalid start or end slot for system staves.");
    }
    for (size_t slot = startSlot.value(); slot <= endSlot.value(); slot++) {
        MUSX_ASSERT_IF (slot >= systemStaves.size()) {
            throw std::logic_error("StaffGroupInfo::iterateStaves encounted invalid slot (" + std::to_string(slot) + ") for system staves.");
        }
        const auto& iUsed = systemStaves[slot];
        if (auto staff = others::StaffComposite::createCurrent(iUsed->getDocument(), iUsed->getRequestedPartId(), iUsed->staffId, measId, eduPosition)) {
            if (!iterator(staff)) {
                break;
            }
        } else {
            MUSX_INTEGRITY_ERROR("StaffGroupInfo::iterateStaves could not find staff " + std::to_string(iUsed->staffId) + " in slot " + std::to_string(slot));
        }
    }
}

bool StaffGroupInfo::calcIsSingleInstrumentSection(MeasCmper measureId) const
{
    const auto numGroupStaves = numStaves();
    if (!numGroupStaves || numGroupStaves.value() < 2) {
        return false;
    }

    bool hasUuid = false;
    bool isSection = true;
    std::string sectionUuid;
    std::unordered_set<StaffCmper> groupStaffIds;

    iterateStaves(measureId, 0, [&](const MusxInstance<others::StaffComposite>& staff) {
        if (!staff->hasInstrumentAssigned()) {
            isSection = false;
            return false;
        }
        if (!hasUuid) {
            sectionUuid = staff->instUuid;
            hasUuid = true;
        } else if (staff->instUuid != sectionUuid) {
            isSection = false;
            return false;
        }
        groupStaffIds.insert(staff->getCmper());
        return true;
    });

    if (!isSection || !hasUuid || groupStaffIds.size() < 2) {
        return false;
    }

    const auto doc = group->getDocument();
    const auto partId = group->getRequestedPartId();
    DeferredReference<InstrumentMap> instrumentMap;

    if (partId == SCORE_PARTID) {
        instrumentMap.bind(doc->getInstruments());
    } else {
        instrumentMap.emplace(doc->createInstrumentMap(partId));
    }

    for (const auto& instrument : *instrumentMap) {
        const auto& instInfo = instrument.second;
        if (instInfo.staves.size() != groupStaffIds.size()) {
            continue;
        }
        bool matchesInstrument = true;
        for (const auto& staff : instInfo.staves) {
            const auto staffId = staff.first;
            if (groupStaffIds.find(staffId) == groupStaffIds.end()) {
                matchesInstrument = false;
                break;
            }
        }
        if (matchesInstrument) {
            return false;
        }
    }

    return true;
}

std::vector<StaffGroupInfo> StaffGroupInfo::getGroupsAtMeasure(MeasCmper measureId, Cmper linkedPartId,
    const MusxInstanceList<others::StaffUsed>& systemStaves)
{
    if (systemStaves.empty()) {
        util::Logger::log(util::Logger::LogLevel::Info, "Attempted to find groups for empty system staves. [measure " + std::to_string(measureId)
            + ", part " + std::to_string(linkedPartId) +"] Returning an empty vector.");
        return {};
    }
    const auto doc = systemStaves.getDocument();
    auto rawGroups = doc->getDetails()->getArray<StaffGroup>(linkedPartId, doc->calcScrollViewCmper(linkedPartId));
    std::vector<StaffGroupInfo> retval;
    for (const auto& rawGroup : rawGroups) {
        if (rawGroup->startMeas <= measureId && rawGroup->endMeas >= measureId) {
            StaffGroupInfo group(rawGroup, systemStaves);
            if (group.startSlot && group.endSlot) {
                retval.emplace_back(std::move(group));
            }
        }
    }
    return retval;
}

// ************************
// ***** TieAlterBase *****
// ************************

MusxInstance<TieAlterBase> TieAlterBase::fromNoteInfo(const NoteInfoPtr& noteInfoPtr, bool forTieEnd)
{
    const auto entry = noteInfoPtr.getEntryInfo()->getEntry();
    if (forTieEnd) {
        return entry->getDocument()->getDetails()->getForNote<TieAlterEnd>(noteInfoPtr);
    }
    return entry->getDocument()->getDetails()->getForNote<TieAlterStart>(noteInfoPtr);
}

} // namespace details
} // namespace dom
} // namespace musx
