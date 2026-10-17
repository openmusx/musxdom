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

#include <algorithm>

using namespace musx::dom;

constexpr static musxtest::string_view staffGroupXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <instUsed cmper="0" inci="0">
      <inst>1</inst>
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
      <inst>3</inst>
      <trackType>staff</trackType>
      <distFromTop>-264</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <instUsed cmper="0" inci="2">
      <inst>2</inst>
      <trackType>staff</trackType>
      <distFromTop>-528</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <instUsed cmper="0" inci="3">
      <inst>4</inst>
      <trackType>staff</trackType>
      <distFromTop>-956</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <partDef cmper="0">
      <nameID>1</nameID>
      <partOrder>0</partOrder>
      <copies>1</copies>
      <printPart/>
    </partDef>
    <partGlobals cmper="65534">
      <showTransposed/>
      <studioViewIUlist>65400</studioViewIUlist>
    </partGlobals>
  </others>
  <details>
    <staffGroup cmper1="0" cmper2="1">
      <startInst>1</startInst>
      <endInst>2</endInst>
      <startMeas>1</startMeas>
      <endMeas>32767</endMeas>
      <fullID>42</fullID>
      <fullXadj>-247</fullXadj>
      <fullYadj>6</fullYadj>
      <bracket>
        <id>8</id>
        <bracPos>-13</bracPos>
        <bracTop>1</bracTop>
        <bracBot>2</bracBot>
        <onSingle/>
      </bracket>
      <barline>double</barline>
      <fullJustify>center</fullJustify>
      <abbrvJustify>center</abbrvJustify>
      <groupBarlineStyle>Mensurstriche</groupBarlineStyle>
      <ownBarline/>
      <fullIndivPos/>
      <abbrvIndivPos/>
      <hideName/>
      <abbrvID>46</abbrvID>
      <abbrvXadj>-106</abbrvXadj>
      <abbrvYadj>4</abbrvYadj>
      <fullHAlign>center</fullHAlign>
      <abbrvHAlign>center</abbrvHAlign>
      <fullExpand/>
      <abbrvExpand/>
      <optimize>none</optimize>
    </staffGroup>
  </details>
</finale>
)xml";

constexpr static musxtest::string_view unresolvedInstrumentMapDiagnosticXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <instUsed cmper="0" inci="0">
      <inst>1</inst>
      <trackType>staff</trackType>
    </instUsed>
    <partDef cmper="0">
      <defaultNameStaff>1</defaultNameStaff>
    </partDef>
    <partGlobals cmper="65534"/>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
    </staffSpec>
  </others>
  <details>
    <staffGroup cmper1="0" cmper2="1">
      <startInst>1</startInst>
      <endInst>2</endInst>
      <startMeas>1</startMeas>
      <endMeas>32767</endMeas>
    </staffGroup>
  </details>
</finale>
)xml";

constexpr static musxtest::string_view singleInstrumentSectionXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <instUsed cmper="0" inci="0">
      <inst>1</inst>
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
      <inst>2</inst>
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
      <inst>3</inst>
      <trackType>staff</trackType>
      <distFromTop>-576</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <instUsed cmper="0" inci="3">
      <inst>4</inst>
      <trackType>staff</trackType>
      <distFromTop>-864</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <instUsed cmper="0" inci="4">
      <inst>5</inst>
      <trackType>staff</trackType>
      <distFromTop>-1152</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <multiStaffInstGroup cmper="1">
      <staffNum1>4</staffNum1>
      <staffNum2>5</staffNum2>
    </multiStaffInstGroup>
    <multiStaffGroupID cmper="1">
      <staffGroupID>4</staffGroupID>
    </multiStaffGroupID>
    <partDef cmper="0">
      <nameID>1</nameID>
      <partOrder>0</partOrder>
      <copies>1</copies>
      <printPart/>
    </partDef>
    <partGlobals cmper="65534">
      <showTransposed/>
      <studioViewIUlist>65400</studioViewIUlist>
    </partGlobals>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>2575136d-9927-47bd-a727-f554c169257e</instUuid>
    </staffSpec>
    <staffSpec cmper="2">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>2575136d-9927-47bd-a727-f554c169257e</instUuid>
    </staffSpec>
    <staffSpec cmper="3">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>172a455a-d1b9-4a03-aa80-c863d587f209</instUuid>
    </staffSpec>
    <staffSpec cmper="4">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>776a2734-3f38-4cac-8fa2-90e62b368ec1</instUuid>
    </staffSpec>
    <staffSpec cmper="5">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>776a2734-3f38-4cac-8fa2-90e62b368ec1</instUuid>
    </staffSpec>
  </others>
  <details>
    <staffGroup cmper1="0" cmper2="1">
      <startInst>1</startInst>
      <endInst>2</endInst>
      <startMeas>1</startMeas>
      <endMeas>32767</endMeas>
      <fullID>101</fullID>
    </staffGroup>
    <staffGroup cmper1="0" cmper2="2">
      <startInst>1</startInst>
      <endInst>3</endInst>
      <startMeas>1</startMeas>
      <endMeas>32767</endMeas>
      <fullID>102</fullID>
    </staffGroup>
    <staffGroup cmper1="0" cmper2="3">
      <startInst>1</startInst>
      <endInst>1</endInst>
      <startMeas>1</startMeas>
      <endMeas>32767</endMeas>
      <fullID>103</fullID>
    </staffGroup>
    <staffGroup cmper1="0" cmper2="4">
      <startInst>4</startInst>
      <endInst>5</endInst>
      <startMeas>1</startMeas>
      <endMeas>32767</endMeas>
      <fullID>104</fullID>
    </staffGroup>
  </details>
</finale>
)xml";

TEST(StaffGroupTest, PopulateFields)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(staffGroupXml);
    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    auto staffGroup = details->get<details::StaffGroup>(SCORE_PARTID, doc->calcScrollViewCmper(SCORE_PARTID), 1);
    ASSERT_TRUE(staffGroup) << "StaffGroup with cmper1 0 and cmper2 1 not found";

    EXPECT_EQ(staffGroup->startInst, 1);
    EXPECT_EQ(staffGroup->endInst, 2);
    EXPECT_EQ(staffGroup->startMeas, 1);
    EXPECT_EQ(staffGroup->endMeas, 32767);
    EXPECT_EQ(staffGroup->fullNameId, 42);
    EXPECT_EQ(staffGroup->fullNameXadj, -247);
    EXPECT_EQ(staffGroup->fullNameYadj, 6);

    ASSERT_TRUE(staffGroup->bracket);
    EXPECT_EQ(staffGroup->bracket->style, details::Bracket::BracketStyle::DeskBracket);
    EXPECT_EQ(staffGroup->bracket->horzAdjLeft, -13);
    EXPECT_EQ(staffGroup->bracket->vertAdjTop, 1);
    EXPECT_EQ(staffGroup->bracket->vertAdjBot, 2);
    EXPECT_TRUE(staffGroup->bracket->showOnSingleStaff);

    EXPECT_EQ(staffGroup->barlineType, details::StaffGroup::BarlineType::Double);
    EXPECT_EQ(staffGroup->fullNameJustify, AlignJustify::Center);
    EXPECT_EQ(staffGroup->abbrvNameJustify, AlignJustify::Center);
    EXPECT_EQ(staffGroup->drawBarlines, details::StaffGroup::DrawBarlineStyle::Mensurstriche);
    EXPECT_EQ(staffGroup->abbrvNameId, 46);
    EXPECT_EQ(staffGroup->abbrvNameXadj, -106);
    EXPECT_EQ(staffGroup->abbrvNameYadj, 4);
    EXPECT_EQ(staffGroup->fullNameAlign, AlignJustify::Center);
    EXPECT_EQ(staffGroup->abbrvNameAlign, AlignJustify::Center);
    EXPECT_TRUE(staffGroup->ownBarline);
    EXPECT_TRUE(staffGroup->fullNameIndivPos);
    EXPECT_TRUE(staffGroup->abbrvNameIndivPos);
    EXPECT_TRUE(staffGroup->fullNameExpand);
    EXPECT_TRUE(staffGroup->abbrvNameExpand);
    EXPECT_EQ(staffGroup->hideStaves, details::StaffGroup::HideStaves::None);
    EXPECT_TRUE(staffGroup->hideName);

    EXPECT_EQ(staffGroup->staves.size(), 3) << "group has 3 staves";
    EXPECT_NE(staffGroup->staves.find(1), staffGroup->staves.end()) << "group contains staff 1";
    EXPECT_NE(staffGroup->staves.find(2), staffGroup->staves.end()) << "group contains staff 2";
    EXPECT_NE(staffGroup->staves.find(3), staffGroup->staves.end()) << "group contains staff 3";
    EXPECT_EQ(staffGroup->staves.find(4), staffGroup->staves.end()) << "group does not contain staff 4";
}

TEST(StaffGroupTest, CustomBarShape)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "group_cust_barline.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);
    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    auto staffGroup = details->get<details::StaffGroup>(SCORE_PARTID, doc->calcScrollViewCmper(SCORE_PARTID), 1);
    ASSERT_TRUE(staffGroup);
    EXPECT_EQ(staffGroup->barlineType, details::StaffGroup::BarlineType::Custom);
    EXPECT_TRUE(staffGroup->ownBarline);
    EXPECT_EQ(staffGroup->customBarShape, 5);

    auto shape = doc->getOthers()->get<others::ShapeDef>(SCORE_PARTID, staffGroup->customBarShape);
    EXPECT_TRUE(shape) << "custom barline shape should exist";
}

TEST(StaffGroupTest, DiagnosticPartNameDoesNotRequireInstrumentMap)
{
    auto previousLogger = musx::util::Logger::getCallback();
    struct LoggerRestorer {
        musx::util::Logger::LogCallback callback;
        ~LoggerRestorer() { musx::util::Logger::setCallback(std::move(callback)); }
    } loggerRestorer{previousLogger};

    std::string diagnostic;
    musx::util::Logger::setCallback([&](musx::util::Logger::LogLevel, const std::string& message) {
        diagnostic = message;
    });

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(unresolvedInstrumentMapDiagnosticXml);
    ASSERT_TRUE(doc);
    EXPECT_NE(diagnostic.find("non-existent start or end staff cmpers"), std::string::npos);
    EXPECT_NO_THROW((void)doc->getInstruments());
}

TEST(StaffGroupTest, MeasureRangeDiagnostic)
{
    auto previousLogger = musx::util::Logger::getCallback();
    struct LoggerRestorer {
        musx::util::Logger::LogCallback callback;
        ~LoggerRestorer() { musx::util::Logger::setCallback(std::move(callback)); }
    } loggerRestorer{previousLogger};

    std::vector<std::string> diagnostics;
    musx::util::Logger::setCallback([&](musx::util::Logger::LogLevel, const std::string& message) {
        if (message.find("starts at measure") != std::string::npos) {
            diagnostics.push_back(message);
        }
    });

    const auto withRange = [](const std::string& start, const std::string& end) {
        std::string xml(staffGroupXml);
        const auto group = xml.find("<staffGroup");
        xml.replace(xml.find("<startMeas>1<", group), 13, "<startMeas>" + start + "<");
        xml.replace(xml.find("<endMeas>32767<", group), 15, "<endMeas>" + end + "<");
        return xml;
    };

    // The end measure is inclusive, so a group may span a single measure.
    ASSERT_TRUE(musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(withRange("3", "3")));
    EXPECT_TRUE(diagnostics.empty());

    ASSERT_TRUE(musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(withRange("3", "2")));
    EXPECT_EQ(diagnostics.size(), 1);
}

TEST(StaffGroupTest, CalcIsSingleInstrumentSection)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(singleInstrumentSectionXml);
    ASSERT_TRUE(doc);

    auto scrollView = doc->getScrollViewStaves(SCORE_PARTID);
    auto groups = details::StaffGroupInfo::getGroupsAtMeasure(1, SCORE_PARTID, scrollView);
    ASSERT_EQ(groups.size(), 4);

    auto findGroup = [&](Cmper groupId) {
        auto it = std::find_if(groups.begin(), groups.end(), [groupId](const details::StaffGroupInfo& groupInfo) {
            return groupInfo.group->getCmper2() == groupId;
        });
        EXPECT_NE(it, groups.end()) << "group " << groupId << " not found";
        return it;
    };

    auto fluteSection = findGroup(1);
    ASSERT_NE(fluteSection, groups.end());
    EXPECT_TRUE(fluteSection->calcIsSingleInstrumentSection(1));

    auto mixedGroup = findGroup(2);
    ASSERT_NE(mixedGroup, groups.end());
    EXPECT_FALSE(mixedGroup->calcIsSingleInstrumentSection(1));

    auto singleStaffGroup = findGroup(3);
    ASSERT_NE(singleStaffGroup, groups.end());
    EXPECT_FALSE(singleStaffGroup->calcIsSingleInstrumentSection(1));

    auto pianoInstrument = findGroup(4);
    ASSERT_NE(pianoInstrument, groups.end());
    EXPECT_FALSE(pianoInstrument->calcIsSingleInstrumentSection(1));
}
