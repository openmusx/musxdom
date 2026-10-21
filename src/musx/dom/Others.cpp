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
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <type_traits>

#include "musx/musx.h"
#include "musx/util/EnigmaString.h"

namespace musx {
namespace dom {
namespace others {

static std::string formatMeasureNumberText(const MeasureNumberRegion& region, int displayNumber)
{
    std::string result = region.prefix;
    if (region.base <= 1) {
        result += util::EnigmaString::toU8(region.startChar);
        result += region.suffix;
        return result;
    }

    const bool numericStyle = region.countFromOne;
    const bool negative = displayNumber < 0;
    auto absDisplayNumber = static_cast<long long>(std::llabs(static_cast<long long>(displayNumber)));
    std::size_t digitCount = 1;
    auto widthProbe = absDisplayNumber;
    while (true) {
        if (!numericStyle && widthProbe > 0 && (widthProbe % region.base) == 0) {
            widthProbe--;
        }
        widthProbe /= region.base;
        if (widthProbe == 0) {
            break;
        }
        if (region.doubleUp) {
            digitCount += static_cast<std::size_t>(widthProbe);
            break;
        }
        digitCount++;
    }

    if (negative) {
        result += '-';
    }

    std::u32string digits(digitCount, U'\0');
    for (std::size_t index = digitCount; index-- > 0;) {
        auto numericValue = absDisplayNumber;
        if (!numericStyle) {
            numericValue -= 1;
        }
        const auto digit = region.startChar + static_cast<char32_t>(numericValue % region.base);
        digits[index] = digit;
        if (region.doubleUp) {
            for (std::size_t repeatedIndex = index; repeatedIndex-- > 0;) {
                digits[repeatedIndex] = digit;
            }
            break;
        }
        if (!numericStyle && (absDisplayNumber % region.base) == 0) {
            absDisplayNumber--;
        }
        absDisplayNumber /= region.base;
    }

    result += util::EnigmaString::toU8(digits);
    result += region.suffix;
    return result;
}

// ***************************
// ***** ArticulationDef *****
// ***************************

ArticulationDef::SelectedSymbol ArticulationDef::calcSelectedSymbol(bool placeAbove) const
{
    const bool usesAlternate = placeAbove ? aboveSymbolAlt : belowSymbolAlt;

    SelectedSymbol result;
    result.usesAlternate = usesAlternate;
    result.isShape = usesAlternate ? altIsShape : mainIsShape;
    result.shapeId = usesAlternate ? altShape : mainShape;
    result.xOffset = usesAlternate ? xOffsetAlt : xOffsetMain;
    result.yOffset = usesAlternate ? yOffsetAlt : yOffsetMain;
    result.character = usesAlternate ? charAlt : charMain;
    result.font = usesAlternate ? fontAlt : fontMain;
    return result;
}

// *****************
// ***** Frame *****
// *****************

bool Frame::iterateRawEntries(std::function<bool(const MusxInstance<Entry>& entry)> iterator) const
{
    bool result = true;
    auto firstEntry = startEntry ? getDocument()->getEntries()->get(startEntry) : nullptr;
    if (firstEntry) {
        for (auto entry = firstEntry; entry; entry = entry->getNext()) {
            if (!iterator(entry)) {
                return false;
            }
            if (entry->getEntryNumber() == endEntry) {
                return true;
            }
        }
    } else {
        result = false;
        MUSX_INTEGRITY_ERROR("Frame " + std::to_string(getCmper()) + " inci " + std::to_string(getInci().value_or(-1)) + " is not iterable.");
    }
    return result;
}

MusxInstanceList<Entry> Frame::getEntries() const
{
    MusxInstanceList<Entry> retval(getDocument(), getRequestedPartId());
    iterateRawEntries([&](const MusxInstance<Entry>& entry) -> bool {
        retval.emplace_back(entry);
        return true;
    });
    return retval;
}

// **************************
// ***** FretboardGroup *****
// **************************

MusxInstance<FretInstrument> FretboardGroup::getFretInstrument() const
{
    return getDocument()->getOthers()->get<FretInstrument>(getRequestedPartId(), fretInstId);
}

MusxInstanceList<details::FretboardDiagram> FretboardGroup::getFretboardDiagrams() const
{
    MusxInstanceList<details::FretboardDiagram> result(getDocument(), getRequestedPartId());
    MUSX_ASSERT_IF(!getInci().has_value()) {
        throw std::logic_error("FretboardGroup " + std::to_string(getCmper()) + " has no inci.");
    }
    Cmper cmper2Base = Cmper(getInci().value()) * 16;
    for (Cmper cmper2Offset = 0; cmper2Offset < music_theory::STANDARD_12EDO_STEPS; cmper2Offset++) {
        if (auto nextDiagram = getDocument()->getDetails()->get<details::FretboardDiagram>(getRequestedPartId(), getCmper(), cmper2Base + cmper2Offset)) {
            result.push_back(nextDiagram);
        } else {
            result.clear();
            MUSX_INTEGRITY_ERROR("FretboardGroup " + std::to_string(getCmper()) + " inci " + std::to_string(getInci().value())
                + " is missing a fretboard diagram for pitch class " + std::to_string(cmper2Offset));
#ifndef MUSX_THROW_ON_INTEGRITY_CHECK_FAIL
            break;
#endif
        }
    }
    return result;
}

// ****************************
// ***** MarkingCategiory *****
// ****************************

std::string MarkingCategory::getName() const
{
    auto catName = getDocument()->getOthers()->get<MarkingCategoryName>(getRequestedPartId(), getCmper());
    if (catName) {
        return catName->name;
    }
    return {};
}

CategoryStaffListSet MarkingCategory::createStaffListSet() const
{
    return CategoryStaffListSet(getDocument(), getRequestedPartId(), staffList);
}

std::optional<Cmper> importMarkingCategoryInto(const DocumentPtr& target,
    const DocumentPtr& source, Cmper cmper, const ImportObjectCallback& onImported)
{
    MUSX_ASSERT_IF(!target) {
        throw std::invalid_argument("importMarkingCategoryInto received a null target document");
    }
    MUSX_ASSERT_IF(!source) {
        throw std::invalid_argument("importMarkingCategoryInto received a null source document");
    }
    const auto sourceCategory = source->getOthers()->get<MarkingCategory>(SCORE_PARTID, cmper);
    if (!sourceCategory
        || target->getOthers()->get<MarkingCategory>(SCORE_PARTID, cmper)
        || target->getOthers()->get<MarkingCategoryName>(SCORE_PARTID, cmper)) {
        return std::nullopt;
    }

    const auto importFont = [&](const std::shared_ptr<FontInfo>& sourceFont) {
        if (!sourceFont) return std::shared_ptr<FontInfo>{};
        auto result = std::make_shared<FontInfo>(target);
        *result = *sourceFont;
        const auto sourceDefinition = source->getOthers()->get<FontDefinition>(
            SCORE_PARTID, sourceFont->fontId);
        if (sourceDefinition) {
            if (const auto importedId = importFontDefinitionInto(target, sourceDefinition, onImported)) {
                result->fontId = *importedId;
            }
        }
        return result;
    };

    auto importedCategory = std::make_shared<MarkingCategory>(
        target, SCORE_PARTID, EnigmaBase::ShareMode::All, cmper);
    *importedCategory = *sourceCategory;
    importedCategory->textFont = importFont(sourceCategory->textFont);
    importedCategory->musicFont = importFont(sourceCategory->musicFont);
    importedCategory->numberFont = importFont(sourceCategory->numberFont);
    target->getOthers()->add(MarkingCategory::XmlNodeName, importedCategory);
    if (onImported) onImported(*importedCategory);

    if (const auto sourceName = source->getOthers()->get<MarkingCategoryName>(SCORE_PARTID, cmper)) {
        auto importedName = std::make_shared<MarkingCategoryName>(
            target, SCORE_PARTID, EnigmaBase::ShareMode::All, cmper);
        *importedName = *sourceName;
        target->getOthers()->add(MarkingCategoryName::XmlNodeName, importedName);
        if (onImported) onImported(*importedName);
    }
    return cmper;
}

// *******************
// ***** Measure *****
// *******************

void Measure::checkMeasureCmperSequence(const DocumentPtr& document)
{
    const auto measures = document->getOthers()->getArray<Measure>(SCORE_PARTID);
    for (size_t i = 0; i < measures.size(); i++) {
        const auto expected = Cmper(i + 1);
        if (measures[i]->getCmper() != expected) {
            MUSX_INTEGRITY_ERROR("Expected <measSpec> elements to have cmper values sequentially starting with 1. Expected "
                + std::to_string(expected) + " but found " + std::to_string(measures[i]->getCmper()) + ".");
        }
    }
}

MusxInstance<MeasureNumberRegion> Measure::findMeasureNumberRegion() const
{
    auto regions = getDocument()->getOthers()->getArray<MeasureNumberRegion>(getRequestedPartId());

    MusxInstance<MeasureNumberRegion> fallback;

    for (const auto& region : regions) {
        if (!region->calcIncludesMeasure(getCmper())) {
            continue;
        }
        // Prefer regions that show on the start of the system
        if (region->scoreData->showOnStart || region->partData->showOnStart) {
            return region;
        }
        // Keep the first region that includes the measure as a fallback
        if (!fallback) {
            fallback = region;
        }
    }

    return fallback; // may be nullptr if no region contains the measure
}

std::optional<int> Measure::calcDisplayNumber() const
{
    if (noMeasNum) {
        return std::nullopt;
    }
    if (const auto region = findMeasureNumberRegion()) {
        return region->calcDisplayNumberFor(getCmper());
    }
    return std::nullopt;
}

std::optional<std::string> Measure::calcDisplayNumberText() const
{
    if (noMeasNum) {
        return std::nullopt;
    }
    if (const auto region = findMeasureNumberRegion()) {
        return region->calcDisplayNumberTextFor(getCmper());
    }
    return std::nullopt;
}

MusxInstance<KeySignature> Measure::createKeySignature(const std::optional<StaffCmper>& forStaff) const
{
    std::shared_ptr<KeySignature> result;
    MusxInstance<Staff> staff;
    if (forStaff) {
        staff = StaffComposite::createCurrent(getDocument(), getRequestedPartId(), forStaff.value(), getCmper(), 0);
        if (staff && staff->floatKeys) {
            if (auto floats = getDocument()->getDetails()->get<details::IndependentStaffDetails>(getRequestedPartId(), forStaff.value(), getCmper())) {
                if (floats->hasKey) {
                    result = std::make_shared<KeySignature>(*floats->keySig);
                }
            }
        }
    }
    if (!result) {
        result = std::make_shared<KeySignature>(*globalKeySig);
    }
    if (result && staff) {
        result->setTransposition(staff);
    }
    return result;
}

MusxInstance<TimeSignature> Measure::createTimeSignature(const std::optional<StaffCmper>& forStaff) const
{
    if (forStaff) {
        if (auto staff = StaffComposite::createCurrent(getDocument(), getRequestedPartId(), forStaff.value(), getCmper(), 0)) {
            if (staff->floatTime) {
                if (auto floats = getDocument()->getDetails()->get<details::IndependentStaffDetails>(getRequestedPartId(), forStaff.value(), getCmper())) {
                    if (floats->hasTime) {
                        return floats->createTimeSignature();
                    }
                }
            }
        }
    }
    return MusxInstance<TimeSignature>(new TimeSignature(getDocument(), beats, divBeat, compositeNumerator, compositeDenominator));
}

MusxInstance<TimeSignature> Measure::createDisplayTimeSignature(const std::optional<StaffCmper>& forStaff) const
{
    if (forStaff) {
        if (auto staff = StaffComposite::createCurrent(getDocument(), getRequestedPartId(), forStaff.value(), getCmper(), 0)) {
            if (staff->floatTime) {
                if (auto floats = getDocument()->getDetails()->get<details::IndependentStaffDetails>(getRequestedPartId(), forStaff.value(), getCmper())) {
                    if (floats->hasTime) {
                        return floats->createDisplayTimeSignature();
                    }
                }
            }
        }
    }
    if (!useDisplayTimesig) {
        return createTimeSignature(forStaff);
    }
    return MusxInstance<TimeSignature>(new TimeSignature(getDocument(), dispBeats, dispDivbeat, compositeDispNumerator, compositeDispDenominator,
        abbrvTime ? TimeSignature::Abbreviation::Abbreviated : TimeSignature::Abbreviation::Numeric));
}

util::Fraction Measure::calcMinLegacyPickupSpacer(StaffCmper forStaffId) const
{
    const auto globalSpacer = calcMinLegacyPickupSpacer();
    if (globalSpacer == 0) {
        return 0; // save work in by far the most common case.
    }
    if (auto gfHold = details::GFrameHoldContext(getDocument(), getRequestedPartId(), forStaffId, getCmper())) {
        if (gfHold->calcContainsMusic()) {
            return gfHold.calcMinLegacyPickupSpacer();
        }
    }
    return globalSpacer / calcTimeStretch(forStaffId); // return staff-level value.
}

std::optional<Duration> Measure::calcDefaultPickupRestValue() const
{
    if (getCmper() == 1) { // only check first measure for a pickup: this is observed Finale behavior
        if (const auto miscOptions = getDocument()->getOptions()->get<options::MiscOptions>()) {
            if (miscOptions->pickupValue > 0) {
                // It may be necessary to check STUDIO_VIEW_SCROLL_VIEW_ID to see if it has a pickup spacer,
                // but for now we trust MiscOptions.
                return calcDurationInfoFromEdu(miscOptions->pickupValue);
            }
        }
    }
    return std::nullopt;
}

util::Fraction Measure::calcMinLegacyPickupSpacer() const
{
    util::Fraction result = -1;
    auto doc = getDocument();

    auto checkStaves = [&](const MusxInstanceList<others::StaffUsed>& staves) {
        for (const auto& staffUsed : staves) {
            if (auto gfHold = details::GFrameHoldContext(doc, getRequestedPartId(), staffUsed->staffId, getCmper())) {
                if (gfHold->calcContainsMusic()) {
                    // only consider gfHold if it contains music, because pickup spacers are attached to frames.
                    const auto nextValue = gfHold.calcMinLegacyPickupSpacer() * calcTimeStretch(staffUsed->staffId);
                    if (result < 0 || nextValue < result) {
                        result = nextValue;
                    }
                    if (result == 0) { // no need to keep searching if we are at zero.
                        return;
                    }
                }
            }
        }
    };
        
    checkStaves(doc->getScrollViewStaves(getRequestedPartId()));
    if (result < 0) { // only check Studio View if nothing was found in Scroll View.
        checkStaves(doc->getStudioViewStaves(getRequestedPartId()));
    }

    return (std::max)(util::Fraction(0), result);
}

util::Fraction Measure::calcDuration(const std::optional<StaffCmper>& forStaff) const
{
    auto timeSig = createTimeSignature(forStaff);
    return timeSig->calcTotalDuration();
}

Measure::Spacing Measure::calcSpacing(BeatChartJustification justification) const
{
    const auto doc = getDocument();
    const auto spacingOptions = doc->getOptions()->get<options::MusicSpacingOptions>();
    Spacing spacing;
    spacing.musicStart = frontSpaceExtra + (spacingOptions ? spacingOptions->musFront : 0);
    spacing.musicWidth = width - spacing.musicStart - backSpaceExtra - (spacingOptions ? spacingOptions->musBack : 0);
    spacing.endEdu = calcDuration().calcEduDuration();
    const Edu duration = spacing.endEdu;
    const Evpu musicWidth = spacing.musicWidth;
    const auto chart = doc->getOthers()->getArray<BeatChartElement>(getRequestedPartId(), getCmper());
    const bool usesChart = positioningMode == PositioningType::BeatChart
        || positioningMode == PositioningType::BeatChartPlusPositioning;
    if (duration == 0 || !usesChart || chart.size() < 2 || !chart.front()->control || chart.front()->control->totalDur == 0) {
        spacing.spacingSlots.push_back({0, musicWidth, 0});
        return spacing;
    }
    using Span = SpacingSlot;
    const auto& control = *chart.front()->control;
    const Evpu totalWidth = (std::max)(Evpu(1), control.totalWidth);
    const Evpu minWidth = (std::max)(Evpu(1), control.minWidth);
    const Evpu allotWidth = control.allotWidth ? control.allotWidth : totalWidth;
    const Evpu firstMin = chart[1]->minPos;
    const bool legacy = justification == BeatChartJustification::Legacy;
    const double durationRatio = double(control.totalDur) / duration;
    const size_t count = chart.size() - 1;
    auto& spans = spacing.spacingSlots;
    spans.resize(count);
    auto rounded = [](double value) { return static_cast<Evpu>(std::lround(value)); };
    auto blend = [&](Span a, Span b, double fraction) {
        return Span{a.offset + rounded((b.offset - a.offset) * fraction),
                    a.endOffset + rounded((b.endOffset - a.endOffset) * fraction)};
    };
    auto ideal = [&](size_t index, double scale) {
        return Span{firstMin + rounded(chart[index + 1]->pos * scale),
                    firstMin + rounded(chart[index + 1]->endPos * scale)};
    };

    if (musicWidth <= minWidth) {
        const double scale = double(musicWidth) / minWidth * (legacy ? durationRatio : 1.0);
        for (size_t i = 0; i < count; ++i) {
            spans[i].offset = rounded(chart[i + 1]->minPos * scale);
            if (i > 0) {
                spans[i - 1].endOffset = spans[i].offset;
            }
        }
        spans.back().endOffset = musicWidth;
    } else if (minWidth < 2 || musicWidth >= allotWidth) {
        const double scale = double(musicWidth - firstMin) / totalWidth * (legacy ? durationRatio : 1.0);
        for (size_t i = 0; i < count; ++i) {
            spans[i] = ideal(i, scale);
        }
    } else if (legacy) {
        const double scale = double(allotWidth - firstMin) / totalWidth * durationRatio;
        const double fraction = double(musicWidth - minWidth) / (allotWidth - minWidth);
        for (size_t i = 0; i < count; ++i) {
            const Span minimum{rounded(chart[i + 1]->minPos * durationRatio),
                i + 1 < count ? rounded(chart[i + 2]->minPos * durationRatio) : minWidth};
            spans[i] = blend(minimum, ideal(i, scale), fraction);
        }
    } else {
        // Natural spacing preserves both the ideal distances between starts and the minimum distances.
        // Its span ends retain any gap between an ideal span end and the following start.
        std::vector<Span> natural(count);
        Evpu naturalWidth = firstMin;
        for (size_t i = 0; i < count; ++i) {
            const auto& element = *chart[i + 1];
            const Evpu nextPos = i + 1 < count ? chart[i + 2]->pos : totalWidth;
            const Evpu nextMin = i + 1 < count ? chart[i + 2]->minPos : minWidth;
            natural[i].offset = naturalWidth;
            naturalWidth += (std::max)(nextPos - element.pos, nextMin - element.minPos);
            natural[i].endOffset = naturalWidth - nextPos + element.endPos;
        }
        const double scale = double(allotWidth - firstMin) / totalWidth;
        for (size_t i = 0; i < count; ++i) {
            if (musicWidth < naturalWidth) {
                const Span minimum{chart[i + 1]->minPos, i + 1 < count ? chart[i + 2]->minPos : minWidth};
                const double fraction = naturalWidth == minWidth ? 0
                    : double(musicWidth - minWidth) / (naturalWidth - minWidth);
                spans[i] = blend(minimum, natural[i], fraction);
            } else {
                const double fraction = allotWidth == naturalWidth ? 0
                    : double(musicWidth - naturalWidth) / (allotWidth - naturalWidth);
                spans[i] = blend(natural[i], ideal(i, scale), fraction);
            }
            if (duration != control.totalDur) {
                spans[i].offset = firstMin + rounded(double(spans[i].offset - firstMin) / durationRatio);
                spans[i].endOffset = firstMin + rounded(double(spans[i].endOffset - firstMin) / durationRatio);
            }
        }
    }
    for (size_t i = 0; i + 1 < count; ++i) {
        spans[i].endOffset = (std::min)(spans[i].endOffset, spans[i + 1].offset);
    }

    for (size_t i = 0; i < count; ++i) {
        spacing.spacingSlots[i].edu = chart[i + 1]->dur;
    }
    spacing.chartDuration = control.totalDur;
    return spacing;
}

EvpuFloat Measure::calcFirstBeatEvpu(BeatChartJustification justification) const
{
    const auto spacing = calcSpacing(justification);
    return spacing.musicStart + spacing.spacingSlots.front().offset;
}

Edu Measure::calcEduFromEvpu(Evpu evpuFromLeftEdge, BeatChartJustification justification, bool allowNegative) const
{
    const auto spacing = calcSpacing(justification);
    const Edu duration = spacing.endEdu;
    if (duration == 0) {
        return 0;
    }
    const Evpu offset = evpuFromLeftEdge - spacing.musicStart;
    if (!allowNegative && offset < 0) {
        return 0;
    }
    if (spacing.chartDuration == 0) {
        return offset <= 0 || spacing.musicWidth == 0 ? offset
            : static_cast<Edu>(std::lround(double(offset) * duration / spacing.musicWidth));
    }
    const auto& spacingSlots = spacing.spacingSlots;
    size_t index = 0;
    while (index + 1 < spacingSlots.size() && offset >= spacingSlots[index].endOffset) {
        ++index;
    }
    Edu result = spacingSlots[index].edu;
    const Evpu start = spacingSlots[index].offset;
    // Use the stored span end until it is reached, then extrapolate to the available width.
    const Evpu end = offset < spacingSlots[index].endOffset ? spacingSlots[index].endOffset : spacing.musicWidth;
    const Edu nextEdu = index + 1 < spacingSlots.size() ? spacingSlots[index + 1].edu : duration;
    if (offset < start) {
        const auto miscOptions = getDocument()->getOptions()->get<options::MiscOptions>();
        const Edu pickup = getCmper() == 1 && miscOptions ? miscOptions->pickupValue : 0;
        if (result == 0 || result == duration - pickup) {
            result = offset - start;
        }
    } else if (end != start) {
        result += static_cast<Edu>(std::lround(double(nextEdu - result) * (offset - start) / (end - start)));
    }
    if (result > 0 && spacing.chartDuration != duration) {
        result = static_cast<Edu>(std::lround(double(result) * duration / spacing.chartDuration));
    }
    return result;
}

// *****************************
// ***** MeasureExprAssign *****
// *****************************

bool MeasureExprAssign::calcIsSameDefinition(const MeasureExprAssign& src) const
{
    if (textExprId || src.textExprId) {
        return textExprId && src.textExprId && textExprId == src.textExprId;
    }
    if (shapeExprId || src.shapeExprId) {
        return shapeExprId && src.shapeExprId && shapeExprId == src.shapeExprId;
    }

    return false;
}

MusxInstance<TextExpressionDef> MeasureExprAssign::getTextExpression() const
{
    if (!textExprId) {
        return nullptr;
    }
    return getDocument()->getOthers()->get<TextExpressionDef>(getRequestedPartId(), textExprId);
}

MusxInstance<ShapeExpressionDef> MeasureExprAssign::getShapeExpression() const
{
    if (!shapeExprId) {
        return nullptr;
    }
    return getDocument()->getOthers()->get<ShapeExpressionDef>(getRequestedPartId(), shapeExprId);
}

MusxInstance<MarkingCategory> MeasureExprAssign::getMarkingCategory() const
{
    Cmper categoryId = 0;
    if (const auto textExpression = getTextExpression()) {
        categoryId = textExpression->categoryId;
    } else if (const auto shapeExpression = getShapeExpression()) {
        categoryId = shapeExpression->categoryId;
    }
    if (!categoryId) {
        return nullptr;
    }
    return getDocument()->getOthers()->get<MarkingCategory>(getRequestedPartId(), categoryId);
}

CategoryStaffListSet MeasureExprAssign::createStaffListSet() const
{
    return CategoryStaffListSet(getDocument(), getRequestedPartId(), staffList);
}

bool MeasureExprAssign::calcIsPartOfStaffListAssignment() const
{
    if (staffGroup != 0 || staffList != 0) {
        return true;
    }
    if (const auto category = getMarkingCategory()) {
        return category->usesStaffList && category->staffList != 0;
    }
    return false;
}

std::optional<HorizontalMeasExprAlign> MeasureExprAssign::calcEntryAlignmentType() const
{
    auto checkAlign = [](const auto& def) -> std::optional<HorizontalMeasExprAlign> {
        using InstanceType = std::remove_cv_t<std::remove_reference_t<decltype(def)>>;
        using ElementType = typename InstanceType::element_type;
        using Def = std::remove_const_t<ElementType>;
        static_assert(std::is_same_v<Def, TextExpressionDef> || std::is_same_v<Def, ShapeExpressionDef>,
            "Def must be an expression definition.");
        if (!def) {
            return std::nullopt; // allows direct pass-in of getTextExpression and getShapeExpression.
        }
        const HorizontalMeasExprAlign val = def->horzMeasExprAlign;
        switch(val)
        {
            case HorizontalMeasExprAlign::Manual:
            case HorizontalMeasExprAlign::LeftOfAllNoteheads:
            case HorizontalMeasExprAlign::LeftOfPrimaryNotehead:
            case HorizontalMeasExprAlign::Stem:
            case HorizontalMeasExprAlign::CenterPrimaryNotehead:
            case HorizontalMeasExprAlign::CenterAllNoteheads:
            case HorizontalMeasExprAlign::RightOfAllNoteheads:
                return val;

            default:
                break;
        }
        return std::nullopt;
    };

    if (textExprId) {
        return checkAlign(getTextExpression());
    } else if (shapeExprId) {
        return checkAlign(getShapeExpression());
    }
    return std::nullopt;
}

EntryInfoPtr MeasureExprAssign::calcAssociatedEntry() const
{
    // Finale at least sometimes uses large sentinel-like values for graceNoteIndex
    // at the end of measures that end with grace notes. kMaxExpectedGraceNoteIndex
    // allows us to treat such values as a main note (by converting them to 0) and
    // not find any trailing grace note as an associated entry.
    constexpr unsigned kMaxExpectedGraceNoteIndex = 0x3fff;

    constexpr bool findExact = true;
    if (staffAssign > 0) {
        if (!calcEntryAlignmentType()) {
            return {};
        }
        if (auto gfHold = details::GFrameHoldContext(getDocument(), getRequestedPartId(), staffAssign, getCmper())) {
            const auto matchLayer = layer ? std::make_optional(LayerIndex(layer - 1)) : std::nullopt;
            const auto matchVoice = voice2 ? MatchVoice::Voice2 : MatchVoice::Default;
            const auto position = util::Fraction::fromEdu(eduPosition);
            const auto graceIndex = this->graceNoteIndex <= kMaxExpectedGraceNoteIndex ? this->graceNoteIndex : 0u;
            if (const auto result = gfHold.calcNearestEntryAtGraceIndex(position, graceIndex, findExact, matchLayer, matchVoice)) {
                return result;
            }
            if (graceIndex > 0) {
                if (const auto mainEntry = gfHold.calcNearestEntry(position, findExact, matchLayer, matchVoice)) {
                    util::Logger::log(util::Logger::LogLevel::Info,
                        "Dangling graceNoteIndex " + std::to_string(this->graceNoteIndex) +
                        " on expression assignment in measure " + std::to_string(getCmper()) +
                        ". Falling back to the main entry.");
                    return mainEntry;
                }
            }
        }
    }
    return {};
}

bool MeasureExprAssign::calcIsAssignedInRequestedPart() const
{
    if (getRequestedPartId() != SCORE_PARTID) {
        return showStaffList != ShowStaffList::ScoreOnly;
    }
    return showStaffList != ShowStaffList::PartOnly;
}

bool MeasureExprAssign::calcAppliesToLayer(LayerIndex layerIndex) const
{
    return (layer == 0 || layer - 1 == static_cast<int>(layerIndex));
}

StaffCmper MeasureExprAssign::calcAssignedStaffId(bool forPageView) const
{
    if (staffAssign >= 0) {
        return staffAssign;
    }
    const auto systemStaves = [&]() -> std::optional<MusxInstanceList<StaffUsed>> {
        if (forPageView) {
            const auto part = getDocument()->getOthers()->get<PartDefinition>(SCORE_PARTID, getRequestedPartId());
            if (part && part->isLayoutCalculated()) {
                if (auto system = getDocument()->calcSystemFromMeasure(getRequestedPartId(), getCmper())) {
                    return getDocument()->getOthers()->getArray<StaffUsed>(getRequestedPartId(), system->getCmper());
                }
            }
        }
        return getDocument()->getScrollViewStaves(getRequestedPartId());
    }();
    switch (static_cast<StaffList::FloatingValues>(staffAssign)) {
        case StaffList::FloatingValues::TopStaff: return systemStaves->getTopStaffId();
        case StaffList::FloatingValues::BottomStaff: return systemStaves->getBottomStaffId();
        default: break;
    }
    return 0;
}

MusxInstance<StaffComposite> MeasureExprAssign::createCurrentStaff(bool forPageView) const
{
    return StaffComposite::createCurrent(getDocument(), getRequestedPartId(), calcAssignedStaffId(forPageView), getCmper(), eduPosition);
}

bool MeasureExprAssign::calcIsHiddenByAlternateNotation() const
{
    if (calcIsPartOfStaffListAssignment()) {
        return false; // assignments with staff lists are never hidden by alternate notation: observed behavior
    }
    auto staff = createCurrentStaff();
    MUSX_ASSERT_IF(!staff) {
        throw std::logic_error("Unable to create current staff for expression assignment.");
    }
    if (calcAppliesToLayer(staff->altLayer)) {
        return staff->altHideExpressions;
    } else {
        return staff->altHideOtherExpressions;
    }
}

std::optional<Evpu> MeasureExprAssign::calcBaselinePosition(bool forAbove) const
{
    std::optional<Evpu> result = std::nullopt;
    constexpr bool forPageView = true;
    const auto part = getDocument()->getOthers()->get<PartDefinition>(SCORE_PARTID, getRequestedPartId());
    if (!part || !part->isLayoutCalculated()) {
        return result;
    }
    if (const auto sys = getDocument()->calcSystemFromMeasure(getRequestedPartId(), getCmper())) {
        const StaffCmper assignedStaffId = calcAssignedStaffId(forPageView);
        if (const auto systemStaff = StaffComposite::createCurrent(getDocument(), getRequestedPartId(), assignedStaffId, sys->startMeas, 0)) {
            result = forAbove
                   ? systemStaff->calcBaselinePosition<details::BaselineExpressionsAbove>(sys->getCmper())
                   : systemStaff->calcBaselinePosition<details::BaselineExpressionsBelow>(sys->getCmper());
        } else {
            MUSX_INTEGRITY_ERROR("staff " + std::to_string(assignedStaffId) + " not found for system " + std::to_string(sys->getCmper()));
        }
    } else {
        MUSX_INTEGRITY_ERROR("system not found for measure " + std::to_string(getCmper()));
    }
    return result;
}

std::optional<Evpu> MeasureExprAssign::calcDefaultVerticalPosition() const
{
    const auto calcDefaultVerticalPositionForDef = [&](const auto& def) -> std::optional<Evpu> {
        using InstanceType = std::remove_cv_t<std::remove_reference_t<decltype(def)>>;
        using ElementType = typename InstanceType::element_type;
        using Def = std::remove_const_t<ElementType>;
        static_assert(std::is_same_v<Def, TextExpressionDef> || std::is_same_v<Def, ShapeExpressionDef>,
            "Def must be an expression definition.");
        if (!def) {
            return std::nullopt;
        }

        const auto align = def->vertMeasExprAlign;
        const Evpu entryYOffset = def->yAdjustEntry;
        const Evpu baselineYOffset = def->yAdjustBaseline;
        switch (align) {
            case VerticalMeasExprAlign::Manual:
            case VerticalMeasExprAlign::RefLine:
                return baselineYOffset;
            case VerticalMeasExprAlign::AboveStaff:
                if (const auto pos = calcBaselinePosition(/*forAbove*/ true)) {
                    return pos.value() + baselineYOffset;
                }
                return std::nullopt;
            case VerticalMeasExprAlign::BelowStaff:
                if (const auto pos = calcBaselinePosition(/*forAbove*/ false)) {
                    return pos.value() + baselineYOffset;
                }
                return std::nullopt;
            case VerticalMeasExprAlign::AboveEntry:
                if (const auto entryInfo = calcAssociatedEntry()) {
                    const auto [top, bot] = entryInfo.calcTopBottomExtent();
                    return top + entryYOffset;
                }
                return baselineYOffset; // if there is no entry, Finale treats these like RefLine
            case VerticalMeasExprAlign::BelowEntry:
                if (const auto entryInfo = calcAssociatedEntry()) {
                    const auto [top, bot] = entryInfo.calcTopBottomExtent();
                    return bot + entryYOffset;
                }
                return baselineYOffset; // if there is no entry, Finale treats these like RefLine
            case VerticalMeasExprAlign::AboveStaffOrEntry: {
                auto result = calcBaselinePosition(/*forAbove*/ true);
                if (!result) return std::nullopt;
                if (const auto entryInfo = calcAssociatedEntry()) {
                    const auto [top, bot] = entryInfo.calcTopBottomExtent();
                    if (top > result) {
                        return top + entryYOffset;
                    }
                }
                return result.value() + baselineYOffset;
            }
            case VerticalMeasExprAlign::BelowStaffOrEntry: {
                auto result = calcBaselinePosition(/*forAbove*/ false);
                if (!result) return std::nullopt;
                if (const auto entryInfo = calcAssociatedEntry()) {
                    const auto [top, bot] = entryInfo.calcTopBottomExtent();
                    if (bot < result) {
                        return bot + entryYOffset;
                    }
                }
                return result.value() + baselineYOffset;
            }
            case VerticalMeasExprAlign::TopNote:
                if (const auto entryInfo = calcAssociatedEntry()) {
                    const auto [top, bot] = entryInfo.calcTopBottomStaffPositions();
                    return static_cast<Evpu>(top * EVPU_PER_STAFF_POSITION) + entryYOffset;
                }
                return baselineYOffset; // if there is no entry, Finale treats these like RefLine
            case VerticalMeasExprAlign::BottomNote:
                if (const auto entryInfo = calcAssociatedEntry()) {
                    const auto [top, bot] = entryInfo.calcTopBottomStaffPositions();
                    return static_cast<Evpu>(bot * EVPU_PER_STAFF_POSITION) + entryYOffset;
                }
                return baselineYOffset; // if there is no entry, Finale treats these like RefLine
        }
        return std::nullopt;
    };

    if (const auto textExp = getTextExpression()) {
        return calcDefaultVerticalPositionForDef(textExp);
    }
    if (const auto shapeExp = getShapeExpression()) {
        return calcDefaultVerticalPositionForDef(shapeExp);
    }
    return std::nullopt;
}

VerticalPlacement MeasureExprAssign::calcVerticalPlacement() const
{
    const auto defaultY = calcDefaultVerticalPosition();
    if (!defaultY) {
        return VerticalPlacement::NotApplicable;
    }

    const auto staff = createCurrentStaff();
    if (!staff) {
        return VerticalPlacement::NotApplicable;
    }

    const Evpu y = *defaultY + vertEvpuOff;

    if (y >= staff->calcTopLineEvpu()) {
        return VerticalPlacement::Above;
    } else if (y < staff->calcBottomLineEvpu()) {
        return VerticalPlacement::Below;
    }

    return VerticalPlacement::Float;
}

std::optional<utils::PseudoTieShapeInfo> MeasureExprAssign::calcPseudoTieShape() const
{
    const auto shapeExp = getShapeExpression();
    if (!shapeExp || shapeExp->shapeDef == 0) {
        return std::nullopt;
    }
    const auto shape = shapeExp->getShape();
    if (!shape) {
        return std::nullopt;
    }
    auto knownType = shape->recognize();
    if (knownType == KnownShapeDefType::Unrecognized) {
        return std::nullopt;
    }
    utils::PseudoTieShapeInfo info;
    info.shape = shape;
    info.shapeType = knownType;
    return info;
}

bool MeasureExprAssign::calcIsPseudoTie(utils::PseudoTieMode mode, const EntryInfoPtr& forStartEntry) const
{
    using Align = HorizontalMeasExprAlign;
    if (!forStartEntry || forStartEntry.calcDisplaysAsRest()) {
        return false;
    }
    const auto entry = forStartEntry->getEntry();
    if (!entry || entry->notes.empty()) {
        return false;
    }
    auto alignmentType = calcEntryAlignmentType();
    if (!alignmentType || alignmentType == Align::LeftOfAllNoteheads || alignmentType == Align::Stem) {
        return false;
    }
    auto shapeInfo = calcPseudoTieShape();
    if (!shapeInfo || (shapeInfo->shapeType != KnownShapeDefType::SlurTieCurveRight
        && shapeInfo->shapeType != KnownShapeDefType::SlurTieCurveLeft)) {
        return false;
    }
    if (!forStartEntry.isSameEntry(calcAssociatedEntry())) {
        return false;
    }

    if (mode == utils::PseudoTieMode::TieEnd && shapeInfo->shapeType == KnownShapeDefType::SlurTieCurveLeft) {
        return true;
    }
    if (mode == utils::PseudoTieMode::LaissezVibrer && shapeInfo->shapeType != KnownShapeDefType::SlurTieCurveRight) {
        return false;
    }
    if (mode == utils::PseudoTieMode::TieEnd && shapeInfo->shapeType != KnownShapeDefType::SlurTieCurveRight) {
        return false;
    }
    if (alignmentType == Align::RightOfAllNoteheads) {
        return mode == utils::PseudoTieMode::LaissezVibrer;
    }
    const auto startOffset = horzEvpuOff;
    const auto endOffset = startOffset + shapeInfo->calcWidthOffset();
    switch (mode) {
    case utils::PseudoTieMode::LaissezVibrer:
        return utils::calcIsPseudoForwardTie(startOffset, endOffset);
    case utils::PseudoTieMode::TieEnd:
        return utils::calcIsPseudoBackwardTie(startOffset, endOffset);
    }
    return false;
}

static std::string rehearsalMarkText(RehearsalMarkStyle style, int sequence)
{
    if (sequence <= 0) {
        return {};
    }

    auto repeatedLetter = [sequence](char base) -> std::string {
        const int index = sequence - 1;
        const char letter = static_cast<char>(base + (index % 26));
        const int repeat = index / 26 + 1;
        return std::string(repeat, letter);
    };

    auto letterNumber = [sequence](char base) -> std::string {
            if (sequence <= 26) {
            return std::string(1, static_cast<char>(base + sequence - 1));
        }

        const int index = sequence - 27;
        return std::string(1, static_cast<char>(base + (index % 26)))
            + std::to_string(index / 26 + 1);
    };

    switch (style) {
    case RehearsalMarkStyle::None:
        return {};

    case RehearsalMarkStyle::Letters:
        return repeatedLetter('A');

    case RehearsalMarkStyle::LetterNumbers:
        return letterNumber('A');

    case RehearsalMarkStyle::LettersLowerCase:
        return repeatedLetter('a');

    case RehearsalMarkStyle::LettersNumbersLowerCase:
        return letterNumber('a');

    case RehearsalMarkStyle::Numbers:
        return std::to_string(sequence);

    case RehearsalMarkStyle::MeasureNumber:
        assert(false && "rehearsalMarkText called for a measure number rehearsal mark");
        break;
    }

    return {};
}

util::EnigmaParsingContext MeasureExprAssign::getRawTextCtx(Cmper forPartId) const
{
    if (const auto def = getTextExpression()) {
        const auto document = getDocument();
        const auto measureId = getCmper();
        return def->getRawTextCtx(forPartId, [document, measureId, def, forPartId](const std::vector<std::string>& components) -> std::optional<std::string> {
            if (components[0] == "rehearsal") {
                if (def->rehearsalMarkStyle == others::RehearsalMarkStyle::MeasureNumber) {
                    if (const auto measure = document->getOthers()->get<Measure>(forPartId, measureId)) {
                        return measure->calcDisplayNumberText().value_or(std::to_string(measure->getCmper()));
                    }
                    return std::to_string(measureId);
                }
                if (const auto markInfo = document->getRehearsalMarkInfo(measureId, def->getCmper())) {
                    return rehearsalMarkText(def->rehearsalMarkStyle, markInfo->rehearsalSequence);
                }
                return std::string{};
            }
            return std::nullopt;
        });
    }
    return {};
}

util::EnigmaParsingContext TextExpressionDef::getRawTextCtx(Cmper forPartId,
    util::EnigmaString::TextInsertCallback defaultInsertFunc) const
{
    if (auto textBlock = getTextBlock()) {
        const auto exprValue = value;
        const auto exprAuxData1 = auxData1;
        const auto exprPlayPass = playPass;
        return textBlock->getRawTextCtx(forPartId, std::nullopt, [defaultInsertFunc, exprValue, exprAuxData1, exprPlayPass](const std::vector<std::string>& components) -> std::optional<std::string> {
            if (auto result = defaultInsertFunc(components)) {
                return result;
            }
            if (components[0] == "value") {
                return std::to_string(exprValue);
            } else if (components[0] == "control") {
                return std::to_string(exprAuxData1);
            } else if (components[0] == "pass") {
                return std::to_string(exprPlayPass);
            }
            return std::nullopt;
        });
    }
    return {};
}

// *******************************
// ***** MeasureNumberRegion *****
// *******************************

std::optional<int> MeasureNumberRegion::calcDisplayNumberFor(MeasCmper measureId) const
{
    MUSX_ASSERT_IF(!calcIncludesMeasure(measureId)) {
        throw std::logic_error("Measure id " + std::to_string(measureId) + " is not contained in measure number region " + std::to_string(getCmper()));
    }
    int result = int(measureId) - int(startMeas) + getStartNumber();
    for (MeasCmper nextMeasId = startMeas; nextMeasId <= measureId; nextMeasId++) {
        if (auto measure = getDocument()->getOthers()->get<Measure>(getRequestedPartId(), nextMeasId)) {
            if (measure->noMeasNum) {
                if (measure->getCmper() == measureId) {
                    return std::nullopt;
                }
                result--;
            }
        }
    }
    return result;
}

std::optional<std::string> MeasureNumberRegion::calcDisplayNumberTextFor(MeasCmper measureId) const
{
    const auto displayNumber = calcDisplayNumberFor(measureId);
    if (!displayNumber) {
        return std::nullopt;
    }
    return formatMeasureNumberText(*this, *displayNumber);
}

std::optional<MeasCmper> MeasureNumberRegion::calcFirstDisplayedMeasureId() const
{
    for (MeasCmper startMeasId = startMeas; startMeasId < endMeas; startMeasId++) {
        if (auto measure = getDocument()->getOthers()->get<Measure>(getRequestedPartId(), startMeasId)) {
            if (!measure->noMeasNum) {
                return startMeasId;
            }
        } else {
            break;
        }
    }
    return std::nullopt;
}

std::optional<MeasCmper> MeasureNumberRegion::calcLastDisplayedMeasureId() const
{
    for (MeasCmper endMeasId = endMeas - 1; endMeasId >= startMeas; endMeasId--) {
        if (auto measure = getDocument()->getOthers()->get<Measure>(getRequestedPartId(), endMeasId)) {
            if (!measure->noMeasNum) {
                return endMeasId;
            }
        } else {
            break;
        }
    }
    return std::nullopt;
}

std::optional<int> MeasureNumberRegion::calcFirstDisplayNumber() const
{
    if (const auto startMeasId = calcFirstDisplayedMeasureId()) {
        return calcDisplayNumberFor(startMeasId.value());
    }
    return std::nullopt;
}

std::optional<int> MeasureNumberRegion::calcLastDisplayNumber() const
{
    if (const auto endMeasId = calcLastDisplayedMeasureId()) {
        return calcDisplayNumberFor(endMeasId.value());
    }
    return std::nullopt;
}

// ****************************
// ***** MultimeasureRest *****
// ****************************

bool MultimeasureRest::calcUsesSymbols() const
{
    return useSymbols && calcNumberOfMeasures() < symbolThreshold;
}

// *************************************
// ***** MultiStaffInstrumentGroup *****
// *************************************

MusxInstance<Staff> MultiStaffInstrumentGroup::getStaffInstanceAtIndex(size_t x) const
{
    if (x >= staffNums.size()) return nullptr;
    auto retval = getDocument()->getOthers()->get<Staff>(getRequestedPartId(), staffNums[x]);
    if (!retval) {
        MUSX_INTEGRITY_ERROR("Staff " + std::to_string(staffNums[x])
            + " not found for multiple staff instrument " + std::to_string(getCmper()));
    }
    return retval;
}

MusxInstance<Staff> MultiStaffInstrumentGroup::getFirstStaffInstance() const
{
    MusxInstance<Staff> result;
    if (staffNums.empty()) {
        MUSX_INTEGRITY_ERROR("MultiStaffInstrumentGroup " + std::to_string(getCmper()) + " contains no staves.");
    } else {
        result = getStaffInstanceAtIndex(0);
    }
    return result;
}

MusxInstance<details::StaffGroup> MultiStaffInstrumentGroup::getStaffGroup(Cmper forPartId) const
{
    auto document = getDocument();
    auto groupIdRecord = document->getOthers()->get<MultiStaffGroupId>(forPartId, getCmper());
    if (!groupIdRecord || groupIdRecord->staffGroupId == 0) {
        // staffGroupId can be 0 in upgraded files where there is another StaffGroup showing the instrument name.
        return nullptr;
    }
    auto result = document->getDetails()->get<details::StaffGroup>(forPartId, document->calcScrollViewCmper(forPartId), groupIdRecord->staffGroupId);
    if (!result) {
        MUSX_INTEGRITY_ERROR("StaffGroup " + std::to_string(groupIdRecord->staffGroupId)
            + " not found for MultiStaffInstrumentGroup " + std::to_string(getCmper()));
    }
    return result;
}

void MultiStaffInstrumentGroup::calcAllMultiStaffGroupIds(const DocumentPtr& document)
{
    auto instGroups = document->getOthers()->getArray<MultiStaffInstrumentGroup>(SCORE_PARTID);
    for (const auto& instance : instGroups) {
        for (size_t x = 0; x < instance->staffNums.size(); x++) {
            auto staff = instance->getStaffInstanceAtIndex(x);
            if (staff) {
                if (staff->multiStaffInstId != instance->getCmper()) {
                    if (staff->multiStaffInstId) {
                        musx::util::Logger::log(musx::util::Logger::LogLevel::Verbose,
                            "Staff " + std::to_string(staff->getCmper()) + " (" + staff->getFullName()
                                + ") appears in more than one instance of MultiStaffInstrumentGroup.");
                    } else {
                        Staff* staffMutable = const_cast<Staff*>(staff.get());
                        staffMutable->multiStaffInstId = instance->getCmper();
                    }
                }
            }
        }
    }
    // guarantee calcAllAutoNumberValues is called after calcAllMultiStaffGroupIds.
    Staff::calcAllAutoNumberValues(document);
}

// ****************
// ***** Page *****
// ****************

void Page::calcSystemInfo(const DocumentPtr& document)
{
    const auto measures = document->getOthers()->getArray<Measure>(SCORE_PARTID);
    const MeasCmper lastMeasureOfPiece = measures.empty() ? 0 : measures.back()->getCmper();
    auto linkedParts = document->getOthers()->getArray<PartDefinition>(SCORE_PARTID);
    for (const auto& part : linkedParts) {
        auto pages = document->getOthers()->getArray<Page>(part->getCmper());
        auto systems = document->getOthers()->getArray<StaffSystem>(part->getCmper());
        // Finale can save an uncalculated or stale layout for any part, though it is much more common
        // for linked parts, and recalculates it on open. Each instance is verbose; the score's state
        // is reported once below.
        bool layoutUncalculated = false;
        const auto reportUncalculatedLayout = [&](const std::string& message) {
            util::Logger::log(util::Logger::LogLevel::Verbose, message);
            layoutUncalculated = true;
        };
        for (const auto& system : systems) {
            StaffSystem* mutableSystem = const_cast<StaffSystem*>(system.get());
            mutableSystem->pageId = 0; // initialize
        }
        for (size_t x = 0; x < pages.size(); x++) {
            auto page = pages[x];
            Page* mutablePage = const_cast<Page*>(page.get());
            mutablePage->lastSystemId = std::nullopt;
            mutablePage->firstMeasureId = std::nullopt;
            mutablePage->lastMeasureId = std::nullopt;
            if (page->isBlank()) {
                continue;
            }
            if (page->firstSystemId <= 0) {
                reportUncalculatedLayout("Layout for page " + std::to_string(page->getCmper())
                    + " of part " + std::to_string(part->getCmper()) + " has not been calculated.");
                continue;
            }

            std::optional<SystemCmper> lastSystemId;
            bool foundFollowingNonBlankPage = false;
            size_t nextIndex = x + 1;
            while (nextIndex < pages.size()) {
                const auto& nextPage = pages[nextIndex++];
                if (!nextPage->isBlank()) {
                    foundFollowingNonBlankPage = true;
                    if (nextPage->firstSystemId > 0) {
                        lastSystemId = SystemCmper(nextPage->firstSystemId - 1);
                    }
                    break;
                }
            }
            if (!lastSystemId && !foundFollowingNonBlankPage) {
                if (!systems.empty()) {
                    lastSystemId = systems.back()->getCmper();
                }
            }
            if (!lastSystemId || lastSystemId.value() < page->firstSystemId) {
                reportUncalculatedLayout("The systems on page " + std::to_string(page->getCmper()) + " of part " + std::to_string(part->getCmper())
                    + " cannot be determined.");
                continue;
            }

            std::vector<MusxInstance<StaffSystem>> pageSystems;
            bool hasInvalidSystem = false;
            std::optional<SystemCmper> firstSystemBeyondPiece;
            for (SystemCmper systemId = page->firstSystemId; systemId <= lastSystemId.value(); ++systemId) {
                auto system = document->getOthers()->get<StaffSystem>(part->getCmper(), systemId);
                if (!system) {
                    reportUncalculatedLayout("Page " + std::to_string(page->getCmper()) + " of part " + std::to_string(part->getCmper())
                        + " has no system instance for system " + std::to_string(systemId) + ".");
                    hasInvalidSystem = true;
                    break;
                }
                if (system->startMeas == 0 || system->endMeas == 0) {
                    reportUncalculatedLayout("Layout for system " + std::to_string(systemId) + " of part "
                        + std::to_string(part->getCmper()) + " has not been calculated.");
                    hasInvalidSystem = true;
                    break;
                }
                // A system that holds only the first part of a split measure stores that measure as both its start
                // and its end, since the end is the first measure of the next system.
                const bool holdsOnlySplitStart = system->endMeas == system->startMeas
                    && document->getOthers()->get<SplitMeasure>(part->getCmper(), system->startMeas);
                if (system->endMeas < system->startMeas || (system->endMeas == system->startMeas && !holdsOnlySplitStart)) {
                    reportUncalculatedLayout("Page " + std::to_string(page->getCmper()) + " of part " + std::to_string(part->getCmper())
                        + " has an invalid measure range for system " + std::to_string(systemId) + ".");
                    hasInvalidSystem = true;
                    break;
                }
                // A layout saved before measures were deleted keeps systems for the missing measures. The layout
                // stops at the first of them, and the rest of the systems belong to no page.
                if (lastMeasureOfPiece && system->startMeas > lastMeasureOfPiece) {
                    firstSystemBeyondPiece = systemId;
                    break;
                }
                pageSystems.emplace_back(std::move(system));
            }
            if (hasInvalidSystem) {
                continue;
            }
            if (firstSystemBeyondPiece) {
                util::Logger::log(util::Logger::LogLevel::Verbose, "System " + std::to_string(*firstSystemBeyondPiece)
                    + " of part " + std::to_string(part->getCmper()) + " starts after the last measure ("
                    + std::to_string(lastMeasureOfPiece) + "). It and the systems after it are ignored.");
                if (pageSystems.empty()) {
                    reportUncalculatedLayout("Page " + std::to_string(page->getCmper()) + " of part "
                        + std::to_string(part->getCmper()) + " has no systems within the piece.");
                    continue;
                }
                lastSystemId = pageSystems.back()->getCmper();
            }

            mutablePage->lastSystemId = lastSystemId;
            mutablePage->firstMeasureId = pageSystems.front()->startMeas;
            mutablePage->lastMeasureId = pageSystems.back()->getLastMeasure();
            for (const auto& system : pageSystems) {
                StaffSystem* mutableSystem = const_cast<StaffSystem*>(system.get());
                mutableSystem->pageId = PageCmper(page->getCmper());
            }
        }
        if (layoutUncalculated && part->isScore()) {
            util::Logger::log(util::Logger::LogLevel::Info, "The score's layout has not been fully calculated.");
        }
    }
}

// **************************
// ***** PageTextAssign *****
// **************************

MusxInstance<TextBlock> PageTextAssign::getTextBlock() const
{
    return getDocument()->getOthers()->get<TextBlock>(getRequestedPartId(), block);
}

util::EnigmaParsingContext PageTextAssign::getRawTextCtx(Cmper forPartId, std::optional<Cmper> forPageId) const
{
    if (auto textBlock = getTextBlock()) {
        if (getCmper() > 0) {
            forPageId = calcStartPageNumber(forPartId);
        }
        return textBlock->getRawTextCtx(forPartId, forPageId);
    }
    return {};
}

MusxInstance<PageTextAssign> PageTextAssign::getForPageId(const DocumentPtr& document, Cmper partId, PageCmper pageId, Inci inci)
{
    if (auto part = document->getOthers()->get<PartDefinition>(SCORE_PARTID, partId)) {
        const PageCmper pageAssignmentId = part->calcAssignmentIdFromPageNumber(pageId);
        return document->getOthers()->get<PageTextAssign>(partId, pageAssignmentId, inci);
    }
    return nullptr;
}

MusxInstanceList<PageTextAssign> PageTextAssign::getArrayForPageId(const DocumentPtr& document, Cmper partId, PageCmper pageId)
{
    if (auto part = document->getOthers()->get<PartDefinition>(SCORE_PARTID, partId)) {
        const PageCmper pageAssignmentId = part->calcAssignmentIdFromPageNumber(pageId);
        return document->getOthers()->getArray<PageTextAssign>(partId, pageAssignmentId);
    }
    return MusxInstanceList<PageTextAssign>(document, partId);
}

std::optional<PageCmper> PageTextAssign::calcStartPageNumber(Cmper forPartId) const
{
    if (auto part = getDocument()->getOthers()->get<PartDefinition>(SCORE_PARTID, forPartId)) {
        if (auto calcValue = part->calcPageNumberFromAssignmentId(getCmper() ? getCmper() : startPage)) {
            if (calcValue.value() <= part->numberOfPages) {
                return calcValue;
            }
        } else if (isMultiPage()) {
            if (auto endPageNum = calcEndPageNumber(forPartId)) {
                if (part->numberOfLeadingBlankPages < endPageNum.value()) {
                    return PageCmper(part->numberOfLeadingBlankPages + 1);
                }
            }
        }
    }
    return std::nullopt;
}

std::optional<PageCmper> PageTextAssign::calcEndPageNumber(Cmper forPartId) const
{
    if (auto part = getDocument()->getOthers()->get<PartDefinition>(SCORE_PARTID, forPartId)) {
        if (isMultiAssignedThruLastPage()) {
            return PageCmper(part->numberOfPages);
        }
        if (auto calcValue = part->calcPageNumberFromAssignmentId(getCmper() ? getCmper() : endPage)) {
            if (calcValue.value() > part->numberOfPages) {
                calcValue = PageCmper(part->numberOfPages);
            }
            return calcValue;
        }
    }
    return std::nullopt;
}

// **************************
// ***** PartDefinition *****
// **************************

util::EnigmaParsingContext PartDefinition::getNameRawTextCtx() const
{
    /// @todo perhaps additional logic as in getName, but not until something is broken.
    if (nameId) {
        if (auto textBlock = getDocument()->getOthers()->get<TextBlock>(getRequestedPartId(), nameId)) {
            return textBlock->getRawTextCtx(getCmper());
        }
    }
    return {};
}

std::string PartDefinition::getName(util::EnigmaString::AccidentalStyle accidentalStyle) const
{
    if (auto nameRawText = getNameRawTextCtx()) {
        // Although the Finale U.I. prevents ^partname inserts in partname enigma strings, one might have crept in.
        std::unordered_set<std::string_view> ignoreTags = { "partname" }; // do not parse ^partname inserts
        return nameRawText.getText(true, accidentalStyle, ignoreTags);
    }
    if (defaultNameStaff) {
        if (auto staff = getDocument()->getOthers()->get<Staff>(SCORE_PARTID, defaultNameStaff)) {
            return staff->getFullInstrumentName(accidentalStyle, true); // true: prefer staff name
        } else {
            MUSX_INTEGRITY_ERROR("Part " + std::to_string(getCmper()) + " uses nonexistent Staff " + std::to_string(defaultNameStaff) + " for part name.");
        }
    }
    if (defaultNameGroup) {
        if (auto group = getDocument()->getDetails()->get<details::StaffGroup>(SCORE_PARTID, getDocument()->calcScrollViewCmper(SCORE_PARTID), defaultNameGroup)) {
            return group->getFullInstrumentName(accidentalStyle);
        } else {
            MUSX_INTEGRITY_ERROR("Part " + std::to_string(getCmper()) + " uses nonexistent StaffGroup " + std::to_string(defaultNameGroup) + " for part name.");
        }
    }
    return {};
}

bool PartDefinition::isLayoutCalculated() const
{
    const auto pages = getDocument()->getOthers()->getArray<Page>(getCmper());
    return !pages.empty() && std::all_of(pages.begin(), pages.end(), [](const auto& page) {
        return page->isLayoutCalculated();
    });
}

MusxInstance<PartDefinition> PartDefinition::getScore(const DocumentPtr& document)
{
    MusxInstance<PartDefinition> result{};
    if (auto score = document->getOthers()->get<PartDefinition>(SCORE_PARTID, SCORE_PARTID)) {
        result = score;
    } else {
        MUSX_INTEGRITY_ERROR("The document contains no instance of PartDefinition for the score.");
    }
    return result;
}

MusxInstanceList<PartDefinition> PartDefinition::getInUserOrder(const DocumentPtr& document)
{
    auto result = document->getOthers()->getArray<PartDefinition>(SCORE_PARTID);
    std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
        return lhs->partOrder < rhs->partOrder;
    });
    return result;
}

Cmper PartDefinition::calcScrollViewCmper() const
{
    return getDocument()->calcScrollViewCmper(getCmper());
}

std::optional<PageCmper> PartDefinition::calcPageNumberFromAssignmentId(PageCmper pageAssignmentId) const
{
    MUSX_ASSERT_IF(pageAssignmentId < 0) {
        throw std::logic_error("Attempt to convert negative page assignment " + std::to_string(pageAssignmentId) + " to a page number.");
    }
    std::optional<PageCmper> result = pageAssignmentId;
    if (result.value() > numberOfLeadingBlankPages) {
        const int calcValue = int(result.value()) - getDocument()->getMaxBlankPages() + numberOfLeadingBlankPages;
        if (calcValue > numberOfLeadingBlankPages) {
            result = PageCmper(calcValue);
        } else {
            return std::nullopt;
        }
    }
    return result;
}

PageCmper PartDefinition::calcAssignmentIdFromPageNumber(PageCmper pageId) const
{
    if (pageId != 0) {
        if (pageId > numberOfLeadingBlankPages) {
            const int calcValue = int(pageId) + getDocument()->getMaxBlankPages() - numberOfLeadingBlankPages;
            return PageCmper(calcValue);
        }
    }
    return pageId;
}

// ***********************
// ***** PartGlobals *****
// ***********************

Cmper PartGlobals::calcScrollViewCmper() const
{
    if (specialPartExtractionIUList) {
        return specialPartExtractionIUList;
    }
    return BASE_SYSTEM_ID;
}

// ***********************
// ***** PartVoicing *****
// ***********************

bool PartVoicing::calcShowsLayer(LayerIndex layerIndex, bool frameIsMultilayerInScore) const
{
    if (!enabled) {
        return true;
    }
    if (voicingType == VoicingType::UseSingleLayer) {
        return layerIndex == singleLayer;
    }
    return !frameIsMultilayerInScore || layerIndex == multiLayer;
}

bool PartVoicing::calcShowsNote(const NoteInfoPtr& noteInfoPtr) const
{
    if (!enabled || !noteInfoPtr) {
        return true;
    }

    const auto& entryInfoPtr = noteInfoPtr.getEntryInfo();
    const auto& frame = entryInfoPtr.getFrame();
    const auto layerIndex = frame->getLayerIndex();
    if (voicingType == VoicingType::UseSingleLayer) {
        return layerIndex == singleLayer;
    }

    const bool isMultiLayer = frame->getContext()->calcIsMultiLayer();
    if (isMultiLayer) {
        return layerIndex == multiLayer;
    }

    const size_t noteCount = entryInfoPtr->getEntry()->notes.size();
    const size_t noteIndex = noteInfoPtr.getNoteIndex();

    if (selectSingleNote && noteCount == 1) {
        return true;
    }

    // Notes are ordered lowest -> highest.
    const size_t rankFromBottom = noteIndex;                 // 0 = bottom note
    const size_t rankFromTop = (noteCount - 1) - noteIndex;  // 0 = top note
    const size_t rank = selectFromBottom ? rankFromBottom : rankFromTop;

    switch (singleLayerVoiceType) {
    case SingleLayerVoiceType::AllNotes:
        return true;

    case SingleLayerVoiceType::TopNote:
        return noteIndex + 1 == noteCount; // highest

    case SingleLayerVoiceType::BottomNote:
        return noteIndex == 0; // lowest

    case SingleLayerVoiceType::SelectedNotes:
        if (rank == 0) return select1st;
        if (rank == 1) return select2nd;
        if (rank == 2) return select3rd;
        if (rank == 3) return select4th;
        if (rank == 4) return select5th;
        return false;
    }

    return true; // fail open
}

// ******************************
// ***** PercussionNoteInfo *****
// ******************************

const percussion::PercussionNoteType& PercussionNoteInfo::getNoteType() const
{
    return percussion::getPercussionNoteTypeFromId(getBaseNoteTypeId());
}

// **********************
// ***** RepeatBack *****
// **********************

template<typename T>
static MusxInstance<T> getIndividualPositioningImpl(const MusxInstanceList<T> indivPos, StaffCmper staffId, std::optional<MeasCmper> measureId = std::nullopt)
{
    static_assert(std::is_base_of<others::RepeatIndividualPositioning, T>::value, "T must derive from RepeatIndividualPositioning");

    const auto it = std::find_if(indivPos.begin(), indivPos.end(),
        [&](const auto& ptr)
        {
            MUSX_ASSERT_IF(!ptr) {
                throw std::logic_error("Individual position array should not contain nulls!");
            }
            if (ptr->staffId != staffId) {
                return false;
            }
            return !measureId || ptr->measureId == measureId.value();
        });
    return (it != indivPos.end()) ? *it : nullptr;
}

MusxInstance<RepeatIndividualPositioning> RepeatBack::getIndividualPositioning(StaffCmper staffId) const
{
    return getIndividualPositioningImpl(getDocument()->getOthers()->getArray<RepeatBackIndividualPositioning>(getRequestedPartId(), getCmper()), staffId);
}

RepeatStaffListSet RepeatBack::createStaffListSet() const
{
    return RepeatStaffListSet(getDocument(), getRequestedPartId(), staffList);
}

std::optional<MeasCmper> RepeatBack::calcTargetMeasure() const
{
    switch (jumpAction) {
    case RepeatActionType::JumpAbsolute:
        return (targetValue > 0) ? std::optional<MeasCmper>(MeasCmper(targetValue)) : std::nullopt;

    case RepeatActionType::JumpRelative: {
        const auto target = static_cast<MeasCmper>(getCmper() + targetValue);
        return (target > 0) ? std::optional<MeasCmper>(target) : std::nullopt;
    }

    case RepeatActionType::JumpAuto: {
        for (MeasCmper meas = getCmper(); meas > 1; --meas) {
            if (auto measure = getDocument()->getOthers()->get<Measure>(getRequestedPartId(), meas - 1)) {
                if (measure->forwardRepeatBar) {
                    return MeasCmper(meas - 1);
                }
            } else {
                break;
            }
        }
        return MeasCmper{1};
    }

    default:
        return std::nullopt;
    }
}

// *****************************
// ***** RepeatEndingStart *****
// *****************************

int RepeatEndingStart::calcEndingLength() const
{
    int maxLength = (std::numeric_limits<int>::max)();

    switch (jumpAction) {
        case RepeatActionType::JumpAuto:
            break;
        case RepeatActionType::JumpAbsolute:
            maxLength = targetValue - getCmper();
            break;
        case RepeatActionType::JumpRelative:
            maxLength = targetValue;
            break;
        default:
            return 1;
    }
    if (maxLength <= 0) {
        return 1;
    }
    Cmper x = getCmper() + 1;
    while (true) {
        auto measure = getDocument()->getOthers()->get<Measure>(getRequestedPartId(), x);
        if (!measure) {
            return 1;
        }
        if (measure->hasEnding && getDocument()->getOthers()->get<RepeatEndingStart>(getRequestedPartId(), x)) {
            break;
        }
        if (--maxLength <= 0) {
            break;
        }
        x++;
    }
    return x - getCmper();
}

std::optional<MeasCmper> RepeatEndingStart::calcTargetMeasure() const
{
    switch (jumpAction) {
    case RepeatActionType::JumpAbsolute:
        return (targetValue > 0) ? std::optional<MeasCmper>(MeasCmper(targetValue)) : std::nullopt;

    case RepeatActionType::JumpRelative: {
        const auto target = static_cast<MeasCmper>(getCmper() + targetValue);
        return (target > 0) ? std::optional<MeasCmper>(target) : std::nullopt;
    }

    case RepeatActionType::JumpAuto: {
        const auto target = static_cast<MeasCmper>(getCmper() + calcEndingLength());
        return (target > 0) ? std::optional<MeasCmper>(target) : std::nullopt;
    }

    case RepeatActionType::JumpToMark:
    case RepeatActionType::Stop:
    case RepeatActionType::NoJump:
    default:
        return std::nullopt;
    }
}

bool RepeatEndingStart::calcIsOpen() const
{
    if (endLineVPos < 0) {
        return false;
    }
    if (jumpAction == RepeatActionType::NoJump) {
        return true;
    }
    for (Cmper x = getCmper(); true; x++) {
        auto measure = getDocument()->getOthers()->get<Measure>(getRequestedPartId(), x);
        if (!measure) {
            break;
        }
        if (measure->backwardsRepeatBar) {
            if (auto backRepeat = getDocument()->getOthers()->get<RepeatBack>(getRequestedPartId(), x)) {
                if (auto repeatOptions = getDocument()->getOptions()->get<options::RepeatOptions>()) {
                    return (backRepeat->leftVPos - backRepeat->rightVPos) == repeatOptions->bracketHookLen;
                }
                return true;
            }
        }
    }
    return false;
}

MusxInstance<RepeatIndividualPositioning> RepeatEndingStart::getIndividualPositioning(StaffCmper staffId) const
{
    return getIndividualPositioningImpl(getDocument()->getOthers()->getArray<RepeatEndingStartIndividualPositioning>(getRequestedPartId(), getCmper()), staffId);
}

MusxInstance<RepeatIndividualPositioning> RepeatEndingStart::getTextIndividualPositioning(StaffCmper staffId) const
{
    return getIndividualPositioningImpl(getDocument()->getOthers()->getArray<RepeatEndingTextIndividualPositioning>(getRequestedPartId(), getCmper()), staffId);
}

std::string RepeatEndingStart::createEndingText() const
{
    if (auto userText = getDocument()->getOthers()->get<RepeatEndingText>(getRequestedPartId(), getCmper())) {
        return userText->text;
    }
    std::string result;
    if (auto passList = getDocument()->getOthers()->get<RepeatPassList>(getRequestedPartId(), getCmper())) {
        for (int pass : passList->values) {
            if (!result.empty()) {
                result += ',';
                result += ' ';
            }
            result += std::to_string(pass);
        }
        if (auto repeatOptions = getDocument()->getOptions()->get<options::RepeatOptions>()) {
            if (repeatOptions->addPeriod) {
                result += '.';
            }
        }
    }
    return result;
}

RepeatStaffListSet RepeatEndingStart::createStaffListSet() const
{
    return RepeatStaffListSet(getDocument(), getRequestedPartId(), staffList);
}

// ******************************
// ***** ShapeExpressionDef *****
// ******************************

MusxInstance<ShapeDef> ShapeExpressionDef::getShape() const
{
    return getDocument()->getOthers()->get<others::ShapeDef>(getRequestedPartId(), shapeDef);
}

// ************************
// ***** StaffListSet *****
// ************************

template <class ScoreList, class PartsList, class ScoreForcedList, class PartsForcedList>
StaffListSet<ScoreList, PartsList, ScoreForcedList, PartsForcedList>::StaffListSet(
    const DocumentPtr& document, Cmper partId, Cmper staffListId) noexcept
{
    if (partId == SCORE_PARTID) {
        m_staffList = document->getOthers()->get<ScoreList>(partId, staffListId);
        if constexpr (!std::is_same_v<ScoreForcedList, void>) {
            m_forcedStaffList = document->getOthers()->get<ScoreForcedList>(partId, staffListId);
        }
    } else {
        m_staffList = document->getOthers()->get<PartsList>(partId, staffListId);
        if constexpr (!std::is_same_v<PartsForcedList, void>) {
            m_forcedStaffList = document->getOthers()->get<PartsForcedList>(partId, staffListId);
        }
    }
}

template <class ScoreList, class PartsList, class ScoreForcedList, class PartsForcedList>
bool StaffListSet<ScoreList, PartsList, ScoreForcedList, PartsForcedList>::contains(
    StaffCmper staffId, const MusxInstanceList<StaffUsed>& systemStaves, bool isHidden) const noexcept
{
    const StaffCmper topStaffId = systemStaves.getTopStaffId();
    const StaffCmper botStaffId = systemStaves.getBottomStaffId();

    auto staffListContainsStaff = [&](const MusxInstance<StaffList>& staffList) -> bool {
        if (!staffList) {
            return false;
        }
        MUSX_ASSERT_IF(staffList->getRequestedPartId() != systemStaves.getRequestedPartId()) {
            throw std::logic_error("Staff system part id [" + std::to_string(systemStaves.getRequestedPartId())
                + "] does not match part id for staff list set [" + std::to_string(staffList->getRequestedPartId()) + "].");
        }
        if (staffId == topStaffId && staffList->containsValue(static_cast<StaffCmper>(StaffList::FloatingValues::TopStaff))) {
            return true;
        }
        if (staffId == botStaffId && staffList->containsValue(static_cast<StaffCmper>(StaffList::FloatingValues::BottomStaff))) {
            return true;
        }
        /// @todo It may be necessary to add some fuzzy logic around StaffGroup here, at least for category staff lists.
        /// Finale includes a staff if it is the top staff in a group, when the top staff of a group is included. We defer this
        /// to another day.
        return staffList->containsValue(staffId);
    };

    if (!isHidden && staffListContainsStaff(m_staffList)) {
        return true;
    }

    return staffListContainsStaff(m_forcedStaffList);
}

template class StaffListSet<StaffListCategoryScore, StaffListCategoryParts>;
template class StaffListSet<StaffListRepeatScore, StaffListRepeatParts, StaffListRepeatScoreForced, StaffListRepeatPartsForced>;

namespace {

template <typename Target>
void importCategoryStaffListType(const DocumentPtr& target, const DocumentPtr& source,
    Cmper cmper, const ImportObjectCallback& onImported)
{
    for (const auto& sourceList : source->getOthers()->getAllSources<Target>()) {
        if (sourceList->getCmper() != cmper) {
            continue;
        }
        auto result = std::make_shared<Target>(target, sourceList->getSourcePartId(),
            sourceList->getShareMode(), sourceList->getCmper());
        *result = *sourceList;
        target->getOthers()->add(Target::XmlNodeName, result);
        if (onImported) {
            onImported(*result);
        }
    }
}

template <typename Target>
bool documentHasCategoryStaffListCmper(const DocumentPtr& document, Cmper cmper)
{
    for (const auto& list : document->getOthers()->getAllSources<Target>()) {
        if (list->getCmper() == cmper) {
            return true;
        }
    }
    return false;
}

bool documentHasCategoryStaffListCmper(const DocumentPtr& document, Cmper cmper)
{
    return documentHasCategoryStaffListCmper<StaffListCategoryName>(document, cmper)
        || documentHasCategoryStaffListCmper<StaffListCategoryParts>(document, cmper)
        || documentHasCategoryStaffListCmper<StaffListCategoryScore>(document, cmper);
}

} // namespace

std::optional<Cmper> importCategoryStaffListInto(const DocumentPtr& target, const DocumentPtr& source,
    Cmper cmper, const ImportObjectCallback& onImported)
{
    MUSX_ASSERT_IF(!target) {
        throw std::invalid_argument("importCategoryStaffListInto received a null target document");
    }
    MUSX_ASSERT_IF(!source) {
        throw std::invalid_argument("importCategoryStaffListInto received a null source document");
    }
    if (documentHasCategoryStaffListCmper(target, cmper)
        || !documentHasCategoryStaffListCmper(source, cmper)) {
        return std::nullopt;
    }
    importCategoryStaffListType<StaffListCategoryName>(target, source, cmper, onImported);
    importCategoryStaffListType<StaffListCategoryParts>(target, source, cmper, onImported);
    importCategoryStaffListType<StaffListCategoryScore>(target, source, cmper, onImported);
    return cmper;
}

// ***********************
// ***** StaffSystem *****
// ***********************

MusxInstance<Page> StaffSystem::getPage() const
{
    const auto part = getDocument()->getOthers()->get<PartDefinition>(SCORE_PARTID, getRequestedPartId());
    if (!part || !part->isLayoutCalculated()) {
        return nullptr;
    }
    const auto page = getDocument()->getOthers()->get<Page>(getRequestedPartId(), pageId);
    return page && page->isLayoutCalculated() ? page : nullptr;
}

util::Fraction StaffSystem::calcEffectiveScaling() const
{
    auto result = calcSystemScaling();
    if (auto page = getPage()) {
        result *= page->calcPageScaling();
    }
    return result;
}

util::Fraction StaffSystem::calcStaffScaling(StaffCmper staffId) const
{
    if (hasStaffScaling) {
        if (const auto staffSize = getDocument()->getDetails()->get<details::StaffSize>(getRequestedPartId(), getCmper(), staffId)) {
            return util::Fraction::fromPercent(staffSize->staffPercent);
        }
    }
    return 1;
}

std::pair<util::Fraction, util::Fraction> StaffSystem::calcMinMaxStaffSizes() const
{
    if (hasStaffScaling) {
        auto systemStaves = getDocument()->getOthers()->getArray<StaffUsed>(getRequestedPartId(), getCmper());
        if (!systemStaves.empty()) {
            std::pair<util::Fraction, util::Fraction> result = std::make_pair((std::numeric_limits<util::Fraction>::max)(), (std::numeric_limits<util::Fraction>::min)());
            for (const auto& systemStaff : systemStaves) {
                const auto val = calcStaffScaling(systemStaff->getCmper());
                if (val < result.first) result.first = val;
                if (val > result.second) result.second = val;
            }
            return result;
        }
    }
    return std::make_pair(util::Fraction(1), util::Fraction(1));;
}

// ***********************
// ***** TempoChange *****
// ***********************

double TempoChange::getAbsoluteTempo(NoteType noteType) const
{
    if (isRelative) {
        throw std::logic_error("Tempo change at measure " + std::to_string(getCmper()) + " inci " + std::to_string(getInci().value())
            + " is a relative tempo change.");
    }
    /* The value here is relative to 60 BPM == 1024 */
    const double ratioUnits = 65536.0 * double(noteType);
    const double exactTempo = (double(ratio) * double(unit) * 60.0) / ratioUnits;
    if (!unit || exactTempo <= 0.0) {
        return exactTempo;
    }

    // #ratio is an integer and so cannot encode most tempos exactly: at unit 1000, a quarter-note
    // tempo of 50 is stored as 55924, which reads back as 49.99995. Every tempo that converts to
    // the same ratio states this instance equally well, so report the shortest of them. That keeps
    // a stored 50 reading as 50 while leaving a genuinely fractional tempo at its own value, and it
    // cannot invent precision: a decimal too fine for the ratio to distinguish never converts back.
    const auto ratioFor = [&](double tempo) {
        return std::llround((tempo * ratioUnits) / (double(unit) * 60.0));
    };
    constexpr int maxDecimalPlaces = 9;
    double scale = 1.0;
    for (int decimalPlaces = 0; decimalPlaces <= maxDecimalPlaces; decimalPlaces++, scale *= 10.0) {
        const double candidate = std::round(exactTempo * scale) / scale;
        if (ratioFor(candidate) == ratio) {
            return candidate;
        }
    }
    return exactTempo;
}

// *********************
// ***** TextBlock *****
// *********************

util::EnigmaParsingContext TextBlock::getRawTextCtx(Cmper forPartId, std::optional<Cmper> forPageId, util::EnigmaString::TextInsertCallback defaultInsertFunc) const
{
    MusxInstance<TextsBase> rawText;
    switch (textType) {
        default:
            break;
        case TextType::Block:
            rawText = getDocument()->getTexts()->get<texts::BlockText>(textId);
            break;
        case TextType::Expression:
            rawText = getDocument()->getTexts()->get<texts::ExpressionText>(textId);
            break;
    }
    if (rawText) {
        return rawText->getRawTextCtx(rawText, forPartId, forPageId, defaultInsertFunc);
    }
    return {};
}

std::string TextBlock::getText(const DocumentPtr& document, const Cmper textId, Cmper forPartId, bool trimTags, util::EnigmaString::AccidentalStyle accidentalStyle)
{
    auto textBlock = document->getOthers()->get<TextBlock>(forPartId, textId);
    if (textBlock) {
        return textBlock->getRawTextCtx(forPartId).getText(trimTags, accidentalStyle);
    }
    return {};
}

// *****************************
// ***** TextExpressionDef *****
// *****************************

MusxInstance<TextBlock> TextExpressionDef::getTextBlock() const
{
    return getDocument()->getOthers()->get<TextBlock>(getRequestedPartId(), textIdKey);
}


MusxInstance<Enclosure> TextExpressionDef::getEnclosure() const
{
    if (!hasEnclosure) return nullptr;
    return getDocument()->getOthers()->get<TextExpressionEnclosure>(getRequestedPartId(), getCmper());
}

// *********************
// ***** StaffUsed *****
// *********************

util::Fraction StaffUsed::calcEffectiveScaling() const
{
    util::Fraction result(1);
    if (SystemCmper(getCmper()) > 0) { // if this is a page-view system
        if (auto system = getDocument()->getOthers()->get<StaffSystem>(getRequestedPartId(), getCmper())) {
            result = system->calcEffectiveScaling() * system->calcStaffScaling(staffId);
        }
    }
    return result;
}

MusxInstance<Staff> StaffUsed::getStaffInstance() const
{
    auto retval = getDocument()->getOthers()->get<Staff>(getRequestedPartId(), staffId);
    if (!retval) {
        MUSX_INTEGRITY_ERROR("Staff " + std::to_string(staffId) + " not found for StaffUsed list " + std::to_string(getCmper()));
    }
    return retval;
}

MusxInstance<StaffComposite> StaffUsed::getStaffInstance(MeasCmper measureId, Edu eduPosition) const
{
    auto retval = StaffComposite::createCurrent(getDocument(), getRequestedPartId(), staffId, measureId, eduPosition);
    if (!retval) {
        MUSX_INTEGRITY_ERROR("Composite staff " + std::to_string(staffId) + " not found for StaffUsed list " + std::to_string(getCmper())
            + " at measure " + std::to_string(measureId) + " eduPosition " + std::to_string(eduPosition));
    }
    return retval;
}

// ****************************
// ***** TextRepeatAssign *****
// ****************************

MusxInstance<RepeatIndividualPositioning> TextRepeatAssign::getIndividualPositioning(StaffCmper staffId) const
{
    return getIndividualPositioningImpl(getDocument()->getOthers()->getArray<TextRepeatIndividualPositioning>(getRequestedPartId(), textRepeatId),
        staffId, static_cast<MeasCmper>(getCmper()));
}

RepeatStaffListSet TextRepeatAssign::createStaffListSet() const
{
    return RepeatStaffListSet(getDocument(), getRequestedPartId(), staffList);
}

std::optional<MeasCmper> TextRepeatAssign::calcTargetMeasure() const
{
    switch (jumpAction) {
    case RepeatActionType::JumpAbsolute:
        return (targetValue > 0) ? std::optional<MeasCmper>(MeasCmper(targetValue)) : std::nullopt;

    case RepeatActionType::JumpRelative: {
        const auto target = static_cast<MeasCmper>(getCmper() + targetValue);
        return (target > 0) ? std::optional<MeasCmper>(target) : std::nullopt;
    }

    case RepeatActionType::JumpToMark: {
        const auto assigns = getDocument()->getOthers()->getArray<others::TextRepeatAssign>(getRequestedPartId());
        const auto it = std::find_if(assigns.begin(), assigns.end(), [&](const auto& assign) {
            return assign && assign->textRepeatId == targetValue;
        });
        if (it != assigns.end()) {
            return (*it)->getCmper();
        }
        return std::nullopt;
    }

    default:
        return std::nullopt;
    }
}

} // namespace others
} // namespace dom
} // namespace musx
