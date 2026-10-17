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

constexpr static musxtest::string_view pageXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <pageSpec cmper="1">
      <height>3744</height>
      <width>2880</width>
      <percent>80</percent>
      <firstSystem>1</firstSystem>
      <margTop>-288</margTop>
      <margLeft>288</margLeft>
      <margBottom>288</margBottom>
      <margRight>-288</margRight>
    </pageSpec>
    <pageSpec cmper="2">
      <height>1584</height>
      <width>2448</width>
      <percent>95</percent>
      <firstSystem>-1</firstSystem>
      <margTop>-288</margTop>
      <margLeft>288</margLeft>
      <margBottom>288</margBottom>
      <margRight>-288</margRight>
    </pageSpec>
  </others>
</finale>
)xml";

constexpr static musxtest::string_view uncalculatedLinkedPartLayoutXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <pageSpec cmper="1">
      <firstSystem>1</firstSystem>
    </pageSpec>
    <pageSpec cmper="1" part="1" shared="false">
      <firstSystem>1</firstSystem>
    </pageSpec>
    <pageSpec cmper="2" part="1" shared="false"/>
    <staffSystemSpec cmper="1">
      <startMeas>1</startMeas>
      <endMeas>2</endMeas>
    </staffSystemSpec>
    <staffSystemSpec cmper="1" part="1" shared="false">
      <startMeas>1</startMeas>
      <endMeas>2</endMeas>
    </staffSystemSpec>
    <staffSystemSpec cmper="2" part="1" shared="false">
      <startMeas>0</startMeas>
      <endMeas>0</endMeas>
    </staffSystemSpec>
    <partDef cmper="0"/>
    <partDef cmper="1">
      <needsRecalc/>
    </partDef>
    <partGlobals cmper="65534"/>
    <partGlobals cmper="65534" part="1" shared="false"/>
  </others>
</finale>
)xml";

constexpr static musxtest::string_view uncalculatedScoreLayoutXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <pageSpec cmper="1"/>
    <staffSystemSpec cmper="1">
      <startMeas>0</startMeas>
      <endMeas>0</endMeas>
    </staffSystemSpec>
    <partDef cmper="0"/>
    <partGlobals cmper="65534"/>
  </others>
</finale>
)xml";

TEST(PageTest, PopulateFields)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(pageXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto page = others->get<others::Page>(SCORE_PARTID, 1);
    ASSERT_TRUE(page) << "Page with cmper 1 not found";

    EXPECT_EQ(page->height, 3744);
    EXPECT_EQ(page->width, 2880);
    EXPECT_EQ(page->percent, 80);
    EXPECT_EQ(page->firstSystemId, 1);
    EXPECT_FALSE(page->holdMargins);
    EXPECT_EQ(page->margTop, -288);
    EXPECT_EQ(page->margLeft, 288);
    EXPECT_EQ(page->margBottom, 288);
    EXPECT_EQ(page->margRight, -288);
    EXPECT_FALSE(page->isBlank());

    page = others->get<others::Page>(SCORE_PARTID, 2);
    ASSERT_TRUE(page) << "Page with cmper 2 not found";
    EXPECT_TRUE(page->isBlank());
}

TEST(PageTest, PartPage)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "part_page.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    static constexpr Cmper kPartId = 1;
    auto page1 = doc->getOthers()->get<others::Page>(kPartId, 1);
    ASSERT_TRUE(page1);
    EXPECT_EQ(page1->getSourcePartId(), kPartId);
    EXPECT_EQ(page1->firstMeasureId, 1);
    EXPECT_EQ(page1->lastMeasureId, 101);
    auto page2 = doc->getOthers()->get<others::Page>(kPartId, 2);
    ASSERT_FALSE(page2);
}

TEST(PageTest, UncalculatedLinkedPartLayoutIsVerbose)
{
    auto previousLogger = musx::util::Logger::getCallback();
    struct LoggerRestorer {
        musx::util::Logger::LogCallback callback;
        ~LoggerRestorer() { musx::util::Logger::setCallback(std::move(callback)); }
    } loggerRestorer{previousLogger};

    std::vector<std::pair<musx::util::Logger::LogLevel, std::string>> diagnostics;
    musx::util::Logger::setCallback([&](musx::util::Logger::LogLevel level, const std::string& message) {
        diagnostics.emplace_back(level, message);
    });

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(uncalculatedLinkedPartLayoutXml);
    ASSERT_TRUE(doc);

    auto scorePage = doc->getOthers()->get<others::Page>(SCORE_PARTID, 1);
    ASSERT_TRUE(scorePage);
    EXPECT_EQ(scorePage->lastSystemId, 1);
    EXPECT_EQ(scorePage->firstMeasureId, 1);
    EXPECT_EQ(scorePage->lastMeasureId, 1);
    EXPECT_TRUE(scorePage->isLayoutCalculated());
    auto scoreSystem = doc->getOthers()->get<others::StaffSystem>(SCORE_PARTID, 1);
    ASSERT_TRUE(scoreSystem);
    EXPECT_EQ(scoreSystem->pageId, 1);

    auto partPage1 = doc->getOthers()->get<others::Page>(1, 1);
    auto partPage2 = doc->getOthers()->get<others::Page>(1, 2);
    ASSERT_TRUE(partPage1);
    ASSERT_TRUE(partPage2);
    EXPECT_FALSE(partPage1->lastSystemId);
    EXPECT_FALSE(partPage1->firstMeasureId);
    EXPECT_FALSE(partPage1->lastMeasureId);
    EXPECT_FALSE(partPage2->lastSystemId);
    EXPECT_FALSE(partPage2->firstMeasureId);
    EXPECT_FALSE(partPage2->lastMeasureId);
    EXPECT_FALSE(partPage1->isLayoutCalculated());
    EXPECT_FALSE(partPage2->isLayoutCalculated());

    auto score = doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, SCORE_PARTID);
    auto part = doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, 1);
    ASSERT_TRUE(score);
    ASSERT_TRUE(part);
    EXPECT_TRUE(score->isLayoutCalculated());
    EXPECT_FALSE(part->isLayoutCalculated());
    EXPECT_EQ(score->numberOfPages, 1);
    EXPECT_EQ(part->numberOfPages, 2);
    std::string totalPages;
    musx::util::EnigmaString::parseEnigmaText(doc, 1, "^totpages()",
        [&](const std::string& text, const musx::util::EnigmaStyles&) {
            totalPages += text;
            return true;
        });
    EXPECT_EQ(totalPages, "2");
    EXPECT_EQ(doc->calcPageFromMeasure(1, 1), nullptr);
    EXPECT_EQ(doc->calcSystemFromMeasure(1, 1), nullptr);
    auto partSystem = doc->getOthers()->get<others::StaffSystem>(1, 1);
    ASSERT_TRUE(partSystem);
    EXPECT_EQ(partSystem->getPage(), nullptr);

    ASSERT_EQ(diagnostics.size(), 3);
    for (const auto& [level, message] : diagnostics) {
        EXPECT_EQ(level, musx::util::Logger::LogLevel::Verbose) << message;
    }
    EXPECT_NE(diagnostics[0].second.find("system 2"), std::string::npos);
    EXPECT_NE(diagnostics[0].second.find("has not been calculated"), std::string::npos);
    EXPECT_NE(diagnostics[1].second.find("of part 1 cannot be determined"), std::string::npos);
    EXPECT_NE(diagnostics[2].second.find("has not been calculated"), std::string::npos);
}

TEST(PageTest, UncalculatedScoreLayoutIsInformational)
{
    auto previousLogger = musx::util::Logger::getCallback();
    struct LoggerRestorer {
        musx::util::Logger::LogCallback callback;
        ~LoggerRestorer() { musx::util::Logger::setCallback(std::move(callback)); }
    } loggerRestorer{previousLogger};

    std::vector<std::pair<musx::util::Logger::LogLevel, std::string>> diagnostics;
    musx::util::Logger::setCallback([&](musx::util::Logger::LogLevel level, const std::string& message) {
        diagnostics.emplace_back(level, message);
    });

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(uncalculatedScoreLayoutXml);
    ASSERT_TRUE(doc);

    auto page = doc->getOthers()->get<others::Page>(SCORE_PARTID, 1);
    ASSERT_TRUE(page);
    EXPECT_FALSE(page->lastSystemId);
    EXPECT_FALSE(page->firstMeasureId);
    EXPECT_FALSE(page->lastMeasureId);
    EXPECT_FALSE(page->isLayoutCalculated());

    auto score = doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, SCORE_PARTID);
    ASSERT_TRUE(score);
    EXPECT_FALSE(score->isLayoutCalculated());
    EXPECT_EQ(score->numberOfPages, 1);
    EXPECT_EQ(doc->calcPageFromMeasure(SCORE_PARTID, 1), nullptr);
    EXPECT_EQ(doc->calcSystemFromMeasure(SCORE_PARTID, 1), nullptr);

    // Each uncalculated system is verbose; the score's state is reported once at Info.
    ASSERT_EQ(diagnostics.size(), 3);
    EXPECT_EQ(diagnostics.back().first, musx::util::Logger::LogLevel::Info);
    EXPECT_EQ(diagnostics.back().second, "The score's layout has not been fully calculated.");
    diagnostics.pop_back();
    for (const auto& [level, message] : diagnostics) {
        EXPECT_EQ(level, musx::util::Logger::LogLevel::Verbose) << message;
        EXPECT_NE(message.find("has not been calculated"), std::string::npos);
    }
}

constexpr static musxtest::string_view splitMeasureSystemXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <measSpec cmper="1">
      <width>600</width>
      <posSplit/>
    </measSpec>
    <measSpec cmper="2">
      <width>600</width>
    </measSpec>
    <pageSpec cmper="1">
      <firstSystem>1</firstSystem>
    </pageSpec>
    <staffSystemSpec cmper="1">
      <startMeas>1</startMeas>
      <endMeas>1</endMeas>
    </staffSystemSpec>
    <staffSystemSpec cmper="2">
      <startMeas>1</startMeas>
      <endMeas>3</endMeas>
    </staffSystemSpec>
    <splitMeas cmper="1">
      <data>300</data>
    </splitMeas>
    <partDef cmper="0"/>
    <partGlobals cmper="65534"/>
  </others>
</finale>
)xml";

TEST(PageTest, SystemHoldingOnlySplitMeasureStart)
{
    // System 1 holds only the first part of measure 1, so it stores measure 1 as both start and end.
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(splitMeasureSystemXml);
    ASSERT_TRUE(doc);
    auto page = doc->getOthers()->get<others::Page>(SCORE_PARTID, 1);
    ASSERT_TRUE(page);
    EXPECT_EQ(page->lastSystemId, 2);
    EXPECT_EQ(page->firstMeasureId, 1);
    EXPECT_EQ(page->lastMeasureId, 2);

    // Without the split point, the same range is a stale layout, so the page is not linked.
    std::string unsplitXml(splitMeasureSystemXml);
    const auto splitStart = unsplitXml.find("<splitMeas");
    const auto splitEnd = unsplitXml.find("</splitMeas>") + std::string("</splitMeas>").size();
    unsplitXml.erase(splitStart, splitEnd - splitStart);
    auto unsplit = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(unsplitXml);
    ASSERT_TRUE(unsplit);
    auto unsplitPage = unsplit->getOthers()->get<others::Page>(SCORE_PARTID, 1);
    ASSERT_TRUE(unsplitPage);
    EXPECT_FALSE(unsplitPage->firstMeasureId);
    EXPECT_FALSE(unsplitPage->isLayoutCalculated());
}
