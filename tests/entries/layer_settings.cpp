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

#include "gtest/gtest.h"
#include "musx/musx.h"
#include "test_utils.h"

namespace {

class LayerSettingsApplicabilityTest : public ::testing::Test
{
protected:
    using LayerAttributes = musx::dom::others::LayerAttributes;
    using Staff = musx::dom::others::Staff;
    static constexpr musx::dom::StaffCmper staffId = 3;
    static constexpr musx::dom::MeasCmper measureId = 3;
    static constexpr musx::dom::LayerIndex sourceLayer = 0;
    static constexpr musx::dom::LayerIndex otherLayer = 1;
    static constexpr musx::dom::Edu quarterDuration = musx::dom::Edu(musx::dom::NoteType::Quarter);
    musx::dom::DocumentPtr document;

    void SetUp() override
    {
        std::vector<char> xml;
        musxtest::readFile(musxtest::getInputPath() / "stemdirection.enigmaxml", xml);
        document = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml,
            musx::dom::PartVoicingPolicy::Apply);
        layer(sourceLayer)->onlyIfOtherLayersHaveNotes = true;
        layer(sourceLayer)->ignoreHiddenNotesOnly = true;
    }

    std::shared_ptr<LayerAttributes> layer(musx::dom::LayerIndex index)
    {
        return std::const_pointer_cast<LayerAttributes>(document->getOthers()->get<LayerAttributes>(
            musx::dom::SCORE_PARTID, musx::dom::Cmper(index)));
    }

    std::shared_ptr<Staff> staff()
    {
        return std::const_pointer_cast<Staff>(document->getOthers()->get<Staff>(musx::dom::SCORE_PARTID, staffId));
    }

    musx::dom::EntryInfoPtr entry(size_t index = 0, musx::dom::Cmper partId = musx::dom::SCORE_PARTID,
        musx::dom::LayerIndex layerIndex = sourceLayer)
    {
        const musx::dom::details::GFrameHoldContext context(document, partId, staffId, measureId);
        return musx::dom::EntryInfoPtr(context.createEntryFrame(layerIndex), index);
    }

    void hideOtherEntriesExcept(std::optional<size_t> visibleIndex = std::nullopt)
    {
        const auto frame = entry(0, musx::dom::SCORE_PARTID, otherLayer).getFrame();
        for (size_t index = 0; index < frame->getEntries().size(); ++index) {
            auto raw = std::const_pointer_cast<musx::dom::Entry>(frame->getEntries()[index]->getEntry());
            raw->isHidden = !visibleIndex || index != *visibleIndex;
        }
    }

    std::shared_ptr<musx::dom::others::StaffStyle> addStyle(musx::dom::Edu start, musx::dom::Edu end)
    {
        using namespace musx::dom;
        constexpr Cmper styleId = 100;
        auto style = std::make_shared<others::StaffStyle>(document, SCORE_PARTID, EnigmaBase::ShareMode::All, styleId);
        style->masks = std::make_shared<others::StaffStyle::Masks>(style);
        const auto assignments = document->getOthers()->getArray<others::StaffStyleAssign>(SCORE_PARTID, staffId);
        auto assignment = std::make_shared<others::StaffStyleAssign>(document, SCORE_PARTID,
            EnigmaBase::ShareMode::All, staffId, static_cast<Inci>(assignments.size()));
        assignment->styleId = styleId;
        assignment->startMeas = measureId;
        assignment->endMeas = measureId;
        assignment->startEdu = start;
        assignment->endEdu = end;
        document->getOthers()->add(others::StaffStyle::XmlNodeName, style);
        document->getOthers()->add(others::StaffStyleAssign::XmlNodeName, assignment);
        staff()->hasStyles = true;
        return style;
    }
};

TEST_F(LayerSettingsApplicabilityTest, UnconditionalAndSingleLayer)
{
    using namespace musx::dom;
    auto hold = std::const_pointer_cast<details::GFrameHold>(
        document->getDetails()->get<details::GFrameHold>(SCORE_PARTID, staffId, measureId));
    hold->frames[otherLayer] = 0;
    EXPECT_FALSE(entry().calcIfLayerSettingsApply());
    layer(sourceLayer)->onlyIfOtherLayersHaveNotes = false;
    EXPECT_TRUE(entry().calcIfLayerSettingsApply());
}

TEST_F(LayerSettingsApplicabilityTest, HiddenLayerNeverSatisfiesPopulationRequirement)
{
    for (const bool ignoreHiddenLayers : {false, true}) {
        layer(sourceLayer)->ignoreHiddenLayers = ignoreHiddenLayers;
        for (const bool ignoreHiddenNotes : {false, true}) {
            layer(sourceLayer)->ignoreHiddenNotesOnly = ignoreHiddenNotes;
            layer(otherLayer)->hideLayer = true;
            EXPECT_FALSE(entry().calcIfLayerSettingsApply());
            layer(otherLayer)->hideLayer = false;
            layer(sourceLayer)->hideLayer = true;
            EXPECT_TRUE(entry().calcIfLayerSettingsApply());
            layer(sourceLayer)->hideLayer = false;
        }
    }
}

TEST_F(LayerSettingsApplicabilityTest, VisibleRestsCountWithoutRhythmicOverlap)
{
    constexpr size_t finalRestIndex = 2;
    hideOtherEntriesExcept();
    EXPECT_FALSE(entry().calcIfLayerSettingsApply());
    layer(sourceLayer)->ignoreHiddenNotesOnly = false;
    EXPECT_TRUE(entry().calcIfLayerSettingsApply());
    layer(sourceLayer)->ignoreHiddenNotesOnly = true;
    hideOtherEntriesExcept(finalRestIndex);
    const auto rest = entry(finalRestIndex, musx::dom::SCORE_PARTID, otherLayer);
    ASSERT_TRUE(rest.calcDisplaysAsRest());
    ASSERT_GT(rest->elapsedDuration, entry()->elapsedDuration + entry()->actualDuration);
    EXPECT_TRUE(entry().calcIfLayerSettingsApply());
}

TEST_F(LayerSettingsApplicabilityTest, OnlyDesignatedAlternateNotationLayerIsExcluded)
{
    auto rawStaff = staff();
    rawStaff->altLayer = otherLayer;
    for (const auto mode : {Staff::AlternateNotation::Blank, Staff::AlternateNotation::BlankWithRests,
        Staff::AlternateNotation::OneBarRepeat, Staff::AlternateNotation::TwoBarRepeat,
        Staff::AlternateNotation::SlashBeats}) {
        rawStaff->altNotation = mode;
        EXPECT_FALSE(entry().calcIfLayerSettingsApply());
    }
    for (const auto mode : {Staff::AlternateNotation::Normal, Staff::AlternateNotation::Rhythmic}) {
        rawStaff->altNotation = mode;
        EXPECT_TRUE(entry().calcIfLayerSettingsApply());
    }
    rawStaff->altNotation = Staff::AlternateNotation::Blank;
    rawStaff->altLayer = sourceLayer;
    rawStaff->altHideOtherNotes = true;
    EXPECT_TRUE(entry().calcIfLayerSettingsApply());
}

TEST_F(LayerSettingsApplicabilityTest, AlternateNotationTracksCurrentStaffStyleRange)
{
    using namespace musx::dom;
    constexpr size_t secondQuarterIndex = 2;
    constexpr size_t thirdQuarterIndex = 3;
    auto style = addStyle(quarterDuration, 2 * quarterDuration - 1);
    style->masks->altNotation = true;
    style->altNotation = Staff::AlternateNotation::Blank;
    style->altLayer = otherLayer;
    EXPECT_TRUE(entry().calcIfLayerSettingsApply());
    EXPECT_FALSE(entry(secondQuarterIndex).calcIfLayerSettingsApply());
    EXPECT_TRUE(entry(thirdQuarterIndex).calcIfLayerSettingsApply());
}

TEST_F(LayerSettingsApplicabilityTest, CutawayDrawingSuppressionDoesNotHideEntries)
{
    using namespace musx::dom;
    constexpr size_t otherSecondQuarterIndex = 1;
    constexpr size_t sourceSecondQuarterIndex = 2;
    constexpr size_t sourceThirdQuarterIndex = 3;
    hideOtherEntriesExcept(otherSecondQuarterIndex);
    auto style = addStyle(quarterDuration, 2 * quarterDuration - 1);
    style->masks->hideStaff = true;
    style->hideMode = Staff::HideMode::Cutaway;
    EXPECT_TRUE(entry().calcIfLayerSettingsApply());
    EXPECT_TRUE(entry(sourceSecondQuarterIndex).calcIfLayerSettingsApply());
    EXPECT_TRUE(entry(sourceThirdQuarterIndex).calcIfLayerSettingsApply());
    style->hideMode = Staff::HideMode::None;
    EXPECT_TRUE(entry(sourceThirdQuarterIndex).calcIfLayerSettingsApply());
}

TEST_F(LayerSettingsApplicabilityTest, PartVoicingOnlyFiltersVisibleEntryRequirement)
{
    using namespace musx::dom;
    constexpr Cmper partId = 1;
    auto voicing = std::make_shared<others::PartVoicing>(document, partId, EnigmaBase::ShareMode::None, staffId);
    voicing->enabled = true;
    voicing->voicingType = others::PartVoicing::VoicingType::UseSingleLayer;
    voicing->singleLayer = sourceLayer;
    document->getOthers()->add(others::PartVoicing::XmlNodeName, voicing);
    EXPECT_FALSE(entry(0, partId).calcIfLayerSettingsApply());
    EXPECT_TRUE(entry().calcIfLayerSettingsApply());
    layer(sourceLayer)->ignoreHiddenNotesOnly = false;
    EXPECT_TRUE(entry(0, partId).calcIfLayerSettingsApply());
}

TEST_F(LayerSettingsApplicabilityTest, StemForcingAndRestOffsetsUseSameGate)
{
    using namespace musx::dom;
    using musx::util::Stem;
    constexpr size_t restIndex = 2;
    constexpr int restOffset = 3;
    auto source = layer(sourceLayer);
    source->freezeLayer = true;
    source->freezeStemsUp = true;
    source->useRestOffset = true;
    source->restOffset = restOffset;
    auto misc = std::const_pointer_cast<options::MiscOptions>(document->getOptions()->get<options::MiscOptions>());
    misc->consolidateRestsAcrossLayers = false;
    EXPECT_EQ(Stem::calcForcedDirection(entry()).reason, Stem::Reason::LayerFreeze);
    const auto shiftedRest = entry(restIndex).calcZeroNotePosition();
    layer(otherLayer)->hideLayer = true;
    EXPECT_NE(Stem::calcForcedDirection(entry()).reason, Stem::Reason::LayerFreeze);
    EXPECT_EQ(entry(restIndex).calcZeroNotePosition(), shiftedRest - restOffset);
}

} // namespace
