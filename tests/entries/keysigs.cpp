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

TEST(KeySigs, Test12EDO)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "keysigs.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<std::optional<int>> expectedKeyAlters = { -4, 5, -2, 3, 0 };
    std::vector<unsigned> expectedIndices = { 5, 4, 2, 0, 1 }; // AbMaj, G#min, Eb Lydian, C# Phrygian, D Freyish (non-linear)
    ASSERT_EQ(expectedKeyAlters.size(), expectedIndices.size());
    std::vector<std::optional<music_theory::DiatonicMode>> expectedModes = {
        music_theory::DiatonicMode::Ionian, music_theory::DiatonicMode::Aeolian,
        music_theory::DiatonicMode::Lydian, music_theory::DiatonicMode::Phrygian,
        std::nullopt
    };
    ASSERT_EQ(expectedKeyAlters.size(), expectedModes.size());
    std::vector<std::optional<std::vector<int>>> expectedKeyMaps = {
        std::nullopt,
        std::nullopt,
        std::vector<int>{0, 2, 4, 6, 7, 9, 11},
        std::vector<int>{0, 1, 3, 5, 7, 8, 10},
        std::vector<int>{0, 1, 4, 5, 7, 8, 10}
    };
    ASSERT_EQ(expectedKeyAlters.size(), expectedKeyMaps.size());

    std::vector<music_theory::NoteName> expectedNotes = {
        music_theory::NoteName::E, music_theory::NoteName::F, music_theory::NoteName::G, music_theory::NoteName::A,
        music_theory::NoteName::B, music_theory::NoteName::C, music_theory::NoteName::D, music_theory::NoteName::E };
    std::vector<int> expectedOctaves = { 4, 4, 4, 4, 4, 5, 5, 5 };
    std::vector<int> expectedAlters = { 0, 1, 0, -1, -1, 0, 0, -1 };
    std::vector<int> expectedPositions = { -8, -7, -6, -5, -4, -3, -2, -1 };

    auto measures = others->getArray<others::Measure>(SCORE_PARTID);
    ASSERT_GE(measures.size(), expectedKeyAlters.size());

    for (size_t i = 0; i < expectedKeyAlters.size(); i++) {
        auto key = measures[i]->createKeySignature();
        EXPECT_EQ(key->getAlteration(KeySignature::KeyContext::Written), expectedKeyAlters[i]);
        EXPECT_EQ(key->calcTonalCenterIndex(KeySignature::KeyContext::Written), expectedIndices[i]);
        EXPECT_EQ(key->calcDiatonicMode(), expectedModes[i]);
        EXPECT_EQ(key->calcKeyMap(), expectedKeyMaps[i]);
        if (auto keyMap = key->calcKeyMap()) {
            EXPECT_EQ(keyMap->size(), 7);
        }
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, measures[i]->getCmper());
        ASSERT_TRUE(gfhold);
        size_t x = 0;
        gfhold.iterateEntries([&](const EntryInfoPtr& entryInfo) -> bool {
            EXPECT_LT(x, expectedNotes.size()) << "too few expected values";
            if (x >= expectedNotes.size()) return false;
            //ASSERT_GE(entryInfo->getEntry()->notes.size(), 1);
            auto note = entryInfo->getEntry()->notes[0];
            auto [noteName, octave, alter, position] = note->calcNoteProperties(key, KeySignature::KeyContext::Written, entryInfo->clefIndex, nullptr);
            EXPECT_EQ(expectedOctaves[x], octave);
            EXPECT_EQ(expectedAlters[x], alter);
            EXPECT_EQ(expectedPositions[x], position);
            x++;
            return true;
        });
        EXPECT_EQ(x, expectedNotes.size());
    }
}

TEST(KeySigs, Test31EDO)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "keysigs.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    std::vector<std::optional<int>> expectedKeyAlters = { 4, -5 };
    std::vector<unsigned> expectedIndices = { 2, 6 }; // EMaj, Bbmin
    ASSERT_EQ(expectedKeyAlters.size(), expectedIndices.size());
    std::vector<std::optional<music_theory::DiatonicMode>> expectedModes = {
        music_theory::DiatonicMode::Ionian, music_theory::DiatonicMode::Aeolian
    };
    ASSERT_EQ(expectedKeyAlters.size(), expectedModes.size());
    std::vector<std::optional<std::vector<int>>> expectedKeyMaps = {
        std::vector<int>{0, 5, 10, 13, 18, 23, 28},
        std::vector<int>{0, 5, 8, 13, 18, 21, 26}
    };
    ASSERT_EQ(expectedKeyAlters.size(), expectedKeyMaps.size());

    std::vector<music_theory::NoteName> expectedNotes = {
        music_theory::NoteName::E, music_theory::NoteName::F, music_theory::NoteName::G, music_theory::NoteName::A,
        music_theory::NoteName::B, music_theory::NoteName::C, music_theory::NoteName::D, music_theory::NoteName::E };
    std::vector<int> expectedOctaves = { 4, 4, 4, 4, 4, 5, 5, 5 };
    std::vector<int> expectedAlters = { 0, 2, 0, -2, -2, 3, 0, -1 };
    std::vector<int> expectedPositions = { -8, -7, -6, -5, -4, -3, -2, -1 };

    static constexpr size_t FIRST_31EDO_MEASURE_INDEX = 5;

    auto measures = others->getArray<others::Measure>(SCORE_PARTID);
    ASSERT_GE(measures.size(), expectedKeyAlters.size() + FIRST_31EDO_MEASURE_INDEX);

    for (size_t i = 0; i < expectedKeyAlters.size(); i++) {
        auto measure = measures[i + FIRST_31EDO_MEASURE_INDEX];
        auto key = measure->createKeySignature();
        EXPECT_EQ(key->getAlteration(KeySignature::KeyContext::Written), expectedKeyAlters[i]);
        EXPECT_EQ(key->calcTonalCenterIndex(KeySignature::KeyContext::Written), expectedIndices[i]);
        EXPECT_EQ(key->calcDiatonicMode(), expectedModes[i]);
        EXPECT_EQ(key->calcKeyMap(), expectedKeyMaps[i]);
        if (auto keyMap = key->calcKeyMap()) {
            EXPECT_EQ(keyMap->size(), 7);
        }
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, measure->getCmper());
        ASSERT_TRUE(gfhold);
        size_t x = 0;
        gfhold.iterateEntries([&](const EntryInfoPtr& entryInfo) -> bool {
            EXPECT_LT(x, expectedNotes.size()) << "too few expected values";
            if (x >= expectedNotes.size()) return false;
            //ASSERT_GE(entryInfo->getEntry()->notes.size(), 1);
            auto note = entryInfo->getEntry()->notes[0];
            auto [noteName, octave, alter, position] = note->calcNoteProperties(key,KeySignature::KeyContext::Written, entryInfo->clefIndex, nullptr);
            EXPECT_EQ(expectedOctaves[x], octave);
            EXPECT_EQ(expectedAlters[x], alter);
            EXPECT_EQ(expectedPositions[x], position);
            x++;
            return true;
        });
        EXPECT_EQ(x, expectedNotes.size());
    }
}

TEST(KeySigs, PopulateKeyAttributes)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "keysigs.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto keyAttributes = others->get<others::KeyAttributes>(SCORE_PARTID, 5);
    ASSERT_TRUE(keyAttributes);

    // Fields populated from XML
    EXPECT_EQ(keyAttributes->middleCKey, 60);
    EXPECT_EQ(keyAttributes->fontSym, Cmper(5));
    EXPECT_EQ(keyAttributes->gotoKey, 1);
    EXPECT_EQ(keyAttributes->symbolList, Cmper(2));

    // Unspecified fields should retain default values
    EXPECT_EQ(keyAttributes->harmRefer, 0);
    EXPECT_FALSE(keyAttributes->hasClefOctv);
}

TEST(KeySigs, PopulateKeySymbolList)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "keysigs.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    auto keySymbols = details->getArray<details::KeySymbolListElement>(SCORE_PARTID, 2);
    ASSERT_EQ(int(keySymbols.size()), 15);

    EXPECT_EQ(keySymbols[0]->getAlterationValue(), 0);
    EXPECT_EQ(keySymbols[0]->accidentalString, "n");

    EXPECT_EQ(keySymbols[1]->getAlterationValue(), 1);
    EXPECT_EQ(keySymbols[1]->accidentalString, "µ");

    EXPECT_EQ(keySymbols[2]->getAlterationValue(), 2);
    EXPECT_EQ(keySymbols[2]->accidentalString, "m");

    EXPECT_EQ(keySymbols[3]->getAlterationValue(), 3);
    EXPECT_EQ(keySymbols[3]->accidentalString, "÷");

    EXPECT_EQ(keySymbols[4]->getAlterationValue(), 4);
    EXPECT_EQ(keySymbols[4]->accidentalString, "M");

    EXPECT_EQ(keySymbols[5]->getAlterationValue(), 5);
    EXPECT_EQ(keySymbols[5]->accidentalString, "Mµ");

    EXPECT_EQ(keySymbols[6]->getAlterationValue(), 6);
    EXPECT_EQ(keySymbols[6]->accidentalString, "Mm");

    EXPECT_EQ(keySymbols[7]->getAlterationValue(), 7);
    EXPECT_EQ(keySymbols[7]->accidentalString, "M÷");

    EXPECT_EQ(keySymbols[8]->getAlterationValue(), -7);
    EXPECT_EQ(keySymbols[8]->accidentalString, "Bbbb");

    EXPECT_EQ(keySymbols[9]->getAlterationValue(), -6);
    EXPECT_EQ(keySymbols[9]->accidentalString, "bbb");

    EXPECT_EQ(keySymbols[10]->getAlterationValue(), -5);
    EXPECT_EQ(keySymbols[10]->accidentalString, "Bº");

    EXPECT_EQ(keySymbols[11]->getAlterationValue(), -4);
    EXPECT_EQ(keySymbols[11]->accidentalString, "º");

    EXPECT_EQ(keySymbols[12]->getAlterationValue(), -3);
    EXPECT_EQ(keySymbols[12]->accidentalString, "Bb");

    EXPECT_EQ(keySymbols[13]->getAlterationValue(), -2);
    EXPECT_EQ(keySymbols[13]->accidentalString, "b");

    EXPECT_EQ(keySymbols[14]->getAlterationValue(), -1);
    EXPECT_EQ(keySymbols[14]->accidentalString, "B");
}

TEST(KeySigs, StavesThatReadInC)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "keysigs.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    // Measure 1 is in A-flat major.
    auto measure = doc->getOthers()->get<others::Measure>(SCORE_PARTID, 1);
    ASSERT_TRUE(measure);
    auto staff = const_cast<others::Staff*>(doc->getOthers()->get<others::Staff>(SCORE_PARTID, 1).get());
    ASSERT_TRUE(staff);
    ASSERT_FALSE(staff->transposition);
    EXPECT_EQ(measure->createKeySignature(1)->getAlteration(KeySignature::KeyContext::Concert), -4);

    // Ignoring key signatures reads the staff in C major.
    staff->noKey = true;
    auto key = measure->createKeySignature(1);
    EXPECT_EQ(key->getAlteration(KeySignature::KeyContext::Concert), 0);
    EXPECT_TRUE(key->isMajor());
    EXPECT_FALSE(key->keyless);
    staff->noKey = false;

    // Percussion notation alone keeps the measure's key.
    staff->notationStyle = others::Staff::NotationStyle::Percussion;
    staff->percussionMapId = Cmper(0);
    EXPECT_EQ(measure->createKeySignature(1)->getAlteration(KeySignature::KeyContext::Concert), -4);

    // A percussion map reads the staff in C major.
    staff->percussionMapId = Cmper(1);
    key = measure->createKeySignature(1);
    EXPECT_EQ(key->getAlteration(KeySignature::KeyContext::Concert), 0);
    EXPECT_TRUE(key->isMajor());
    staff->notationStyle = others::Staff::NotationStyle::Standard;
    staff->percussionMapId.reset();

    // Without a staff the measure's own key applies.
    EXPECT_EQ(measure->createKeySignature()->getAlteration(KeySignature::KeyContext::Concert), -4);
}
