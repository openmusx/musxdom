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
#include <string_view>
#include <vector>

#include "gtest/gtest.h"
#include "musx/musx.h"
#include "test_utils.h"

using namespace musx::dom;

TEST(LyricsTest, WordExtensions)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "wordext.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 1, 0);
    ASSERT_TRUE(staff);
    auto lyricLineInfos = staff->createLyricsLineInfo(1);
    ASSERT_EQ(lyricLineInfos.size(), 2);

    {
        auto lyricLineInfo = lyricLineInfos[0];
        ASSERT_EQ(lyricLineInfo.assignments.size(), 3);
        {
            auto wextEndPoint = lyricLineInfo.assignments[0]->calcWordExtensionEndpoint();
            ASSERT_TRUE(wextEndPoint);
            EXPECT_EQ(wextEndPoint.getMeasure(), 1);
            EXPECT_EQ(wextEndPoint.getIndexInFrame(), 2);
        }
        {
            auto wextEndPoint = lyricLineInfo.assignments[1]->calcWordExtensionEndpoint();
            ASSERT_TRUE(wextEndPoint);
            EXPECT_EQ(wextEndPoint.getMeasure(), 2);
            EXPECT_EQ(wextEndPoint.getIndexInFrame(), 1);
        }
        {
            auto wextEndPoint = lyricLineInfo.assignments[2]->calcWordExtensionEndpoint();
            ASSERT_TRUE(wextEndPoint);
            EXPECT_EQ(wextEndPoint.getMeasure(), 2);
            EXPECT_EQ(wextEndPoint.getIndexInFrame(), 5);
        }
    }
    {
        auto lyricLineInfo = lyricLineInfos[1];
        ASSERT_EQ(lyricLineInfo.assignments.size(), 3);
        {
            auto wextEndPoint = lyricLineInfo.assignments[0]->calcWordExtensionEndpoint();
            EXPECT_FALSE(wextEndPoint);
        }
        {
            auto wextEndPoint = lyricLineInfo.assignments[1]->calcWordExtensionEndpoint();
            ASSERT_TRUE(wextEndPoint);
            EXPECT_EQ(wextEndPoint.getMeasure(), 2);
            EXPECT_EQ(wextEndPoint.getIndexInFrame(), 0);
        }
        {
            auto wextEndPoint = lyricLineInfo.assignments[2]->calcWordExtensionEndpoint();
            ASSERT_TRUE(wextEndPoint);
            EXPECT_EQ(wextEndPoint.getMeasure(), 2);
            EXPECT_EQ(wextEndPoint.getIndexInFrame(), 5);
        }
    }
}

TEST(LyricsTest, WordExtensionEndpointFollowsSmartWordExtensionOption)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "wordext.enigmaxml", xml);
    std::string text(xml.begin(), xml.end());

    auto endpointWithOptions = [&](std::string_view removedOption) {
        std::string modified = text;
        const auto pos = modified.find(removedOption);
        EXPECT_NE(pos, std::string::npos) << removedOption;
        modified.erase(pos, removedOption.size());
        std::vector<char> modifiedXml(modified.begin(), modified.end());
        auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(modifiedXml);
        EXPECT_TRUE(doc);
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 1, 0);
        EXPECT_TRUE(staff);
        auto lyricLineInfos = staff->createLyricsLineInfo(1);
        EXPECT_EQ(lyricLineInfos.size(), 2);
        return lyricLineInfos[0].assignments[0]->calcWordExtensionEndpoint();
    };

    // Smart hyphens do not decide whether a word extension has a shape; smart word extensions do.
    EXPECT_TRUE(endpointWithOptions("<useSmartHyphens/>"));
    EXPECT_FALSE(endpointWithOptions("<useSmartWordExtensions/>"));
}

constexpr static musxtest::string_view displayNumberXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <options>
    <lyricOptions>
      <showAutoNumbersOnVerses/>
      <showAutoNumbersOnSections/>
    </lyricOptions>
  </options>
  <details>
    <lyrDataVerse entnum="1" inci="0">
      <lyricNumber>5</lyricNumber>
      <syll>3</syll>
    </lyrDataVerse>
    <lyrDataVerse entnum="2" inci="0">
      <lyricNumber>7</lyricNumber>
      <syll>3</syll>
      <displayVerseNum/>
    </lyrDataVerse>
    <lyrDataVerse entnum="3" inci="0">
      <lyricNumber>2</lyricNumber>
      <syll>1</syll>
    </lyrDataVerse>
    <lyrDataChorus entnum="4" inci="0">
      <lyricNumber>3</lyricNumber>
      <syll>1</syll>
    </lyrDataChorus>
    <lyrDataSection entnum="5" inci="0">
      <lyricNumber>4</lyricNumber>
      <syll>1</syll>
    </lyrDataSection>
  </details>
</finale>
)xml";

TEST(LyricsTest, CalcDisplayNumberText)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(displayNumberXml);
    ASSERT_TRUE(doc);

    // Neither a per-assignment override nor the global auto-number option (syllable 3, not 1): no number.
    {
        auto assigns = doc->getDetails()->getArray<details::LyricAssignVerse>(SCORE_PARTID, 1);
        ASSERT_EQ(assigns.size(), 1);
        EXPECT_FALSE(assigns[0]->calcDisplayNumberText());
    }

    // Per-assignment override (displayVerseNum), even though this is not the first syllable.
    {
        auto assigns = doc->getDetails()->getArray<details::LyricAssignVerse>(SCORE_PARTID, 2);
        ASSERT_EQ(assigns.size(), 1);
        auto number = assigns[0]->calcDisplayNumberText();
        ASSERT_TRUE(number);
        EXPECT_EQ(*number, "7.");
    }

    // Global auto-number option (showAutoNumbersOnVerses), first syllable, no per-assignment override.
    {
        auto assigns = doc->getDetails()->getArray<details::LyricAssignVerse>(SCORE_PARTID, 3);
        ASSERT_EQ(assigns.size(), 1);
        auto number = assigns[0]->calcDisplayNumberText();
        ASSERT_TRUE(number);
        EXPECT_EQ(*number, "2.");
    }

    // Chorus type is unaffected by showAutoNumbersOnVerses/Sections: no number.
    {
        auto assigns = doc->getDetails()->getArray<details::LyricAssignChorus>(SCORE_PARTID, 4);
        ASSERT_EQ(assigns.size(), 1);
        EXPECT_FALSE(assigns[0]->calcDisplayNumberText());
    }

    // Section type picks up showAutoNumbersOnSections independently of the other two.
    {
        auto assigns = doc->getDetails()->getArray<details::LyricAssignSection>(SCORE_PARTID, 5);
        ASSERT_EQ(assigns.size(), 1);
        auto number = assigns[0]->calcDisplayNumberText();
        ASSERT_TRUE(number);
        EXPECT_EQ(*number, "4.");
    }
}
