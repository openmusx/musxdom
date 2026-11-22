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
#include <limits>

#include "gtest/gtest.h"
#include "musx/musx.h"
#include "test_utils.h"

using namespace musx::dom;

TEST(ArticulationDefTest, PopulateFields)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <articDef cmper="5">
    <charMain>58528</charMain>
    <fontMain>10</fontMain>
    <sizeMain>24</sizeMain>
    <efxMain>
        <bold/>
        <underline/>
    </efxMain>
    <copyMode>both</copyMode>
    <autoHorz/>
    <autoVert/>
    <autoVertMode>autoNoteStem</autoVertMode>
    <outsideStaff/>
    <aboveSymbolAlt/>
    <autoStack/>
    <slurInteractionMode>avoidSlur</slurInteractionMode>
    <charAlt>58529</charAlt>
    <fontAlt>10</fontAlt>
    <xOffsetMain>1</xOffsetMain>
    <yOffsetMain>2</yOffsetMain>
    <defVertPos>14</defVertPos>
    <playArtic/>
    <sizeAlt>24</sizeAlt>
    <efxAlt>
        <bold/>
        <underline/>
    </efxAlt>
    <xOffsetAlt>3</xOffsetAlt>
    <yOffsetAlt>4</yOffsetAlt>
    <startTopNotePercent>7</startTopNotePercent>
    <startBotNotePercent>8</startBotNotePercent>
    <durTopNotePercent>1</durTopNotePercent>
    <durBotNotePercent>2</durBotNotePercent>
    <ampTopNoteDelta>125</ampTopNoteDelta>
    <ampBotNoteDelta>125</ampBotNoteDelta>
    </articDef>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto articDef = others->get<others::ArticulationDef>(SCORE_PARTID, 5);
    ASSERT_TRUE(articDef) << "ArticulationDef 5 not found but does exist";

    EXPECT_EQ(articDef->charMain, 58528);
    EXPECT_EQ(articDef->fontMain->fontId, 10);
    EXPECT_EQ(articDef->fontMain->fontSize, 24);
    EXPECT_FALSE(articDef->fontMain->italic);
    EXPECT_TRUE(articDef->fontMain->bold);
    EXPECT_TRUE(articDef->fontMain->underline);
    EXPECT_EQ(articDef->copyMode, others::ArticulationDef::CopyMode::Vertical);
    EXPECT_TRUE(articDef->autoHorz);
    EXPECT_TRUE(articDef->autoVert);
    EXPECT_EQ(articDef->autoVertMode, others::ArticulationDef::AutoVerticalMode::AutoNoteStem);
    EXPECT_TRUE(articDef->outsideStaff);
    EXPECT_TRUE(articDef->aboveSymbolAlt);
    EXPECT_TRUE(articDef->autoStack);
    EXPECT_EQ(articDef->slurInteractionMode, others::ArticulationDef::SlurInteractionMode::AvoidSlur);
    EXPECT_EQ(articDef->charAlt, 58529);
    EXPECT_EQ(articDef->fontAlt->fontId, 10);
    EXPECT_EQ(articDef->fontAlt->fontSize, 24);
    EXPECT_FALSE(articDef->fontAlt->italic);
    EXPECT_TRUE(articDef->fontAlt->bold);
    EXPECT_TRUE(articDef->fontAlt->underline);
    EXPECT_EQ(articDef->xOffsetMain, 1);
    EXPECT_EQ(articDef->yOffsetMain, 2);
    EXPECT_EQ(articDef->defVertPos, 14);
    EXPECT_TRUE(articDef->playArtic);
    EXPECT_EQ(articDef->xOffsetAlt, 3);
    EXPECT_EQ(articDef->yOffsetAlt, 4);
    EXPECT_EQ(articDef->startTopNotePercent, 7);
    EXPECT_EQ(articDef->startBotNotePercent, 8);
    EXPECT_EQ(articDef->durTopNotePercent, 1);
    EXPECT_EQ(articDef->durBotNotePercent, 2);
    EXPECT_EQ(articDef->ampTopNoteDelta, 125);
    EXPECT_EQ(articDef->ampBotNoteDelta, 125);

    const auto aboveSymbol = articDef->calcSelectedSymbol(true);
    EXPECT_TRUE(aboveSymbol.usesAlternate);
    EXPECT_FALSE(aboveSymbol.isShape);
    EXPECT_EQ(aboveSymbol.character, articDef->charAlt);
    EXPECT_EQ(aboveSymbol.font, articDef->fontAlt);
    EXPECT_EQ(aboveSymbol.xOffset, articDef->xOffsetAlt);
    EXPECT_EQ(aboveSymbol.yOffset, articDef->yOffsetAlt);

    const auto belowSymbol = articDef->calcSelectedSymbol(false);
    EXPECT_FALSE(belowSymbol.usesAlternate);
    EXPECT_FALSE(belowSymbol.isShape);
    EXPECT_EQ(belowSymbol.character, articDef->charMain);
    EXPECT_EQ(belowSymbol.font, articDef->fontMain);
    EXPECT_EQ(belowSymbol.xOffset, articDef->xOffsetMain);
    EXPECT_EQ(belowSymbol.yOffset, articDef->yOffsetMain);
}

TEST(ArticulationAssignTest, Populate)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <details>
    <articAssign entnum="125" inci="0">
      <articDef>67</articDef>
      <horzOff>-35</horzOff>
      <vertOff>-2</vertOff>
      <vertAdd>-86</vertAdd>
      <overridePlacement/>
      <numSlursAvoided>-1</numSlursAvoided>
    </articAssign>
    <articAssign entnum="125" inci="1">
      <articDef>17</articDef>
      <horzOff>16</horzOff>
      <horzAdd>204</horzAdd>
      <vertOff>51</vertOff>
      <hide/>
      <neverStack/>
      <avoidSlur/>
      <overridePlacement/>
      <aboveEntry/>
      <numSlursAvoided>-1</numSlursAvoided>
    </articAssign>
  </details>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    auto details = doc->getDetails();
    ASSERT_TRUE(details) << "Details node not found in XML";

    // Check first articulation assignment
    auto artic1 = details->get<details::ArticulationAssign>(SCORE_PARTID, 125, 0);
    ASSERT_TRUE(artic1) << "ArticulationAssign with entnum=125, inci=0 not found but does exist";
    EXPECT_EQ(artic1->articDef, 67);         // From XML
    EXPECT_EQ(artic1->horzOffset, -35);      // From XML
    EXPECT_EQ(artic1->vertOffset, -2);       // From XML
    EXPECT_EQ(artic1->vertAdd, -86);         // From XML
    EXPECT_TRUE(artic1->overridePlacement);  // From XML
    EXPECT_EQ(artic1->numSlursAvoided, -1);  // From XML
    EXPECT_FALSE(artic1->hide);              // Not present in XML
    EXPECT_FALSE(artic1->neverStack);        // Not present in XML
    EXPECT_FALSE(artic1->avoidSlur);         // Not present in XML
    EXPECT_FALSE(artic1->aboveEntry);        // Not present in XML

    // Check second articulation assignment
    auto artic2 = details->get<details::ArticulationAssign>(SCORE_PARTID, 125, 1);
    ASSERT_TRUE(artic2) << "ArticulationAssign with entnum=125, inci=1 not found but does exist";
    EXPECT_EQ(artic2->articDef, 17);         // From XML
    EXPECT_EQ(artic2->horzOffset, 16);       // From XML
    EXPECT_EQ(artic2->horzAdd, 204);         // From XML
    EXPECT_EQ(artic2->vertOffset, 51);       // From XML
    EXPECT_TRUE(artic2->hide);               // From XML
    EXPECT_TRUE(artic2->neverStack);         // From XML
    EXPECT_TRUE(artic2->avoidSlur);          // From XML
    EXPECT_TRUE(artic2->overridePlacement);  // From XML
    EXPECT_TRUE(artic2->aboveEntry);         // From XML
    EXPECT_EQ(artic2->numSlursAvoided, -1);  // From XML

    // Ensure that an invalid articulation assignment isn't found
    auto articInvalid = details->get<details::ArticulationAssign>(SCORE_PARTID, 999, 0);
    EXPECT_FALSE(articInvalid) << "ArticulationAssign with entnum=999 found but does not exist";
}

TEST(ArticulationAssignTest, CalcSelectedSymbolContextHonorsOverridePlacement)
{
    std::vector<char> defXml;
    musxtest::readFile(musxtest::getInputPath() / "finale_maestro_default.enigmaxml", defXml);
    auto defDoc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(defXml);
    ASSERT_TRUE(defDoc);

    std::vector<char> entryXml;
    musxtest::readFile(musxtest::getInputPath() / "stemdirection.enigmaxml", entryXml);
    auto entryDoc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(entryXml);
    ASSERT_TRUE(entryDoc);

    auto gfhold = details::GFrameHoldContext(entryDoc, SCORE_PARTID, 3, 3);
    ASSERT_TRUE(gfhold);
    auto entryFrame = gfhold.createEntryFrame(0);
    ASSERT_TRUE(entryFrame);

    auto entryInfo = EntryInfoPtr(entryFrame, 0);
    ASSERT_TRUE(entryInfo);
    ASSERT_TRUE(entryInfo.calcUpStem());

    details::ArticulationAssign assign(defDoc, SCORE_PARTID, EnigmaBase::ShareMode::All, 0, 0);
    assign.articDef = 8;
    assign.overridePlacement = true;
    assign.aboveEntry = false;

    const auto context = assign.calcSelectedSymbolContext(entryInfo);
    ASSERT_TRUE(context);
    EXPECT_EQ(context->placement, VerticalPlacement::Below);
    EXPECT_TRUE(context->definition->belowSymbolAlt);
    EXPECT_TRUE(context->symbol.usesAlternate);
    EXPECT_EQ(context->symbol.character, context->definition->charAlt);
}

TEST(ArticulationAssignTest, CalcSelectedSymbolContextFloatsAboveEntryInMultiLayerStemSideContexts)
{
    std::vector<char> defXml;
    musxtest::readFile(musxtest::getInputPath() / "finale_maestro_default.enigmaxml", defXml);
    auto defDoc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(defXml);
    ASSERT_TRUE(defDoc);

    std::vector<char> entryXml;
    musxtest::readFile(musxtest::getInputPath() / "stemdirection.enigmaxml", entryXml);
    auto entryDoc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(entryXml);
    ASSERT_TRUE(entryDoc);

    auto gfhold = details::GFrameHoldContext(entryDoc, SCORE_PARTID, 3, 3);
    ASSERT_TRUE(gfhold);
    auto upperFrame = gfhold.createEntryFrame(0);
    auto lowerFrame = gfhold.createEntryFrame(1);
    ASSERT_TRUE(upperFrame);
    ASSERT_TRUE(lowerFrame);

    auto upperEntry = EntryInfoPtr(upperFrame, 0);
    auto lowerEntry = EntryInfoPtr(lowerFrame, 0);
    ASSERT_TRUE(upperEntry);
    ASSERT_TRUE(lowerEntry);
    ASSERT_TRUE(upperEntry.calcUpStem());
    ASSERT_FALSE(lowerEntry.calcUpStem());

    details::ArticulationAssign assign(defDoc, SCORE_PARTID, EnigmaBase::ShareMode::All, 0, 0);
    assign.articDef = 8;

    const auto upperContext = assign.calcSelectedSymbolContext(upperEntry);
    ASSERT_TRUE(upperContext);
    EXPECT_EQ(upperContext->placement, VerticalPlacement::Float);
    EXPECT_FALSE(upperContext->symbol.usesAlternate);
    EXPECT_EQ(upperContext->symbol.character, upperContext->definition->charMain);

    const auto lowerContext = assign.calcSelectedSymbolContext(lowerEntry);
    ASSERT_TRUE(lowerContext);
    EXPECT_EQ(lowerContext->placement, VerticalPlacement::Float);
    EXPECT_TRUE(lowerContext->symbol.usesAlternate);
    EXPECT_EQ(lowerContext->symbol.character, lowerContext->definition->charAlt);
}

TEST(ArticulationAssignTest, CalcSelectedSymbolContextKeepsBelowEntryFixedInMultiLayerStemSideContexts)
{
    constexpr static musxtest::string_view defXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <articDef cmper="42">
      <charMain>58960</charMain>
      <sizeMain>24</sizeMain>
      <autoHorz/>
      <autoVert/>
      <autoVertMode>belowEntry</autoVertMode>
      <outsideStaff/>
      <autoStack/>
      <slurInteractionMode>avoidSlur</slurInteractionMode>
      <charAlt>58961</charAlt>
      <defVertPos>24</defVertPos>
      <isStemSideWhenMultipleLayers/>
      <sizeAlt>24</sizeAlt>
    </articDef>
  </others>
</finale>
    )xml";

    auto defDoc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(defXml);
    ASSERT_TRUE(defDoc);

    std::vector<char> entryXml;
    musxtest::readFile(musxtest::getInputPath() / "stemdirection.enigmaxml", entryXml);
    auto entryDoc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(entryXml);
    ASSERT_TRUE(entryDoc);

    auto gfhold = details::GFrameHoldContext(entryDoc, SCORE_PARTID, 3, 3);
    ASSERT_TRUE(gfhold);
    auto upperFrame = gfhold.createEntryFrame(0);
    auto lowerFrame = gfhold.createEntryFrame(1);
    ASSERT_TRUE(upperFrame);
    ASSERT_TRUE(lowerFrame);

    auto upperEntry = EntryInfoPtr(upperFrame, 0);
    auto lowerEntry = EntryInfoPtr(lowerFrame, 0);
    ASSERT_TRUE(upperEntry);
    ASSERT_TRUE(lowerEntry);
    ASSERT_TRUE(upperEntry.calcUpStem());
    ASSERT_FALSE(lowerEntry.calcUpStem());

    details::ArticulationAssign assign(defDoc, SCORE_PARTID, EnigmaBase::ShareMode::All, 0, 0);
    assign.articDef = 42;

    const auto upperContext = assign.calcSelectedSymbolContext(upperEntry);
    ASSERT_TRUE(upperContext);
    EXPECT_EQ(upperContext->placement, VerticalPlacement::Below);
    EXPECT_FALSE(upperContext->symbol.usesAlternate);
    EXPECT_EQ(upperContext->symbol.character, upperContext->definition->charMain);

    const auto lowerContext = assign.calcSelectedSymbolContext(lowerEntry);
    ASSERT_TRUE(lowerContext);
    EXPECT_EQ(lowerContext->placement, VerticalPlacement::Below);
    EXPECT_FALSE(lowerContext->symbol.usesAlternate);
    EXPECT_EQ(lowerContext->symbol.character, lowerContext->definition->charMain);
}

namespace {

class ArticulationHandlePositionTest : public ::testing::Test
{
protected:
    static constexpr Cmper staffId = 1;
    static constexpr MeasCmper measureId = 1;
    static constexpr Cmper definitionId = 1;
    static constexpr size_t chordIndex = 1;
    static constexpr Evpu standardLineSpace = 24;
    static constexpr int bottomPosition = -8;
    static constexpr int topPosition = -4;

    DocumentPtr document;
    EntryInfoPtr entryInfo;
    std::shared_ptr<Entry> entry;
    std::shared_ptr<others::ArticulationDef> definition;
    std::shared_ptr<others::StaffComposite> staff;
    std::unique_ptr<details::ArticulationAssign> assignment;

    void SetUp() override
    {
        std::vector<char> xml;
        musxtest::readFile(musxtest::getInputPath() / "lvshapes.enigmaxml", xml);
        document = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
        ASSERT_TRUE(document);
        const details::GFrameHoldContext context(document, SCORE_PARTID, staffId, measureId);
        ASSERT_TRUE(context);
        const auto frame = context.createEntryFrame(0);
        ASSERT_TRUE(frame);
        entryInfo = EntryInfoPtr(frame, chordIndex);
        ASSERT_TRUE(entryInfo);
        entry = std::const_pointer_cast<Entry>(entryInfo->getEntry());
        definition = std::const_pointer_cast<others::ArticulationDef>(
            document->getOthers()->get<others::ArticulationDef>(SCORE_PARTID, definitionId));
        ASSERT_TRUE(definition);
        staff = std::const_pointer_cast<others::StaffComposite>(entryInfo.createCurrentStaff());
        ASSERT_TRUE(staff);
        ASSERT_EQ(staff->lineSpace, standardLineSpace);
        ASSERT_EQ(entryInfo.calcTopBottomStaffPositions(), std::make_pair(topPosition, bottomPosition));
        assignment = std::make_unique<details::ArticulationAssign>(document, SCORE_PARTID,
            EnigmaBase::ShareMode::All, entry->getEntryNumber(), 0);
        assignment->articDef = definitionId;
    }

    void expectPosition(double expected)
    {
        const auto result = assignment->calcHandleStaffPosition(entryInfo);
        ASSERT_TRUE(result);
        EXPECT_DOUBLE_EQ(*result, expected);
    }

    using Anchor = details::ArticulationAssign::HorizontalOffset::Anchor;

    void expectHorizontalOffset(double expected, Anchor anchor = Anchor::EntryOrigin)
    {
        const auto result = assignment->calcHorizontalOffset(entryInfo);
        ASSERT_TRUE(result);
        EXPECT_DOUBLE_EQ(result->offset, expected);
        EXPECT_EQ(result->anchor, anchor);
    }
};

TEST_F(ArticulationHandlePositionTest, AddsHorizontalAssignmentAndDefinitionOffsets)
{
    constexpr Evpu displacement = 19;
    for (Evpu assignmentOffset : {0, displacement, -displacement}) {
        for (Evpu definitionOffset : {0, displacement, -displacement}) {
            assignment->horzOffset = assignmentOffset;
            definition->xOffsetMain = definitionOffset;
            expectHorizontalOffset(static_cast<double>(assignmentOffset) + definitionOffset);
        }
    }
    // Conversion must precede addition, even when the stored values span the EVPU range.
    constexpr Evpu maximumOffset = (std::numeric_limits<Evpu>::max)();
    assignment->horzOffset = maximumOffset;
    definition->xOffsetMain = maximumOffset;
    expectHorizontalOffset(static_cast<double>(maximumOffset) + maximumOffset);
}

TEST_F(ArticulationHandlePositionTest, HorizontalOffsetUsesSymbolSelectedBeforeDefinitionOffsets)
{
    constexpr Evpu mainOffset = -17;
    constexpr Evpu alternateOffset = 23;
    definition->xOffsetMain = mainOffset;
    definition->xOffsetAlt = alternateOffset;
    definition->aboveSymbolAlt = true;
    definition->belowSymbolAlt = false;
    definition->yOffsetAlt = -standardLineSpace;
    definition->yOffsetMain = standardLineSpace;
    assignment->overridePlacement = true;
    for (bool aboveEntry : {false, true}) {
        assignment->aboveEntry = aboveEntry;
        for (Evpu verticalOffset : {-standardLineSpace, 0, standardLineSpace}) {
            assignment->vertOffset = verticalOffset;
            expectHorizontalOffset(verticalOffset >= 0 ? alternateOffset : mainOffset);
        }
    }
}

TEST_F(ArticulationHandlePositionTest, HorizontalOffsetDistinguishesCoordinateAndSymbolCertainty)
{
    constexpr Evpu mainOffset = 13;
    constexpr Evpu alternateOffset = -21;
    definition->aboveSymbolAlt = true;
    definition->belowSymbolAlt = false;
    definition->xOffsetMain = mainOffset;
    assignment->avoidSlur = true; // Slur avoidance keeps the vertical position, and thus the symbol, uncertain.
    for (bool* flag : {&definition->autoVert, &definition->outsideStaff, &definition->avoidStaffLines}) {
        *flag = true;
        definition->xOffsetAlt = mainOffset;
        const auto context = assignment->calcSelectedSymbolContext(entryInfo);
        ASSERT_TRUE(context);
        EXPECT_TRUE(context->symbolIsEstimate);
        expectHorizontalOffset(mainOffset);
        definition->xOffsetAlt = alternateOffset;
        EXPECT_FALSE(assignment->calcHorizontalOffset(entryInfo));
        *flag = false;
    }
    definition->autoVert = true;
    definition->belowSymbolAlt = true;
    expectHorizontalOffset(alternateOffset);
    definition->aboveSymbolAlt = false;
    definition->belowSymbolAlt = false;
    expectHorizontalOffset(mainOffset);
}

TEST_F(ArticulationHandlePositionTest, HorizontalOffsetCentersOnNoteheadsAndDeclinesStemCenteringAndCopying)
{
    constexpr Evpu displacement = 19;
    assignment->horzOffset = displacement;
    definition->autoHorz = true;
    expectHorizontalOffset(displacement, Anchor::NoteheadCenter);
    // Stem centering is not supported, with or without automatic horizontal positioning.
    definition->centerOnStem = true;
    EXPECT_FALSE(assignment->calcHorizontalOffset(entryInfo));
    definition->autoHorz = false;
    EXPECT_FALSE(assignment->calcHorizontalOffset(entryInfo));
    definition->centerOnStem = false;
    assignment->horzOffset = 0;
    using AD = others::ArticulationDef;
    for (auto mode : {AD::CopyMode::Vertical, AD::CopyMode::Horizontal}) {
        definition->copyMode = mode;
        EXPECT_FALSE(assignment->calcHorizontalOffset(entryInfo));
    }
    definition->copyMode = AD::CopyMode::None;
    expectHorizontalOffset(0);
}

TEST_F(ArticulationHandlePositionTest, HorizontalOffsetDoesNotRequireVerticalPositionSupport)
{
    constexpr Evpu displacement = -19;
    assignment->horzOffset = displacement;
    for (bool* flag : {&assignment->avoidSlur, &definition->insideSlur, &entry->graceNote, &entry->crossStaff}) {
        *flag = true;
        expectHorizontalOffset(displacement);
        *flag = false;
    }
    definition->slurInteractionMode = others::ArticulationDef::SlurInteractionMode::AvoidSlur;
    expectHorizontalOffset(displacement);
    for (auto style : {others::Staff::NotationStyle::Percussion, others::Staff::NotationStyle::Tablature}) {
        staff->notationStyle = style;
        expectHorizontalOffset(displacement);
    }
    staff->altNotation = others::Staff::AlternateNotation::Rhythmic;
    expectHorizontalOffset(displacement);
    entry->isNote = false;
    expectHorizontalOffset(displacement);
    entry->notes.clear();
    expectHorizontalOffset(displacement);
}

TEST_F(ArticulationHandlePositionTest, HorizontalOffsetIgnoresGlyphMetricsSpacingAndResizing)
{
    constexpr Evpu displacement = 19;
    constexpr int largeFontSize = 72;
    constexpr int reducedPercent = 65;
    constexpr Cmper systemId = 1;
    assignment->horzOffset = displacement;
    auto entrySize = std::make_shared<details::EntrySize>(document, SCORE_PARTID,
        EnigmaBase::ShareMode::All, entry->getEntryNumber());
    entrySize->percent = reducedPercent;
    document->getDetails()->add(details::EntrySize::XmlNodeName, entrySize);
    auto staffSize = std::make_shared<details::StaffSize>(document, SCORE_PARTID,
        EnigmaBase::ShareMode::None, systemId, staffId);
    staffSize->staffPercent = reducedPercent;
    document->getDetails()->add(details::StaffSize::XmlNodeName, staffSize);
    auto noteAlteration = std::make_shared<details::NoteAlterations>(document, SCORE_PARTID,
        EnigmaBase::ShareMode::All, entry->getEntryNumber(), 0);
    noteAlteration->noteId = entry->notes.front()->getNoteId();
    noteAlteration->percent = reducedPercent;
    noteAlteration->nxdisp = displacement;
    noteAlteration->allowVertPos = true;
    noteAlteration->nydisp = displacement;
    document->getDetails()->add(details::NoteAlterations::XmlNodeName, noteAlteration);
    entry->noteDetail = true;
    definition->fontMain->fontSize = largeFontSize;
    definition->fontMain->absolute = true;
    definition->defVertPos = displacement;
    staff->lineSpace = 0;
    expectHorizontalOffset(displacement);
    definition->mainIsShape = true;
    expectHorizontalOffset(displacement);
}

TEST_F(ArticulationHandlePositionTest, SelectsManualSymbolBeforeOffsetsAndIgnoresPlacementOverride)
{
    definition->aboveSymbolAlt = true;
    definition->belowSymbolAlt = false;
    definition->yOffsetAlt = -standardLineSpace;
    assignment->overridePlacement = true;
    assignment->aboveEntry = false;
    for (bool useTopNote : {false, true}) {
        definition->useTopNote = useTopNote;
        for (Evpu offset : {0, standardLineSpace, -standardLineSpace}) {
            assignment->vertOffset = offset;
            const auto result = assignment->calcSelectedSymbolContext(entryInfo);
            ASSERT_TRUE(result);
            EXPECT_EQ(result->symbol.usesAlternate, offset >= 0);
            EXPECT_FALSE(result->symbolIsEstimate);
            EXPECT_EQ(result->placement, VerticalPlacement::Below);
        }
    }
}

TEST_F(ArticulationHandlePositionTest, MarksLayoutDependentSymbolSelectionAsEstimate)
{
    definition->aboveSymbolAlt = true;
    definition->belowSymbolAlt = false;
    auto expectEstimate = [&](bool expected) {
        const auto result = assignment->calcSelectedSymbolContext(entryInfo);
        ASSERT_TRUE(result);
        EXPECT_EQ(result->symbolIsEstimate, expected);
    };
    expectEstimate(false);
    assignment->avoidSlur = true; // Slur avoidance depends on layout.
    expectEstimate(false);
    definition->outsideStaff = true;
    expectEstimate(true);
    definition->outsideStaff = false;
    definition->avoidStaffLines = true;
    expectEstimate(true);
    definition->avoidStaffLines = false;
    definition->autoVert = true;
    expectEstimate(true);
    definition->belowSymbolAlt = true;
    expectEstimate(false); // Layout cannot change the selected symbol when both sides agree.
}

TEST_F(ArticulationHandlePositionTest, CopyingUsesMainCharacterRegardlessOfSideOrShapeFlags)
{
    using AD = others::ArticulationDef;
    constexpr char32_t mainCharacter = U'a';
    constexpr char32_t alternateCharacter = U'b';
    definition->charMain = mainCharacter;
    definition->charAlt = alternateCharacter;
    definition->aboveSymbolAlt = true;
    definition->belowSymbolAlt = true;
    definition->mainIsShape = true;
    definition->altIsShape = true;
    definition->xOffsetMain = standardLineSpace;
    definition->yOffsetMain = -standardLineSpace;
    for (auto mode : {AD::CopyMode::Horizontal, AD::CopyMode::Vertical}) {
        definition->copyMode = mode;
        for (Evpu offset : {standardLineSpace, -standardLineSpace}) {
            assignment->vertOffset = offset;
            const auto result = assignment->calcSelectedSymbolContext(entryInfo);
            ASSERT_TRUE(result);
            EXPECT_FALSE(result->symbolIsEstimate);
            EXPECT_FALSE(result->symbol.usesAlternate);
            EXPECT_FALSE(result->symbol.isShape);
            EXPECT_EQ(result->symbol.shapeId, 0);
            EXPECT_EQ(result->symbol.character, mainCharacter);
            EXPECT_EQ(result->symbol.font, definition->fontMain);
            EXPECT_EQ(result->symbol.xOffset, standardLineSpace);
            EXPECT_EQ(result->symbol.yOffset, -standardLineSpace);
        }
    }
}

TEST_F(ArticulationHandlePositionTest, PredictsAutomaticReferenceSideFromForcedStemStatus)
{
    using Mode = others::ArticulationDef::AutoVerticalMode;
    definition->autoVert = true;
    definition->aboveSymbolAlt = true;
    definition->belowSymbolAlt = false;
    assignment->avoidSlur = true; // Slur avoidance makes every mode fall back to the placement policy.
    const auto frame = entryInfo.getFrame();
    for (bool forced : {false, true}) {
        for (bool stemUp : {false, true}) {
            entry->freezeStemScore = forced;
            entry->upStemScore = stemUp;
            const EntryInfoPtr freshEntry(frame, chordIndex);
            ASSERT_EQ(freshEntry.calcStemDirectionForced(), forced
                ? (stemUp ? StemDirection::AlwaysUp : StemDirection::AlwaysDown) : StemDirection::Default);
            const bool actualStemUp = freshEntry.calcUpStem();
            for (auto mode : {Mode::AlwaysNoteheadSide, Mode::AutoNoteStem, Mode::StemSide,
                    Mode::AlwaysOnStem, Mode::AboveEntry, Mode::BelowEntry}) {
                definition->autoVertMode = mode;
                bool expectedAbove = actualStemUp;
                switch (mode) {
                case Mode::AlwaysNoteheadSide: expectedAbove = !actualStemUp; break;
                case Mode::AutoNoteStem: expectedAbove = forced ? stemUp : !actualStemUp; break;
                case Mode::AboveEntry: expectedAbove = true; break;
                case Mode::BelowEntry: expectedAbove = false; break;
                default: break;
                }
                const auto result = assignment->calcSelectedSymbolContext(freshEntry);
                ASSERT_TRUE(result);
                EXPECT_EQ(result->symbol.usesAlternate, expectedAbove);
                EXPECT_TRUE(result->symbolIsEstimate);
            }
        }
    }
}

TEST_F(ArticulationHandlePositionTest, AboveEntryRequiresForcedDownStemForMultilayerException)
{
    using Mode = others::ArticulationDef::AutoVerticalMode;
    definition->autoVert = true;
    definition->autoVertMode = Mode::AboveEntry;
    definition->aboveSymbolAlt = true;
    definition->belowSymbolAlt = false;
    assignment->avoidSlur = true; // Slur avoidance makes every mode fall back to the placement policy.
    constexpr MeasCmper downstemMeasureId = 3;
    const details::GFrameHoldContext context(document, SCORE_PARTID, staffId, downstemMeasureId);
    const auto frame = context.createEntryFrame(0);
    ASSERT_TRUE(frame);
    auto layerAttributes = std::const_pointer_cast<others::LayerAttributes>(frame->getLayerAttributes());
    ASSERT_TRUE(layerAttributes);
    layerAttributes->freezeLayer = false; // Keep synthetic extra layers from forcing stems implicitly.
    const EntryInfoPtr downEntry(frame, 0);
    ASSERT_FALSE(downEntry.calcUpStem());
    ASSERT_EQ(downEntry.calcStemDirectionForced(), StemDirection::Default);
    for (auto mode : {Mode::AlwaysNoteheadSide, Mode::AutoNoteStem}) {
        definition->autoVertMode = mode;
        const auto result = assignment->calcSelectedSymbolContext(downEntry);
        ASSERT_TRUE(result);
        EXPECT_TRUE(result->symbol.usesAlternate);
        EXPECT_TRUE(result->symbolIsEstimate);
    }
    definition->autoVertMode = Mode::AboveEntry;
    auto rawEntry = std::const_pointer_cast<Entry>(downEntry->getEntry());
    auto hold = std::const_pointer_cast<details::GFrameHold>(
        document->getDetails()->get<details::GFrameHold>(SCORE_PARTID, staffId, downstemMeasureId));
    ASSERT_TRUE(hold);
    constexpr size_t secondLayer = 1;
    for (bool multipleLayers : {false, true}) {
        hold->frames[secondLayer] = multipleLayers ? hold->frames.front() : 0;
        ASSERT_EQ(context->calcIsMultiLayer(), multipleLayers);
        for (bool modifier : {false, true}) {
            definition->isStemSideWhenMultipleLayers = modifier;
            for (bool forced : {false, true}) {
                rawEntry->freezeStemScore = forced;
                rawEntry->upStemScore = false;
                const EntryInfoPtr freshEntry(frame, 0);
                ASSERT_EQ(freshEntry.calcStemDirectionForced(), forced ? StemDirection::AlwaysDown : StemDirection::Default);
                const auto result = assignment->calcSelectedSymbolContext(freshEntry);
                ASSERT_TRUE(result);
                EXPECT_EQ(result->symbol.usesAlternate, !(multipleLayers && modifier && forced));
                EXPECT_TRUE(result->symbolIsEstimate);
            }
        }
    }
}

TEST_F(ArticulationHandlePositionTest, MatchesMeasuredManualPositionsRegardlessOfStem)
{
    struct Measurement
    {
        bool useTopNote;
        Evpu offset;
        Evpu handleY;
    };
    // Independently reported Finale coordinates.
    constexpr Measurement measurements[] = {
        {false, 0, -96}, {true, 0, -48},
        {false, 24, -72}, {true, 24, -24},
        {false, -24, -120}, {true, -24, -72}
    };
    for (bool stemUp : {false, true}) {
        entry->freezeStemScore = true;
        entry->upStemScore = stemUp;
        for (const auto& measurement : measurements) {
            definition->useTopNote = measurement.useTopNote;
            assignment->vertOffset = measurement.offset;
            expectPosition(measurement.handleY / (static_cast<double>(standardLineSpace) / 2));
        }
    }
}

TEST_F(ArticulationHandlePositionTest, AddsSelectedSymbolOffsetBeforeConvertingToStaffPositions)
{
    constexpr Evpu symbolOffset = 12;
    constexpr Evpu fractionalOffset = 3;
    constexpr Evpu alternateOffset = -60;
    constexpr double fractionalPosition = -6.75;
    constexpr double crossedPosition = -11;
    definition->mainIsShape = false;
    definition->yOffsetMain = symbolOffset;
    assignment->vertOffset = fractionalOffset;
    expectPosition(fractionalPosition);
    definition->mainIsShape = true;
    expectPosition(fractionalPosition);

    definition->aboveSymbolAlt = true;
    definition->yOffsetAlt = alternateOffset;
    assignment->vertOffset = standardLineSpace;
    expectPosition(crossedPosition); // Symbol offset crosses the reference without reselecting the symbol.
    assignment->vertOffset = 0;
    expectPosition(bottomPosition + alternateOffset / (static_cast<double>(standardLineSpace) / 2));
    definition->aboveSymbolAlt = false;
    definition->belowSymbolAlt = true;
    assignment->vertOffset = -standardLineSpace;
    expectPosition(bottomPosition + (-standardLineSpace + alternateOffset) / (static_cast<double>(standardLineSpace) / 2));
}

TEST_F(ArticulationHandlePositionTest, UsesActualLineSpacingAndPreservesZeroReference)
{
    constexpr Evpu widerLineSpace = 48;
    constexpr Evpu oddLineSpace = 25;
    constexpr int customLineCount = 1;
    for (Evpu spacing : {standardLineSpace, widerLineSpace, oddLineSpace}) {
        staff->lineSpace = spacing;
        staff->staffLines = customLineCount;
        assignment->vertOffset = 0;
        expectPosition(bottomPosition);
        assignment->vertOffset = standardLineSpace;
        expectPosition(bottomPosition + standardLineSpace / (static_cast<double>(spacing) / 2));
    }
}

TEST_F(ArticulationHandlePositionTest, IgnoresDefaultVerticalPositionAndFontSize)
{
    constexpr Evpu defaultOffset = 96;
    constexpr int largeFontSize = 72;
    definition->defVertPos = defaultOffset;
    definition->fontMain->fontSize = largeFontSize;
    definition->fontMain->absolute = true;
    expectPosition(bottomPosition);
}

TEST_F(ArticulationHandlePositionTest, DeclinesUnsupportedPlacement)
{
    for (bool* flag : {&definition->insideSlur, &assignment->overridePlacement, &assignment->avoidSlur, &entry->crossStaff}) {
        *flag = true;
        EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
        *flag = false;
    }
    using AD = others::ArticulationDef;
    for (auto mode : {AD::CopyMode::Vertical, AD::CopyMode::Horizontal}) {
        definition->copyMode = mode;
        EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
    }
    definition->copyMode = AD::CopyMode::None;
    definition->slurInteractionMode = AD::SlurInteractionMode::AvoidSlur;
    EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
    definition->slurInteractionMode = AD::SlurInteractionMode::Ignore;
    for (auto style : {others::Staff::NotationStyle::Percussion, others::Staff::NotationStyle::Tablature}) {
        staff->notationStyle = style;
        EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
    }
    staff->notationStyle = others::Staff::NotationStyle::Standard;
    staff->altNotation = others::Staff::AlternateNotation::Rhythmic;
    EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
    staff->altNotation = others::Staff::AlternateNotation::Normal;
    entry->notes.clear();
    EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
}

TEST_F(ArticulationHandlePositionTest, IgnoresStaffEntryAndNoteheadSizePercentages)
{
    constexpr Cmper systemId = 1;
    constexpr int reducedPercent = 65;
    constexpr int normalPercent = 100;
    constexpr int enlargedPercent = 150;
    auto entrySize = std::make_shared<details::EntrySize>(document, SCORE_PARTID,
        EnigmaBase::ShareMode::All, entry->getEntryNumber());
    auto staffSize = std::make_shared<details::StaffSize>(document, SCORE_PARTID,
        EnigmaBase::ShareMode::None, systemId, staffId);
    auto noteAlteration = std::make_shared<details::NoteAlterations>(document, SCORE_PARTID,
        EnigmaBase::ShareMode::All, entry->getEntryNumber(), 0);
    noteAlteration->noteId = entry->notes.front()->getNoteId();
    document->getDetails()->add(details::EntrySize::XmlNodeName, entrySize);
    document->getDetails()->add(details::StaffSize::XmlNodeName, staffSize);
    document->getDetails()->add(details::NoteAlterations::XmlNodeName, noteAlteration);
    entry->noteDetail = true;
    entrySize->percent = normalPercent;
    staffSize->staffPercent = normalPercent;
    noteAlteration->percent = normalPercent;
    for (int* size : {&entrySize->percent, &staffSize->staffPercent, &noteAlteration->percent}) {
        for (int percent : {reducedPercent, normalPercent, enlargedPercent}) {
            *size = percent;
            assignment->vertOffset = 0;
            expectPosition(bottomPosition);
            assignment->vertOffset = standardLineSpace;
            expectPosition(bottomPosition + 2);
        }
        *size = normalPercent;
    }
    noteAlteration->allowVertPos = true;
    noteAlteration->nydisp = standardLineSpace;
    EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
}

TEST_F(ArticulationHandlePositionTest, PositionsGraceNotesWithoutScalingOffsets)
{
    entry->graceNote = true;
    assignment->vertOffset = 0;
    expectPosition(bottomPosition);
    assignment->vertOffset = standardLineSpace;
    expectPosition(bottomPosition + 2);
}

TEST_F(ArticulationHandlePositionTest, ReportsInvalidDefinitionAndSpacing)
{
    assignment->articDef = 0;
    EXPECT_THROW((void)assignment->calcHorizontalOffset(entryInfo), integrity_error);
    EXPECT_THROW((void)assignment->calcHandleStaffPosition(entryInfo), integrity_error);
    assignment->articDef = definitionId;
    staff->lineSpace = 0;
    EXPECT_THROW((void)assignment->calcHandleStaffPosition(entryInfo), integrity_error);
}

} // namespace

namespace {

class ArticulationAutomaticHandlePositionTest : public ArticulationHandlePositionTest
{
protected:
    static constexpr int noteheadTop = topPosition + 1;
    static constexpr int noteheadBottom = bottomPosition - 1;
    static constexpr Evpu mainSymbolOffset = -21;
    static constexpr Evpu alternateSymbolOffset = 3;

    void SetUp() override
    {
        ArticulationHandlePositionTest::SetUp();
        definition->defVertPos = 0;
        definition->yOffsetMain = mainSymbolOffset;
        definition->yOffsetAlt = alternateSymbolOffset;
        definition->aboveSymbolAlt = false;
        definition->belowSymbolAlt = true;
    }

    void setStem(bool stemUp)
    {
        entry->freezeStemScore = true;
        entry->upStemScore = stemUp;
        entryInfo = EntryInfoPtr(entryInfo.getFrame(), chordIndex);
        ASSERT_EQ(entryInfo.calcUpStem(), stemUp);
    }

    static double toPositions(Evpu evpu)
    {
        return evpu / (static_cast<double>(standardLineSpace) / 2);
    }

    void expectSymbol(bool usesAlternate)
    {
        const auto context = assignment->calcSelectedSymbolContext(entryInfo);
        ASSERT_TRUE(context);
        EXPECT_FALSE(context->symbolIsEstimate);
        EXPECT_EQ(context->symbol.usesAlternate, usesAlternate);
    }
};

TEST_F(ArticulationAutomaticHandlePositionTest, MeasuresNoteheadSideFromNoteheadEdge)
{
    using Mode = others::ArticulationDef::AutoVerticalMode;
    constexpr Evpu defaultOffset = 24;
    constexpr Evpu assignmentOffset = 12;
    definition->autoVert = true;
    definition->defVertPos = defaultOffset;
    assignment->vertOffset = assignmentOffset;
    definition->autoVertMode = Mode::AlwaysNoteheadSide;
    setStem(true);
    expectSymbol(true);
    expectPosition(noteheadBottom - toPositions(defaultOffset + assignmentOffset) + toPositions(alternateSymbolOffset));
    setStem(false);
    expectSymbol(false);
    expectPosition(noteheadTop + toPositions(defaultOffset + assignmentOffset) + toPositions(mainSymbolOffset));
    // Automatic notehead/stem placement uses the notehead side only when the stem direction is not forced.
    definition->autoVertMode = Mode::AutoNoteStem;
    entry->freezeStemScore = false;
    entryInfo = EntryInfoPtr(entryInfo.getFrame(), chordIndex);
    ASSERT_EQ(entryInfo.calcStemDirectionForced(), StemDirection::Default);
    if (entryInfo.calcUpStem()) {
        expectPosition(noteheadBottom - toPositions(defaultOffset + assignmentOffset) + toPositions(alternateSymbolOffset));
    } else {
        expectPosition(noteheadTop + toPositions(defaultOffset + assignmentOffset) + toPositions(mainSymbolOffset));
    }
    setStem(true);
    EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo)); // A forced up-stem places it on the stem side.
    definition->autoVertMode = Mode::BelowEntry;
    setStem(true);
    expectPosition(noteheadBottom - toPositions(defaultOffset + assignmentOffset) + toPositions(alternateSymbolOffset));
    definition->autoVertMode = Mode::AboveEntry;
    setStem(false);
    expectPosition(noteheadTop + toPositions(defaultOffset + assignmentOffset) + toPositions(mainSymbolOffset));
}

TEST_F(ArticulationAutomaticHandlePositionTest, SelectsSymbolFromPositionRelativeToReferenceNote)
{
    // A below-placed articulation dragged above its reference note uses the above-side symbol.
    constexpr Evpu raisedOffset = -48;
    definition->autoVert = true;
    assignment->vertOffset = raisedOffset;
    setStem(true);
    const double position = noteheadBottom - toPositions(raisedOffset);
    ASSERT_GT(position, bottomPosition);
    ASSERT_LT(position, topPosition);
    definition->useTopNote = false;
    expectSymbol(false);
    expectPosition(position + toPositions(mainSymbolOffset));
    definition->useTopNote = true;
    expectSymbol(true);
    expectPosition(position + toPositions(alternateSymbolOffset));
}

TEST_F(ArticulationAutomaticHandlePositionTest, HonorsPlacementOverrideOnNoteheadSide)
{
    definition->autoVert = true;
    assignment->overridePlacement = true;
    assignment->aboveEntry = true;
    setStem(false);
    expectPosition(noteheadTop + toPositions(mainSymbolOffset));
    setStem(true);
    EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
    assignment->aboveEntry = false;
    expectPosition(noteheadBottom + toPositions(alternateSymbolOffset));
}

TEST_F(ArticulationAutomaticHandlePositionTest, TreatsStemlessEntriesAsNoteheadSideBothWays)
{
    using Mode = others::ArticulationDef::AutoVerticalMode;
    definition->autoVert = true;
    entry->duration = Edu(NoteType::Whole);
    for (bool stemUp : {false, true}) {
        setStem(stemUp);
        definition->autoVertMode = Mode::AboveEntry;
        expectPosition(noteheadTop + toPositions(mainSymbolOffset));
        definition->autoVertMode = Mode::BelowEntry;
        expectPosition(noteheadBottom + toPositions(alternateSymbolOffset));
    }
}

TEST_F(ArticulationAutomaticHandlePositionTest, DeclinesStemSidePlacementAndReverseStems)
{
    using Mode = others::ArticulationDef::AutoVerticalMode;
    definition->autoVert = true;
    setStem(true);
    for (auto mode : {Mode::StemSide, Mode::AlwaysOnStem, Mode::AboveEntry}) {
        definition->autoVertMode = mode;
        EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
        const auto context = assignment->calcSelectedSymbolContext(entryInfo);
        ASSERT_TRUE(context);
        EXPECT_TRUE(context->symbolIsEstimate);
    }
    definition->autoVertMode = Mode::AlwaysNoteheadSide;
    ASSERT_TRUE(assignment->calcHandleStaffPosition(entryInfo));
    for (bool* flag : {&entry->reverseUpStem, &entry->reverseDownStem}) {
        *flag = true;
        EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
        *flag = false;
    }
}

TEST_F(ArticulationAutomaticHandlePositionTest, ScalesNoteheadEdgesForGraceNotesAndResizedNoteheads)
{
    using Mode = others::ArticulationDef::AutoVerticalMode;
    definition->autoVert = true;
    definition->autoVertMode = Mode::AlwaysNoteheadSide;
    setStem(true);
    expectPosition(noteheadBottom + toPositions(alternateSymbolOffset));

    // The edge moves with the notehead size; the articulation offsets are not scaled.
    constexpr Evpu offset = 12;
    assignment->vertOffset = offset;
    const auto graceOptions = document->getOptions()->get<options::GraceNoteOptions>();
    ASSERT_TRUE(graceOptions);
    const double graceScale = static_cast<double>(graceOptions->gracePerc) / FULL_SIZE_PERCENT;
    ASSERT_LT(graceScale, 1.0);
    entry->graceNote = true;
    expectPosition(bottomPosition - STAFF_POSITIONS_PER_HALF_SPACE * graceScale - toPositions(offset)
        + toPositions(alternateSymbolOffset));
    entry->graceNote = false;

    constexpr int reducedPercent = 65;
    const auto lowest = *std::min_element(entry->notes.begin(), entry->notes.end(),
        [](const auto& a, const auto& b) { return a->harmLev < b->harmLev; });
    auto noteAlteration = std::make_shared<details::NoteAlterations>(document, SCORE_PARTID,
        EnigmaBase::ShareMode::All, entry->getEntryNumber(), 0);
    noteAlteration->noteId = lowest->getNoteId();
    noteAlteration->percent = reducedPercent;
    document->getDetails()->add(details::NoteAlterations::XmlNodeName, noteAlteration);
    entry->noteDetail = true;
    const double noteScale = static_cast<double>(reducedPercent) / FULL_SIZE_PERCENT;
    expectPosition(bottomPosition - STAFF_POSITIONS_PER_HALF_SPACE * noteScale - toPositions(offset)
        + toPositions(alternateSymbolOffset));
    entry->graceNote = true;
    expectPosition(bottomPosition - STAFF_POSITIONS_PER_HALF_SPACE * noteScale * graceScale - toPositions(offset)
        + toPositions(alternateSymbolOffset));
}

TEST_F(ArticulationAutomaticHandlePositionTest, DeclinesStackingWithOtherArticulations)
{
    definition->autoVert = true;
    definition->autoStack = true;
    setStem(true);
    ASSERT_TRUE(document->getDetails()->getArray<details::ArticulationAssign>(SCORE_PARTID, entry->getEntryNumber()).empty());
    expectPosition(noteheadBottom + toPositions(alternateSymbolOffset));
    constexpr Inci assignmentInci = 1;
    assignment = std::make_unique<details::ArticulationAssign>(document, SCORE_PARTID,
        EnigmaBase::ShareMode::All, entry->getEntryNumber(), assignmentInci);
    assignment->articDef = definitionId;
    auto other = std::make_shared<details::ArticulationAssign>(document, SCORE_PARTID,
        EnigmaBase::ShareMode::All, entry->getEntryNumber(), 0);
    other->articDef = definitionId;
    document->getDetails()->add(details::ArticulationAssign::XmlNodeName, other);
    entry->articDetail = true;
    EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
    assignment->neverStack = true;
    expectPosition(noteheadBottom + toPositions(alternateSymbolOffset));
    assignment->neverStack = false;
    definition->autoStack = false;
    expectPosition(noteheadBottom + toPositions(alternateSymbolOffset));
}

TEST_F(ArticulationAutomaticHandlePositionTest, PlacesArticulationsOutsideStaffOnNoteheadSide)
{
    constexpr int staffTop = 0;
    constexpr int staffBottom = -8;
    constexpr Evpu raisedOffset = 72;
    constexpr Evpu clearOffset = 120;
    definition->outsideStaff = true;
    definition->yOffsetAlt = 0;
    definition->yOffsetMain = 0;
    for (bool avoidStaffLines : {false, true}) {
        definition->avoidStaffLines = avoidStaffLines;
        setStem(true);
        assignment->vertOffset = 0;
        expectPosition((std::min)(noteheadBottom, staffBottom) - 1);
        expectSymbol(true);
        assignment->vertOffset = raisedOffset;
        EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
        setStem(false);
        expectPosition((std::max)(noteheadTop, staffTop) + 1);
        assignment->vertOffset = 0;
        EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
        assignment->vertOffset = clearOffset;
        expectPosition(bottomPosition + toPositions(clearOffset));
    }
}

TEST_F(ArticulationAutomaticHandlePositionTest, MovesArticulationsOffStaffLines)
{
    struct Case
    {
        Evpu offset;
        double expected;
    };
    // Relative to the bottom note on the bottom line of a five-line staff.
    constexpr Case cases[] = {
        {6, -7},    // within a space
        {0, -9},    // bottom line: away from the middle line
        {24, -7},   // line below the middle line
        {48, -3},   // middle line: the space above
        {72, -1},   // line above the middle line
        {96, 1},    // top line
        {120, 2},   // more than half a space above the staff: unchanged
        {-12, -9},  // already outside the staff: unchanged
    };
    definition->avoidStaffLines = true;
    definition->yOffsetAlt = 0;
    definition->yOffsetMain = 0;
    for (const auto& testCase : cases) {
        SCOPED_TRACE(testCase.offset);
        assignment->vertOffset = testCase.offset;
        expectPosition(testCase.expected);
    }
    assignment->vertOffset = 0;
    expectSymbol(true); // The adjusted position is below the reference note.
    constexpr Evpu middleLineOffset = 48;
    assignment->vertOffset = middleLineOffset;
    constexpr int evenLineCount = 4;
    staff->staffLines = evenLineCount;
    EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
    staff->staffLines = 0;
    EXPECT_FALSE(assignment->calcHandleStaffPosition(entryInfo));
}

} // namespace

namespace {

using HorizontalAnchor = details::ArticulationAssign::HorizontalOffset::Anchor;

TEST_F(ArticulationAutomaticHandlePositionTest, MeasuresManualPlacementOnRestsFromRestPosition)
{
    entry->isNote = false;
    entry->floatRest = true;
    entry->notes.clear();
    const int restPosition = entryInfo.calcTopBottomStaffPositions().first;
    definition->autoVert = false;
    definition->outsideStaff = false;
    definition->avoidStaffLines = false;
    constexpr Evpu offset = 30;
    assignment->vertOffset = offset;
    expectPosition(restPosition + toPositions(offset) + toPositions(mainSymbolOffset));
    expectSymbol(false);
    assignment->vertOffset = -offset;
    expectPosition(restPosition - toPositions(offset) + toPositions(alternateSymbolOffset));
    expectSymbol(true);

    assignment->vertOffset = 0;
    definition->avoidStaffLines = true;
    const auto avoided = assignment->calcHandleStaffPosition(entryInfo);
    ASSERT_TRUE(avoided);
    const double spaceCenter = *avoided - toPositions(mainSymbolOffset) - staff->calcMiddleStaffPosition();
    EXPECT_DOUBLE_EQ(std::fmod(std::abs(spaceCenter), STAFF_POSITIONS_PER_SPACE), STAFF_POSITIONS_PER_HALF_SPACE);
    definition->avoidStaffLines = false;

    // Automatic placement is measured from the estimated rest extent: a quarter rest spans 1.5 spaces either side
    // of its position, and a whole rest hangs between half a space and a space above it.
    using Mode = others::ArticulationDef::AutoVerticalMode;
    definition->autoVert = true;
    assignment->vertOffset = offset;
    entry->duration = Edu(NoteType::Quarter);
    definition->autoVertMode = Mode::AboveEntry;
    expectPosition(restPosition + 1.5 * STAFF_POSITIONS_PER_SPACE + toPositions(offset) + toPositions(mainSymbolOffset));
    definition->autoVertMode = Mode::BelowEntry;
    expectPosition(restPosition - 1.5 * STAFF_POSITIONS_PER_SPACE - toPositions(offset) + toPositions(alternateSymbolOffset));
    entry->duration = Edu(NoteType::Whole);
    const int wholeRestPosition = entryInfo.calcTopBottomStaffPositions().first;
    definition->autoVertMode = Mode::AboveEntry;
    expectPosition(wholeRestPosition + STAFF_POSITIONS_PER_SPACE + toPositions(offset) + toPositions(mainSymbolOffset));
    definition->autoVertMode = Mode::BelowEntry;
    expectPosition(wholeRestPosition + STAFF_POSITIONS_PER_HALF_SPACE - toPositions(offset)
        + toPositions(alternateSymbolOffset));
    // Rests have no stem, so placement is never on a stem side.
    definition->autoVertMode = Mode::StemSide;
    EXPECT_TRUE(assignment->calcHandleStaffPosition(entryInfo));

    definition->autoHorz = true;
    expectHorizontalOffset(0, HorizontalAnchor::NoteheadCenter);
}

TEST_F(ArticulationAutomaticHandlePositionTest, CentersOnPrimaryColumnRegardlessOfNoteLayout)
{
    constexpr Evpu displacement = -7;
    assignment->horzOffset = displacement;
    definition->autoHorz = true;
    expectHorizontalOffset(displacement, HorizontalAnchor::NoteheadCenter);
    // Move the highest note a second above the one below it.
    std::vector<std::shared_ptr<Note>> notes = entry->notes;
    ASSERT_GE(notes.size(), 2u);
    std::sort(notes.begin(), notes.end(), [](const auto& a, const auto& b) { return a->harmLev < b->harmLev; });
    const auto& upper = notes[notes.size() - 1];
    const auto& lower = notes[notes.size() - 2];
    upper->harmLev = lower->harmLev + 1;
    upper->upStemSecond = true;
    lower->downStemSecond = true;
    for (bool stemUp : {false, true}) {
        setStem(stemUp);
        expectHorizontalOffset(displacement, HorizontalAnchor::NoteheadCenter);
    }
    for (bool* flag : {&entry->crossStaff, &entry->notes.front()->crossStaff, &entry->reverseUpStem}) {
        *flag = true;
        expectHorizontalOffset(displacement, HorizontalAnchor::NoteheadCenter);
        *flag = false;
    }
    entry->duration = Edu(NoteType::Whole);
    expectHorizontalOffset(displacement, HorizontalAnchor::NoteheadCenter);
}

} // namespace
