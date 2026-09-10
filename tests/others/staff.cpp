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

#include <vector>

using namespace musx::dom;

constexpr static musxtest::string_view staffXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <instUsed cmper="0" inci="0">
      <inst>15</inst>
      <trackType>staff</trackType>
      <distFromTop>0</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <instUsed cmper="0" inci="1">
      <inst>16</inst>
      <trackType>staff</trackType>
      <distFromTop>-288</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <instUsed cmper="0" inci="2">
      <inst>11</inst>
      <trackType>staff</trackType>
      <distFromTop>-288</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <staffSpec cmper="11">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <defaultClef>1</defaultClef>
      <transposedClef>2</transposedClef>
      <dwRestOffset>-4</dwRestOffset>
      <wRestOffset>-4</wRestOffset>
      <hRestOffset>-4</hRestOffset>
      <otherRestOffset>-4</otherRestOffset>
      <stemReversal>-1</stemReversal>
      <fullName>75</fullName>
      <abbrvName>76</abbrvName>
      <topRepeatDotOff>-3</topRepeatDotOff>
      <botRepeatDotOff>-5</botRepeatDotOff>
      <vertTabNumOff>-1024</vertTabNumOff>
      <useAutoNum/>
    </staffSpec>
    <staffSpec cmper="15">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <floatKeys/>
      <showNameParts/>
      <defaultClef>1</defaultClef>
      <transposedClef>2</transposedClef>
      <dwRestOffset>-4</dwRestOffset>
      <wRestOffset>-4</wRestOffset>
      <hRestOffset>-4</hRestOffset>
      <otherRestOffset>-4</otherRestOffset>
      <stemReversal>-1</stemReversal>
      <fullName>75</fullName>
      <abbrvName>76</abbrvName>
      <topRepeatDotOff>-3</topRepeatDotOff>
      <botRepeatDotOff>-5</botRepeatDotOff>
      <vertTabNumOff>-1024</vertTabNumOff>
      <hideStems/>
      <stemDir>alwaysUp</stemDir>
    </staffSpec>
    <staffSpec cmper="16">
      <staffLines>0</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <floatTime/>
      <defaultClef>3</defaultClef>
      <transposedClef>4</transposedClef>
      <hideStfNameInScore/>
      <dwRestOffset>-4</dwRestOffset>
      <wRestOffset>-4</wRestOffset>
      <hRestOffset>-4</hRestOffset>
      <otherRestOffset>-4</otherRestOffset>
      <stemReversal>-2</stemReversal>
      <fullName>77</fullName>
      <abbrvName>78</abbrvName>
      <topRepeatDotOff>-2</topRepeatDotOff>
      <botRepeatDotOff>-6</botRepeatDotOff>
      <vertTabNumOff>-1024</vertTabNumOff>
      <hideBeams/>
      <stemDir>alwaysDown</stemDir>
      <autoNum>arabicPrefix</autoNum>
      <fretInstID>1</fretInstID>
      <useAutoNum/>
    </staffSpec>
  </others>
</finale>
    )xml";

TEST(StaffTest, PopulateFields)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(staffXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto staff1 = others->get<others::Staff>(SCORE_PARTID, 15);
    ASSERT_TRUE(staff1) << "Staff with cmper 15 not found";

    EXPECT_EQ(staff1->staffLines.value_or(-1), 5);
    EXPECT_EQ(staff1->lineSpace, 24);
    EXPECT_EQ(staff1->instUuid, "54422b22-4627-4100-abbf-064eedc15fe3");
    EXPECT_TRUE(staff1->floatKeys);
    EXPECT_FALSE(staff1->floatTime);
    EXPECT_EQ(staff1->defaultClef, 1);
    EXPECT_TRUE(staff1->showNameInParts);
    EXPECT_EQ(staff1->transposedClef, 2);
    EXPECT_FALSE(staff1->hideNameInScore);
    EXPECT_EQ(staff1->dwRestOffset, -4);
    EXPECT_EQ(staff1->wRestOffset, -4);
    EXPECT_EQ(staff1->hRestOffset, -4);
    EXPECT_EQ(staff1->otherRestOffset, -4);
    EXPECT_EQ(staff1->stemReversal, -1);
    EXPECT_EQ(staff1->fullNameTextId, 75);
    EXPECT_EQ(staff1->abbrvNameTextId, 76);
    EXPECT_EQ(staff1->topRepeatDotOff, -3);
    EXPECT_EQ(staff1->botRepeatDotOff, -5);
    EXPECT_EQ(staff1->vertTabNumOff, -1024);
    EXPECT_TRUE(staff1->hideStems);
    EXPECT_FALSE(staff1->hideBeams);
    EXPECT_EQ(staff1->stemDirection, StemDirection::AlwaysUp);
    EXPECT_EQ(staff1->autoNumbering, others::Staff::AutoNumberingStyle::ArabicSuffix);
    EXPECT_EQ(staff1->useAutoNumbering, false);

    auto staff2 = others->get<others::Staff>(SCORE_PARTID, 16);
    ASSERT_TRUE(staff2) << "Staff with cmper 16 not found";

    EXPECT_EQ(staff2->staffLines.value_or(-1), 0);
    EXPECT_EQ(staff2->lineSpace, 24);
    EXPECT_EQ(staff2->instUuid, "54422b22-4627-4100-abbf-064eedc15fe3");
    EXPECT_FALSE(staff2->floatKeys);
    EXPECT_TRUE(staff2->floatTime);
    EXPECT_EQ(staff2->defaultClef, 3);
    EXPECT_FALSE(staff2->showNameInParts);
    EXPECT_EQ(staff2->transposedClef, 4);
    EXPECT_TRUE(staff2->hideNameInScore);
    EXPECT_EQ(staff2->dwRestOffset, -4);
    EXPECT_EQ(staff2->wRestOffset, -4);
    EXPECT_EQ(staff2->hRestOffset, -4);
    EXPECT_EQ(staff2->otherRestOffset, -4);
    EXPECT_EQ(staff2->stemReversal, -2);
    EXPECT_EQ(staff2->fullNameTextId, 77);
    EXPECT_EQ(staff2->abbrvNameTextId, 78);
    EXPECT_EQ(staff2->topRepeatDotOff, -2);
    EXPECT_EQ(staff2->botRepeatDotOff, -6);
    EXPECT_EQ(staff2->vertTabNumOff, -1024);
    EXPECT_FALSE(staff2->hideStems);
    EXPECT_TRUE(staff2->hideBeams);
    EXPECT_EQ(staff2->stemDirection, StemDirection::AlwaysDown);
    EXPECT_EQ(staff2->autoNumbering, others::Staff::AutoNumberingStyle::ArabicPrefix);
    EXPECT_EQ(staff2->fretInstId, 1);
    EXPECT_EQ(staff2->useAutoNumbering, true);

    auto staff3 = others->get<others::Staff>(SCORE_PARTID, 11);
    ASSERT_TRUE(staff2) << "Staff with cmper 11 not found";

    EXPECT_EQ(staff3->stemDirection, StemDirection::Default);
}

TEST(StaffTest, RestOffsets)
{
    constexpr static musxtest::string_view restOffsetXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <dwRestOffset>-5</dwRestOffset>
      <wRestOffset>-6</wRestOffset>
      <hRestOffset>-7</hRestOffset>
      <otherRestOffset>-8</otherRestOffset>
    </staffSpec>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(restOffsetXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto staff = others->get<others::Staff>(SCORE_PARTID, 1);
    ASSERT_TRUE(staff);

    EXPECT_EQ(staff->calcRestOffset(Edu(NoteType::Breve)), -5);
    EXPECT_EQ(staff->calcRestOffset(Edu(NoteType::Longa)), -8);
    EXPECT_EQ(staff->calcRestOffset(Edu(NoteType::Maxima)), -8);
    EXPECT_EQ(staff->calcRestOffset(Edu(NoteType::Whole)), -6);
    EXPECT_EQ(staff->calcRestOffset(Edu(NoteType::Whole) + Edu(NoteType::Half)), -6);
    EXPECT_EQ(staff->calcRestOffset(Edu(NoteType::Half)), -7);
    EXPECT_EQ(staff->calcRestOffset(Edu(NoteType::Quarter)), -8);
}

TEST(StaffTest, IterateClefChangesAtMeasureSingleClef)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <measSpec cmper="1">
      <width>600</width>
    </measSpec>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <defaultClef>0</defaultClef>
    </staffSpec>
  </others>
  <details>
    <gfhold cmper1="1" cmper2="1">
      <clefID>3</clefID>
      <clefMode>forced</clefMode>
      <clefAfterBarline/>
      <clefPercent>75</clefPercent>
    </gfhold>
  </details>
</finale>
)xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    auto staff = doc->getOthers()->get<others::Staff>(SCORE_PARTID, 1);
    ASSERT_TRUE(staff);

    std::vector<others::Staff::ClefChange> changes;
    EXPECT_TRUE(staff->iterateClefChangesAtMeasure(1, false, [&](const others::Staff::ClefChange& change) {
        changes.push_back(change);
        return true;
    }));

    ASSERT_EQ(changes.size(), 1);
    EXPECT_EQ(changes[0].clefIndex, 3);
    EXPECT_EQ(changes[0].position, musx::util::Fraction());
    EXPECT_EQ(changes[0].showClefMode, ShowClefMode::Always);
    EXPECT_TRUE(changes[0].afterBarline);
    EXPECT_EQ(changes[0].yEvpuPos, 0);
    EXPECT_EQ(changes[0].percent, 75);
    EXPECT_EQ(changes[0].xEvpuOffset, 0);
}

TEST(StaffTest, IterateClefChangesAtMeasureClefList)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <measSpec cmper="1">
      <width>600</width>
      <beats>4</beats>
      <divbeat>1024</divbeat>
    </measSpec>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <defaultClef>0</defaultClef>
    </staffSpec>
    <frameSpec cmper="10" inci="0">
      <startEntry>1001</startEntry>
      <endEntry>1002</endEntry>
    </frameSpec>
    <clefEnum cmper="25" inci="0">
      <clef>2</clef>
      <xEduPos>0</xEduPos>
      <yEvpuPos>0</yEvpuPos>
      <percent>100</percent>
      <xEvpuOffset>0</xEvpuOffset>
      <clefMode>hidden</clefMode>
      <afterBarline/>
    </clefEnum>
    <clefEnum cmper="25" inci="1">
      <clef>4</clef>
      <xEduPos>1025</xEduPos>
      <yEvpuPos>-12</yEvpuPos>
      <percent>80</percent>
      <xEvpuOffset>7</xEvpuOffset>
      <clefMode>forced</clefMode>
      <afterBarline/>
    </clefEnum>
  </others>
  <details>
    <gfhold cmper1="1" cmper2="1">
      <clefListID>25</clefListID>
      <frame1>10</frame1>
    </gfhold>
  </details>
  <entries>
    <entry entnum="1001" prev="0" next="1002">
      <dura>1024</dura>
      <numNotes>0</numNotes>
      <isValid/>
      <floatRest/>
    </entry>
    <entry entnum="1002" prev="1001" next="0">
      <dura>1024</dura>
      <numNotes>0</numNotes>
      <isValid/>
      <floatRest/>
    </entry>
  </entries>
</finale>
)xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    auto staff = doc->getOthers()->get<others::Staff>(SCORE_PARTID, 1);
    ASSERT_TRUE(staff);

    std::vector<others::Staff::ClefChange> changes;
    EXPECT_TRUE(staff->iterateClefChangesAtMeasure(1, false, [&](const others::Staff::ClefChange& change) {
        changes.push_back(change);
        return true;
    }));

    ASSERT_EQ(changes.size(), 2);
    EXPECT_EQ(changes[0].clefIndex, 2);
    EXPECT_EQ(changes[0].position, musx::util::Fraction());
    EXPECT_EQ(changes[0].showClefMode, ShowClefMode::Never);
    EXPECT_TRUE(changes[0].afterBarline);
    EXPECT_EQ(changes[0].percent, 100);

    EXPECT_EQ(changes[1].clefIndex, 4);
    EXPECT_EQ(changes[1].position, musx::util::Fraction::fromEdu(1024));
    EXPECT_EQ(changes[1].showClefMode, ShowClefMode::Always);
    EXPECT_FALSE(changes[1].afterBarline);
    EXPECT_EQ(changes[1].yEvpuPos, -12);
    EXPECT_EQ(changes[1].percent, 80);
    EXPECT_EQ(changes[1].xEvpuOffset, 7);
}

TEST(StaffTest, IterateClefChangesAtMeasureStopsWhenCallbackReturnsFalse)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <measSpec cmper="1">
      <width>600</width>
    </measSpec>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
    </staffSpec>
    <clefEnum cmper="25" inci="0">
      <clef>2</clef>
      <xEduPos>0</xEduPos>
    </clefEnum>
    <clefEnum cmper="25" inci="1">
      <clef>4</clef>
      <xEduPos>1024</xEduPos>
    </clefEnum>
  </others>
  <details>
    <gfhold cmper1="1" cmper2="1">
      <clefListID>25</clefListID>
    </gfhold>
  </details>
</finale>
)xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    auto staff = doc->getOthers()->get<others::Staff>(SCORE_PARTID, 1);
    ASSERT_TRUE(staff);

    int calls = 0;
    EXPECT_FALSE(staff->iterateClefChangesAtMeasure(1, false, [&](const others::Staff::ClefChange&) {
        calls++;
        return false;
    }));
    EXPECT_EQ(calls, 1);
}

TEST(StaffTest, IterateClefChangesAtMeasureUsesWrittenPitchTransposedClef)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <instUsed cmper="0" inci="0">
      <inst>1</inst>
      <trackType>staff</trackType>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <measSpec cmper="1">
      <width>600</width>
    </measSpec>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <defaultClef>0</defaultClef>
      <transposedClef>5</transposedClef>
      <transposition>
        <setToClef/>
        <keysig>
          <interval>1</interval>
          <adjust>0</adjust>
        </keysig>
      </transposition>
    </staffSpec>
  </others>
  <details>
    <gfhold cmper1="1" cmper2="1">
      <clefID>3</clefID>
    </gfhold>
  </details>
</finale>
)xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    auto staff = doc->getOthers()->get<others::Staff>(SCORE_PARTID, 1);
    ASSERT_TRUE(staff);

    std::vector<others::Staff::ClefChange> changes;
    EXPECT_TRUE(staff->iterateClefChangesAtMeasure(1, true, [&](const others::Staff::ClefChange& change) {
        changes.push_back(change);
        return true;
    }));

    ASSERT_EQ(changes.size(), 1);
    EXPECT_EQ(changes[0].clefIndex, 5);
    EXPECT_EQ(changes[0].position, musx::util::Fraction());
    EXPECT_EQ(changes[0].showClefMode, ShowClefMode::WhenNeeded);
    EXPECT_FALSE(changes[0].afterBarline);
    EXPECT_EQ(changes[0].percent, 100);
}

TEST(StaffTest, BarlineOffsetsFromCenter)
{
    constexpr static musxtest::string_view barlineOffsetXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
    </staffSpec>
    <staffSpec cmper="2">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <topBarlineOffset>-36</topBarlineOffset>
      <botBarlineOffset>36</botBarlineOffset>
    </staffSpec>
    <staffSpec cmper="3">
      <staffLines>1</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
    </staffSpec>
    <staffSpec cmper="4">
      <staffLines>1</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
      <topBarlineOffset>36</topBarlineOffset>
      <botBarlineOffset>-36</botBarlineOffset>
    </staffSpec>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(barlineOffsetXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    {
        auto staff = others->get<others::Staff>(SCORE_PARTID, 1);
        ASSERT_TRUE(staff);
        EXPECT_EQ(staff->calcBarlineOffsetsFromCenter(), std::make_pair(Evpu(48), Evpu(-48)));
    }
    {
        auto staff = others->get<others::Staff>(SCORE_PARTID, 2);
        ASSERT_TRUE(staff);
        EXPECT_EQ(staff->calcBarlineOffsetsFromCenter(), std::make_pair(Evpu(12), Evpu(-12)));
    }
    {
        auto staff = others->get<others::Staff>(SCORE_PARTID, 3);
        ASSERT_TRUE(staff);
        EXPECT_EQ(staff->calcBarlineOffsetsFromCenter(), std::make_pair(Evpu(0), Evpu(0)));
    }
    {
        auto staff = others->get<others::Staff>(SCORE_PARTID, 4);
        ASSERT_TRUE(staff);
        EXPECT_EQ(staff->calcBarlineOffsetsFromCenter(), std::make_pair(Evpu(36), Evpu(-36)));
    }
}

TEST(StaffTest, AutoNumbering)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(staffXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto staff1Const = others->get<others::Staff>(SCORE_PARTID, 15);
    ASSERT_TRUE(staff1Const) << "Staff with cmper 15 not found";
    auto staff1 = const_cast<others::Staff*>(staff1Const.get());

    EXPECT_FALSE(staff1->autoNumberValue.has_value());
    EXPECT_EQ(staff1->addAutoNumbering("Name"), "Name");
    staff1->useAutoNumbering = true;
    others::Staff::calcAllAutoNumberValues(doc);
    EXPECT_EQ(staff1->autoNumberValue.value_or(-1), 1);
    EXPECT_EQ(staff1->addAutoNumbering("Name"), "Name 1");
    staff1->autoNumbering = others::Staff::AutoNumberingStyle::RomanSuffix;
    EXPECT_EQ(staff1->addAutoNumbering("Name"), "Name I");
    staff1->autoNumbering = others::Staff::AutoNumberingStyle::OrdinalPrefix;
    EXPECT_EQ(staff1->addAutoNumbering("Name"), "1st Name");
    staff1->autoNumbering = others::Staff::AutoNumberingStyle::AlphaSuffix;
    EXPECT_EQ(staff1->addAutoNumbering("Name"), "Name A");
    staff1->autoNumbering = others::Staff::AutoNumberingStyle::ArabicPrefix;
    EXPECT_EQ(staff1->addAutoNumbering("Name"), "1. Name");

    auto staff2Const = others->get<others::Staff>(SCORE_PARTID, 16);
    ASSERT_TRUE(staff2Const) << "Staff with cmper 16 not found";
    auto staff2 = const_cast<others::Staff*>(staff2Const.get());

    EXPECT_EQ(staff2->autoNumberValue.value_or(-1), 2);
    EXPECT_EQ(staff2->addAutoNumbering("Name"), "2. Name");
    staff2->autoNumbering = others::Staff::AutoNumberingStyle::RomanSuffix;
    EXPECT_EQ(staff2->addAutoNumbering("Name"), "Name II");
    staff2->autoNumbering = others::Staff::AutoNumberingStyle::OrdinalPrefix;
    EXPECT_EQ(staff2->addAutoNumbering("Name"), "2nd Name");
    staff2->autoNumbering = others::Staff::AutoNumberingStyle::AlphaSuffix;
    EXPECT_EQ(staff2->addAutoNumbering("Name"), "Name B");
    staff2->autoNumbering = others::Staff::AutoNumberingStyle::ArabicSuffix;
    EXPECT_EQ(staff2->addAutoNumbering("Name"), "Name 2");

    auto staff3Const = others->get<others::Staff>(SCORE_PARTID, 11);
    ASSERT_TRUE(staff3Const) << "Staff with cmper 11 not found";
    auto staff3 = const_cast<others::Staff*>(staff3Const.get());

    EXPECT_EQ(staff3->autoNumberValue.value_or(-1), 3);
    EXPECT_EQ(staff3->addAutoNumbering("Name"), "Name 3");
    staff3->autoNumbering = others::Staff::AutoNumberingStyle::RomanSuffix;
    EXPECT_EQ(staff3->addAutoNumbering("Name"), "Name III");
    staff3->autoNumbering = others::Staff::AutoNumberingStyle::OrdinalPrefix;
    EXPECT_EQ(staff3->addAutoNumbering("Name"), "3rd Name");
    staff3->autoNumbering = others::Staff::AutoNumberingStyle::AlphaSuffix;
    EXPECT_EQ(staff3->addAutoNumbering("Name"), "Name C");
    staff3->autoNumbering = others::Staff::AutoNumberingStyle::ArabicPrefix;
    EXPECT_EQ(staff3->addAutoNumbering("Name"), "3. Name");
}

TEST(StaffText, StaffLines)
{
  constexpr static musxtest::string_view staffLinesXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
  <finale>
    <others>
      <staffSpec cmper="1">
        <staffLines>5</staffLines>
        <lineSpace>24</lineSpace>
        <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
        <dwRestOffset>-4</dwRestOffset>
        <wRestOffset>-4</wRestOffset>
        <hRestOffset>-4</hRestOffset>
        <otherRestOffset>-4</otherRestOffset>
        <stemReversal>-4</stemReversal>
        <botRepeatDotOff>-5</botRepeatDotOff>
        <topRepeatDotOff>-3</topRepeatDotOff>
        <vertTabNumOff>-1024</vertTabNumOff>
      </staffSpec>
      <staffSpec cmper="2">
        <customStaff>
          <staffLine>13</staffLine>
        </customStaff>
        <lineSpace>24</lineSpace>
        <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
        <botBarlineOffset>-24</botBarlineOffset>
        <topBarlineOffset>24</topBarlineOffset>
        <dwRestOffset>-5</dwRestOffset>
        <wRestOffset>-6</wRestOffset>
        <hRestOffset>-4</hRestOffset>
        <otherRestOffset>-4</otherRestOffset>
        <stemReversal>-4</stemReversal>
        <botRepeatDotOff>-5</botRepeatDotOff>
        <topRepeatDotOff>-3</topRepeatDotOff>
        <vertTabNumOff>-1024</vertTabNumOff>
      </staffSpec>
      <staffSpec cmper="3">
        <staffLines>1</staffLines>
        <lineSpace>24</lineSpace>
        <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
        <dwRestOffset>-4</dwRestOffset>
        <wRestOffset>-4</wRestOffset>
        <hRestOffset>-4</hRestOffset>
        <otherRestOffset>-4</otherRestOffset>
        <stemReversal>-4</stemReversal>
        <botRepeatDotOff>-5</botRepeatDotOff>
        <topRepeatDotOff>-3</topRepeatDotOff>
        <vertTabNumOff>-1024</vertTabNumOff>
      </staffSpec>
      <staffSpec cmper="4">
        <customStaff>
          <staffLine>11</staffLine>
          <staffLine>12</staffLine>
          <staffLine>13</staffLine>
          <staffLine>14</staffLine>
          <staffLine>15</staffLine>
          <staffLine>18</staffLine>
        </customStaff>
        <lineSpace>24</lineSpace>
        <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
        <dwRestOffset>-4</dwRestOffset>
        <wRestOffset>-4</wRestOffset>
        <hRestOffset>-4</hRestOffset>
        <otherRestOffset>-4</otherRestOffset>
        <stemReversal>-4</stemReversal>
        <botRepeatDotOff>-5</botRepeatDotOff>
        <topRepeatDotOff>-3</topRepeatDotOff>
        <vertTabNumOff>-1024</vertTabNumOff>
      </staffSpec>
      <staffSpec cmper="5">
        <staffLines>3</staffLines>
        <lineSpace>24</lineSpace>
        <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
        <dwRestOffset>-4</dwRestOffset>
        <wRestOffset>-4</wRestOffset>
        <hRestOffset>-4</hRestOffset>
        <otherRestOffset>-4</otherRestOffset>
        <stemReversal>-4</stemReversal>
        <botRepeatDotOff>-5</botRepeatDotOff>
        <topRepeatDotOff>-3</topRepeatDotOff>
        <vertTabNumOff>-1024</vertTabNumOff>
      </staffSpec>
      <staffSpec cmper="6">
        <customStaff>
          <staffLine>12</staffLine>
          <staffLine>13</staffLine>
          <staffLine>14</staffLine>
        </customStaff>
        <lineSpace>24</lineSpace>
        <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
        <dwRestOffset>-4</dwRestOffset>
        <wRestOffset>-4</wRestOffset>
        <hRestOffset>-4</hRestOffset>
        <otherRestOffset>-4</otherRestOffset>
        <stemReversal>-4</stemReversal>
        <botRepeatDotOff>-5</botRepeatDotOff>
        <topRepeatDotOff>-3</topRepeatDotOff>
        <vertTabNumOff>-1024</vertTabNumOff>
      </staffSpec>
      <staffSpec cmper="7">
        <staffLines>0</staffLines>
        <lineSpace>24</lineSpace>
        <instUuid>54422b22-4627-4100-abbf-064eedc15fe3</instUuid>
        <dwRestOffset>-4</dwRestOffset>
        <wRestOffset>-4</wRestOffset>
        <hRestOffset>-4</hRestOffset>
        <otherRestOffset>-4</otherRestOffset>
        <stemReversal>-4</stemReversal>
        <botRepeatDotOff>-5</botRepeatDotOff>
        <topRepeatDotOff>-3</topRepeatDotOff>
        <vertTabNumOff>-1024</vertTabNumOff>
      </staffSpec>
    </others>
  </finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(staffLinesXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto staves = others->getArray<others::Staff>(SCORE_PARTID);

    const std::vector<int> expectedValues = { -4, -4, 0, -7, -2, -4, -4 };
    size_t x = 0;
    for (; x < staves.size(); x++) {
      EXPECT_LT(x, expectedValues.size()) << "too few expected values";
      if (x >= expectedValues.size()) break;
      const auto& staff = staves[x];
      EXPECT_EQ(staff->calcMiddleStaffPosition(), expectedValues[x]);
    }
    EXPECT_EQ(x, staves.size());

    auto emptyCustomStaff = const_cast<others::Staff*>(staves.back().get());
    emptyCustomStaff->staffLines.reset();
    emptyCustomStaff->customStaff.emplace();
    EXPECT_EQ(emptyCustomStaff->calcMiddleStaffPosition(), -4);
}

TEST(StaffTest, Transposition)
{
    std::vector<char> transposeXml;
    musxtest::readFile(musxtest::getInputPath() / "transpose.enigmaxml", transposeXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(transposeXml);
    ASSERT_TRUE(doc);

    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    using Result = std::tuple<int, int, int, int>; // note index, octave, alteration, staff line

    static constexpr std::array<Result, 2> expectedConcertResult = {
        Result{5, 4, 0, 1}, // A4
        Result{6, 4, 0, 2}  // B4
    };

    static const std::array<std::array<Result, 2>, 5> expectedWrittenResults = {{
        { Result{5, 4, 0, 1}, Result{6, 4, 0, 2} },
        { Result{0, 5, 0, 3}, Result{1, 5, 0, 4} },
        { Result{6, 5, 0, 3}, Result{0, 6, 1, 4} },
        { Result{4, 5, -1, 1}, Result{5, 5, -1, 2} },
        { Result{3, 6, 1, 7}, Result{4, 6, 1, 8} },
    }};

    for (Cmper measId = 1; measId <= 5; measId++) {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, measId);
        ASSERT_TRUE(gfhold);

        auto entries = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entries);
        ASSERT_GE(entries->getEntries().size(), 2);

        for (size_t x = 0; x < 2; x++) {
            auto note = NoteInfoPtr(EntryInfoPtr(entries, x), 0);

            NotePropertiesOptions writtenOptions;
            writtenOptions.pitchMode = PitchMode::Written;
            auto [wNote, wOctave, wAlter, wStaff] = note.calcNoteProperties(writtenOptions);
            NotePropertiesOptions concertOptions;
            concertOptions.pitchMode = PitchMode::Concert;
            auto [cNote, cOctave, cAlter, cStaff] = note.calcNoteProperties(concertOptions);

            const auto& [expWNote, expWOct, expWAlt, expWStaff] = expectedWrittenResults[measId - 1][x];
            const auto& [expCNote, expCOct, expCAlt, expCStaff] = expectedConcertResult[x];

            // Written checks
            EXPECT_EQ(int(wNote), expWNote) << "Written note mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(wOctave, expWOct) << "Written octave mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(wAlter, expWAlt) << "Written alteration mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(wStaff, expWStaff) << "Written staff mismatch at entry " << x << " in measure " << measId;

            // Concert checks
            EXPECT_EQ(int(cNote), expCNote) << "Concert note mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(cOctave, expCOct) << "Concert octave mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(cAlter, expCAlt) << "Concert alteration mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(cStaff, expCStaff) << "Concert staff mismatch at entry " << x << " in measure " << measId;

            const auto entryInfo = note.getEntryInfo();
            StaffPositionPitchOptions writtenPitchOptions;
            writtenPitchOptions.pitchMode = PitchMode::Written;
            const auto writtenPitch = entryInfo.calcPitchFromStaffPosition(wStaff, writtenPitchOptions);
            EXPECT_EQ(writtenPitch.noteName, wNote);
            EXPECT_EQ(writtenPitch.octave, wOctave);
            EXPECT_EQ(writtenPitch.alteration,
                entryInfo.getKeySignature()->calcAlterationOnNote(unsigned(wNote), KeySignature::KeyContext::Written));

            writtenPitchOptions.actualAlteration = 0;
            const auto naturalWrittenPitch = entryInfo.calcPitchFromStaffPosition(wStaff, writtenPitchOptions);
            EXPECT_EQ(naturalWrittenPitch.noteName, wNote);
            EXPECT_EQ(naturalWrittenPitch.octave, wOctave);
            EXPECT_EQ(naturalWrittenPitch.alteration, 0);

            StaffPositionPitchOptions concertPitchOptions;
            concertPitchOptions.pitchMode = PitchMode::Concert;
            const auto concertPitch = entryInfo.calcPitchFromStaffPosition(cStaff, concertPitchOptions);
            EXPECT_EQ(concertPitch.noteName, cNote);
            EXPECT_EQ(concertPitch.octave, cOctave);
            EXPECT_EQ(concertPitch.alteration, cAlter);

            const auto viewPitch = entryInfo.calcPitchFromStaffPosition(wStaff);
            EXPECT_EQ(viewPitch.noteName, writtenPitch.noteName);
            EXPECT_EQ(viewPitch.octave, writtenPitch.octave);
            EXPECT_EQ(viewPitch.alteration, writtenPitch.alteration);

    /*
            // Optional: print results
            static constexpr std::array<char, 7> noteNames = { 'C', 'D', 'E', 'F', 'G', 'A', 'B' };
            std::cout << "Entry index " << x << " measure " << measId << std::endl;
            std::cout << "    W: " << noteNames[int(wNote)] << ", " << wOctave << ", " << wAlter << ", " << wStaff << std::endl;
            std::cout << "    C: " << noteNames[int(cNote)] << ", " << cOctave << ", " << cAlter << ", " << cStaff << std::endl;
    */
        }
    }

    std::string concertXml(transposeXml.begin(), transposeXml.end());
    const std::string showTransposedElement = "<showTransposed/>";
    const auto showTransposedPosition = concertXml.find(showTransposedElement);
    ASSERT_NE(showTransposedPosition, std::string::npos);
    concertXml.erase(showTransposedPosition, showTransposedElement.size());
    auto concertDoc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(concertXml);
    ASSERT_TRUE(concertDoc);

    auto gfhold = details::GFrameHoldContext(concertDoc, SCORE_PARTID, 1, 2);
    ASSERT_TRUE(gfhold);
    auto entries = gfhold.createEntryFrame(0);
    ASSERT_TRUE(entries);
    auto note = NoteInfoPtr(EntryInfoPtr(entries, 0), 0);
    NotePropertiesOptions concertOptions;
    concertOptions.pitchMode = PitchMode::Concert;
    const auto [concertNote, concertOctave, concertAlteration, concertStaffPosition] = note.calcNoteProperties(concertOptions);
    const auto viewPitch = note.getEntryInfo().calcPitchFromStaffPosition(concertStaffPosition);
    EXPECT_EQ(viewPitch.noteName, concertNote);
    EXPECT_EQ(viewPitch.octave, concertOctave);
    EXPECT_EQ(viewPitch.alteration, concertAlteration);

    std::string retainedOctaveXml = concertXml;
    const std::string nonOctaveInterval = "<interval>12</interval>";
    const auto nonOctaveIntervalPosition = retainedOctaveXml.find(nonOctaveInterval);
    ASSERT_NE(nonOctaveIntervalPosition, std::string::npos);
    retainedOctaveXml.replace(nonOctaveIntervalPosition, nonOctaveInterval.size(), "<interval>7</interval>");
    const std::string nonOctaveAdjustment = "<adjust>3</adjust>";
    const auto nonOctaveAdjustmentPosition = retainedOctaveXml.find(nonOctaveAdjustment, nonOctaveIntervalPosition);
    ASSERT_NE(nonOctaveAdjustmentPosition, std::string::npos);
    retainedOctaveXml.replace(nonOctaveAdjustmentPosition, nonOctaveAdjustment.size(), "<adjust>0</adjust>");
    auto retainedOctaveDoc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(retainedOctaveXml);
    ASSERT_TRUE(retainedOctaveDoc);

    constexpr StaffCmper testStaffId = 1;
    constexpr MeasCmper octaveTranspositionMeasure = 5;
    auto retainedOctaveFrame = details::GFrameHoldContext(
        retainedOctaveDoc, SCORE_PARTID, testStaffId, octaveTranspositionMeasure).createEntryFrame(0);
    ASSERT_TRUE(retainedOctaveFrame);
    auto retainedOctaveNote = NoteInfoPtr(EntryInfoPtr(retainedOctaveFrame, 0), 0);
    NotePropertiesOptions writtenOptions;
    writtenOptions.pitchMode = PitchMode::Written;
    const auto writtenProperties = retainedOctaveNote.calcNoteProperties(writtenOptions);
    NotePropertiesOptions retainedConcertOptions;
    retainedConcertOptions.pitchMode = PitchMode::Concert;
    const auto retainedConcertProperties = retainedOctaveNote.calcNoteProperties(retainedConcertOptions);
    const auto retainedViewProperties = retainedOctaveNote.calcNoteProperties();
    EXPECT_EQ(retainedViewProperties, writtenProperties);
    EXPECT_NE(retainedViewProperties.staffPosition, retainedConcertProperties.staffPosition);

    const auto retainedViewPitch = retainedOctaveNote.getEntryInfo().calcPitchFromStaffPosition(writtenProperties.staffPosition);
    EXPECT_EQ(retainedViewPitch.noteName, writtenProperties.noteName);
    EXPECT_EQ(retainedViewPitch.octave, writtenProperties.octave);

    const std::string retainOctaveElement = "<retainOctaveTransInConcertPitch/>";
    const auto retainOctavePosition = retainedOctaveXml.find(retainOctaveElement);
    ASSERT_NE(retainOctavePosition, std::string::npos);
    retainedOctaveXml.erase(retainOctavePosition, retainOctaveElement.size());
    auto unretainedOctaveDoc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(retainedOctaveXml);
    ASSERT_TRUE(unretainedOctaveDoc);
    auto unretainedOctaveFrame = details::GFrameHoldContext(
        unretainedOctaveDoc, SCORE_PARTID, testStaffId, octaveTranspositionMeasure).createEntryFrame(0);
    ASSERT_TRUE(unretainedOctaveFrame);
    auto unretainedOctaveNote = NoteInfoPtr(EntryInfoPtr(unretainedOctaveFrame, 0), 0);
    NotePropertiesOptions unretainedConcertOptions;
    unretainedConcertOptions.pitchMode = PitchMode::Concert;
    EXPECT_EQ(unretainedOctaveNote.calcNoteProperties(), unretainedOctaveNote.calcNoteProperties(unretainedConcertOptions));
}

TEST(StaffTest, Transposition31Edo)
{
    std::vector<char> transposeXml;
    musxtest::readFile(musxtest::getInputPath() / "transpose_31edo.enigmaxml", transposeXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(transposeXml);
    ASSERT_TRUE(doc);

    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    using Result = std::tuple<int, int, int, int>; // note index, octave, alteration, staff line

    [[maybe_unused]] static constexpr std::array<Result, 2> expectedConcertResult = {
        Result{5, 4, 0, 1}, // A4
        Result{6, 4, 0, 2}  // B4
    };

    [[maybe_unused]] static const std::array<std::array<Result, 2>, 5> expectedWrittenResults = {{
        { Result{5, 4, 0, 1}, Result{6, 4, 0, 2} },
        { Result{0, 5, 0, 3}, Result{1, 5, 0, 4} },
        { Result{6, 5, 0, 3}, Result{0, 6, 2, 4} },
        { Result{4, 5, -2, 1}, Result{5, 5, -2, 2} },
        { Result{3, 6, 2, 7}, Result{4, 6, 2, 8} },
    }};

    for (Cmper measId = 1; measId <= 5; measId++) {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, measId);
        ASSERT_TRUE(gfhold);

        auto entries = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entries);
        ASSERT_GE(entries->getEntries().size(), 2);

        for (size_t x = 0; x < 2; x++) {
            auto note = NoteInfoPtr(EntryInfoPtr(entries, x), 0);

            NotePropertiesOptions writtenOptions;
            writtenOptions.pitchMode = PitchMode::Written;
            auto [wNote, wOctave, wAlter, wStaff] = note.calcNoteProperties(writtenOptions);
            NotePropertiesOptions concertOptions;
            concertOptions.pitchMode = PitchMode::Concert;
            auto [cNote, cOctave, cAlter, cStaff] = note.calcNoteProperties(concertOptions);

            const auto& [expWNote, expWOct, expWAlt, expWStaff] = expectedWrittenResults[measId - 1][x];
            const auto& [expCNote, expCOct, expCAlt, expCStaff] = expectedConcertResult[x];

            // Written checks
            EXPECT_EQ(int(wNote), expWNote) << "Written note mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(wOctave, expWOct) << "Written octave mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(wAlter, expWAlt) << "Written alteration mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(wStaff, expWStaff) << "Written staff mismatch at entry " << x << " in measure " << measId;

            // Concert checks
            EXPECT_EQ(int(cNote), expCNote) << "Concert note mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(cOctave, expCOct) << "Concert octave mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(cAlter, expCAlt) << "Concert alteration mismatch at entry " << x << " in measure " << measId;
            EXPECT_EQ(cStaff, expCStaff) << "Concert staff mismatch at entry " << x << " in measure " << measId;

            const auto entryInfo = note.getEntryInfo();
            StaffPositionPitchOptions writtenPitchOptions;
            writtenPitchOptions.pitchMode = PitchMode::Written;
            const auto writtenPitch = entryInfo.calcPitchFromStaffPosition(wStaff, writtenPitchOptions);
            EXPECT_EQ(writtenPitch.noteName, wNote);
            EXPECT_EQ(writtenPitch.octave, wOctave);
            EXPECT_EQ(writtenPitch.alteration,
                entryInfo.getKeySignature()->calcAlterationOnNote(unsigned(wNote), KeySignature::KeyContext::Written));

            writtenPitchOptions.actualAlteration = 0;
            const auto naturalWrittenPitch = entryInfo.calcPitchFromStaffPosition(wStaff, writtenPitchOptions);
            EXPECT_EQ(naturalWrittenPitch.noteName, wNote);
            EXPECT_EQ(naturalWrittenPitch.octave, wOctave);
            EXPECT_EQ(naturalWrittenPitch.alteration, 0);

            StaffPositionPitchOptions concertPitchOptions;
            concertPitchOptions.pitchMode = PitchMode::Concert;
            const auto concertPitch = entryInfo.calcPitchFromStaffPosition(cStaff, concertPitchOptions);
            EXPECT_EQ(concertPitch.noteName, cNote);
            EXPECT_EQ(concertPitch.octave, cOctave);
            EXPECT_EQ(concertPitch.alteration, cAlter);

    /*
            // Optional: print results
            static constexpr std::array<char, 7> noteNames = { 'C', 'D', 'E', 'F', 'G', 'A', 'B' };
            std::cout << "Entry index " << x << " measure " << measId << std::endl;
            std::cout << "    W: " << noteNames[int(wNote)] << ", " << wOctave << ", " << wAlter << ", " << wStaff << std::endl;
            std::cout << "    C: " << noteNames[int(cNote)] << ", " << cOctave << ", " << cAlter << ", " << cStaff << std::endl;
    */
        }
    }
}

TEST(StaffTest, PercussionMapStyle)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "percstaffstyle.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto staff1 = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 1, 0);
    ASSERT_TRUE(staff1);
    EXPECT_EQ(staff1->percussionMapId, 12);

    auto staff2 = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 2, 0);
    ASSERT_TRUE(staff2);
    EXPECT_EQ(staff2->percussionMapId, 25);
}

TEST(StaffTest, NamePositioning)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "namepos.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    using Align = AlignJustify;

    auto checkNamePos = [&](StaffCmper staffId, MeasCmper measId,
            Evpu expectedFullX, Evpu expectedFullY, Align expectedFullAlign,
            Evpu expectedAbrvX, Evpu expectedAbrvY, Align expectedAbrvAlign) {
        std::string msg = "Staff " + std::to_string(staffId) + " Measure " + std::to_string(measId);
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, staffId, measId, 0);
        ASSERT_TRUE(staff) << msg;
        auto namePos = staff->getFullNamePosition();
        EXPECT_EQ(namePos->horzOff, expectedFullX);
        EXPECT_EQ(namePos->vertOff, expectedFullY);
        EXPECT_EQ(namePos->hAlign, expectedFullAlign);
        namePos = staff->getAbbreviatedNamePosition();
        EXPECT_EQ(namePos->horzOff, expectedAbrvX);
        EXPECT_EQ(namePos->vertOff, expectedAbrvY);
        EXPECT_EQ(namePos->hAlign, expectedAbrvAlign);
    };

    // bar 2 (1st system)
    checkNamePos(1, 1, -72, 0, Align::Right, -64, -12, Align::Right);  // forced by staff style "Default Namepos"
    checkNamePos(2, 1, -72, 0, Align::Right, -144, -48, Align::Left);  // staff values (abbrv is overridden in the staff)
    // bar 4 (2nd system)
    checkNamePos(1, 4, -72, -24, Align::Center, -64, -12, Align::Right);  // staff values (full is overridden in the staff)
    checkNamePos(2, 4, -72, 0, Align::Right, -64, -12, Align::Right);  // forced by staff style "Default Namepos"
    // bar 7 (3rd system)
    checkNamePos(1, 7, 0, 24, Align::Right, 0, -60, Align::Right);  // staff style values from "Override staff style"
    checkNamePos(2, 7, 0, 24, Align::Right, 0, -60, Align::Right);  // staff style values from "Override staff style"
    // bar 8 (4th system)
    checkNamePos(1, 8, -72, -24, Align::Center, -64, -12, Align::Right);  // staff values (full is overridden in the staff)
    checkNamePos(2, 8, -72, 0, Align::Right, -144, -48, Align::Left);  // staff values (abbrv is overridden in the staff)
    // bar 11 (5th system)
    checkNamePos(1, 11, -72, -24, Align::Center, -64, -12, Align::Right);  // staff values (full is overridden in the staff)
    checkNamePos(2, 11, -72, 0, Align::Right, -144, -48, Align::Left);  // staff values (abbrv is overridden in the staff)
}

TEST(StaffTest, NoteShapesStaff)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "note_shapes.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 1, 0);
        ASSERT_TRUE(staff);
        EXPECT_TRUE(staff->useNoteShapes);
        EXPECT_EQ(staff->noteShapesId, 0);
        EXPECT_FALSE(staff->noteShapesFromStyle);
        auto noteShapes = staff->getNoteShapes();
        EXPECT_FALSE(noteShapes);
    }
    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 2, 1, 0);
        ASSERT_TRUE(staff);
        EXPECT_TRUE(staff->useNoteShapes);
        EXPECT_EQ(staff->noteShapesId, staff->getCmper());
        EXPECT_FALSE(staff->noteShapesFromStyle);
        auto noteShapes = staff->getNoteShapes();
        ASSERT_TRUE(noteShapes);
        EXPECT_EQ(noteShapes->getCmper1(), staff->getCmper());
        EXPECT_TRUE(noteShapes->arrangedByPitch);
    }
    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 3, 1, 0);
        ASSERT_TRUE(staff);
        EXPECT_FALSE(staff->useNoteShapes);
        EXPECT_EQ(staff->noteShapesId, 0);
        EXPECT_FALSE(staff->noteShapesFromStyle);
        auto noteShapes = staff->getNoteShapes();
        ASSERT_TRUE(noteShapes);
        EXPECT_EQ(noteShapes->getCmper1(), 0);
        EXPECT_FALSE(noteShapes->arrangedByPitch);
    }
}

TEST(StaffTest, NoteShapesStaffStyle)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "note_shapes.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 2, 2, 0);
        ASSERT_TRUE(staff);
        static constexpr Cmper kExpectedStaffStyleId = 4;
        EXPECT_TRUE(staff->useNoteShapes);
        EXPECT_EQ(staff->noteShapesId, kExpectedStaffStyleId);
        EXPECT_TRUE(staff->noteShapesFromStyle);
        auto noteShapes = staff->getNoteShapes();
        ASSERT_TRUE(noteShapes);
        EXPECT_EQ(noteShapes->getCmper1(), kExpectedStaffStyleId);
        EXPECT_FALSE(noteShapes->arrangedByPitch);
    }
    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 3, 2, 0);
        ASSERT_TRUE(staff);
        EXPECT_TRUE(staff->useNoteShapes);
        EXPECT_EQ(staff->noteShapesId, 0);
        EXPECT_TRUE(staff->noteShapesFromStyle);
        auto noteShapes = staff->getNoteShapes();
        EXPECT_FALSE(noteShapes);
    }
}

TEST(StaffTest, NoteShapeDefaultOff)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "note_shapes.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto noteRestOptions = doc->getOptions()->get<options::NoteRestOptions>();
    ASSERT_TRUE(noteRestOptions);
    options::NoteRestOptions* mutableNoteRestOptions = const_cast<options::NoteRestOptions*>(noteRestOptions.get());
    mutableNoteRestOptions->doShapeNotes = false;

    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 2, 2, 0);
        ASSERT_TRUE(staff);
        static constexpr Cmper kExpectedStaffStyleId = 4;
        EXPECT_TRUE(staff->useNoteShapes);
        EXPECT_EQ(staff->noteShapesId, kExpectedStaffStyleId);
        EXPECT_TRUE(staff->noteShapesFromStyle);
        auto noteShapes = staff->getNoteShapes();
        ASSERT_TRUE(noteShapes);
        EXPECT_EQ(noteShapes->getCmper1(), kExpectedStaffStyleId);
        EXPECT_FALSE(noteShapes->arrangedByPitch);
    }
    {
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 3, 1, 0);
        ASSERT_TRUE(staff);
        EXPECT_FALSE(staff->useNoteShapes);
        EXPECT_EQ(staff->noteShapesId, 0);
        EXPECT_FALSE(staff->noteShapesFromStyle);
        auto noteShapes = staff->getNoteShapes();
        EXPECT_FALSE(noteShapes);
    }
}

TEST(StaffTest, LyricsLineInfo)
{
    std::vector<char> syllXml;
    musxtest::readFile(musxtest::getInputPath() / "syllables.enigmaxml", syllXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(syllXml);
    ASSERT_TRUE(doc);

    auto staff = doc->getOthers()->get<others::Staff>(SCORE_PARTID, 1);
    ASSERT_TRUE(staff);
    {
        auto lyricsLineInfo = staff->createLyricsLineInfo(1);
        ASSERT_EQ(lyricsLineInfo.size(), 3);
        EXPECT_EQ(lyricsLineInfo[0].lyricNumber, 1);
        EXPECT_EQ(lyricsLineInfo[1].lyricNumber, 2);
        EXPECT_EQ(lyricsLineInfo[2].lyricNumber, 3);
    }
    {
        auto lyricsLineInfo = staff->createLyricsLineInfo(2);
        ASSERT_EQ(lyricsLineInfo.size(), 2);
        EXPECT_EQ(lyricsLineInfo[0].lyricNumber, 4);
        EXPECT_EQ(lyricsLineInfo[1].lyricNumber, 6);
    }
}
