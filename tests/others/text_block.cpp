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

TEST(TextBlockTest, PopulateFields)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <textBlock cmper="32">
      <textID>31</textID>
      <lineSpacingPercent>100</lineSpacingPercent>
      <justify>full</justify>
      <newPos36/>
      <showShape/>
      <noExpandSingleWord/>
      <wordWrap/>
      <width>436</width>
      <height>117</height>
      <roundCorners/>
      <cornerRadius>512</cornerRadius>
      <textTag>block</textTag>
    </textBlock>
    <textBlock cmper="33">
      <textID>221</textID>
      <lineSpacingEvpu>78</lineSpacingEvpu>
      <justify>forcedFull</justify>
      <newPos36/>
      <showShape/>
      <wordWrap/>
      <textTag>expression</textTag>
    </textBlock>
    <textBlock cmper="34">
      <textID>222</textID>
      <textTag>block</textTag>
    </textBlock>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    // Test first text block
    auto textBlock1 = others->get<others::TextBlock>(SCORE_PARTID, 32);
    ASSERT_TRUE(textBlock1) << "TextBlock 32 not found but does exist";
    EXPECT_EQ(textBlock1->textId, 31);
    EXPECT_EQ(textBlock1->lineSpacingPercentage, 100);
    EXPECT_FALSE(textBlock1->lineSpacingEvpu.has_value());
    EXPECT_EQ(textBlock1->justify, others::TextBlock::TextJustify::Full);
    EXPECT_TRUE(textBlock1->newPos36);
    EXPECT_TRUE(textBlock1->showShape);
    EXPECT_TRUE(textBlock1->noExpandSingleWord);
    EXPECT_TRUE(textBlock1->wordWrap);
    EXPECT_EQ(textBlock1->width, 436);
    EXPECT_EQ(textBlock1->height, 117);
    EXPECT_TRUE(textBlock1->roundCorners);
    EXPECT_EQ(textBlock1->cornerRadius, 512);
    EXPECT_EQ(textBlock1->textType, others::TextBlock::TextType::Block);

    // Test second text block
    auto textBlock2 = others->get<others::TextBlock>(SCORE_PARTID, 33);
    ASSERT_TRUE(textBlock2) << "TextBlock 33 not found but does exist";
    EXPECT_EQ(textBlock2->textId, 221);
    EXPECT_FALSE(textBlock2->lineSpacingPercentage.has_value());
    EXPECT_EQ(textBlock2->lineSpacingEvpu, 78);
    EXPECT_EQ(textBlock2->justify, others::TextBlock::TextJustify::ForcedFull);
    EXPECT_TRUE(textBlock2->newPos36);
    EXPECT_TRUE(textBlock2->showShape);
    EXPECT_FALSE(textBlock2->noExpandSingleWord);
    EXPECT_TRUE(textBlock2->wordWrap);
    EXPECT_EQ(textBlock2->width, 0);  // Default value for optional fields
    EXPECT_EQ(textBlock2->height, 0);
    EXPECT_FALSE(textBlock2->roundCorners);
    EXPECT_EQ(textBlock2->cornerRadius, 0);
    EXPECT_EQ(textBlock2->textType, others::TextBlock::TextType::Expression);

    auto textBlock3 = others->get<others::TextBlock>(SCORE_PARTID, 34);
    ASSERT_TRUE(textBlock3) << "TextBlock 34 not found but does exist";
    EXPECT_FALSE(textBlock3->lineSpacingPercentage.has_value());
    EXPECT_EQ(textBlock3->lineSpacingEvpu, 0);
}

TEST(PageTextAssignTest, PopulateFields)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <pageTextAssign cmper="0" inci="0">
      <block>2</block>
      <xdisp>5</xdisp>
      <ydisp>7</ydisp>
      <startPage>1</startPage>
      <endPage>2</endPage>
      <hposLp>right</hposLp>
      <hposRp>center</hposRp>
      <postIt/>
      <vpos>bottom</vpos>
      <indRpPos/>
      <rightPgXdisp>-13</rightPgXdisp>
      <rightPgYdisp>139</rightPgYdisp>
    </pageTextAssign>
    <pageTextAssign cmper="0" inci="0" part="1" shared="true">
      <postIt>
        <offInPart/>
      </postIt>
      <hposPageEdge/>
      <indRpPos/>
      <rightPgXdisp>70</rightPgXdisp>
      <rightPgYdisp>93</rightPgYdisp>
    </pageTextAssign>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    // Test first page text assignment (SCORE_PARTID)
    auto pageText1 = others->get<others::PageTextAssign>(SCORE_PARTID, 0, 0);
    ASSERT_TRUE(pageText1) << "PageTextAssign 0 not found for SCORE_PARTID but does exist";
    EXPECT_EQ(pageText1->block, 2);
    EXPECT_EQ(pageText1->xDisp, 5);
    EXPECT_EQ(pageText1->yDisp, 7);
    EXPECT_EQ(pageText1->startPage, 1);
    EXPECT_EQ(pageText1->endPage, 2);
    EXPECT_EQ(pageText1->hPosLp, others::PageTextAssign::HorizontalAlignment::Right);
    EXPECT_EQ(pageText1->hPosRp, others::PageTextAssign::HorizontalAlignment::Center);
    EXPECT_TRUE(pageText1->hidden);   // postIt sets screen-only visibility
    EXPECT_FALSE(pageText1->hPosPageEdge); // present postIt again
    EXPECT_EQ(pageText1->vPos, others::PageTextAssign::VerticalAlignment::Bottom);
    EXPECT_TRUE(pageText1->indRpPos);
    EXPECT_EQ(pageText1->rightPgXDisp, -13);
    EXPECT_EQ(pageText1->rightPgYDisp, 139);

    // Test second page text assignment (part 1) — should inherit from first
    auto pageText2 = others->get<others::PageTextAssign>(1, 0, 0);
    ASSERT_TRUE(pageText2) << "PageTextAssign 0 not found for part 1 but does exist";

    // Inherited fields
    EXPECT_EQ(pageText2->block, pageText1->block);
    EXPECT_EQ(pageText2->xDisp, pageText1->xDisp);
    EXPECT_EQ(pageText2->yDisp, pageText1->yDisp);
    EXPECT_EQ(pageText2->startPage, pageText1->startPage);
    EXPECT_EQ(pageText2->endPage, pageText1->endPage);
    EXPECT_EQ(pageText2->hPosLp, pageText1->hPosLp);
    EXPECT_EQ(pageText2->hPosRp, pageText1->hPosRp);
    EXPECT_EQ(pageText2->vPos, pageText1->vPos);
    EXPECT_TRUE(pageText2->hPosPageEdge);
    EXPECT_TRUE(pageText2->indRpPos);

    // Overridden fields
    EXPECT_FALSE(pageText2->hidden);   // postIt sets screen-only visibility
    EXPECT_EQ(pageText2->rightPgXDisp, 70);
    EXPECT_EQ(pageText2->rightPgYDisp, 93);
}

TEST(MeasureTextAssignTest, PopulateFields)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <details>
    <measTextAssign cmper1="2" cmper2="1" inci="0">
      <block>2</block>
      <xdispEdu>728</xdispEdu>
      <ydisp>104</ydisp>
      <postIt/>
    </measTextAssign>
  </details>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    auto assign = details->get<details::MeasureTextAssign>(SCORE_PARTID, 2, 1, 0);
    ASSERT_TRUE(assign);

    EXPECT_EQ(assign->block, 2);
    EXPECT_EQ(assign->xDispEdu, 728);
    EXPECT_EQ(assign->yDisp, 104);
    EXPECT_TRUE(assign->hidden);
}

TEST(PageTextAssignText, ParseSinglePageAssignment1)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "page_text.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    {
        auto pageAssign = others::PageTextAssign::getForPageId(doc, SCORE_PARTID, 2, 0);
        std::string text = pageAssign->getRawTextCtx(SCORE_PARTID).getText();
        EXPECT_EQ(text, "Single Page Text - Page 2 of 3");
    }

    {
        auto pageAssigns = others::PageTextAssign::getArrayForPageId(doc, 1, 1);
        ASSERT_EQ(pageAssigns.size(), 1);
        std::string text = pageAssigns[0]->getRawTextCtx(1).getText();
        EXPECT_EQ(text, "Single Page Text - Page 1 of 1");
    }

    {
        auto pageAssign = others::PageTextAssign::getForPageId(doc, 2, 4, 0);
        std::string text = pageAssign->getRawTextCtx(2).getText();
        EXPECT_EQ(text, "Single Page Text - Page 4 of 6");
    }

    {
        auto pageAssign = doc->getOthers()->get<others::PageTextAssign>(SCORE_PARTID, 2, 0);
        EXPECT_FALSE(pageAssign);
    }
}

TEST(PageTextAssignText, ParseSinglePageAssignment2)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "page_text.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    PageCmper pageAssignmentId = 3;
    std::vector<std::optional<PageCmper>> expectedValues = { std::nullopt, std::nullopt, 3 };
    for (Cmper partId = 0; partId < expectedValues.size(); partId++) {
        auto pageAssign = doc->getOthers()->get<others::PageTextAssign>(partId, pageAssignmentId, 0);
        ASSERT_TRUE(pageAssign) << "page assignment " << pageAssignmentId << " should exist in all parts";
        EXPECT_EQ(expectedValues[partId], pageAssign->calcStartPageNumber(partId));
        EXPECT_EQ(expectedValues[partId], pageAssign->calcEndPageNumber(partId));
    }

    pageAssignmentId = 1;
    expectedValues = { 1, std::nullopt, 1 };
    for (Cmper partId = 0; partId < expectedValues.size(); partId++) {
        auto pageAssign = doc->getOthers()->get<others::PageTextAssign>(partId, pageAssignmentId, 0);
        ASSERT_TRUE(pageAssign) << "page assignment " << pageAssignmentId << " should exist in all parts";
        EXPECT_EQ(expectedValues[partId], pageAssign->calcStartPageNumber(partId));
        EXPECT_EQ(expectedValues[partId], pageAssign->calcEndPageNumber(partId));
    }
}

static void testMultiPageAssignment(const DocumentPtr& doc, Cmper partId, Inci inci, Cmper expectedMinPage, Cmper expectedMaxPage,
    const std::string& expectedPre, const std::string& expectedPost)
{
    auto part = doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, partId);
    ASSERT_TRUE(part) << "unable to find part " << partId;
    auto pageAssign = doc->getOthers()->get<others::PageTextAssign>(partId, 0, inci);
    ASSERT_TRUE(pageAssign) << "unable to find page text at assignement id: 0 inci: " << inci;
    PageCmper minPage = pageAssign->calcStartPageNumber(partId).value_or(0);
    ASSERT_EQ(minPage, expectedMinPage);
    PageCmper maxPage = pageAssign->calcEndPageNumber(partId).value_or(0);
    ASSERT_EQ(maxPage, expectedMaxPage);
    for (PageCmper pageId = minPage; pageId <= maxPage; pageId++) {
        std::string text = pageAssign->getRawTextCtx(partId, pageId).getText();
        std::string expectedText = expectedPre + std::to_string(pageId) + expectedPost;
        EXPECT_EQ(text, expectedText);
    }
}

TEST(PageTextAssignText, ParseMultiPageAssignment1)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "page_text.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    testMultiPageAssignment(doc, SCORE_PARTID, 0, 2, 3, "Multiple Page Text - Page ", " of 3");
    testMultiPageAssignment(doc, 1, 0, 1, 1, "Multiple Page Text - Page ", " of 1");
    testMultiPageAssignment(doc, 2, 0, 4, 5, "Multiple Page Text - Page ", " of 6");
}

TEST(PageTextAssignText, ParseMultiPageAssignment2)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "page_text.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    testMultiPageAssignment(doc, SCORE_PARTID, 1, 1, 3, "All Page Text - Page ", " of 3 Score");
    testMultiPageAssignment(doc, 1, 1, 1, 1, "All Page Text - Page ", " of 1 Part 1");
    testMultiPageAssignment(doc, 2, 1, 1, 6, "All Page Text - Page ", " of 6 Part 2");
}

// The score has three pages and no leading blank page; the part has one leading blank page and three
// more, so page assignment ID 2 is page 1 of the score and page 2 of the part.
TEST(PageTextAssignTest, PageNumbersApplyOddEvenToEveryAssignment)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <pageSpec cmper="1"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>1</firstSystem></pageSpec>
    <pageSpec cmper="2"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>2</firstSystem></pageSpec>
    <pageSpec cmper="3"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>3</firstSystem></pageSpec>
    <pageSpec cmper="1" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>-1</firstSystem></pageSpec>
    <pageSpec cmper="2" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>1</firstSystem></pageSpec>
    <pageSpec cmper="3" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>2</firstSystem></pageSpec>
    <pageSpec cmper="4" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>3</firstSystem></pageSpec>
    <partDef cmper="0"><partOrder>0</partOrder><copies>1</copies></partDef>
    <partDef cmper="1"><partOrder>1</partOrder><copies>1</copies></partDef>
    <pageTextAssign cmper="0" inci="0">
      <startPage>1</startPage>
      <oddEven>odd</oddEven>
    </pageTextAssign>
    <pageTextAssign cmper="2" inci="0">
      <startPage>2</startPage>
      <endPage>2</endPage>
    </pageTextAssign>
    <pageTextAssign cmper="3" inci="0">
      <startPage>3</startPage>
      <endPage>3</endPage>
      <oddEven>even</oddEven>
    </pageTextAssign>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);
    ASSERT_EQ(doc->getMaxBlankPages(), 1);
    const auto others = doc->getOthers();

    const auto oddPages = others->get<others::PageTextAssign>(SCORE_PARTID, 0, 0);
    ASSERT_TRUE(oddPages);
    EXPECT_TRUE(oddPages->calcMatchesOddEven(1));
    EXPECT_FALSE(oddPages->calcMatchesOddEven(2));
    EXPECT_EQ(oddPages->calcPageNumbers(SCORE_PARTID), std::vector<PageCmper>({1, 3}));
    EXPECT_EQ(oddPages->calcPageNumbers(1), std::vector<PageCmper>({1, 3}));
    EXPECT_TRUE(oddPages->calcAppearsOnPage(SCORE_PARTID, 3));
    EXPECT_FALSE(oddPages->calcAppearsOnPage(SCORE_PARTID, 2));
    EXPECT_FALSE(oddPages->calcAppearsOnPage(SCORE_PARTID, 5)) << "beyond the last page";

    const auto singlePage = others->get<others::PageTextAssign>(SCORE_PARTID, 2, 0);
    ASSERT_TRUE(singlePage);
    EXPECT_EQ(singlePage->calcPageNumbers(SCORE_PARTID), std::vector<PageCmper>({1}));
    EXPECT_EQ(singlePage->calcPageNumbers(1), std::vector<PageCmper>({2}));

    // A single-page assignment obeys its odd/even setting too: page 2 of the score is even, but
    // the same assignment ID is page 3 of the part, where it does not appear.
    const auto evenSinglePage = others->get<others::PageTextAssign>(SCORE_PARTID, 3, 0);
    ASSERT_TRUE(evenSinglePage);
    EXPECT_EQ(evenSinglePage->calcPageNumbers(SCORE_PARTID), std::vector<PageCmper>({2}));
    EXPECT_TRUE(evenSinglePage->calcPageNumbers(1).empty());
    EXPECT_FALSE(evenSinglePage->calcAppearsOnPage(1, 3));
}

// Part 1 has one leading blank page and part 2 has two, so assignment IDs 1 and 2 are both
// leading-blank slots. ID 2 names no page in part 1, which therefore has a start page but no end page.
TEST(PageTextAssignTest, PageNumbersWithoutAnEndPageAreTheStartPage)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <pageSpec cmper="1"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>1</firstSystem></pageSpec>
    <pageSpec cmper="1" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>-1</firstSystem></pageSpec>
    <pageSpec cmper="2" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>1</firstSystem></pageSpec>
    <pageSpec cmper="1" part="2" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>-1</firstSystem></pageSpec>
    <pageSpec cmper="2" part="2" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>-1</firstSystem></pageSpec>
    <pageSpec cmper="3" part="2" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>1</firstSystem></pageSpec>
    <partDef cmper="0"><partOrder>0</partOrder><copies>1</copies></partDef>
    <partDef cmper="1"><partOrder>1</partOrder><copies>1</copies></partDef>
    <partDef cmper="2"><partOrder>2</partOrder><copies>1</copies></partDef>
    <pageTextAssign cmper="0" inci="0">
      <startPage>1</startPage>
      <endPage>2</endPage>
    </pageTextAssign>
    <pageTextAssign cmper="0" inci="1">
      <startPage>1</startPage>
      <endPage>2</endPage>
      <oddEven>even</oddEven>
    </pageTextAssign>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);
    ASSERT_EQ(doc->getMaxBlankPages(), 2);
    const auto others = doc->getOthers();

    const auto assignment = others->get<others::PageTextAssign>(SCORE_PARTID, 0, 0);
    ASSERT_TRUE(assignment);
    ASSERT_TRUE(assignment->calcStartPageNumber(1));
    ASSERT_FALSE(assignment->calcEndPageNumber(1));
    EXPECT_EQ(assignment->calcPageNumbers(1), std::vector<PageCmper>({1}));
    EXPECT_TRUE(assignment->calcAppearsOnPage(1, 1));
    EXPECT_FALSE(assignment->calcAppearsOnPage(1, 2));
    EXPECT_EQ(assignment->calcPageNumbers(2), std::vector<PageCmper>({1, 2})) << "part 2 has both slots";
    EXPECT_TRUE(assignment->calcPageNumbers(SCORE_PARTID).empty()) << "the score has neither slot";

    // The start page alone still obeys the odd/even setting.
    const auto evenPages = others->get<others::PageTextAssign>(SCORE_PARTID, 0, 1);
    ASSERT_TRUE(evenPages);
    EXPECT_TRUE(evenPages->calcPageNumbers(1).empty());
    EXPECT_FALSE(evenPages->calcAppearsOnPage(1, 1));
    EXPECT_EQ(evenPages->calcPageNumbers(2), std::vector<PageCmper>({2}));
}

TEST(PageTextAssignTest, RightPagesUseTheirOwnValuesOnlyWhenIndependent)
{
    others::PageTextAssign assignment(DocumentWeakPtr{}, SCORE_PARTID, CommonClassBase::ShareMode::All, 0, 0);
    assignment.hPosLp = others::PageTextAssign::HorizontalAlignment::Left;
    assignment.hPosRp = others::PageTextAssign::HorizontalAlignment::Right;
    assignment.xDisp = 10;
    assignment.yDisp = 20;
    assignment.rightPgXDisp = 30;
    assignment.rightPgYDisp = 40;

    EXPECT_FALSE(assignment.calcUsesRightPageValues(1));
    EXPECT_EQ(assignment.calcHorizontalAlignment(1), others::PageTextAssign::HorizontalAlignment::Left);
    EXPECT_EQ(assignment.calcDisplacement(1), std::pair(Evpu(10), Evpu(20)));
    auto other = assignment;
    other.rightPgXDisp = 99;
    EXPECT_TRUE(assignment.isPlacedLike(other)) << "unused right-page displacements do not count";
    other.yDisp = 21;
    EXPECT_FALSE(assignment.isPlacedLike(other));

    assignment.indRpPos = true;
    EXPECT_TRUE(assignment.calcUsesRightPageValues(1));
    EXPECT_FALSE(assignment.calcUsesRightPageValues(2)) << "even pages are left-hand pages";
    EXPECT_EQ(assignment.calcHorizontalAlignment(1), others::PageTextAssign::HorizontalAlignment::Right);
    EXPECT_EQ(assignment.calcHorizontalAlignment(2), others::PageTextAssign::HorizontalAlignment::Left);
    EXPECT_EQ(assignment.calcDisplacement(1), std::pair(Evpu(30), Evpu(40)));
    EXPECT_EQ(assignment.calcDisplacement(2), std::pair(Evpu(10), Evpu(20)));
    other = assignment;
    EXPECT_TRUE(assignment.isPlacedLike(other));
    other.rightPgXDisp = 99;
    EXPECT_FALSE(assignment.isPlacedLike(other));
}
