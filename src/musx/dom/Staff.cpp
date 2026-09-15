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

#include "musx/musx.h"

namespace musx {
namespace dom {
namespace others {

// *****************
// ***** Staff *****
// *****************

void Staff::calcAllAutoNumberValues(const DocumentPtr& document)
{
    // Auto-numbering must consider every staff in the file, so use the base system list rather than
    // the Scroll View list, which may be the Special Part Extraction subset for the score.
    auto scrollViewList = document->getOthers()->getArray<StaffUsed>(SCORE_PARTID, BASE_SYSTEM_ID);

    // Map to track counts for instUuid
    std::unordered_map<std::string, int> instUuidCounts;
    std::unordered_set<Cmper> countedMultistaffGroups;

    // Pass 1: Check if any instUuid has auto-numbering disabled
    std::unordered_set<std::string> disabledInstUuids;
    for (const auto& instrumentUsed : scrollViewList) {
        auto staff = instrumentUsed->getStaffInstance();
        if (staff && !staff->useAutoNumbering) {
            disabledInstUuids.insert(staff->instUuid);
        }
    }

    // Pass 2: Count occurrences of instUuid, considering multistaff instruments
    for (const auto& instrumentUsed : scrollViewList) {
        auto staff = instrumentUsed->getStaffInstance();
        if (!staff || staff->instUuid.empty() || disabledInstUuids.count(staff->instUuid)) {
            continue;
        }

        // Check if the staff is part of a multistaff instrument
        auto multiStaffGroup = staff->getMultiStaffInstGroup();
        if (multiStaffGroup) {
            if (countedMultistaffGroups.find(multiStaffGroup->getCmper()) != countedMultistaffGroups.end()) {
                continue; // Skip already-counted multistaff groups
            }
            countedMultistaffGroups.insert(multiStaffGroup->getCmper());
        }
        instUuidCounts[staff->instUuid]++;
    }

    // Pass 2.1: Remove singleton or empty instUuid counts, including single multistaff instruments
    for (const auto& count : instUuidCounts) {
        if (count.second <= 1) {
            disabledInstUuids.insert(count.first);
        }
    }

    // Pass 3: Assign auto-numbering values
    std::unordered_map<std::string, int> instUuidNumbers;
    countedMultistaffGroups.clear(); // Reset for numbering

    for (const auto& instrumentUsed : scrollViewList) {
        auto staff = instrumentUsed->getStaffInstance();
        Staff* mutableStaff = const_cast<Staff*>(staff.get());
        if (!staff) continue;
        if (staff->instUuid.empty() || disabledInstUuids.count(staff->instUuid)) {
            mutableStaff->autoNumberValue = std::nullopt; // No numbering for disabled or empty instUuid
            continue;
        }

        // Check if the staff is part of a multistaff instrument
        auto multiStaffGroup = staff->getMultiStaffInstGroup();
        if (multiStaffGroup) {
            if (countedMultistaffGroups.find(multiStaffGroup->getCmper()) == countedMultistaffGroups.end()) {
                // Assign a number for this multistaff group
                countedMultistaffGroups.insert(multiStaffGroup->getCmper());
                instUuidNumbers[staff->instUuid]++;
            }
            // Assign the same number to all staves in the group
            auto groupNumber = instUuidNumbers[staff->instUuid];
            for (size_t x = 0; x < multiStaffGroup->staffNums.size(); x++) {
                auto groupStaff = multiStaffGroup->getStaffInstanceAtIndex(x);
                if (groupStaff) {
                    Staff* mutableGroupStaff = const_cast<Staff*>(groupStaff.get());
                    mutableGroupStaff->autoNumberValue = groupNumber;
                }
            }
            continue; // Skip further processing for the current staff
        }

        // Assign a number for single staves
        instUuidNumbers[staff->instUuid]++;
        mutableStaff->autoNumberValue = instUuidNumbers[staff->instUuid];
    }
}
template <typename SubType>
void Staff::calcAllRuntimeValues(const DocumentPtr& document)
{
    static_assert(std::is_same_v<SubType, Staff>
               || std::is_same_v<SubType, StaffStyle>,
        "SubType template parameter must be Staff or StaffStyle.");
    using DrumStaffType = std::conditional_t<std::is_same_v<SubType, Staff>, DrumStaff, DrumStaffStyle>;
    using NamePositionFullType = std::conditional_t<std::is_same_v<SubType, Staff>, NamePositionFull, NamePositionStyleFull>;
    using NamePositionAbrvType = std::conditional_t<std::is_same_v<SubType, Staff>, NamePositionAbbreviated, NamePositionStyleAbbreviated>;
    using NoteShapesType = std::conditional_t<std::is_same_v<SubType, Staff>, details::ShapeNote, details::ShapeNoteStyle>;
    constexpr bool isForStyle = std::is_same_v<SubType, StaffStyle>;

    auto list = document->getOthers()->getArray<SubType>(SCORE_PARTID);
    for (const auto& item : list) {
        SubType* mutableItem = const_cast<SubType*>(item.get());
        if (item->notationStyle == Staff::NotationStyle::Percussion) {
            if (auto drumStaff = document->getOthers()->get<DrumStaffType>(SCORE_PARTID, item->getCmper())) {
                mutableItem->percussionMapId = drumStaff->whichDrumLib;
            } else {
                mutableItem->percussionMapId = Cmper(0);
                util::Logger::log(util::Logger::LogLevel::Verbose,
                    "Staff or StaffStyle " + std::to_string(item->getCmper())
                        + " is percussion style but has no DrumStaff record.");
            }
        } else {
            mutableItem->percussionMapId = std::nullopt;
        }
        bool checkNoteShapes = true;
        if constexpr (isForStyle) {
            checkNoteShapes = item->masks->useNoteShapes;
        }
        if (checkNoteShapes) {
            // Note shapes are slightly odd because their default values are at staff cmper 0. Therefore, even if this
            // is a staff style, we should only set noteShapeFromStyle if there is a separate note shapes record for the style.
            if (auto shapes = document->getDetails()->get<NoteShapesType>(SCORE_PARTID, item->getCmper(), 0)) {
                mutableItem->noteShapesId = item->getCmper();
            } else {
                mutableItem->noteShapesId = Cmper(0);
            }
            mutableItem->noteShapesFromStyle = isForStyle;
        }
        bool checkFullNeeded = true;
        if constexpr (isForStyle) {
            checkFullNeeded = item->masks->fullNamePos;
        }
        if (checkFullNeeded) {
            if (auto full = document->getOthers()->get<NamePositionFullType>(SCORE_PARTID, item->getCmper())) {
                mutableItem->fullNamePosId = item->getCmper();
            } else {
                mutableItem->fullNamePosId = Cmper(0);
            }
            mutableItem->fullNamePosFromStyle = isForStyle;
        }
        bool checkAbbrvNeeded = true;
        if constexpr (isForStyle) {
            checkAbbrvNeeded = item->masks->abrvNamePos;
        }
        if (checkAbbrvNeeded) {
            if (auto abrv = document->getOthers()->get<NamePositionAbrvType>(SCORE_PARTID, item->getCmper())) {
                mutableItem->abrvNamePosId = item->getCmper();
            } else {
                mutableItem->abrvNamePosId = Cmper(0);
            }
            mutableItem->abrvNamePosFromStyle = isForStyle;
        }
    }
}

template void Staff::calcAllRuntimeValues<Staff>(const DocumentPtr&);
template void Staff::calcAllRuntimeValues<StaffStyle>(const DocumentPtr&);

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
static std::string intToRoman(int num)
{
    if (num <= 0 || num > 3999) {
        throw std::out_of_range("Number out of range for Roman numerals (1-3999)");
    }

    const std::vector<std::pair<int, std::string>> romanMap = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"},
        {100, "C"}, {90, "XC"}, {50, "L"}, {40, "XL"},
        {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}
    };

    std::string result;
    for (const auto& [value, symbol] : romanMap) {
        while (num >= value) {
            result += symbol;
            num -= value;
        }
    }
    return result;
}

std::string intToAlphabetic(int num) {
    if (num <= 0) {
        throw std::out_of_range("Number must be greater than 0");
    }

    std::string result;
    while (num > 0) {
        --num; // Convert to 0-based index
        result.insert(result.begin(), 'A' + (num % 26));
        num /= 26;
    }

    return result;
}

std::string ordinalPrefix(int num)
{
    if (num <= 0) {
        throw std::out_of_range("Number must be greater than 0");
    }

    int lastTwoDigits = num % 100;
    int lastDigit = num % 10;

    if (lastTwoDigits >= 11 && lastTwoDigits <= 13) {
        return std::to_string(num) + "th";
    }

    switch (lastDigit) {
        case 1: return std::to_string(num) + "st";
        case 2: return std::to_string(num) + "nd";
        case 3: return std::to_string(num) + "rd";
        default: return std::to_string(num) + "th";
    }
}
#endif // DOXYGEN_SHOULD_IGNORE_THIS

std::pair<std::string, bool> Staff::calcAutoNumberingAffix() const
{
    if (!autoNumberValue.has_value()) {
        return std::make_pair(std::string(), false); // No numbering needed
    }

    int number = *autoNumberValue;
    switch (autoNumbering) {
        default:
        case AutoNumberingStyle::ArabicSuffix:
            return std::make_pair(" " + std::to_string(number), false);
        case AutoNumberingStyle::RomanSuffix:
            return std::make_pair(" " + intToRoman(number), false);
        case AutoNumberingStyle::OrdinalPrefix:
            return std::make_pair(ordinalPrefix(number) + " ", true);
        case AutoNumberingStyle::AlphaSuffix:
            return std::make_pair(" " + intToAlphabetic(number), false);
        case AutoNumberingStyle::ArabicPrefix:
            return std::make_pair(std::to_string(number) + ". ", true);
    }
}

std::string Staff::addAutoNumbering(const std::string& plainName) const
{
    auto [affix, isPrefix] = calcAutoNumberingAffix();
    if (affix.empty()) {
        return plainName;
    }
    if (isPrefix) {
        return affix + plainName;
    }
    return plainName + affix;
}

std::string Staff::getFullName(util::EnigmaString::AccidentalStyle accidentalStyle) const
{
    return TextBlock::getText(getDocument(), fullNameTextId, SCORE_PARTID, true, accidentalStyle); // true: strip enigma tags
}

std::string Staff::getAbbreviatedName(util::EnigmaString::AccidentalStyle accidentalStyle) const
{
    return TextBlock::getText(getDocument(), abbrvNameTextId, SCORE_PARTID, true, accidentalStyle); // true: strip enigma tags
}

bool Staff::calcShowName() const
{
    if (getRequestedPartId() == SCORE_PARTID) {
        return !hideNameInScore;
    }
    return showNameInParts;
}

MusxInstance<MultiStaffInstrumentGroup> Staff::getMultiStaffInstGroup() const
{
    if (multiStaffInstId) {
        if (auto retval = getDocument()->getOthers()->get<MultiStaffInstrumentGroup>(SCORE_PARTID, multiStaffInstId)) {
            return retval;
        }
        MUSX_INTEGRITY_ERROR("Staff " + std::to_string(getCmper()) + " points to non-existent MultiStaffInstrumentGroup " + std::to_string(multiStaffInstId));
    }
    return nullptr;
}

MusxInstance<details::StaffGroup> Staff::getMultiStaffInstVisualGroup(Cmper forPartId) const
{
    const auto groupId = [&]() -> std::optional<Cmper> {
        if (forPartId == SCORE_PARTID) {
            // This helper can be reached while factory resolvers are formatting diagnostics, before
            // the score instrument map is constructed. In that case, allow the caller to fall back
            // to the staff's own name rather than making name lookup depend on factory ordering.
            if (const auto& instruments = getDocument()->getInstrumentsIfAvailable()) {
                if (const auto result = instruments->getInstrumentForStaff(getCmper())) {
                    return result->staffGroupId;
                }
            }
            return std::nullopt;
        }
        const auto map = getDocument()->createInstrumentMap(forPartId);
        auto result = map.getInstrumentForStaff(getCmper());
        if (result) {
            return result->staffGroupId;
        }
        return std::nullopt;
    }();
    if (groupId.value_or(0) != 0) {
        if (auto retval = getDocument()->getDetails()->get<details::StaffGroup>(forPartId, getDocument()->calcScrollViewCmper(forPartId), groupId.value())) {
            return retval;
        } else {
            MUSX_INTEGRITY_ERROR("Instrument map " + std::to_string(getCmper()) + " points to non-existent StaffGroup " + std::to_string(groupId.value())
                + " for staff " + std::to_string(getCmper()));
        }
    }
    return nullptr;
}

util::EnigmaParsingContext Staff::getFullInstrumentNameCtx(Cmper forPartId, bool preferStaffName) const
{
    auto parsingContext = [&]() -> util::EnigmaParsingContext {
        if (!preferStaffName || !fullNameTextId) {
            if (auto group = getMultiStaffInstVisualGroup(forPartId)) {
                return group->getFullNameCtx();
            }
        }
        if (const auto block = getDocument()->getOthers()->get<TextBlock>(forPartId, fullNameTextId)) {
            return block->getRawTextCtx(forPartId);
        }
        return {};
    }();
    auto [affix, isPrefix] = calcAutoNumberingAffix();
    parsingContext.affixText = affix;
    parsingContext.affixIsPrefix = isPrefix;
    return parsingContext;
}

std::string Staff::getFullInstrumentName(util::EnigmaString::AccidentalStyle accidentalStyle, bool preferStaffName) const
{
    if (auto ctx = getFullInstrumentNameCtx(getRequestedPartId(), preferStaffName)) {
        return ctx.getText(true, accidentalStyle);
    }
    return {};
}

util::EnigmaParsingContext Staff::getAbbreviatedInstrumentNameCtx(Cmper forPartId, bool preferStaffName) const
{
    auto parsingContext = [&]() -> util::EnigmaParsingContext {
        if (!preferStaffName || !abbrvNameTextId) {
            if (auto group = getMultiStaffInstVisualGroup(forPartId)) {
                return group->getAbbreviatedNameCtx();
            }
        }
        if (const auto block = getDocument()->getOthers()->get<TextBlock>(forPartId, abbrvNameTextId)) {
            return block->getRawTextCtx(forPartId);
        }
        return {};
    }();
    auto [affix, isPrefix] = calcAutoNumberingAffix();
    parsingContext.affixText = affix;
    parsingContext.affixIsPrefix = isPrefix;
    return parsingContext;
}

std::string Staff::getAbbreviatedInstrumentName(util::EnigmaString::AccidentalStyle accidentalStyle, bool preferStaffName) const
{
    if (auto ctx = getAbbreviatedInstrumentNameCtx(getRequestedPartId(), preferStaffName)) {
        return ctx.getText(true, accidentalStyle);
    }
    return {};
}

std::string Staff::getPlaybackRouteName() const
{
    const auto staffPlayData = getDocument()->getOthers()->get<StaffPlayData>(getRequestedPartId(), getCmper());
    if (!staffPlayData) {
        return {};
    }

    const auto findRouteName = [this](const std::shared_ptr<StaffPlayData::PlaybackSettings>& settings) {
        if (settings) {
            const auto routeName = getDocument()->getOthers()->get<PlaybackRouteName>(
                getRequestedPartId(), settings->getPlaybackRouteId());
            if (routeName && !routeName->name.empty()) {
                return routeName->name;
            }
        }
        return std::string{};
    };

    for (const auto& layer : staffPlayData->layers) {
        if (auto name = findRouteName(layer); !name.empty()) {
            return name;
        }
    }
    if (auto name = findRouteName(staffPlayData->chords); !name.empty()) {
        return name;
    }
    return findRouteName(staffPlayData->midiExpressions);
}

bool Staff::calcShowInstrumentName() const
{
    if (auto group = getMultiStaffInstVisualGroup(getRequestedPartId())) {
        return !group->hideName;
    }
    return calcShowName();
}

MusxInstance<details::ShapeNoteBase> Staff::getNoteShapes() const
{
    if (useNoteShapes) {
        if (noteShapesId) {
            if (noteShapesFromStyle) {
                return getDocument()->getDetails()->get<details::ShapeNoteStyle>(getRequestedPartId(), noteShapesId, 0);
            }
            return getDocument()->getDetails()->get<details::ShapeNote>(getRequestedPartId(), noteShapesId, 0);
        }
    } else {
        if (auto noteRestOptions = getDocument()->getOptions()->get<options::NoteRestOptions>()) {
            if (noteRestOptions->doShapeNotes) {
                return getDocument()->getDetails()->get<details::ShapeNote>(getRequestedPartId(), 0, 0);
            }
        }
    }
    return nullptr;
}

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
template <typename NamePositionType>
MusxInstance<NamePositioning> Staff::getNamePosition() const
{
    static_assert(std::is_same_v<NamePositionType, NamePositionAbbreviated>
               || std::is_same_v<NamePositionType, NamePositionStyleAbbreviated>
               || std::is_same_v<NamePositionType, NamePositionFull>
               || std::is_same_v<NamePositionType, NamePositionStyleFull>,
        "NamePositionType must be a name positioning type.");
    constexpr bool isForFull = std::is_same_v<NamePositionType, NamePositionFull> || std::is_same_v<NamePositionType, NamePositionStyleFull>;

    const Cmper posCmper = isForFull ? fullNamePosId : abrvNamePosId;
    if (posCmper) {
        if (auto pos = getDocument()->getOthers()->get<NamePositionType>(getRequestedPartId(), posCmper)) {
            return pos;
        }
    }

    MusxInstance<NamePositioning> defaultValue;
    if (auto staffOptions = getDocument()->getOptions()->get<options::StaffOptions>()) {
        if constexpr (isForFull) {
            defaultValue = staffOptions->namePos;
        } else {
            defaultValue = staffOptions->namePosAbbrv;
        }
    } else {
        MUSX_INTEGRITY_ERROR("Unable to retrieve staff options for returning default name positioning.");
    }
    return defaultValue;
}
#endif // DOXYGEN_SHOULD_IGNORE_THIS

MusxInstance<NamePositioning> Staff::getFullNamePosition() const
{
    if (fullNamePosFromStyle) {
        return getNamePosition<NamePositionStyleFull>();
    }
    return getNamePosition<NamePositionFull>();
}

MusxInstance<NamePositioning> Staff::getAbbreviatedNamePosition() const
{
    if (abrvNamePosFromStyle) {
        return getNamePosition<NamePositionStyleAbbreviated>();
    }
    return getNamePosition<NamePositionAbbreviated>();
}

ClefIndex Staff::calcClefIndexAt(MeasCmper measureId, Edu position, bool forWrittenPitch) const
{
    if (forWrittenPitch && transposition && transposition->setToClef) {
        return transposedClef;
    }
    for (MeasCmper tryMeasure = measureId; tryMeasure > 0; tryMeasure--) {
        if (auto gfhold = details::GFrameHoldContext(getDocument(), getRequestedPartId(), getCmper(), tryMeasure)) {
            return gfhold.calcClefIndexAt(position);
        }
        // after the first iteration, we are looking for the clef at the end of the measure
        position = (std::numeric_limits<Edu>::max)();
    }
    return defaultClef;
}

ClefIndex Staff::calcFirstClefIndex(const DocumentPtr& document, Cmper partId, StaffCmper staffCmper)
{
    if (auto staff = StaffComposite::createCurrent(document, partId, staffCmper, 1, 0)) {
        return staff->calcFirstClefIndex();
    } else {
        throw std::logic_error("Unable to find StaffComposite instance for staff " + std::to_string(staffCmper));
    }
}

bool Staff::iterateClefChangesAtMeasure(
    MeasCmper measureId,
    bool forWrittenPitch,
    std::function<bool(const ClefChange&)> iterator) const
{
    const auto document = getDocument();
    const auto partId = getRequestedPartId();
    const auto staffId = getCmper();

    if (forWrittenPitch) {
        const auto currentStaff = StaffComposite::createCurrent(document, partId, staffId, measureId, 0);
        if (currentStaff && currentStaff->transposition && currentStaff->transposition->setToClef) {
            return iterator(ClefChange{
                currentStaff->transposedClef,
                {},
                ShowClefMode::WhenNeeded,
                false,
                0,
                100,
                0
            });
        }
    }

    const auto gfhold = document->getDetails()->get<details::GFrameHold>(partId, staffId, measureId);
    if (!gfhold) {
        if (measureId == 1) {
            const auto currentStaff = StaffComposite::createCurrent(document, partId, staffId, measureId, 0);
            return iterator(ClefChange{
                currentStaff->defaultClef,
                {},
                ShowClefMode::WhenNeeded,
                false,
                0,
                100,
                0
            });
        }
        return true;
    }

    if (gfhold->clefId.has_value()) {
        return iterator(ClefChange{
            gfhold->clefId.value(),
            {},
            gfhold->showClefMode,
            gfhold->clefAfterBarline,
            0,
            gfhold->clefPercent,
            0
        });
    }

    const auto clefList = document->getOthers()->getArray<others::ClefList>(partId, gfhold->clefListId);
    if (clefList.empty()) {
        MUSX_INTEGRITY_ERROR("GFrameHold for staff " + std::to_string(gfhold->getStaff()) + " and measure "
            + std::to_string(gfhold->getMeasure()) + " has non-existent clef list [" + std::to_string(gfhold->clefListId) + "]");
    }
    const auto gfholdContext = details::GFrameHoldContext(gfhold);
    for (const auto& clefItem : clefList) {
        const auto position = util::Fraction::fromEdu(clefItem->xEduPos);
        const auto snappedPosition = gfholdContext.snapLocationToEntryOrKeep(position, /*findExact*/ true);
        if (!iterator(ClefChange{
                clefItem->clefIndex,
                snappedPosition,
                clefItem->clefMode,
                clefItem->xEduPos == 0 && clefItem->afterBarline,
                clefItem->yEvpuPos,
                clefItem->percent,
                clefItem->xEvpuOffset
            })) {
            return false;
        }
    }
    return true;
}

int Staff::calcNumberOfStafflines() const
{
    if (staffLines.has_value()) {
        return staffLines.value();
    } else if (customStaff.has_value()) {
        return int(customStaff.value().size());
    }
    return music_theory::STANDARD_NUMBER_OF_STAFFLINES;
}

int Staff::calcMiddleStaffPosition() const
{
    if (calcNumberOfStafflines() == 0) {
        return -(music_theory::STANDARD_NUMBER_OF_STAFFLINES - 1);
    }
    if (staffLines.has_value()) {
        return -(staffLines.value() - 1);
    } else if (customStaff.has_value()) {
        const auto& lines = customStaff.value();
        if (!lines.empty()) {
            int numLines = lines[lines.size() - 1] - lines[0] + 1;
            int referenceOffset = 2 * (11 - lines[0]);
            return referenceOffset - (numLines - 1);
        }
    }
    return 0;
}

Evpu Staff::calcRestOffset(Edu edu) const
{
    switch (std::get<0>(calcDurationInfoFromEdu(edu))) {
        case NoteType::Longa:
        case NoteType::Maxima:
            return otherRestOffset;
        case NoteType::Breve:
            return dwRestOffset;
        case NoteType::Whole:
            return wRestOffset;
        case NoteType::Half:
            return hRestOffset;
        default:
            return otherRestOffset;
    }
}

std::pair<Evpu, Evpu> Staff::calcBarlineOffsetsFromCenter() const
{
    const Evpu topLine = calcTopLineEvpu();
    const Evpu bottomLine = calcBottomLineEvpu();
    const Evpu center = (topLine + bottomLine) / 2;
    const Evpu topEndpoint = topLine + topBarlineOffset;
    const Evpu bottomEndpoint = bottomLine + botBarlineOffset;

    return {topEndpoint - center, bottomEndpoint - center};
}

int Staff::calcTopLinePosition() const
{
    if (staffLines.has_value()) {
        return 0;
    }
    if (customStaff.has_value()) {
        const auto& lines = customStaff.value();
        if (!lines.empty()) {
            return 2 * (11 - lines.front());
        }
    }
    return 0;
}

int Staff::calcBottomLinePosition() const
{
    if (staffLines.has_value()) {
        return 2 * (1 - staffLines.value());
    }
    if (customStaff.has_value()) {
        const auto& lines = customStaff.value();
        if (!lines.empty()) {
            return 2 * (11 - lines.back());
        }
    }
    return 0;
}

Evpu Staff::calcBaselineZeroPosition() const
{
    // Weird cases involving custom lines above the reference line may require additional logic, but
    // this gets us going for the vast majority of staves in Finale. The baseline zero position here
    // is calculated as halfway between the reference line and the bottom staff lines.
    if (calcNumberOfStafflines() == 0) {
        static constexpr Evpu kNumberOfSpaces = (music_theory::STANDARD_NUMBER_OF_STAFFLINES - 1) / 2;
        return -kNumberOfSpaces * static_cast<Evpu>(EVPU_PER_SPACE); // Finale treats a blank staff like a standard staff for baselines
    }
    return calcBottomLineEvpu() / 2;
}

template<typename BaselineType>
Evpu Staff::calcBaselinePositionImpl(SystemCmper systemId, std::optional<Cmper> lyricNumber) const
{
    static_assert(std::is_base_of<details::Baseline, BaselineType>::value, "BaselineType must derive from Baseline");

    // Shared finder lambda (used for all baseline arrays)
    const auto findBaseline = [&](const auto& array)
    {
        using Elem = typename std::remove_cv_t<
                     typename std::remove_reference_t<decltype(array)>::value_type>; // e.g. std::shared_ptr<const BaselineType>
        const auto it = std::find_if(array.begin(), array.end(),
            [&](const auto& ptr)
            {
                assert(ptr && "input array should not contain nulls!");
                if (!lyricNumber) return true;
                return ptr->lyricNumber == lyricNumber;
            });
        return (it != array.end()) ? *it : Elem{};
    };

    Evpu result = calcBaselineZeroPosition();

    auto globalArray = getDocument()->getDetails()->getArray<BaselineType>(getRequestedPartId(), Cmper{0}, Cmper{0});
    if (auto globalBaseline = findBaseline(globalArray)) {
        result += globalBaseline->baselineDisplacement;
    }
    auto staffArray = getDocument()->getDetails()->getArray<BaselineType>(getRequestedPartId(), Cmper{0}, getCmper());
    if (auto staffBaseline = findBaseline(staffArray)) {
        result += staffBaseline->baselineDisplacement;
    }
    auto systemArray = getDocument()->getDetails()->getArray<typename BaselineType::PerSystemType>(getRequestedPartId(), systemId, getCmper());
    if (auto systemBaseline = findBaseline(systemArray)) {
        result += systemBaseline->baselineDisplacement;
    }

    return result;
}

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
template Evpu Staff::calcBaselinePositionImpl<details::BaselineChords>(SystemCmper, std::optional<Cmper>) const;
template Evpu Staff::calcBaselinePositionImpl<details::BaselineExpressionsAbove>(SystemCmper, std::optional<Cmper>) const;
template Evpu Staff::calcBaselinePositionImpl<details::BaselineExpressionsBelow>(SystemCmper, std::optional<Cmper>) const;
template Evpu Staff::calcBaselinePositionImpl<details::BaselineFretboards>(SystemCmper, std::optional<Cmper>) const;
template Evpu Staff::calcBaselinePositionImpl<details::BaselineLyricsChorus>(SystemCmper, std::optional<Cmper>) const;
template Evpu Staff::calcBaselinePositionImpl<details::BaselineLyricsSection>(SystemCmper, std::optional<Cmper>) const;
template Evpu Staff::calcBaselinePositionImpl<details::BaselineLyricsVerse>(SystemCmper, std::optional<Cmper>) const;
#endif //DOXYGEN_SHOULD_IGNORE_THIS

std::vector<LyricsLineInfo> Staff::createLyricsLineInfo(SystemCmper systemId) const
{
    std::vector<LyricsLineInfo> result;

    auto addLyricsLines = [&](const auto& lyricAssigns) {
        using PtrType = typename std::decay_t<decltype(lyricAssigns)>::value_type;
        using T = typename PtrType::element_type;
        static_assert(std::is_base_of_v<details::LyricAssign, T>, "lyricAssigns must be a subtype of LyricAssign");
        if (lyricAssigns.empty()) {
            return;
        }
        // All items in this range share these:
        constexpr std::string_view typeStr = T::TextType::XmlNodeName;                  // safe: static storage
        for (const auto& lyr : lyricAssigns) {
            const Cmper lyricNo = lyr->lyricNumber;
            // Find an existing line with the same type and lyric number.
            auto it = std::find_if(result.begin(), result.end(),
                [&](const LyricsLineInfo& line) {
                    if (line.lyricsType != typeStr) {
                        return false;
                    }
                    return line.lyricNumber == lyricNo;
                });
            // Create the line once if not found.
            if (it == result.end()) {
                const Evpu baseline = calcBaselinePosition<typename T::BaselineType>(systemId, lyricNo);
                result.emplace_back(getDocument(), getRequestedPartId(), typeStr, lyricNo, baseline);
                it = std::prev(result.end());
            }
            MUSX_ASSERT_IF(it->lyricNumber != lyricNo) {
                throw std::logic_error("lyricNumber mismatch in LyricsLineInfo.");
            }
            it->assignments.emplace_back(lyr);
        }
    };

    if (auto system = getDocument()->getOthers()->get<StaffSystem>(getRequestedPartId(), systemId)) {
        for (MeasCmper measId = system->startMeas; measId < system->endMeas; measId++) {
            if (auto gfHold = getDocument()->getDetails()->get<details::GFrameHold>(getRequestedPartId(), getCmper(), measId)) {
                for (Cmper frameId : gfHold->frames) {
                    if (frameId != 0) {
                        auto frames = getDocument()->getOthers()->getArray<Frame>(getRequestedPartId(), frameId);
                        for (const auto& frame : frames) {
                            if (!frame->startEntry) {
                                continue;
                            }
                            frame->iterateRawEntries([&](const MusxInstance<Entry>& entry) -> bool {
                                if (entry->lyricDetail) {
                                    addLyricsLines(getDocument()->getDetails()->getArray<details::LyricAssignVerse>(getRequestedPartId(), entry->getEntryNumber()));
                                    addLyricsLines(getDocument()->getDetails()->getArray<details::LyricAssignChorus>(getRequestedPartId(), entry->getEntryNumber()));
                                    addLyricsLines(getDocument()->getDetails()->getArray<details::LyricAssignSection>(getRequestedPartId(), entry->getEntryNumber()));
                                }
                                return true;
                            });
                        }
                    }
                }
            }
        }
    }

    std::sort(result.begin(), result.end(),
        [](const LyricsLineInfo& a, const LyricsLineInfo& b) {
            return a.baselinePosition > b.baselinePosition;
        });
    return result;
}

bool Staff::hasInstrumentAssigned() const
{
    if (instUuid.empty() || instUuid == uuid::BlankStaff || instUuid == uuid::BlankStaff2 || instUuid == uuid::Unknown) {
        return false;
    }
    return true;
}

std::pair<int, int> Staff::calcTranspositionInterval() const
{
    if (transposition) {
        if (transposition->chromatic) {
            return std::make_pair(transposition->chromatic->diatonic, transposition->chromatic->alteration);
        } else if (transposition->keysig) {
            const int alteration = music_theory::calcAlterationFromKeySigChange(transposition->keysig->interval, transposition->keysig->adjust);
            return std::make_pair(transposition->keysig->interval, alteration);
        }
    }
    return std::make_pair(0, 0);
}

MusxInstanceList<PartDefinition> Staff::getContainingParts(bool includeScore) const
{
    MusxInstanceList<PartDefinition> result(getDocument(), SCORE_PARTID);
    auto parts = getDocument()->getOthers()->getArray<PartDefinition>(SCORE_PARTID);
    for (const auto& part : parts) {
        if (!includeScore && part->getCmper() == SCORE_PARTID) {
            continue;
        }
        auto scrollView = getDocument()->getScrollViewStaves(part->getCmper());
        for (const auto& next : scrollView) {
            if (next->staffId == this->getCmper()) {
                result.push_back(part);
                break;
            }
        }
    }
    return result;
}

MusxInstance<PartDefinition> Staff::firstContainingPart() const
{
    auto parts = getDocument()->getOthers()->getArray<PartDefinition>(SCORE_PARTID);
    for (const auto& part : parts) {
        if (part->getCmper() != SCORE_PARTID) {
            auto scrollView = getDocument()->getScrollViewStaves(part->getCmper());
            for (const auto& next : scrollView) {
                if (next->staffId == this->getCmper()) {
                    return part;
                }
            }
        }
    }
    return nullptr;
}

bool Staff::calcAlternateNotationHidesEntries(LayerIndex forLayerIndex) const
{
    const bool replacesEntries = [this]() {
        switch (altNotation) {
            case others::Staff::AlternateNotation::Blank:
            case others::Staff::AlternateNotation::BlankWithRests:
            case others::Staff::AlternateNotation::OneBarRepeat:
            case others::Staff::AlternateNotation::TwoBarRepeat:
            case others::Staff::AlternateNotation::SlashBeats:
                return true;
            // Rhythmic notation restyles the entries rather than replacing them, so the layer still
            // counts as having notes.
            case others::Staff::AlternateNotation::Rhythmic:
            case others::Staff::AlternateNotation::Normal:
                return false;
        }
        return false;
    }();
    return replacesEntries && (altLayer == forLayerIndex || altHideOtherNotes);
}

bool Staff::iterateEntries(std::function<bool(const EntryInfoPtr&)> iterator) const
{
    auto doc = getDocument();
    auto scrollView = doc->getScrollViewStaves(getRequestedPartId());
    auto index = scrollView.getIndexForStaff(getCmper());
    MUSX_ASSERT_IF(!index) {
        throw std::logic_error("Staff's requested part [" + std::to_string(getRequestedPartId()) + "] does not contain the staff [" + std::to_string(getCmper()) + "].");
    }
    return scrollView.iterateEntries(index.value(), index.value(), doc->calcEntireDocument(), iterator);
}

bool Staff::calcIsSameNotationStyle(const Staff& other) const
{
    if (notationStyle != other.notationStyle) {
        return false;
    }
    if (capoPos != other.capoPos) {
        return false;
    }
    if (lowestFret != other.lowestFret) {
        return false;
    }
    if (vertTabNumOff != other.vertTabNumOff) {
        return false;
    }
    if (showTabClefAllSys != other.showTabClefAllSys) {
        return false;
    }
    if (useTabLetters != other.useTabLetters) {
        return false;
    }
    if (breakTabLinesAtNotes != other.breakTabLinesAtNotes) {
        return false;
    }
    if (hideTuplets != other.hideTuplets) {
        return false;
    }
    if (fretInstId != other.fretInstId) {
        return false;
    }    
    if (percussionMapId != other.percussionMapId) {
        return false;
    }
    return true;
}

// ********************************
// ***** Staff::Transposition *****
// ********************************

bool Staff::Transposition::isSame(const Staff::Transposition& other) const
{
    if (setToClef != other.setToClef) {
        return false;
    }
    if (setToClef) {
        const auto& thisParent = getParent<Staff>();
        const auto& otherParent = other.getParent<Staff>();
        MUSX_ASSERT_IF(!thisParent || !otherParent) {
            throw std::logic_error("Unable to find parent staff instance for transposition instance.");
        }
        if (thisParent->transposedClef != otherParent->transposedClef) {
            return false;
        }
    }
    
    if (noSimplifyKey != other.noSimplifyKey) {
        return false;
    }

    // keysig: both present or both absent?
    if (static_cast<bool>(keysig) != static_cast<bool>(other.keysig)) {
        return false;
    }
    if (keysig && other.keysig) {
        if (keysig->interval != other.keysig->interval) {
            return false;
        }
        if (keysig->adjust != other.keysig->adjust) {
            return false;
        }
    }

    // chromatic: both present or both absent?
    if (static_cast<bool>(chromatic) != static_cast<bool>(other.chromatic)) {
        return false;
    }
    if (chromatic && other.chromatic) {
        if (chromatic->alteration != other.chromatic->alteration) {
            return false;
        }
        if (chromatic->diatonic != other.chromatic->diatonic) {
            return false;
        }
    }

    return true;
}

// **************************
// ***** StaffComposite *****
// **************************

void StaffComposite::applyStyle(const MusxInstance<StaffStyle>& staffStyle)
{
    auto srcMasks = staffStyle->masks;

    if (staffStyle->hasInstrumentAssigned()) {
        instUuid = staffStyle->instUuid;
        m_instUuidChanged = true;     // indicates that we overrode the raw staff's instUuid.
    }
    if (srcMasks->floatNoteheadFont) {
        noteFont = staffStyle->noteFont;
        useNoteFont = staffStyle->useNoteFont;
        masks->floatNoteheadFont = true;
    }
    if (srcMasks->useNoteShapes) {
        useNoteShapes = staffStyle->useNoteShapes;
        noteShapesId = staffStyle->noteShapesId;
        noteShapesFromStyle = true;
        masks->useNoteShapes = true;
    }
    if (srcMasks->flatBeams) {
        flatBeams = staffStyle->flatBeams;
        masks->flatBeams = true;
    }
    if (srcMasks->blankMeasureRest) {
        blankMeasure = staffStyle->blankMeasure;
        masks->blankMeasureRest = true;
    }
    if (srcMasks->noOptimize) {
        noOptimize = staffStyle->noOptimize;
        masks->noOptimize = true;
    }
    if (srcMasks->notationStyle) {
        notationStyle = staffStyle->notationStyle;
        capoPos = staffStyle->capoPos;
        lowestFret = staffStyle->lowestFret;
        vertTabNumOff = staffStyle->vertTabNumOff;
        showTabClefAllSys = staffStyle->showTabClefAllSys;
        useTabLetters = staffStyle->useTabLetters;
        breakTabLinesAtNotes = staffStyle->breakTabLinesAtNotes;
        hideTuplets = staffStyle->hideTuplets;
        fretInstId = staffStyle->fretInstId;
        percussionMapId = staffStyle->percussionMapId;
        masks->notationStyle = true;
    }
    if (srcMasks->defaultClef) {
        defaultClef = staffStyle->defaultClef;
        masks->defaultClef = true;
    }
    if (srcMasks->staffType) {
        staffLines = staffStyle->staffLines;
        customStaff = staffStyle->customStaff;
        lineSpace = staffStyle->lineSpace;
        topBarlineOffset = staffStyle->topBarlineOffset;
        botBarlineOffset = staffStyle->botBarlineOffset;
        dwRestOffset = staffStyle->dwRestOffset;
        wRestOffset = staffStyle->wRestOffset;
        hRestOffset = staffStyle->hRestOffset;
        otherRestOffset = staffStyle->otherRestOffset;
        botRepeatDotOff = staffStyle->botRepeatDotOff;
        topRepeatDotOff = staffStyle->topRepeatDotOff;
        stemReversal = staffStyle->stemReversal;
        hideRepeatBottomDot = staffStyle->hideRepeatBottomDot;
        hideRepeatTopDot = staffStyle->hideRepeatTopDot;
        masks->staffType = true;
    }
    if (srcMasks->transposition) {
        transposedClef = staffStyle->transposedClef;
        transposition = staffStyle->transposition;
        masks->transposition = true;
    }
    if (srcMasks->blineBreak) {
        blineBreak = staffStyle->blineBreak;
        masks->blineBreak = true;
    }
    if (srcMasks->rbarBreak) {
        rbarBreak = staffStyle->rbarBreak;
        masks->rbarBreak = true;
    }
    if (srcMasks->negMnumb) {
        hideMeasNums = staffStyle->hideMeasNums;
        masks->negMnumb = true;
    }
    if (srcMasks->negRepeat) {
        hideRepeats= staffStyle->hideRepeats;
        masks->negRepeat = true;
    }
    if (srcMasks->negNameScore) {
        hideNameInScore = staffStyle->hideNameInScore;
        masks->negNameScore = true;
    }
    if (srcMasks->hideBarlines) {
        hideBarlines = staffStyle->hideBarlines;
        masks->hideBarlines = true;
    }
    if (srcMasks->fullName) {
        fullNameTextId = staffStyle->fullNameTextId;
        masks->fullName = true;
    }
    if (srcMasks->abrvName) {
        abbrvNameTextId = staffStyle->abbrvNameTextId;
        masks->abrvName = true;
    }
    if (srcMasks->floatKeys) {
        floatKeys = staffStyle->floatKeys;
        masks->floatKeys = true;
    }
    if (srcMasks->floatTime) {
        floatTime = staffStyle->floatTime;
        masks->floatTime = true;
    }
    if (srcMasks->hideRptBars) {
        hideRptBars = staffStyle->hideRptBars;
        masks->hideRptBars = true;
    }
    if (srcMasks->fullNamePos) {
        fullNamePosId = staffStyle->fullNamePosId;
        fullNamePosFromStyle = true;
        masks->fullNamePos = true;
    }
    if (srcMasks->abrvNamePos) {
        abrvNamePosId = staffStyle->abrvNamePosId;
        abrvNamePosFromStyle = true;
        masks->abrvNamePos = true;
    }
    if (srcMasks->altNotation) {
        altNotation = staffStyle->altNotation;
        altLayer = staffStyle->altLayer;
        altHideArtics = staffStyle->altHideArtics;
        altHideLyrics = staffStyle->altHideLyrics;
        altHideSmartShapes = staffStyle->altHideSmartShapes;
        altRhythmStemsUp = staffStyle->altRhythmStemsUp;
        altSlashDots = staffStyle->altSlashDots;
        altHideOtherNotes = staffStyle->altHideOtherNotes;
        altHideOtherArtics = staffStyle->altHideOtherArtics;
        altHideExpressions = staffStyle->altHideExpressions;
        altHideOtherLyrics = staffStyle->altHideOtherLyrics;
        altHideOtherSmartShapes = staffStyle->altHideOtherSmartShapes;
        altHideOtherExpressions = staffStyle->altHideOtherExpressions;
        masks->altNotation = true;
    }
    if (srcMasks->negKey) {
        hideKeySigs = staffStyle->hideKeySigs;
        masks->negKey = true;
    }
    if (srcMasks->negTime) {
        hideTimeSigs = staffStyle->hideTimeSigs;
        masks->negTime = true;
    }
    if (srcMasks->negClef) {
        hideClefs = staffStyle->hideClefs;
        masks->negClef = true;
    }
    if (srcMasks->hideStaff) {
        hideMode = staffStyle->hideMode;
        masks->hideStaff = true;
    }
    if (srcMasks->noKey) {
        noKey = staffStyle->noKey;
        masks->noKey = true;
    }
    if (srcMasks->showTies) {
        hideTies = staffStyle->hideTies;
        masks->showTies = true;
    }
    if (srcMasks->showDots) {
        hideDots = staffStyle->hideDots;
        masks->showDots = true;
    }
    if (srcMasks->showRests) {
        hideRests = staffStyle->hideRests;
        masks->showRests = true;
    }
    if (srcMasks->showStems) {
        hideStems = staffStyle->hideStems;
        hideBeams = staffStyle->hideBeams;
        stemDirection = staffStyle->stemDirection;
        stemStartFromStaff = staffStyle->stemStartFromStaff;
        stemsFixedEnd = staffStyle->stemsFixedEnd;
        stemsFixedStart = staffStyle->stemsFixedStart;
        horzStemOffUp = staffStyle->horzStemOffUp;
        horzStemOffDown = staffStyle->horzStemOffDown;
        vertStemStartOffUp = staffStyle->vertStemStartOffUp;
        vertStemStartOffDown = staffStyle->vertStemStartOffDown;
        vertStemEndOffUp = staffStyle->vertStemEndOffUp;
        vertStemEndOffDown = staffStyle->vertStemEndOffDown;
        masks->showStems = true;
    }
    if (srcMasks->hideChords) {
        hideChords = staffStyle->hideChords;
        masks->hideChords = true;
    }
    if (srcMasks->hideFretboards) {
        hideFretboards = staffStyle->hideFretboards;
        masks->hideFretboards = true;
    }
    if (srcMasks->hideLyrics) {
        hideLyrics = staffStyle->hideLyrics;
        masks->hideLyrics = true;
    }
    if (srcMasks->showNameParts) {
        showNameInParts = staffStyle->showNameInParts;
        masks->showNameParts = true;
    }
    if (srcMasks->showNoteColors) {
        showNoteColors = staffStyle->showNoteColors;
        masks->showNoteColors = true;
    }
    if (srcMasks->hideStaffLines) {
        hideStaffLines = staffStyle->hideStaffLines;
        masks->hideStaffLines = true;
    }
    if (srcMasks->redisplayLayerAccis) {
        redisplayLayerAccis = staffStyle->redisplayLayerAccis;
        masks->redisplayLayerAccis = true;
    }
    if (srcMasks->negTimeParts) {
        hideTimeSigsInParts = staffStyle->hideTimeSigsInParts;
        masks->negTimeParts = true;
    }
    if (srcMasks->hideKeySigsShowAccis) {
        hideKeySigsShowAccis = staffStyle->hideKeySigsShowAccis;
        masks->hideKeySigsShowAccis = true;
    }
}

MusxInstance<Staff> StaffComposite::getRawStaff() const
{
    auto result = getDocument()->getOthers()->get<Staff>(getRequestedPartId(), getCmper());
    if (!result) {
        MUSX_INTEGRITY_ERROR("Unable to load staff " + std::to_string(getCmper()) + " from StaffComposite.");
    }
    return result;
}

MusxInstance<StaffComposite> StaffComposite::createCurrent(const DocumentPtr& document, Cmper partId,
    StaffCmper staffId, MeasCmper measId, Edu eduPosition)
{
    // Use getRawStaff to guarantee no copy out of the pool.
    auto rawStaff = document->getOthers()->getRawStaff(partId, staffId);
    if (!rawStaff) return nullptr;

    std::shared_ptr<StaffComposite> result(new StaffComposite(rawStaff, measId, eduPosition));
    PartContextCloner::setRequestedPartId(result, partId);
    result->createMasks(result);
    if (result->hasStyles) {
        auto styles = StaffStyle::findAllOverlappingStyles(document, partId, staffId, measId, eduPosition);
        for (const auto& style : styles) {
            result->applyStyle(style);
        }
    }

    return result;
}

// **********************
// ***** StaffStyle *****
// **********************

bool StaffStyle::containsInstrumentChange() const
{
    return masks->notationStyle || hasInstrumentAssigned();
}

MusxInstanceList<StaffStyle> StaffStyle::findAllOverlappingStyles(const DocumentPtr& document,
        Cmper partId, StaffCmper staffId, MeasCmper measId, Edu eduPosition)
{
    auto staffStyleAssignments = document->getOthers()->getArray<StaffStyleAssign>(partId, staffId);
    std::vector<MusxInstance<StaffStyleAssign>> applicableAssignments;
    std::copy_if(staffStyleAssignments.begin(), staffStyleAssignments.end(), std::back_inserter(applicableAssignments),
        [measId, eduPosition](const MusxInstance<StaffStyleAssign>& range) {
            return range->contains(measId, eduPosition);
        });

    MusxInstanceList<StaffStyle> result(document, partId);
    result.reserve(applicableAssignments.size());
    for (const auto& assign : applicableAssignments) {
        if (auto style = assign->getStaffStyle()) {
            result.emplace_back(style);
        }
    }
    return result;
 }

// ****************************
// ***** StaffStyleAssign *****
// ****************************

MusxInstance<others::StaffStyle> others::StaffStyleAssign::getStaffStyle() const
{
    auto result = getDocument()->getOthers()->get<others::StaffStyle>(getRequestedPartId(), styleId);
    if (!result) {
        MUSX_INTEGRITY_ERROR("Staff style assignment has invalid staff style ID " + std::to_string(styleId)
            + ": Part " + std::to_string(getRequestedPartId())
            + " Staff " + std::to_string(getCmper())
        );
    }
    return result;
}

} // namespace others
} // namespace dom
} // namespace musx
