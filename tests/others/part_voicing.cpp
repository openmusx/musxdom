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

using namespace musx::dom;

constexpr static musxtest::string_view partVoicingXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <voicingDef cmper="1" part="1" shared="false">
      <enabled/>
      <voicingType>useMultipleLayers</voicingType>
      <singleVoiceType>botNote</singleVoiceType>
      <select1st/>
      <select3rd/>
      <selectFromBottom/>
      <selectSingleNote/>
      <singleLayer>1</singleLayer>
      <multiLayer>2</multiLayer>
    </voicingDef>
    <voicingDef cmper="1" part="2" shared="false">
      <enabled/>
      <singleVoiceType>selected</singleVoiceType>
      <select2nd/>
      <select4th/>
      <select5th/>
      <selectSingleNote/>
      <singleLayer>1</singleLayer>
      <multiLayer>1</multiLayer>
    </voicingDef>
  </others>
</finale>
)xml";

TEST(PartVoicingTest, PartVoicingPopulationTest)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(partVoicingXml);
    ASSERT_TRUE(doc);

    // cmper (staff) = 1 for both records in the XML above.
    // First record: part=1
    auto v1 = doc->getOthers()->get<others::PartVoicing>(1, 1);
    ASSERT_TRUE(v1);

    EXPECT_TRUE(v1->enabled);
    EXPECT_EQ(v1->voicingType, others::PartVoicing::VoicingType::UseMultipleLayers);
    EXPECT_EQ(v1->singleLayerVoiceType, others::PartVoicing::SingleLayerVoiceType::BottomNote);

    EXPECT_TRUE(v1->select1st);
    EXPECT_FALSE(v1->select2nd);
    EXPECT_TRUE(v1->select3rd);
    EXPECT_FALSE(v1->select4th);
    EXPECT_FALSE(v1->select5th);

    EXPECT_TRUE(v1->selectFromBottom);
    EXPECT_TRUE(v1->selectSingleNote);

    EXPECT_EQ(v1->singleLayer, 1);
    EXPECT_EQ(v1->multiLayer, 2);

    // Second record: part=2
    auto v2 = doc->getOthers()->get<others::PartVoicing>(2, 1);
    ASSERT_TRUE(v2);

    EXPECT_TRUE(v2->enabled);
    // voicingType not present in XML -> default (UseSingleLayer)
    EXPECT_EQ(v2->voicingType, others::PartVoicing::VoicingType::UseSingleLayer);
    EXPECT_EQ(v2->singleLayerVoiceType, others::PartVoicing::SingleLayerVoiceType::SelectedNotes);

    EXPECT_FALSE(v2->select1st);
    EXPECT_TRUE(v2->select2nd);
    EXPECT_FALSE(v2->select3rd);
    EXPECT_TRUE(v2->select4th);
    EXPECT_TRUE(v2->select5th);

    EXPECT_FALSE(v2->selectFromBottom); // not present -> default false
    EXPECT_TRUE(v2->selectSingleNote);

    EXPECT_EQ(v2->singleLayer, 1);
    EXPECT_EQ(v2->multiLayer, 1);

    // Nonexistent combination should return nothing
    auto v_missing = doc->getOthers()->get<others::PartVoicing>(3, 1);
    ASSERT_FALSE(v_missing);
}

static void checkPart(const DocumentPtr& doc, Cmper partId, const std::vector<std::vector<int>>& expectedDisps,
                      const std::vector<Edu>& expectedDuras, const std::vector<LayerIndex>& expectedLayers) {
    auto part = doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, partId);
    ASSERT_TRUE(part) << "Part Id " << partId << " does not exist.";
    auto partName = part->getName();
    EXPECT_FALSE(partName.empty()) << "Part Id " << partId << " has no name.";
    if (partName.empty()) {
        partName = "Part " + std::to_string(partId);
    }
    partName += ": ";
    size_t notesX = 0;
    ASSERT_EQ(expectedDisps.size(), expectedDuras.size()) << partName << "expected displacements and durations do not match.";
    ASSERT_EQ(expectedDisps.size(), expectedLayers.size()) << partName << "expected displacements and layers do not match.";
    doc->iterateEntries(partId, [&](const EntryInfoPtr& entryInfoPtr) {
        const auto entry = entryInfoPtr->getEntry();
        if (!entryInfoPtr.calcDisplaysAsRest()) {
            const bool validX = notesX < expectedDisps.size();
            EXPECT_TRUE(validX) << partName << "not enough expected displacements or durations";
            if (!validX) return false;
            EXPECT_EQ(expectedLayers[notesX], entryInfoPtr.getLayerIndex()) << partName << "unexpected layer " << entryInfoPtr.getLayerIndex() << ".";
            EXPECT_EQ(expectedDuras[notesX], entry->duration) << partName << "unexpected duration " << entry->duration << ".";
            size_t dispX = 0;
            for (size_t noteIndex = 0; noteIndex < entry->notes.size(); noteIndex++) {
                auto noteInfoPtr = NoteInfoPtr(entryInfoPtr, noteIndex);
                if (doc->getPartVoicingPolicy() != PartVoicingPolicy::Apply || noteInfoPtr.calcIsIncludedInVoicing()) {
                    EXPECT_LT(dispX, expectedDisps[notesX].size()) << partName << "not enought expected displacement values for index " << notesX << ".";
                    if (dispX >= expectedDisps[notesX].size()) return false;
                    EXPECT_EQ(expectedDisps[notesX][dispX++], noteInfoPtr->harmLev) << partName << "unexpected pitch value " << noteInfoPtr->harmLev << ".";
                }
            }
            EXPECT_EQ(dispX, expectedDisps[notesX].size()) << partName << "did not encounter all expected displacements for index " << notesX << ".";
            notesX++;
        }
        return true;
    });
    EXPECT_EQ(notesX, expectedDisps.size()) << partName << "did not encounter all expected values.";
};

TEST(PartVoicing, EntryDetection)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "voiced-parts.enigmaxml", xml);
    auto docIgnore = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml, PartVoicingPolicy::Ignore);
    ASSERT_TRUE(docIgnore);
    auto docApply = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml, PartVoicingPolicy::Apply);
    ASSERT_TRUE(docApply);

    /*
        Part 0: Score
        Part 1: Layer 1
        Part 2: Layer 2
        Part 3: Layer 3
        Part 4: Layer 2 Only
    */

    std::vector<std::vector<int>> expectedDispsScore = {
        {7}, {3}, {5},
        {3, 5, 7},
        {3, 5, 7},
        {5, 7}, {3},
        {5}, {5, 7}
    };
    std::vector<Edu> expectedDurasScore = {
        1024, 1024, 1024,
        1024,
        1024,
        1024, 1024,
        512, 512
    };
    std::vector<LayerIndex> expectedLayersScore = {
        0, 1, 2,
        1,
        0,
        0, 1,
        2, 2
    };

    {
        constexpr Cmper kPartId = 1; // Layer 1
        std::vector<std::vector<int>> expectedDisps = { {7}, {7}, {7}, {5, 7}, {5}, {7} };
        std::vector<Edu> expectedDuras = { 1024, 1024, 1024, 1024, 512, 512 };
        std::vector<LayerIndex> expectedLayers = { 0, 1, 0, 0, 2, 2 };
        checkPart(docApply, kPartId, expectedDisps, expectedDuras, expectedLayers);
        checkPart(docIgnore, kPartId, expectedDispsScore, expectedDurasScore, expectedLayersScore);
    }
    {
        constexpr Cmper kPartId = 2; // Layer 2
        std::vector<std::vector<int>> expectedDisps = { {3}, {3}, {3}, {3}, {5} };
        std::vector<Edu> expectedDuras = { 1024, 1024, 1024, 1024, 512 };
        std::vector<LayerIndex> expectedLayers = { 1, 1, 0, 1, 2 };
        checkPart(docApply, kPartId, expectedDisps, expectedDuras, expectedLayers);
        checkPart(docIgnore, kPartId, expectedDispsScore, expectedDurasScore, expectedLayersScore);
    }
    {
        constexpr Cmper kPartId = 3; // Layer 3
        std::vector<std::vector<int>> expectedDisps = { {5}, {5}, {5}, {5}, {5} };
        std::vector<Edu> expectedDuras = { 1024, 1024, 1024, 512, 512 };
        std::vector<LayerIndex> expectedLayers = { 2, 1, 0, 2, 2 };
        checkPart(docApply, kPartId, expectedDisps, expectedDuras, expectedLayers);
        checkPart(docIgnore, kPartId, expectedDispsScore, expectedDurasScore, expectedLayersScore);
    }
    {
        constexpr Cmper kPartId = 4; // Layer 2 Only
        std::vector<std::vector<int>> expectedDisps = { {3}, {3, 5, 7}, {3} };
        std::vector<Edu> expectedDuras = { 1024, 1024, 1024 };
        std::vector<LayerIndex> expectedLayers = { 1, 1, 1 };
        checkPart(docApply, kPartId, expectedDisps, expectedDuras, expectedLayers);
        checkPart(docIgnore, kPartId, expectedDispsScore, expectedDurasScore, expectedLayersScore);
    }
}

TEST(PartVoicing, DetectionWhileIgnored)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "voiced-parts.enigmaxml", xml);
    auto docIgnore = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml, PartVoicingPolicy::Ignore);
    ASSERT_TRUE(docIgnore);

    {
        constexpr Cmper kPartId = 4; // Layer 2 Only
        auto gfHold = details::GFrameHoldContext(docIgnore, kPartId, 1, 3);
        EXPECT_TRUE(gfHold.calcPolicyVoicingIncludesLayer(0));
        EXPECT_FALSE(gfHold.calcVoicingIncludesLayer(0));
        auto frame = gfHold.createEntryFrame(0);
        ASSERT_TRUE(frame);
        ASSERT_GT(frame->getEntries().size(), 0);
        auto entryInfoPtr = EntryInfoPtr(frame, 0);
        EXPECT_FALSE(entryInfoPtr.calcDisplaysAsRest());
        auto entry = entryInfoPtr->getEntry();
        EXPECT_GT(entry->notes.size(), 0);
        for (size_t x = 0; x < entry->notes.size(); x++) {
            EXPECT_FALSE(NoteInfoPtr(entryInfoPtr, x).calcIsIncludedInVoicing());
        }
    }
    {
        constexpr Cmper kPartId = 2; // Layer 2
        auto gfHold = details::GFrameHoldContext(docIgnore, kPartId, 1, 5);
        EXPECT_TRUE(gfHold.calcPolicyVoicingIncludesLayer(2));
        EXPECT_TRUE(gfHold.calcVoicingIncludesLayer(2));
        auto frame = gfHold.createEntryFrame(2);
        ASSERT_TRUE(frame);
        ASSERT_GT(frame->getEntries().size(), 1);
        auto entryInfoPtr = EntryInfoPtr(frame, 1);
        EXPECT_FALSE(entryInfoPtr.calcDisplaysAsRest());
        auto entry = entryInfoPtr->getEntry();
        EXPECT_GT(entry->notes.size(), 0);
        for (size_t x = 0; x < entry->notes.size(); x++) {
            EXPECT_FALSE(NoteInfoPtr(entryInfoPtr, x).calcIsIncludedInVoicing());
        }
    }
}

TEST(PartVoicing, StaffExtremaHonorPolicyAndDriveStemDirection)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "voiced-parts.enigmaxml", xml);
    constexpr Cmper staffId = 1;
    constexpr MeasCmper measureId = 3;
    constexpr int scoreTop = -3;    // C5 in treble clef.
    constexpr int scoreBottom = -7; // F4 in treble clef.
    struct PartExpectation { Cmper partId; int top; int bottom; bool upStem; };
    constexpr PartExpectation expectations[] = {
        {SCORE_PARTID, scoreTop, scoreBottom, true},
        {1, scoreTop, scoreTop, false},
        {2, scoreBottom, scoreBottom, true},
        {3, -5, -5, true} // Selected middle note, A4.
    };
    for (auto policy : {PartVoicingPolicy::Ignore, PartVoicingPolicy::Apply}) {
        const auto document = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml, policy);
        for (const auto& expected : expectations) {
            SCOPED_TRACE(expected.partId);
            SCOPED_TRACE(static_cast<int>(policy));
            const details::GFrameHoldContext context(document, expected.partId, staffId, measureId);
            const auto frame = context.createEntryFrame(0);
            ASSERT_TRUE(frame);
            const EntryInfoPtr entryInfo(frame, 0);
            ASSERT_FALSE(entryInfo.calcDisplaysAsRest());
            EXPECT_FALSE(entryInfo.calcIsZeroNote());
            const bool apply = policy == PartVoicingPolicy::Apply;
            EXPECT_EQ(entryInfo.calcTopBottomStaffPositions(),
                std::make_pair(apply ? expected.top : scoreTop, apply ? expected.bottom : scoreBottom));
            EXPECT_EQ(entryInfo.calcUpStemDefault(), apply ? expected.upStem : true);
            const auto stemOptions = document->getOptions()->get<options::StemOptions>();
            ASSERT_TRUE(stemOptions);
            const int top = apply ? expected.top : scoreTop;
            const int bottom = apply ? expected.bottom : scoreBottom;
            const bool upStem = entryInfo.calcUpStem();
            EXPECT_EQ(entryInfo.calcTopBottomExtent(), std::make_pair(
                static_cast<Evpu>(top * EVPU_PER_STAFF_POSITION) + (upStem ? stemOptions->stemLength : EVPU_PER_STAFF_POSITION),
                static_cast<Evpu>(bottom * EVPU_PER_STAFF_POSITION) - (upStem ? EVPU_PER_STAFF_POSITION : stemOptions->stemLength)));
        }
    }
}

TEST(PartVoicing, ExcludedChordUsesFloatingRestPosition)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "voiced-parts.enigmaxml", xml);
    const auto document = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml, PartVoicingPolicy::Apply);
    constexpr Cmper partId = 2;
    constexpr Cmper staffId = 1;
    constexpr MeasCmper measureId = 5;
    constexpr LayerIndex layerIndex = 2;
    constexpr size_t entryIndex = 1;
    constexpr int defaultRestPosition = -4;
    constexpr int layerOffset = 6;
    const details::GFrameHoldContext context(document, partId, staffId, measureId);
    const auto frame = context.createEntryFrame(layerIndex);
    ASSERT_TRUE(frame);
    const EntryInfoPtr entryInfo(frame, entryIndex);
    ASSERT_TRUE(entryInfo->getEntry()->isNote);
    ASSERT_FALSE(entryInfo->getEntry()->notes.empty());
    ASSERT_TRUE(entryInfo.calcDisplaysAsRest());
    EXPECT_TRUE(entryInfo.calcIsZeroNote());
    EXPECT_EQ(entryInfo.calcZeroNotePosition(), defaultRestPosition);
    EXPECT_EQ(entryInfo.calcTopBottomStaffPositions(), std::make_pair(defaultRestPosition, defaultRestPosition));

    auto layer = std::const_pointer_cast<others::LayerAttributes>(frame->getLayerAttributes());
    ASSERT_TRUE(layer);
    layer->useRestOffset = true;
    layer->onlyIfOtherLayersHaveNotes = false;
    layer->restOffset = layerOffset;
    auto misc = std::const_pointer_cast<options::MiscOptions>(document->getOptions()->get<options::MiscOptions>());
    ASSERT_TRUE(misc);
    misc->consolidateRestsAcrossLayers = false;
    constexpr int displacedPosition = defaultRestPosition + layerOffset;
    EXPECT_EQ(entryInfo.calcTopBottomStaffPositions(), std::make_pair(displacedPosition, displacedPosition));

    // An actual floating rest at the same location follows the same positioning rules.
    auto entry = std::const_pointer_cast<Entry>(entryInfo->getEntry());
    entry->isNote = false;
    entry->floatRest = true;
    EXPECT_EQ(entryInfo.calcTopBottomStaffPositions(), std::make_pair(displacedPosition, displacedPosition));
}

TEST(PartVoicing, ExcludedUnbeamedChordDoesNotDetermineStemDirection)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "voiced-parts.enigmaxml", xml);
    const auto document = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml, PartVoicingPolicy::Apply);
    constexpr Cmper partId = 1;
    constexpr Cmper staffId = 1;
    constexpr MeasCmper measureId = 3;
    auto voicing = std::const_pointer_cast<others::PartVoicing>(document->getOthers()->get<others::PartVoicing>(partId, staffId));
    ASSERT_TRUE(voicing);
    voicing->singleLayerVoiceType = others::PartVoicing::SingleLayerVoiceType::SelectedNotes;
    voicing->select1st = voicing->select2nd = voicing->select3rd = voicing->select4th = voicing->select5th = false;
    const details::GFrameHoldContext context(document, partId, staffId, measureId);
    const auto frame = context.createEntryFrame(0);
    ASSERT_TRUE(frame);
    const EntryInfoPtr entryInfo(frame, 0);
    ASSERT_TRUE(entryInfo.calcDisplaysAsRest());
    ASSERT_TRUE(entryInfo.calcUnbeamed());
    auto entry = std::const_pointer_cast<Entry>(entryInfo->getEntry());
    for (bool upStem : {false, true}) {
        entry->upStemScore = upStem;
        if (const auto partData = entryInfo.getPartFieldData()) {
            std::const_pointer_cast<details::EntryPartFieldDetail>(partData)->upStem = upStem;
        }
        const EntryInfoPtr freshEntry(frame, 0);
        EXPECT_EQ(freshEntry.calcUpStemDefault(), upStem);
    }
}
