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

constexpr static musxtest::string_view partGlobalsXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <partGlobals cmper="65534">
      <showTransposed/>
      <scrollViewIUlist>65507</scrollViewIUlist>
      <studioViewIUlist>65400</studioViewIUlist>
      <pageViewIUlist>65528</pageViewIUlist>
    </partGlobals>
    <partGlobals cmper="65534" part="27" shared="false">
      <studioViewIUlist>65400</studioViewIUlist>
    </partGlobals>
  </others>
</finale>
    )xml";

TEST(PartGlobalsTest, PartGlobalsPropertiesTest)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(partGlobalsXml);
    ASSERT_TRUE(doc);

    auto partGlobals = doc->getOthers()->get<others::PartGlobals>(27, MUSX_GLOBALS_CMPER);
    ASSERT_TRUE(partGlobals);

    EXPECT_FALSE(partGlobals->showTransposed);
    EXPECT_EQ(partGlobals->scrollViewIUlist, 0);
    EXPECT_EQ(partGlobals->studioViewIUlist, STUDIO_VIEW_SYSTEM_ID);
    EXPECT_EQ(partGlobals->specialPartExtractionIUList, 0);
    EXPECT_EQ(doc->calcScrollViewCmper(27), BASE_SYSTEM_ID);
    EXPECT_TRUE(doc->isScrollViewCmper(27, BASE_SYSTEM_ID));
    EXPECT_FALSE(doc->isScrollViewCmper(27, SPECIAL_PART_EXTRACTION_SYSTEM_ID));

    auto partGlobalsForScore = doc->getOthers()->get<others::PartGlobals>(SCORE_PARTID, MUSX_GLOBALS_CMPER);
    ASSERT_TRUE(partGlobalsForScore);

    EXPECT_TRUE(partGlobalsForScore->showTransposed);
    EXPECT_EQ(partGlobalsForScore->scrollViewIUlist, STAFF_SET_1_SYSTEM_ID + 7);
    EXPECT_EQ(partGlobalsForScore->studioViewIUlist, STUDIO_VIEW_SYSTEM_ID);
    EXPECT_EQ(partGlobalsForScore->specialPartExtractionIUList, SPECIAL_PART_EXTRACTION_SYSTEM_ID);
    EXPECT_EQ(doc->calcScrollViewCmper(SCORE_PARTID), SPECIAL_PART_EXTRACTION_SYSTEM_ID);
    EXPECT_TRUE(doc->isScrollViewCmper(SCORE_PARTID, SPECIAL_PART_EXTRACTION_SYSTEM_ID));
    EXPECT_FALSE(doc->isScrollViewCmper(SCORE_PARTID, BASE_SYSTEM_ID));

    // This should return nothing: correctly formatted enigmaxml should have a part global for every part
    auto partGlobalsForNonexistent = doc->getOthers()->get<others::PartGlobals>(1, MUSX_GLOBALS_CMPER);
    ASSERT_FALSE(partGlobalsForNonexistent);
    EXPECT_EQ(doc->calcScrollViewCmper(1), BASE_SYSTEM_ID);
    EXPECT_TRUE(doc->isScrollViewCmper(1, BASE_SYSTEM_ID));
}
