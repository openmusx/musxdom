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

TEST(StemTests, DefaultDirection)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "stemdirection.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 8);

        EXPECT_TRUE(EntryInfoPtr(entryFrame, 1).calcUpStemDefault()); // 2nd grace note in beam
        EXPECT_FALSE(EntryInfoPtr(entryFrame, 4).calcUpStemDefault()); // 2nd note in beam
        EXPECT_TRUE(EntryInfoPtr(entryFrame, 5).calcUpStemDefault()); // cross-staffed quarter
        EXPECT_TRUE(EntryInfoPtr(entryFrame, 6).calcUpStemDefault()); // 1st note in cross-staffed beam
        EXPECT_TRUE(EntryInfoPtr(entryFrame, 7).calcUpStemDefault()); // 1st note in cross-staffed beam
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 2);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 8);

        EXPECT_FALSE(EntryInfoPtr(entryFrame, 1).calcUpStemDefault()); // 2nd note in beam
        EXPECT_FALSE(EntryInfoPtr(entryFrame, 6).calcUpStemDefault()); // Floating rest
        EXPECT_TRUE(EntryInfoPtr(entryFrame, 7).calcUpStemDefault()); // Final chord
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 3, 3);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 4);

        EXPECT_FALSE(EntryInfoPtr(entryFrame, 0).calcUpStemDefault()); // Layer 1 first 8th down is default
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 3, 3);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(1);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 3);

        EXPECT_TRUE(EntryInfoPtr(entryFrame, 0).calcUpStemDefault()); // Layer 1 first quarter up is default.
    }
}


TEST(StemTests, ActualDirection)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "stemdirection.enigmaxml", xml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 8);

        EXPECT_FALSE(EntryInfoPtr(entryFrame, 1).calcUpStem()); // 2nd grace note in beam
        EXPECT_FALSE(EntryInfoPtr(entryFrame, 4).calcUpStem()); // 2nd note in beam
        EXPECT_TRUE(EntryInfoPtr(entryFrame, 5).calcUpStem()); // cross-staffed quarter
        EXPECT_TRUE(EntryInfoPtr(entryFrame, 6).calcUpStem()); // 1st note in cross-staffed beam
        EXPECT_TRUE(EntryInfoPtr(entryFrame, 7).calcUpStem()); // 1st note in cross-staffed beam

        EXPECT_TRUE(EntryInfoPtr(entryFrame, 5).calcCrossStaffDirectionForAll());
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 2);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 3);

        EXPECT_FALSE(EntryInfoPtr(entryFrame, 0).calcUpStem()); // crossed initial quarter
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 1, 3);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 6);

        EXPECT_FALSE(EntryInfoPtr(entryFrame, 1).calcUpStem()); // initial quarter (staff forced down)
        EXPECT_FALSE(EntryInfoPtr(entryFrame, 3).calcUpStem()); // 2nd v2 eighth (staff forced down)
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 3);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 5);

        EXPECT_FALSE(EntryInfoPtr(entryFrame, 0).calcUpStem()); // initial v2Launch quarter (down)
        EXPECT_TRUE(EntryInfoPtr(entryFrame, 2).calcUpStem()); // 2nd v2 eighth (up)
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 3, 3);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(0);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 4);

        EXPECT_TRUE(EntryInfoPtr(entryFrame, 0).calcUpStem()); // Layer 1 first 8th up (ignoring Staff down setting)
    }
    {
        auto gfhold = details::GFrameHoldContext(doc, SCORE_PARTID, 3, 3);
        ASSERT_TRUE(gfhold);
        auto entryFrame = gfhold.createEntryFrame(1);
        ASSERT_TRUE(entryFrame);
        ASSERT_GE(entryFrame->getEntries().size(), 3);

        EXPECT_FALSE(EntryInfoPtr(entryFrame, 0).calcUpStem()); // Layer 1 first quarter down.
    }
}

namespace {
DocumentPtr stemDocument()
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "stemdirection.enigmaxml", xml);
    return musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
}
} // namespace

TEST(StemUtility, BalancedChordVotesAndTranslation)
{
    using musx::util::Stem;
    const auto doc = stemDocument();
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1).createEntryFrame(0);
    const EntryInfoPtr first(frame, 6), second(frame, 7);
    ASSERT_FALSE(first.calcUnbeamed());
    for (auto item : {first, second}) {
        auto raw = std::const_pointer_cast<Entry>(item->getEntry());
        raw->notes.resize(2, raw->notes.front());
        raw->numNotes = 2;
    }
    for (int translation : {0, -123, 321}) {
        Stem::Options options;
        options.geometryResolver = [=](const EntryInfoPtr& item) {
            Stem::Geometry geometry;
            geometry.reversal = translation;
            geometry.entryGeometry = musx::util::EntryGeometry::calc(item);
            const auto positions = item.isSameEntry(first)
                ? std::vector<int64_t>{translation - 6, translation + 6}
                : std::vector<int64_t>{translation - 2, translation - 1};
            for (size_t index = 0; index < positions.size(); ++index) {
                geometry.entryGeometry.notes[index]->verticalPosition = positions[index];
            }
            return Stem::GeometryResult{musx::util::StaffOriginOffsetResolverDecision::Offset, geometry};
        };
        const auto result = Stem::calcDefaultDirection(first, options);
        EXPECT_EQ(result.primaryDirection, Stem::Direction::Up);
        EXPECT_EQ(result.reason, Stem::Reason::BeamTieBreak);
        EXPECT_EQ(Stem::calcDefaultDirection(second, options).primaryDirection, result.primaryDirection);
    }
}

TEST(StemUtility, CrossStaffUnavailableAndCallerGeometry)
{
    using musx::util::Stem;
    const auto doc = stemDocument();
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1).createEntryFrame(0);
    const EntryInfoPtr entry(frame, 5);
    Stem::Options options;
    options.staffOriginOffsetResolver = [](const auto&, auto, const auto&) {
        return musx::util::StaffOriginOffsetResolverResult{musx::util::StaffOriginOffsetResolverDecision::Unavailable, 0};
    };
    auto result = Stem::calcDefaultDirection(entry, options);
    EXPECT_EQ(result.resolution, Stem::Resolution::MissingGeometry);
    EXPECT_FALSE(result.primaryDirection);
    options.geometryResolver = [](const EntryInfoPtr& item) {
        Stem::Geometry geometry;
        geometry.entryGeometry = musx::util::EntryGeometry::calc(item);
        for (auto& note : geometry.entryGeometry.notes) {
            if (note) {
                note->verticalPosition = -12;
            }
        }
        return Stem::GeometryResult{musx::util::StaffOriginOffsetResolverDecision::Offset, geometry};
    };
    result = Stem::calcDefaultDirection(entry, options);
    EXPECT_EQ(result.primaryDirection, Stem::Direction::Up);
    EXPECT_EQ(result.geometrySource, Stem::GeometrySource::Caller);
}

TEST(StemUtility, EarlierStaffOverrideWinsOverLaterFreeze)
{
    using musx::util::Stem;
    const auto doc = stemDocument();
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1).createEntryFrame(0);
    const EntryInfoPtr first(frame, 6), second(frame, 7);
    auto staff = std::const_pointer_cast<others::StaffComposite>(first.createCurrentStaff());
    staff->stemDirection = StemDirection::AlwaysDown;
    auto later = std::const_pointer_cast<Entry>(second->getEntry());
    later->freezeStemScore = true;
    later->upStemScore = true;
    const auto result = Stem::calcForcedDirection(first);
    EXPECT_EQ(result.primaryDirection, Stem::Direction::Down);
    EXPECT_EQ(result.reason, Stem::Reason::Staff);
    EXPECT_TRUE(result.sourceEntry.isSameEntry(first));
}

TEST(StemUtility, DoubleStemAssignmentsAndLegacy)
{
    using musx::util::Stem;
    const auto doc = stemDocument();
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1).createEntryFrame(0);
    const EntryInfoPtr entry(frame, 5);
    auto raw = std::const_pointer_cast<Entry>(entry->getEntry());
    raw->doubleStem = true;
    raw->upStemScore = false;
    const auto result = Stem::calcEffectiveDirection(entry);
    EXPECT_EQ(result.primaryDirection, Stem::Direction::Down);
    ASSERT_EQ(result.assignments.size(), raw->notes.size());
    for (auto assignment : result.assignments) EXPECT_EQ(assignment, Stem::Assignment::Both);
    auto hold = std::const_pointer_cast<details::GFrameHold>(doc->getDetails()->get<details::GFrameHold>(SCORE_PARTID, 2, 1));
    hold->mirrorFrame = true;
    EXPECT_EQ(Stem::calcEffectiveDirection(entry).resolution, Stem::Resolution::UnsupportedLegacyContext);
}


TEST(StemUtility, SingleChordSplitReverseAndSuppression)
{
    using musx::util::Stem;
    const auto doc = stemDocument();
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1).createEntryFrame(0);
    const EntryInfoPtr entry(frame, 5);
    auto raw = std::const_pointer_cast<Entry>(entry->getEntry());
    raw->notes.resize(2, raw->notes.front());
    raw->notes[1] = std::make_shared<Note>(*raw->notes[0]);
    raw->numNotes = 2;
    Stem::Options options;
    options.geometryResolver = [](const EntryInfoPtr& item) {
        Stem::Geometry geometry;
        geometry.entryGeometry = musx::util::EntryGeometry::calc(item);
        geometry.entryGeometry.notes[0]->verticalPosition = -6;
        geometry.entryGeometry.notes[1]->verticalPosition = 6;
        geometry.reverseAdjustment = 2;
        return Stem::GeometryResult{musx::util::StaffOriginOffsetResolverDecision::Offset, geometry};
    };
    EXPECT_EQ(Stem::calcDefaultDirection(entry, options).primaryDirection, Stem::Direction::Down);
    raw->reverseUpStem = true;
    auto stemOptions = std::const_pointer_cast<options::StemOptions>(doc->getOptions()->get<options::StemOptions>());
    stemOptions->noReverseStems = false;
    EXPECT_EQ(Stem::calcDefaultDirection(entry, options).primaryDirection, Stem::Direction::Up);
    stemOptions->noReverseStems = true;
    EXPECT_EQ(Stem::calcDefaultDirection(entry, options).primaryDirection, Stem::Direction::Down);
    raw->splitStem = true;
    raw->upStemScore = true;
    raw->notes[1]->upSplitStem = true;
    const auto result = Stem::calcEffectiveDirection(entry, options);
    EXPECT_EQ(result.primaryDirection, Stem::Direction::Up);
    EXPECT_EQ(result.assignments[0], Stem::Assignment::Down);
    EXPECT_EQ(result.assignments[1], Stem::Assignment::Up);
}

TEST(StemUtility, GraceUsesStoredDirectionAndMissingPositions)
{
    using musx::util::Stem;
    const auto doc = stemDocument();
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1).createEntryFrame(0);
    const EntryInfoPtr entry(frame, 5);
    auto raw = std::const_pointer_cast<Entry>(entry->getEntry());
    raw->graceNote = true;
    raw->upStemScore = false;
    EXPECT_EQ(Stem::calcEffectiveDirection(entry).primaryDirection, Stem::Direction::Down);
    raw->graceNote = false;
    Stem::Options options;
    options.geometryResolver = [](const EntryInfoPtr& item) {
        Stem::Geometry geometry;
        geometry.entryGeometry.notes.resize(item->getEntry()->notes.size());
        return Stem::GeometryResult{musx::util::StaffOriginOffsetResolverDecision::Offset, geometry};
    };
    const auto result = Stem::calcDefaultDirection(entry, options);
    EXPECT_FALSE(result.primaryDirection);
    EXPECT_EQ(result.resolution, Stem::Resolution::NoEligiblePositions);
}


TEST(StemUtility, DefaultWrapperIncludesCrossStaffGeometry)
{
    using musx::util::Stem;
    const auto doc = stemDocument();
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, 2, 1).createEntryFrame(0);
    constexpr size_t crossedQuarterIndex = 5;
    constexpr size_t crossedBeamStartIndex = 6;
    constexpr size_t crossedBeamEndIndex = 7;
    for (const auto index : {crossedQuarterIndex, crossedBeamStartIndex, crossedBeamEndIndex}) {
        const EntryInfoPtr entry(frame, index);
        const auto result = Stem::calcDefaultDirection(entry);
        ASSERT_EQ(result.resolution, Stem::Resolution::Resolved);
        ASSERT_EQ(result.primaryDirection, Stem::Direction::Up);
        EXPECT_TRUE(entry.calcUpStemDefault());
        EXPECT_EQ(entry.calcUpStemDefault(), entry.calcUpStem());
    }
}

TEST(StemUtility, EmbeddedGeometryHonorsAvailability)
{
    using musx::util::EntryGeometry;
    using musx::util::Stem;
    using musx::util::StaffOriginOffsetResolverDecision;
    const auto doc = stemDocument();
    constexpr StaffCmper staffId = 2;
    constexpr MeasCmper measureId = 1;
    constexpr size_t crossedQuarterIndex = 5;
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, staffId, measureId).createEntryFrame(0);
    const EntryInfoPtr entry(frame, crossedQuarterIndex);
    Stem::Geometry geometry;
    geometry.entryGeometry = EntryGeometry::calc(entry);
    const auto staff = entry.createCurrentStaff();
    geometry.reversal = staff->stemReversal * geometry.entryGeometry.sourceStaffPositionSize;
    geometry.reverseAdjustment = doc->getOptions()->get<options::StemOptions>()->revStemAdj;
    Stem::Options providers;
    providers.geometryResolver = [&geometry](const auto&) {
        return Stem::GeometryResult{StaffOriginOffsetResolverDecision::Offset, geometry};
    };
    const auto defaultResult = Stem::calcDefaultDirection(entry);
    const auto suppliedResult = Stem::calcDefaultDirection(entry, providers);
    ASSERT_EQ(suppliedResult.resolution, Stem::Resolution::Resolved);
    EXPECT_EQ(suppliedResult.primaryDirection, defaultResult.primaryDirection);
    EXPECT_EQ(suppliedResult.geometrySource, Stem::GeometrySource::Caller);
    for (const auto resolution : {EntryGeometry::Resolution::MissingGeometry,
        EntryGeometry::Resolution::UnsupportedLegacyContext}) {
        geometry.entryGeometry.resolution = resolution;
        const auto result = Stem::calcDefaultDirection(entry, providers);
        EXPECT_EQ(result.resolution, resolution == EntryGeometry::Resolution::MissingGeometry
            ? Stem::Resolution::MissingGeometry : Stem::Resolution::UnsupportedLegacyContext);
        EXPECT_FALSE(result.primaryDirection);
        EXPECT_EQ(result.geometrySource, Stem::GeometrySource::Caller);
    }
    geometry.entryGeometry.resolution = EntryGeometry::Resolution::Resolved;
    ASSERT_TRUE(geometry.entryGeometry.notes.front());
    geometry.entryGeometry.notes.front()->verticalPosition.reset();
    const auto incomplete = Stem::calcDefaultDirection(entry, providers);
    EXPECT_EQ(incomplete.resolution, Stem::Resolution::MissingGeometry);
    EXPECT_FALSE(incomplete.primaryDirection);
}
