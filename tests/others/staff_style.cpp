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

constexpr static musxtest::string_view staffStyleXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <staffStyle cmper="1">
      <customStaff>
        <staffLine>13</staffLine>
        <staffLine>11</staffLine>
      </customStaff>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <showNameParts/>
      <dwRestOffset>-4</dwRestOffset>
      <wRestOffset>-4</wRestOffset>
      <hRestOffset>-4</hRestOffset>
      <otherRestOffset>-4</otherRestOffset>
      <stemReversal>-4</stemReversal>
      <fullName>12</fullName>
      <abbrvName>13</abbrvName>
      <botRepeatDotOff>-5</botRepeatDotOff>
      <altNotation>slashBeats</altNotation>
      <altLayer>2</altLayer>
      <altHideArtics/>
      <altHideLyrics/>
      <altHideSmartShapes/>
      <altRhythmStemsUp/>
      <altSlashDots/>
      <altHideOtherNotes/>
      <altHideOtherArtics/>
      <altHideExpressions/>
      <altHideOtherLyrics/>
      <altHideOtherSmartShapes/>
      <altHideOtherExpressions/>
      <topRepeatDotOff>-3</topRepeatDotOff>
      <vertTabNumOff>-1024</vertTabNumOff>
      <stemDir>alwaysUp</stemDir>
      <styleName>Names and Stems</styleName>
      <addToMenu/>
      <mask>
        <defaultClef/>
        <floatKeys/>
        <floatTime/>
        <staffType/>
        <negNameScore/>
        <fullName/>
        <abrvName/>
        <altNotation/>
        <showStems/>
        <showNameParts/>
      </mask>
    </staffStyle>
    <staffStyleAssign cmper="6" inci="0">
      <style>1</style>
      <startMeas>1</startMeas>
      <endEdu>2147483647</endEdu>
      <startEdu>512</startEdu>
      <endMeas>4</endMeas>
    </staffStyleAssign>
    <staffStyle cmper="2">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <styleName>Legacy Notation Style</styleName>
      <mask>
        <notationStyle/>
      </mask>
    </staffStyle>
  </others>
</finale>
)xml";

TEST(StaffStyleTest, PopulateFields)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(staffStyleXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    // Retrieve StaffStyle with cmper=1
    auto staffStyle = others->get<others::StaffStyle>(SCORE_PARTID, 1);
    ASSERT_TRUE(staffStyle) << "StaffStyle with cmper 1 not found";

    EXPECT_FALSE(staffStyle->staffLines);
    ASSERT_TRUE(staffStyle->customStaff);
    ASSERT_EQ(staffStyle->customStaff.value().size(), 2);
    EXPECT_EQ(staffStyle->customStaff.value()[0], 11) << "factory did not sort customStaff elements";
    EXPECT_EQ(staffStyle->customStaff.value()[1], 13) << "factory did not sort customStaff elements";
    EXPECT_EQ(staffStyle->lineSpace, 24);
    EXPECT_EQ(staffStyle->instUuid, uuid::BlankStaff);
    EXPECT_TRUE(staffStyle->showNameInParts);
    EXPECT_EQ(staffStyle->dwRestOffset, -4);
    EXPECT_EQ(staffStyle->wRestOffset, -4);
    EXPECT_EQ(staffStyle->hRestOffset, -4);
    EXPECT_EQ(staffStyle->otherRestOffset, -4);
    EXPECT_EQ(staffStyle->stemReversal, -4);
    EXPECT_EQ(staffStyle->fullNameTextId, 12);
    EXPECT_EQ(staffStyle->abbrvNameTextId, 13);
    EXPECT_EQ(staffStyle->botRepeatDotOff, -5);
    EXPECT_EQ(staffStyle->altNotation, others::Staff::AlternateNotation::SlashBeats);
    EXPECT_EQ(staffStyle->altLayer, 2);
    EXPECT_TRUE(staffStyle->altHideArtics);
    EXPECT_TRUE(staffStyle->altHideLyrics);
    EXPECT_TRUE(staffStyle->altHideSmartShapes);
    EXPECT_TRUE(staffStyle->altRhythmStemsUp);
    EXPECT_TRUE(staffStyle->altSlashDots);
    EXPECT_TRUE(staffStyle->altHideOtherNotes);
    EXPECT_TRUE(staffStyle->altHideOtherArtics);
    EXPECT_TRUE(staffStyle->altHideExpressions);
    EXPECT_TRUE(staffStyle->altHideOtherLyrics);
    EXPECT_TRUE(staffStyle->altHideOtherSmartShapes);
    EXPECT_TRUE(staffStyle->altHideOtherExpressions);
    EXPECT_EQ(staffStyle->topRepeatDotOff, -3);
    EXPECT_EQ(staffStyle->vertTabNumOff, -1024);
    EXPECT_EQ(staffStyle->stemDirection, StemDirection::AlwaysUp);
    EXPECT_EQ(staffStyle->styleName, "Names and Stems");
    EXPECT_TRUE(staffStyle->addToMenu);

    // Verify mask properties
    ASSERT_TRUE(staffStyle->masks) << "Masks object is missing in StaffStyle";
    EXPECT_TRUE(staffStyle->masks->defaultClef);
    EXPECT_TRUE(staffStyle->masks->floatKeys);
    EXPECT_TRUE(staffStyle->masks->floatTime);
    EXPECT_TRUE(staffStyle->masks->staffType);
    EXPECT_TRUE(staffStyle->masks->negNameScore);
    EXPECT_TRUE(staffStyle->masks->fullName);
    EXPECT_TRUE(staffStyle->masks->abrvName);
    EXPECT_TRUE(staffStyle->masks->altNotation);
    EXPECT_TRUE(staffStyle->masks->showStems);
    EXPECT_TRUE(staffStyle->masks->showNameParts);
    EXPECT_FALSE(staffStyle->containsInstrumentChange());

    auto legacyNotationStyle = others->get<others::StaffStyle>(SCORE_PARTID, 2);
    ASSERT_TRUE(legacyNotationStyle);
    EXPECT_TRUE(legacyNotationStyle->containsInstrumentChange());
}

TEST(StaffStyleAssignTest, PopulateFields)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(staffStyleXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    // Retrieve StaffStyleAssign with cmper=6
    auto staffStyleAssign = others->get<others::StaffStyleAssign>(SCORE_PARTID, 6, 0);
    ASSERT_TRUE(staffStyleAssign) << "StaffStyleAssign with cmper 6 not found";

    EXPECT_EQ(staffStyleAssign->styleId, 1);
    EXPECT_EQ(staffStyleAssign->startMeas, 1);
    EXPECT_EQ(staffStyleAssign->startEdu, 512);
    EXPECT_EQ(staffStyleAssign->endMeas, 4);
    EXPECT_EQ(staffStyleAssign->endEdu, (std::numeric_limits<Edu>::max)());
}

TEST(StaffStyleInstrument, DetectInstrumentChange)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "inst_change.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 1, 0);
        ASSERT_TRUE(staff);
        EXPECT_EQ(staff->instUuid, uuid::Oboe);
    }

    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 2, 0);
        ASSERT_TRUE(staff);
        EXPECT_EQ(staff->instUuid, uuid::EnglishHorn);
    }

    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 3, 0);
        ASSERT_TRUE(staff);
        EXPECT_EQ(staff->instUuid, uuid::EnglishHorn);
    }
}

TEST(StaffStyleInstrument, InstrumentChangeIdentityReuseInPart1)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "inst_change2.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto part1Map = doc->createInstrumentMap(1);
    const auto* instrument = part1Map.getInstrumentForStaff(1);
    ASSERT_TRUE(instrument);

    const auto changes = instrument->getChanges();
    ASSERT_EQ(changes.size(), 6u);

    const auto rawIdentity = instrument->getInstrumentIdentityAt(MusicPoint{});
    const auto staffStyle28Identity = instrument->getInstrumentIdentityAt(MusicPoint(4, musx::util::Fraction{}));
    const auto rawAgainIdentity = instrument->getInstrumentIdentityAt(MusicPoint(7, musx::util::Fraction{}));
    const auto staffStyle29Identity = instrument->getInstrumentIdentityAt(MusicPoint(10, musx::util::Fraction{}));
    const auto staffStyle28AgainIdentity = instrument->getInstrumentIdentityAt(MusicPoint(13, musx::util::Fraction{}));
    const auto rawFinalIdentity = instrument->getInstrumentIdentityAt(MusicPoint(17, musx::util::Fraction{}));

    EXPECT_EQ(rawIdentity.instUuid, uuid::Flute);
    EXPECT_EQ(staffStyle28Identity.instUuid, uuid::ClarinetBFlat);
    EXPECT_EQ(staffStyle29Identity.instUuid, uuid::AltoSax);
    EXPECT_EQ(rawIdentity, rawAgainIdentity);
    EXPECT_EQ(rawIdentity, rawFinalIdentity);
    EXPECT_EQ(staffStyle28Identity, staffStyle28AgainIdentity);
    EXPECT_NE(rawIdentity, staffStyle28Identity);
    EXPECT_NE(staffStyle28Identity, staffStyle29Identity);

    const auto identities = instrument->getInstrumentIdentities();
    ASSERT_EQ(identities.size(), 3u);
    EXPECT_EQ(identities[0], rawIdentity);
    EXPECT_EQ(identities[1], staffStyle28Identity);
    EXPECT_EQ(identities[2], staffStyle29Identity);

    EXPECT_EQ(changes.at(MusicPoint{}).identity, rawIdentity);
    EXPECT_EQ(changes.at(MusicPoint(4, musx::util::Fraction{})).identity, staffStyle28Identity);
    EXPECT_EQ(changes.at(MusicPoint(7, musx::util::Fraction{})).identity, rawAgainIdentity);
    EXPECT_EQ(changes.at(MusicPoint(10, musx::util::Fraction{})).identity, staffStyle29Identity);
    EXPECT_EQ(changes.at(MusicPoint(13, musx::util::Fraction{})).identity, staffStyle28AgainIdentity);
    EXPECT_EQ(changes.at(MusicPoint(17, musx::util::Fraction{})).identity, rawFinalIdentity);
}

TEST(StaffStyleInstrument, InstrumentChangeIdentityReuseInPart2)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "inst_change2.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto part2Map = doc->createInstrumentMap(2);
    const auto* instrument = part2Map.getInstrumentForStaff(2);
    ASSERT_TRUE(instrument);

    const auto changes = instrument->getChanges();
    ASSERT_EQ(changes.size(), 2u);

    const auto pianoIdentity = instrument->getInstrumentIdentityAt(MusicPoint{});
    const auto harpsichordIdentity = instrument->getInstrumentIdentityAt(MusicPoint(6, musx::util::Fraction{}));

    EXPECT_EQ(pianoIdentity.instUuid, uuid::Piano);
    EXPECT_EQ(harpsichordIdentity.instUuid, uuid::Harpsichord);
    EXPECT_NE(pianoIdentity, harpsichordIdentity);

    const auto identities = instrument->getInstrumentIdentities();
    ASSERT_EQ(identities.size(), 2u);
    EXPECT_EQ(identities[0], pianoIdentity);
    EXPECT_EQ(identities[1], harpsichordIdentity);

    EXPECT_EQ(changes.at(MusicPoint{}).identity, pianoIdentity);
    EXPECT_EQ(changes.at(MusicPoint(6, musx::util::Fraction{})).identity, harpsichordIdentity);
    EXPECT_EQ(instrument->getInstrumentIdentityAt(MusicPoint(6, musx::util::Fraction{})), harpsichordIdentity);
}

TEST(StaffStyleChange, DetectDifferentScorePart)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "staffstyle_score_part.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 3, 0);
        EXPECT_EQ(staff->getRequestedPartId(), SCORE_PARTID);
        EXPECT_EQ(staff->calcNumberOfStafflines(), 5);
        auto styleAssigns = doc->getOthers()->getArray<others::StaffStyleAssign>(SCORE_PARTID);
        EXPECT_EQ(styleAssigns.getRequestedPartId(), SCORE_PARTID);
        for (const auto& assign : styleAssigns) {
            EXPECT_EQ(styleAssigns.getRequestedPartId(), assign->getRequestedPartId());
            EXPECT_EQ(assign->getRequestedPartId(), assign->getSourcePartId());
        }
    }

    {
        static constexpr Cmper kFirstPart = 1;
        auto staff = others::StaffComposite::createCurrent(doc, kFirstPart, 1, 3, 0);
        EXPECT_EQ(staff->getRequestedPartId(), kFirstPart);
        EXPECT_EQ(staff->calcNumberOfStafflines(), 1);
        auto styleAssigns = doc->getOthers()->getArray<others::StaffStyleAssign>(kFirstPart);
        EXPECT_EQ(styleAssigns.getRequestedPartId(), kFirstPart);
        for (const auto& assign : styleAssigns) {
            EXPECT_EQ(styleAssigns.getRequestedPartId(), assign->getRequestedPartId());
            EXPECT_EQ(assign->getRequestedPartId(), assign->getSourcePartId());
        }
    }
}
