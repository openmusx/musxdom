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
using namespace musx::util;

TEST(GFrameHoldTest, PopulateFields)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <details>
    <gfhold cmper1="3" cmper2="915">
      <clefID>0</clefID>
      <clefMode>forced</clefMode>
      <clefPercent>75</clefPercent>
      <frame1>21240</frame1>
    </gfhold>
    <gfhold cmper1="3" cmper2="1083">
      <clefID>3</clefID>
      <clefMode>hidden</clefMode>
      <clefPercent>75</clefPercent>
      <frame1>22464</frame1>
      <frame3>22465</frame3>
    </gfhold>
    <gfhold cmper1="3" cmper2="1129">
      <clefListID>1234</clefListID>
      <clefPercent>75</clefPercent>
      <mirrorFrame/>
      <frame4>22911</frame4>
    </gfhold>
  </details>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    // Test GFrameHold for cmper1=3, cmper2=915
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 3, 915);
        ASSERT_TRUE(gfhold);

        EXPECT_EQ(gfhold->clefId.value_or(-1), 0); // Default to -1 if not set
        EXPECT_EQ(gfhold->clefListId, 0); // Default to zero
        EXPECT_EQ(gfhold->showClefMode, ShowClefMode::Always);
        EXPECT_EQ(gfhold->clefPercent, 75);
        EXPECT_FALSE(gfhold->mirrorFrame);
        EXPECT_EQ(gfhold->frames[0], 21240);
        EXPECT_EQ(gfhold->frames[1], 0);  // Not present, should be default
        EXPECT_EQ(gfhold->frames[2], 0);  // Not present, should be default
        EXPECT_EQ(gfhold->frames[3], 0);  // Not present, should be default
    }

    // Test GFrameHold for cmper1=3, cmper2=1083
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 3, 1083);
        ASSERT_TRUE(gfhold);

        EXPECT_EQ(gfhold->clefId.value_or(-1), 3); // Default to -1 if not set
        EXPECT_EQ(gfhold->clefListId, 0); // Default to zero
        EXPECT_EQ(gfhold->showClefMode, ShowClefMode::Never);
        EXPECT_EQ(gfhold->clefPercent, 75);
        EXPECT_FALSE(gfhold->mirrorFrame);
        EXPECT_EQ(gfhold->frames[0], 22464);
        EXPECT_EQ(gfhold->frames[1], 0);  // Not present, should be default
        EXPECT_EQ(gfhold->frames[2], 22465);
        EXPECT_EQ(gfhold->frames[3], 0);  // Not present, should be default
    }

    // Test GFrameHold for cmper1=3, cmper2=1129
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 3, 1129);
        ASSERT_TRUE(gfhold);

        EXPECT_FALSE(gfhold->clefId.has_value());
        EXPECT_EQ(gfhold->clefListId, 1234); // Default to zero
        EXPECT_EQ(gfhold->showClefMode, ShowClefMode::WhenNeeded);
        EXPECT_EQ(gfhold->clefPercent, 75);
        EXPECT_TRUE(gfhold->mirrorFrame);
        EXPECT_EQ(gfhold->frames[0], 0);  // Not present, should be default
        EXPECT_EQ(gfhold->frames[1], 0);  // Not present, should be default
        EXPECT_EQ(gfhold->frames[2], 0);  // Not present, should be default
        EXPECT_EQ(gfhold->frames[3], 22911);
    }
}

TEST(GFrameHoldTest, IntegrityCheck)
{
    constexpr static musxtest::string_view xmlBothClefs = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <details>
    <gfhold cmper1="3" cmper2="915">
      <clefID>0</clefID>
      <clefListID>123</clefListID>
      <clefMode>forced</clefMode>
      <clefPercent>75</clefPercent>
      <frame1>21240</frame1>
    </gfhold>
  </details>
</finale>
    )xml";

    EXPECT_THROW(
        auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xmlBothClefs),
        musx::dom::integrity_error
    ) << "clef and clef list both specified";

constexpr static musxtest::string_view xmlNoClefs = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <details>
    <gfhold cmper1="3" cmper2="915">
      <clefMode>forced</clefMode>
      <clefPercent>75</clefPercent>
      <frame1>21240</frame1>
    </gfhold>
  </details>
</finale>
    )xml";

    constexpr StaffCmper kNoClefsStaffId = 3;
    constexpr MeasCmper kNoClefsMeasureId = 915;
    auto noClefsDoc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xmlNoClefs);
    ASSERT_TRUE(noClefsDoc);
    auto noClefsHold = details::GFrameHoldContext(noClefsDoc, SCORE_PARTID, kNoClefsStaffId, kNoClefsMeasureId);
    ASSERT_TRUE(noClefsHold);
    ASSERT_TRUE(noClefsHold->clefId.has_value());
    EXPECT_EQ(*noClefsHold->clefId, 0);
    EXPECT_EQ(noClefsHold->clefListId, 0);

    constexpr static musxtest::string_view xmlNotIterable = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <frameSpec cmper="1" inci="0">
      <startEntry>1</startEntry>
      <endEntry>2</endEntry>
    </frameSpec>
    <measSpec cmper="1">
      <width>600</width>
      <keySig>
        <keyless/>
      </keySig>
      <beats>2</beats>
      <divbeat>2048</divbeat>
      <dispBeats>4</dispBeats>
      <dispDivbeat>1024</dispDivbeat>
      <posMode>timesigPlusPos</posMode>
      <barline>normal</barline>
      <forRepBar/>
      <bacRepBar/>
      <barEnding/>
      <abbrvTime/>
      <useDisplayTimesig/>
      <leftBarline>default</leftBarline>
    </measSpec>
    <staffSpec cmper="3">
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
  </others>
  <details>
    <gfhold cmper1="3" cmper2="1">
      <clefID>0</clefID>
      <clefMode>forced</clefMode>
      <clefPercent>75</clefPercent>
      <frame1>1</frame1>
    </gfhold>
  </details>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xmlNotIterable);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 3, 1);
    ASSERT_TRUE(gfhold);

    EXPECT_THROW(
        gfhold.iterateEntries([](const auto&) -> bool { return false; }),
        musx::dom::integrity_error
    ) << "gfhold not iterable";

}

TEST(GFrameHold, IterationTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "layers.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 2);
    ASSERT_TRUE(gfhold);
    bool enteredLoop = false;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        enteredLoop = true;
        auto entry = entryInfo->getEntry();
        EXPECT_TRUE(entryInfo.getLayerIndex() == 0 || entryInfo.getLayerIndex() == 1) << "unexpected layer index " << entryInfo.getLayerIndex();
        if (entryInfo.getLayerIndex() == 0) {
            EXPECT_EQ(entry->duration, Edu(NoteType::Whole)) << "unexpected note duration " << entry->duration;
            EXPECT_TRUE(entry->isNote) << "layerIndex 0 entry is not a note";
        } else {
            EXPECT_EQ(entry->duration, Edu(NoteType::Half)) << "unexpected note duration " << entry->duration;
        }
        return true;
    });
    EXPECT_TRUE(enteredLoop);

    gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1);
    ASSERT_TRUE(gfhold);
    enteredLoop = false;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        enteredLoop = true;
        auto entry = entryInfo->getEntry();
        EXPECT_TRUE(entryInfo.getLayerIndex() == 2) << "unexpected layer index " << entryInfo.getLayerIndex();
        EXPECT_EQ(entry->duration, Edu(NoteType::Whole)) << "unexpected note duration " << entry->duration;
        EXPECT_TRUE(entry->isNote) << "layerIndex 0 entry is not a note";
        return true;
    });
    EXPECT_TRUE(enteredLoop);
}

TEST(GFrameHold, QuintupletTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "quintuplet.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<Fraction> expectedValues = {
      Fraction(1, 10), Fraction(1, 10), Fraction(1, 10), Fraction(1, 10), Fraction(1, 10), Fraction(1, 2)
    };

    std::vector<size_t> expectedStarts = { 0 };
    std::vector<size_t> expectedEnds = { 4 };
    std::vector<Fraction> expectedStartDuras = { Fraction(0, 1) };
    std::vector<Fraction> expectedEndDuras = { Fraction(1, 2) };

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    size_t x = 0;
    Fraction total = 0;
    gfhold.iterateEntries([&](const EntryInfoPtr& entryInfo) -> bool {
        EXPECT_LT(x, expectedValues.size()) << "too few expected values";
        if (x >= expectedValues.size()) return false;
        if (x == 0) {
            const auto frame = entryInfo.getFrame();
            EXPECT_EQ(frame->tupletInfo.size(), expectedStarts.size());
            if (frame->tupletInfo.size() == expectedStarts.size()) {
                for (size_t y = 0; y < frame->tupletInfo.size(); y++) {
                    const auto& tuplInf = frame->tupletInfo[y];
                    EXPECT_EQ(tuplInf.startIndex, expectedStarts[y]);
                    EXPECT_EQ(tuplInf.startDura, expectedStartDuras[y]);
                    EXPECT_EQ(tuplInf.endIndex, expectedEnds[y]);
                    EXPECT_EQ(tuplInf.endDura, expectedEndDuras[y]);
                }
            }
        }
        EXPECT_EQ(expectedValues[x], entryInfo->actualDuration);
        EXPECT_EQ(total, entryInfo->elapsedDuration);
        total += expectedValues[x++];
        return true;
    });
    EXPECT_EQ(x, expectedValues.size());
}

TEST(GFrameHold, TripletTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "triplet.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<Fraction> expectedValues = {
      Fraction(1, 6), Fraction(1, 6), Fraction(1, 6), Fraction(1, 2)
    };

    std::vector<size_t> expectedStarts = { 0 };
    std::vector<size_t> expectedEnds = { 2 };
    std::vector<Fraction> expectedStartDuras = { Fraction(0, 1) };
    std::vector<Fraction> expectedEndDuras = { Fraction(1, 2) };

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    size_t x = 0;
    Fraction total = 0;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        EXPECT_LT(x, expectedValues.size()) << "too few expected values";
        if (x >= expectedValues.size()) return false;
        if (x == 0) {
            const auto frame = entryInfo.getFrame();
            EXPECT_EQ(frame->tupletInfo.size(), expectedStarts.size());
            if (frame->tupletInfo.size() == expectedStarts.size()) {
                for (size_t y = 0; y < frame->tupletInfo.size(); y++) {
                    const auto& tuplInf = frame->tupletInfo[y];
                    EXPECT_EQ(tuplInf.startIndex, expectedStarts[y]);
                    EXPECT_EQ(tuplInf.startDura, expectedStartDuras[y]);
                    EXPECT_EQ(tuplInf.endIndex, expectedEnds[y]);
                    EXPECT_EQ(tuplInf.endDura, expectedEndDuras[y]);
                }
            }
        }
        EXPECT_EQ(expectedValues[x], entryInfo->actualDuration);
        EXPECT_EQ(total, entryInfo->elapsedDuration);
        total += expectedValues[x++];
        return true;
    });
    EXPECT_EQ(x, expectedValues.size());
}

TEST(GFrameHold, NestedTupletTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "nested_tuplets.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<Fraction> expectedValues = {
        Fraction(1, 2),
        Fraction(1, 18), Fraction(1, 18), Fraction(1, 18),
        Fraction(1, 18), Fraction(1, 18), Fraction(1, 18),
        Fraction(1, 12), Fraction(1, 12)
    };

    std::vector<size_t> expectedStarts = { 1, 1, 4 };
    std::vector<size_t> expectedEnds = { 8, 3, 6 };
    std::vector<Fraction> expectedStartDuras = { Fraction(1, 2), Fraction(1, 2), Fraction(2, 3) };
    std::vector<Fraction> expectedEndDuras = { Fraction(1, 1), Fraction(2, 3), Fraction(5, 6) };

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    size_t x = 0;
    Fraction total = 0;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        EXPECT_LT(x, expectedValues.size()) << "too few expected values";
        if (x >= expectedValues.size()) return false;
        if (x == 0) {
            const auto frame = entryInfo.getFrame();
            EXPECT_EQ(frame->tupletInfo.size(), expectedStarts.size());
            if (frame->tupletInfo.size() == expectedStarts.size()) {
                for (size_t y = 0; y < frame->tupletInfo.size(); y++) {
                    const auto& tuplInf = frame->tupletInfo[y];
                    EXPECT_EQ(tuplInf.startIndex, expectedStarts[y]);
                    EXPECT_EQ(tuplInf.startDura, expectedStartDuras[y]);
                    EXPECT_EQ(tuplInf.endIndex, expectedEnds[y]);
                    EXPECT_EQ(tuplInf.endDura, expectedEndDuras[y]);
                }
            }
        }
        EXPECT_EQ(expectedValues[x], entryInfo->actualDuration);
        EXPECT_EQ(total, entryInfo->elapsedDuration);
        total += expectedValues[x++];
        return true;
    });
    EXPECT_EQ(x, expectedValues.size());
}

TEST(GFrameHold, V1V2TupletTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "v1v2tuplets.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<Fraction> expectedValues = {
        Fraction(1, 3),
        Fraction(1, 12), Fraction(1, 12), Fraction(1, 12), Fraction(1, 8), Fraction(1, 8),
        Fraction(0), Fraction(1, 6),
        Fraction(1, 6), Fraction(1, 3),
        Fraction(0), Fraction(1, 12), Fraction(1, 12), Fraction(1, 12), Fraction(1, 4),
    };

    std::vector<size_t> expectedStarts = { 0, 1, 8, 11 };
    std::vector<size_t> expectedEnds = { 7, 3, 9, 13 };
    std::vector<Fraction> expectedStartDuras = { Fraction(0, 1), Fraction(0, 1), Fraction(1, 2), Fraction(2, 3) };
    std::vector<Fraction> expectedEndDuras = { Fraction(1, 2), Fraction(1, 4), Fraction(1, 1), Fraction(11, 12) };

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    size_t x = 0;
    Fraction v1Total = 0;
    Fraction v2Total = 0;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        EXPECT_LT(x, expectedValues.size()) << "too few expected values";
        if (x >= expectedValues.size()) return false;
        if (x == 0) {
            const auto frame = entryInfo.getFrame();
            EXPECT_EQ(frame->tupletInfo.size(), expectedStarts.size());
            if (frame->tupletInfo.size() == expectedStarts.size()) {
                for (size_t y = 0; y < frame->tupletInfo.size(); y++) {
                    const auto& tuplInf = frame->tupletInfo[y];
                    EXPECT_EQ(tuplInf.startIndex, expectedStarts[y]);
                    EXPECT_EQ(tuplInf.startDura, expectedStartDuras[y]);
                    EXPECT_EQ(tuplInf.endIndex, expectedEnds[y]);
                    EXPECT_EQ(tuplInf.endDura, expectedEndDuras[y]);
                }
            }
        }
        EXPECT_EQ(expectedValues[x], entryInfo->actualDuration);
        if (entryInfo->getEntry()->v2Launch) {
          v2Total = v1Total;
        }
        Fraction& total = entryInfo->getEntry()->voice2 ? v2Total : v1Total;
        EXPECT_EQ(total, entryInfo->elapsedDuration);
        total += expectedValues[x++];
    /*
        std::cout << entryInfo->elapsedDuration << '\t' << entryInfo->actualDuration << '\t'
                        << std::to_string(entryInfo->elapsedDuration.calcEduDuration()) << '\t'
                        << std::to_string(entryInfo->actualDuration.calcEduDuration()) << std::endl;
    */
        return true;
    });
    EXPECT_EQ(x, expectedValues.size());
}

TEST(GFrameHold, NestedEndTuplets)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "nested_end_tuplets.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<Fraction> expectedValues = {
        Fraction(1, 2),
        Fraction(1, 10), Fraction(1, 10), Fraction(1, 10),
        Fraction(1, 30), Fraction(1, 30), Fraction(1, 30),
        Fraction(1, 30), Fraction(1, 30), Fraction(1, 30),
    };

    std::vector<size_t> expectedStarts = { 1, 4, 7 };
    std::vector<size_t> expectedEnds = { 9, 6, 9 };
    std::vector<Fraction> expectedStartDuras = { Fraction(1, 2), Fraction(4, 5), Fraction(9, 10) };
    std::vector<Fraction> expectedEndDuras = { Fraction(1, 1), Fraction(9, 10), Fraction(1, 1) };

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    size_t x = 0;
    Fraction v1Total = 0;
    Fraction v2Total = 0;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        EXPECT_LT(x, expectedValues.size()) << "too few expected values";
        if (x >= expectedValues.size()) return false;
        if (x == 0) {
            const auto frame = entryInfo.getFrame();
            EXPECT_EQ(frame->tupletInfo.size(), expectedStarts.size());
            if (frame->tupletInfo.size() == expectedStarts.size()) {
                for (size_t y = 0; y < frame->tupletInfo.size(); y++) {
                    const auto& tuplInf = frame->tupletInfo[y];
                    EXPECT_EQ(tuplInf.startIndex, expectedStarts[y]);
                    EXPECT_EQ(tuplInf.startDura, expectedStartDuras[y]);
                    EXPECT_EQ(tuplInf.endIndex, expectedEnds[y]);
                    EXPECT_EQ(tuplInf.endDura, expectedEndDuras[y]);
                }
            }
        }
        EXPECT_EQ(expectedValues[x], entryInfo->actualDuration);
        if (entryInfo->getEntry()->v2Launch) {
            v2Total = v1Total;
        }
        Fraction& total = entryInfo->getEntry()->voice2 ? v2Total : v1Total;
        EXPECT_EQ(total, entryInfo->elapsedDuration);
        total += expectedValues[x++];
        return true;
    });
    EXPECT_EQ(x, expectedValues.size());
}

TEST(GFrameHold, IncompleteTuplet)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "incomplete_tuplet.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<Fraction> expectedValues = {
      Fraction(1, 2), Fraction(1, 4), Fraction(1, 6), Fraction(1, 6)
    };

    std::vector<size_t> expectedStarts = { 2 };
    std::vector<size_t> expectedEnds = { 3 };
    std::vector<Fraction> expectedStartDuras = { Fraction(3, 4) };
    std::vector<Fraction> expectedEndDuras = { Fraction(13, 12) };

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    size_t x = 0;
    Fraction total = 0;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        EXPECT_LT(x, expectedValues.size()) << "too few expected values";
        if (x >= expectedValues.size()) return false;
        if (x == 0) {
            const auto frame = entryInfo.getFrame();
            EXPECT_EQ(frame->tupletInfo.size(), expectedStarts.size());
            if (frame->tupletInfo.size() == expectedStarts.size()) {
                for (size_t y = 0; y < frame->tupletInfo.size(); y++) {
                    const auto& tuplInf = frame->tupletInfo[y];
                    EXPECT_EQ(tuplInf.startIndex, expectedStarts[y]);
                    EXPECT_EQ(tuplInf.startDura, expectedStartDuras[y]);
                    EXPECT_EQ(tuplInf.endIndex, expectedEnds[y]);
                    EXPECT_EQ(tuplInf.endDura, expectedEndDuras[y]);
                }
            }
        }
        EXPECT_EQ(expectedValues[x], entryInfo->actualDuration);
        EXPECT_EQ(total, entryInfo->elapsedDuration);
        total += expectedValues[x++];
        return true;
    });
    EXPECT_EQ(x, expectedValues.size());
}

TEST(GFrameHold, IncompleteTupletV2)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "incomplete_tupletv2.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<Fraction> expectedValues = {
      Fraction(1, 4), Fraction(1, 4), Fraction(1, 6), Fraction(1, 6), Fraction(1, 2)
    };

    std::vector<size_t> expectedStarts = { 2 };
    std::vector<size_t> expectedEnds = { 3 };
    std::vector<Fraction> expectedStartDuras = { Fraction(1, 4) };
    std::vector<Fraction> expectedEndDuras = { Fraction(7, 12) };

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    size_t x = 0;
    Fraction v1Total = 0;
    Fraction v2Total = 0;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        EXPECT_LT(x, expectedValues.size()) << "too few expected values";
        if (x >= expectedValues.size()) return false;
        if (x == 0) {
            const auto frame = entryInfo.getFrame();
            EXPECT_EQ(frame->tupletInfo.size(), expectedStarts.size());
            if (frame->tupletInfo.size() == expectedStarts.size()) {
                for (size_t y = 0; y < frame->tupletInfo.size(); y++) {
                    const auto& tuplInf = frame->tupletInfo[y];
                    EXPECT_EQ(tuplInf.startIndex, expectedStarts[y]);
                    EXPECT_EQ(tuplInf.startDura, expectedStartDuras[y]);
                    EXPECT_EQ(tuplInf.endIndex, expectedEnds[y]);
                    EXPECT_EQ(tuplInf.endDura, expectedEndDuras[y]);
                }
            }
        }
        EXPECT_EQ(expectedValues[x], entryInfo->actualDuration);
        if (entryInfo->getEntry()->v2Launch) {
            v2Total = v1Total;
        }
        Fraction& total = entryInfo->getEntry()->voice2 ? v2Total : v1Total;
        total += expectedValues[x++];
        return true;
    });
    EXPECT_EQ(x, expectedValues.size());
}

TEST(GFrameHold, ZeroTuplet)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "zero_tuplet.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<Fraction> expectedValues = {
      Fraction(1, 2), Fraction(1, 3), Fraction(0, 1), Fraction(1, 6)
    };

    std::vector<size_t> expectedStarts = { 1, 2 };
    std::vector<size_t> expectedEnds = { 3, 2 };
    std::vector<Fraction> expectedStartDuras = { Fraction(1, 2), Fraction(5, 6) };
    std::vector<Fraction> expectedEndDuras = { Fraction(1, 1), Fraction(5, 6) };

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    size_t x = 0;
    Fraction total = 0;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        EXPECT_LT(x, expectedValues.size()) << "too few expected values";
        if (x >= expectedValues.size()) return false;
        if (x == 0) {
            const auto frame = entryInfo.getFrame();
            EXPECT_EQ(frame->tupletInfo.size(), expectedStarts.size());
            if (frame->tupletInfo.size() == expectedStarts.size()) {
                for (size_t y = 0; y < frame->tupletInfo.size(); y++) {
                    const auto& tuplInf = frame->tupletInfo[y];
                    EXPECT_EQ(tuplInf.startIndex, expectedStarts[y]);
                    EXPECT_EQ(tuplInf.startDura, expectedStartDuras[y]);
                    EXPECT_EQ(tuplInf.endIndex, expectedEnds[y]);
                    EXPECT_EQ(tuplInf.endDura, expectedEndDuras[y]);
                }
            }
        }
        EXPECT_EQ(expectedValues[x], entryInfo->actualDuration);
        EXPECT_EQ(total, entryInfo->elapsedDuration);
        total += expectedValues[x++];
    /*
        std::cout << entryInfo->elapsedDuration << '\t' << entryInfo->actualDuration << '\t'
                        << std::to_string(entryInfo->elapsedDuration.calcEduDuration()) << '\t'
                        << std::to_string(entryInfo->actualDuration.calcEduDuration()) << std::endl;
    */
       return true;
    });
    EXPECT_EQ(x, expectedValues.size());
}

TEST(GFrameHold, GraceNoteIndexTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "grace_notes.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<unsigned> expectedValues = { 0, 1, 2, 3, 4, 5, 0, 1, 2, 3 };
    std::vector<unsigned> expectedReverseValues = { 0, 5, 4, 3, 2, 1, 0, 3, 2, 1 };

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    size_t x = 0;
    gfhold.iterateEntries([&](const auto& entryInfo) -> bool {
        EXPECT_LT(x, expectedValues.size()) << "too few expected values";
        if (x >= expectedValues.size()) return false;
        EXPECT_EQ(expectedValues[x], entryInfo->graceIndex);
        EXPECT_EQ(expectedReverseValues[x], entryInfo.calcReverseGraceIndex());
        x++;
        return true;
    });
    EXPECT_EQ(x, expectedValues.size());
}

TEST(GFrameHold, TremolosTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "tremolos.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
        ASSERT_TRUE(gfhold);
        std::vector<bool> expectedValues = { true, false };
        std::vector<size_t> expectedCounts = { 2, 3 };
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_EQ(entryFrame->tupletInfo.size(), expectedValues.size());
        for (size_t x = 0; x < entryFrame->tupletInfo.size(); x++) {
            const bool isTremolo = entryFrame->tupletInfo[x].calcIsTremolo();
            EXPECT_EQ(isTremolo, expectedValues[x]);
            EXPECT_EQ(entryFrame->tupletInfo[x].numEntries(), expectedCounts[x]);
        }
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 2);
        ASSERT_TRUE(gfhold);
        std::vector<bool> expectedValues = { true, false };
        std::vector<size_t> expectedCounts = { 3, 3 };
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_EQ(entryFrame->tupletInfo.size(), expectedValues.size());
        for (size_t x = 0; x < entryFrame->tupletInfo.size(); x++) {
            const bool isTremolo = entryFrame->tupletInfo[x].calcIsTremolo();
            EXPECT_EQ(isTremolo, expectedValues[x]);
            EXPECT_EQ(entryFrame->tupletInfo[x].numEntries(), expectedCounts[x]);
        }
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 3);
        ASSERT_TRUE(gfhold);
        std::vector<bool> expectedValues = { true, true, true };
        std::vector<size_t> expectedCounts = { 2, 2, 3 };
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_EQ(entryFrame->tupletInfo.size(), expectedValues.size());
        for (size_t x = 0; x < entryFrame->tupletInfo.size(); x++) {
            const bool isTremolo = entryFrame->tupletInfo[x].calcIsTremolo();
            EXPECT_EQ(isTremolo, expectedValues[x]);
            EXPECT_EQ(entryFrame->tupletInfo[x].numEntries(), expectedCounts[x]);
        }
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 4);
        ASSERT_TRUE(gfhold);
        std::vector<bool> expectedValues = { true, false };
        std::vector<size_t> expectedCounts = { 3, 3 };
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_EQ(entryFrame->tupletInfo.size(), expectedValues.size());
        for (size_t x = 0; x < entryFrame->tupletInfo.size(); x++) {
            const bool isTremolo = entryFrame->tupletInfo[x].calcIsTremolo();
            EXPECT_EQ(isTremolo, expectedValues[x]);
            EXPECT_EQ(entryFrame->tupletInfo[x].numEntries(), expectedCounts[x]);
        }
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 5);
        ASSERT_TRUE(gfhold);
        std::vector<bool> expectedValues = { false };
        std::vector<size_t> expectedCounts = { 2 };
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_EQ(entryFrame->tupletInfo.size(), expectedValues.size());
        for (size_t x = 0; x < entryFrame->tupletInfo.size(); x++) {
            const bool isTremolo = entryFrame->tupletInfo[x].calcIsTremolo();
            EXPECT_EQ(isTremolo, expectedValues[x]);
            EXPECT_EQ(entryFrame->tupletInfo[x].numEntries(), expectedCounts[x]);
        }
    }
}

TEST(GFrameHold, SingletonBeamsTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "singleton_beams.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);

    auto checkTuplet = [](const std::shared_ptr<const EntryFrame>& entryFrame, size_t tupletIndex, bool isSingletonRight, bool isSingletonLeft, bool isContinuationRight, bool isContinuationLeft) {
        std::string msg = "Staff " + std::to_string(entryFrame->getStaff()) + " measure " + std::to_string(entryFrame->getMeasure())
            + " tuplet index " + std::to_string(tupletIndex);
        ASSERT_LT(tupletIndex, entryFrame->tupletInfo.size()) << msg << " tuplet index is too big";
        EXPECT_EQ(isSingletonRight, bool(entryFrame->tupletInfo[tupletIndex].calcCreatesSingletonBeamRight())) << msg << " mismatch on singleton right";
        EXPECT_EQ(isSingletonLeft, bool(entryFrame->tupletInfo[tupletIndex].calcCreatesSingletonBeamLeft())) << msg << " mismatch on singleton left";
        EXPECT_EQ(isContinuationRight, bool(EntryInfoPtr(entryFrame, entryFrame->tupletInfo[tupletIndex].startIndex).calcBeamContinuesRightOverBarline())) << msg << " mismatch on continuation left";
        EXPECT_EQ(isContinuationLeft, bool(EntryInfoPtr(entryFrame, entryFrame->tupletInfo[tupletIndex].startIndex).calcBeamContinuesLeftOverBarline())) << msg << " mismatch on continuation right";
    };

    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
        ASSERT_TRUE(gfhold);

        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        checkTuplet(entryFrame, 0, true, false, true, false);
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 2);
        ASSERT_TRUE(gfhold);

        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        checkTuplet(entryFrame, 0, false, true, false, true);
        checkTuplet(entryFrame, 1, true, false, false, false);
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 3);
        ASSERT_TRUE(gfhold);

        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        checkTuplet(entryFrame, 0, false, true, false, false);
    }
}

TEST(GFrameHold, InterpretedIterateSingletonBeamsTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "singbeam.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);

    auto checkSequence = [](const std::shared_ptr<const EntryFrame>& entryFrame, std::vector<size_t> indexSequence) {
        std::string msg = "Staff " + std::to_string(entryFrame->getStaff()) + " measure " + std::to_string(entryFrame->getMeasure());
        size_t counter = 0;
        for (auto next = entryFrame->getFirstInterpretedIterator(1); next; next = next.getNext()) {
            EXPECT_LT(counter, indexSequence.size()) << msg << " iteration exceeded size of expected sequence";
            if (counter < indexSequence.size()) {
                EXPECT_EQ(next.getEntryInfo().getIndexInFrame(), indexSequence[counter]) << msg << " unexpected index";
            } else {
                break;
            }
            counter++;
        }
        EXPECT_EQ(counter, indexSequence.size()) << msg << " iterated forward " << counter << " times: expected " << indexSequence.size();
        EXPECT_GT(indexSequence.size(), 0) << msg << " indexSequence is empty";
        if (indexSequence.size() > 0) {
            counter = indexSequence.size();
            for (auto prev = EntryInfoPtr(entryFrame, indexSequence[counter - 1]).asInterpretedIterator(); prev; prev = prev.getPrevious()) {
                EXPECT_GE(counter - 1, 0) << msg << " iteration exceeded size of expected sequence";
                if (counter > 0) {
                    EXPECT_EQ(prev.getEntryInfo().getIndexInFrame(), indexSequence[counter - 1]) << msg << " unexpected index";
                } else {
                    break;
                }
                counter--;
            }
        }
        EXPECT_EQ(counter, 0) << msg << " iterated backward " << indexSequence.size() - counter << " times: expected " << indexSequence.size();
    };

    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
        ASSERT_TRUE(gfhold);

        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        checkSequence(entryFrame, { 0, 1, 3 });
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 2);
        ASSERT_TRUE(gfhold);

        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        checkSequence(entryFrame, { 0, 2, 3 });
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 3);
        ASSERT_TRUE(gfhold);

        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        checkSequence(entryFrame, { 0, 1, 2, 3 });
    }
}

TEST(GFrameHold, FeatheredBeamsTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "feathered_beams.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto checkEntry = [&](const EntryInfoPtr& entryInfo, bool expSuccess, Evpu expLeftY, Evpu expRightY) {
        Evpu leftY{}, rightY{};
        const bool result = entryInfo.calcIsFeatheredBeamStart(leftY, rightY);
        EXPECT_EQ(result, expSuccess);
        EXPECT_EQ(leftY, expLeftY);
        EXPECT_EQ(rightY, expRightY);
    };

    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
        ASSERT_TRUE(gfhold);

        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);

        checkEntry(EntryInfoPtr(entryFrame, 0), true, 48, 12);
        checkEntry(EntryInfoPtr(entryFrame, 9), true, 24, 48);
    }

    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 2);
        ASSERT_TRUE(gfhold);

        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);

        checkEntry(EntryInfoPtr(entryFrame, 0), false, 0, 0);
        checkEntry(EntryInfoPtr(entryFrame, 5), false, 0, 0);
    }

    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 3);
        ASSERT_TRUE(gfhold);

        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);

        checkEntry(EntryInfoPtr(entryFrame, 0), true, 12, 66);
        checkEntry(EntryInfoPtr(entryFrame, 14), true, 12, 30);
    }
}

TEST(GFrameHold, MaxElapsedEntryTest)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "elapsed-dura.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
        ASSERT_TRUE(gfhold);
        auto frame = gfhold.createEntryFrame(0);
        EXPECT_EQ(frame->maxElapsedStaffDuration, musx::util::Fraction(1, 2));
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 2);
        ASSERT_TRUE(gfhold);
        auto frame = gfhold.createEntryFrame(0);
        EXPECT_EQ(frame->maxElapsedStaffDuration, musx::util::Fraction(3, 8));
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 3);
        ASSERT_TRUE(gfhold);
        auto frame = gfhold.createEntryFrame(0);
        EXPECT_EQ(frame->maxElapsedStaffDuration, musx::util::Fraction(1, 1));
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 4);
        ASSERT_TRUE(gfhold);
        auto frame = gfhold.createEntryFrame(0);
        EXPECT_EQ(frame->maxElapsedStaffDuration, musx::util::Fraction(5, 4));
    }
}

TEST(GFrameHold, CalcNearestEntryNoExactLayer)
{
    // confirms that calcNearestEntry finds the nearest entry *in any layer*
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "expr_floatlayer.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);

    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 1);
    ASSERT_TRUE(gfhold);
    {
        auto entryInfo = gfhold.calcNearestEntry(musx::util::Fraction(1, 2), /*findExact*/false); // no match layer
        ASSERT_TRUE(entryInfo);
        EXPECT_EQ(entryInfo.getFrame()->getLayerIndex(), 1);
        EXPECT_EQ(entryInfo.getIndexInFrame(), 2);
    }
    {
        auto entryInfo = gfhold.calcNearestEntry(musx::util::Fraction(1, 2), /*findExact*/false, /*matchLayer*/0);
        ASSERT_TRUE(entryInfo);
        EXPECT_EQ(entryInfo.getFrame()->getLayerIndex(), 0);
        EXPECT_EQ(entryInfo.getIndexInFrame(), 0);
    }
    {
        auto entryInfo = gfhold.calcNearestEntry(musx::util::Fraction(1, 2), /*findExact*/false, /*matchLayer*/1);
        ASSERT_TRUE(entryInfo);
        EXPECT_EQ(entryInfo.getFrame()->getLayerIndex(), 1);
        EXPECT_EQ(entryInfo.getIndexInFrame(), 2);
    }
}

TEST(GFrameHoldTest, MissingMeasureOrStaffSkipsEntryFrame)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "quintuplet.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    constexpr StaffCmper validStaffId = 1;
    constexpr MeasCmper validMeasureId = 1;
    constexpr Cmper missingId = 30000;
    auto validHold = doc->getDetails()->get<details::GFrameHold>(SCORE_PARTID, validStaffId, validMeasureId);
    ASSERT_TRUE(validHold);
    ASSERT_TRUE(details::GFrameHoldContext(validHold).createEntryFrame(0));
    ASSERT_FALSE(doc->getOthers()->get<others::Staff>(SCORE_PARTID, missingId));
    ASSERT_FALSE(doc->getOthers()->get<others::Measure>(SCORE_PARTID, missingId));
    struct LoggerRestorer {
        Logger::LogCallback callback;
        ~LoggerRestorer() { Logger::setCallback(std::move(callback)); }
    } restorer{Logger::getCallback()};
    std::vector<std::pair<Logger::LogLevel, std::string>> diagnostics;
    Logger::setCallback([&](Logger::LogLevel level, const std::string& message) {
        diagnostics.emplace_back(level, message);
    });
    for (bool missingMeasure : {true, false}) {
        auto hold = std::make_shared<details::GFrameHold>(doc, SCORE_PARTID, EnigmaBase::ShareMode::All,
            missingMeasure ? validStaffId : missingId, missingMeasure ? missingId : validMeasureId);
        hold->frames = validHold->frames;
        details::GFrameHoldContext context(hold);
        diagnostics.clear();
        EXPECT_EQ(context.createEntryFrame(0), nullptr);
        ASSERT_EQ(diagnostics.size(), 1u);
        EXPECT_EQ(diagnostics.front().first, Logger::LogLevel::Warning);
        EXPECT_NE(diagnostics.front().second.find(missingMeasure ? "Measure instance" : "Staff instance"), std::string::npos);
        bool visited = false;
        EXPECT_TRUE(context.iterateEntries(0, [&](const EntryInfoPtr&) {
            visited = true;
            return true;
        }));
        EXPECT_FALSE(visited);
    }
}
