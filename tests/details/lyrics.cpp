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

namespace {

std::shared_ptr<Document> createWordExtDocument(std::string_view removedOption = {})
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "wordext.enigmaxml", xml);
    std::string text(xml.begin(), xml.end());
    if (!removedOption.empty()) {
        const auto pos = text.find(removedOption);
        EXPECT_NE(pos, std::string::npos) << removedOption;
        text.erase(pos, removedOption.size());
    }
    std::vector<char> modifiedXml(text.begin(), text.end());
    return musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(modifiedXml);
}

} // namespace

TEST(LyricsTest, WordExtensionEndpointFollowsSmartWordExtensionOption)
{
    auto endpointWithOptions = [&](std::string_view removedOption) {
        auto doc = createWordExtDocument(removedOption);
        EXPECT_TRUE(doc);
        auto staff = others::StaffComposite::createCurrent(doc, SCORE_PARTID, 1, 1, 0);
        EXPECT_TRUE(staff);
        auto lyricLineInfos = staff->createLyricsLineInfo(1);
        EXPECT_EQ(lyricLineInfos.size(), 2);
        return lyricLineInfos[0].assignments[0]->calcWordExtensionEndpoint();
    };

    // Smart hyphens do not decide whether a word extension has a shape; smart word extensions do.
    EXPECT_TRUE(endpointWithOptions("<useSmartHyphens/>"));
    // Without smart word extensions, the endpoint is inferred from the music instead of read from a shape.
    EXPECT_TRUE(endpointWithOptions("<useSmartWordExtensions/>"));
}

TEST(LyricsTest, LegacyWordExtensionEndpointMatchesSmartShapes)
{
    // The same music with smart word extensions turned off: every inferred endpoint must be the one
    // Finale stored in the corresponding smart shape.
    auto smartDoc = createWordExtDocument();
    auto legacyDoc = createWordExtDocument("<useSmartWordExtensions/>");
    ASSERT_TRUE(smartDoc);
    ASSERT_TRUE(legacyDoc);

    const auto smartAssignments = smartDoc->getDetails()->getAllSources<details::LyricAssignVerse>();
    const auto legacyAssignments = legacyDoc->getDetails()->getAllSources<details::LyricAssignVerse>();
    ASSERT_EQ(smartAssignments.size(), legacyAssignments.size());
    size_t extensionCount = 0;
    for (size_t x = 0; x < smartAssignments.size(); x++) {
        const auto& smart = smartAssignments[x];
        const auto& legacy = legacyAssignments[x];
        ASSERT_EQ(smart->getEntryNumber(), legacy->getEntryNumber());
        const auto smartEndpoint = smart->calcWordExtensionEndpoint();
        const auto legacyEndpoint = legacy->calcWordExtensionEndpoint();
        ASSERT_EQ(bool(smartEndpoint), bool(legacyEndpoint)) << "entry " << smart->getEntryNumber();
        if (smartEndpoint) {
            extensionCount++;
            EXPECT_EQ(smartEndpoint->getEntry()->getEntryNumber(), legacyEndpoint->getEntry()->getEntryNumber())
                << "word extension on entry " << smart->getEntryNumber() << " verse " << smart->lyricNumber;
        }
    }
    EXPECT_EQ(extensionCount, 5);
}

TEST(LyricsTest, SmartWordExtensionWithoutEntryAssignmentIsInferred)
{
    // Finale does not always save the entry assignment that leads to a word extension shape. Without any of them,
    // every endpoint must still be the one the shape stores.
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "wordext.enigmaxml", xml);
    std::string text(xml.begin(), xml.end());
    const std::string openTag = "<smartShapeEntryMark ";
    const std::string closeTag = "</smartShapeEntryMark>";
    size_t removed = 0;
    for (auto pos = text.find(openTag); pos != std::string::npos; pos = text.find(openTag, pos)) {
        const auto end = text.find(closeTag, pos);
        ASSERT_NE(end, std::string::npos);
        text.erase(pos, end + closeTag.size() - pos);
        removed++;
    }
    ASSERT_GT(removed, 0u);
    std::vector<char> strippedXml(text.begin(), text.end());
    auto strippedDoc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(strippedXml);
    auto smartDoc = createWordExtDocument();
    ASSERT_TRUE(strippedDoc);
    ASSERT_TRUE(smartDoc);
    EXPECT_TRUE(strippedDoc->getDetails()->getAllSources<details::SmartShapeEntryAssign>().empty());

    const auto smartAssignments = smartDoc->getDetails()->getAllSources<details::LyricAssignVerse>();
    const auto strippedAssignments = strippedDoc->getDetails()->getAllSources<details::LyricAssignVerse>();
    ASSERT_EQ(smartAssignments.size(), strippedAssignments.size());
    size_t extensionCount = 0;
    for (size_t x = 0; x < smartAssignments.size(); x++) {
        const auto smartEndpoint = smartAssignments[x]->calcWordExtensionEndpoint();
        const auto strippedEndpoint = strippedAssignments[x]->calcWordExtensionEndpoint();
        ASSERT_EQ(bool(smartEndpoint), bool(strippedEndpoint)) << "entry " << smartAssignments[x]->getEntryNumber();
        if (smartEndpoint) {
            extensionCount++;
            EXPECT_EQ(smartEndpoint->getEntry()->getEntryNumber(), strippedEndpoint->getEntry()->getEntryNumber())
                << "word extension on entry " << smartAssignments[x]->getEntryNumber();
        }
    }
    EXPECT_EQ(extensionCount, 5u);
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
