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

using namespace musxtest;
using namespace musx::dom;

TEST(EffectiveClef, CrossedNotesUseTheirStaff)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "cross_staffs.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    // Measure 4: the first note of each entry is crossed to staff 2, whose clef changes mid-measure.
    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 4);
    ASSERT_TRUE(gfhold);
    auto entryFrame = gfhold.createEntryFrame(0);
    ASSERT_TRUE(entryFrame);

    using Expected = std::tuple<size_t, size_t, StaffCmper, ClefIndex>; // entry index, note index, staff, clef
    constexpr std::array<Expected, 4> expected = {
        Expected{0, 0, 2, 3},
        Expected{0, 1, 1, 0},
        Expected{1, 0, 2, 1},
        Expected{1, 2, 1, 0},
    };
    for (const auto& [entryIndex, noteIndex, staffId, clefIndex] : expected) {
        const auto note = musxtest::createNoteInfo(entryFrame, entryIndex, noteIndex);
        ASSERT_TRUE(note);
        const std::string msg = "entry " + std::to_string(entryIndex) + " note " + std::to_string(noteIndex);
        const auto clef = note.calcEffectiveClef(PitchMode::Concert);
        ASSERT_TRUE(clef) << msg;
        EXPECT_EQ(clef.staff->getCmper(), staffId) << msg;
        // The staff is the one at the entry's position, not at the start of the measure.
        EXPECT_EQ(clef.staff->getMeasureId(), note.getEntryInfo().getMeasure()) << msg;
        EXPECT_EQ(clef.staff->getEduPosition(), note.getEntryInfo()->elapsedDuration.calcEduDuration()) << msg;
        EXPECT_EQ(clef.clefIndex, clefIndex) << msg;
        EXPECT_FALSE(clef.forWrittenPitch) << msg;

        // Ignoring cross-staffing gives the entry's own staff and clef.
        const auto entryClef = note.calcEffectiveClef(PitchMode::Concert, true);
        ASSERT_TRUE(entryClef) << msg;
        EXPECT_EQ(entryClef.staff->getCmper(), note.getEntryInfo().getStaff()) << msg;
        EXPECT_EQ(entryClef.clefIndex, note.getEntryInfo()->clefIndexConcert) << msg;
    }

    // A staff position on the crossed staff reads with that staff's clef.
    {
        const auto note = musxtest::createNoteInfo(entryFrame, 0, 0);
        const auto properties = note.calcNoteProperties({.pitchMode = PitchMode::Concert});
        const auto pitch = note.getEntryInfo().calcPitchFromStaffPosition(properties.staffPosition,
            {.pitchMode = PitchMode::Concert, .actualAlteration = std::nullopt, .forStaffId = note.calcStaff()});
        EXPECT_EQ(pitch.noteName, properties.noteName);
        EXPECT_EQ(pitch.octave, properties.octave);
    }

    // A crossed staff that is set to percussion notation reads its clef as a percussion clef.
    auto staff2 = const_cast<others::Staff*>(doc->getOthers()->get<others::Staff>(SCORE_PARTID, 2).get());
    ASSERT_TRUE(staff2);
    staff2->notationStyle = others::Staff::NotationStyle::Percussion;
    const auto crossed = musxtest::createNoteInfo(entryFrame, 0, 0);
    EXPECT_EQ(crossed.calcEffectiveClef().calcInfo().first, music_theory::ClefType::Percussion1);
    EXPECT_NE(musxtest::createNoteInfo(entryFrame, 0, 1).calcEffectiveClef().calcInfo().first, music_theory::ClefType::Percussion1);
}

TEST(EffectiveClef, WrittenClefOfATransposingStaff)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "transpose.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    for (MeasCmper measId = 1; measId <= 5; measId++) {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, measId);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        const std::string msg = "measure " + std::to_string(measId);
        const auto entryInfo = EntryInfoPtr(entryFrame, 0);
        const auto written = entryInfo.calcEffectiveClef(PitchMode::Written);
        EXPECT_TRUE(written.forWrittenPitch) << msg;
        EXPECT_EQ(written.clefIndex, entryInfo->clefIndex) << msg;
        const auto concert = entryInfo.calcEffectiveClef(PitchMode::Concert);
        EXPECT_FALSE(concert.forWrittenPitch) << msg;
        EXPECT_EQ(concert.clefIndex, entryInfo->clefIndexConcert) << msg;
        // Naming the entry's own staff is the same as naming none.
        EXPECT_EQ(entryInfo.calcEffectiveClef(PitchMode::Written, entryInfo.getStaff()).clefIndex, written.clefIndex) << msg;
    }

    // Measure 3's staff style sets the written clef to clef 0. The staff's own clef is clef 1.
    auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 3);
    ASSERT_TRUE(gfhold);
    const auto entryInfo = EntryInfoPtr(gfhold.createEntryFrame(0), 0);
    EXPECT_EQ(entryInfo.calcEffectiveClef(PitchMode::Written).clefIndex, 0);
    EXPECT_EQ(entryInfo.calcEffectiveClef(PitchMode::Concert).clefIndex, 1);
}
