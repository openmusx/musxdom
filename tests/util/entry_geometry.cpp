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

namespace {

musx::dom::DocumentPtr loadEntryGeometryDocument(const char* filename,
    musx::dom::PartVoicingPolicy policy = musx::dom::PartVoicingPolicy::Ignore)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / filename, xml);
    return musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml, policy);
}

} // namespace

TEST(EntryGeometryTest, SourceStaffPositionsAndPartVoicing)
{
    using namespace musx::dom;
    using musx::util::EntryGeometry;
    const auto doc = loadEntryGeometryDocument("voiced-parts.enigmaxml", PartVoicingPolicy::Apply);
    constexpr StaffCmper staffId = 1;
    constexpr MeasCmper measureId = 3;
    constexpr Cmper topNotePartId = 1;
    constexpr int topPosition = -3;
    constexpr int bottomPosition = -7;
    constexpr int64_t positionSize = 12;
    for (const auto partId : {SCORE_PARTID, topNotePartId}) {
        const auto frame = details::GFrameHoldContext(doc, partId, staffId, measureId).createEntryFrame(0);
        const EntryInfoPtr entry(frame, 0);
        const auto result = EntryGeometry::calc(entry);
        ASSERT_EQ(result.resolution, EntryGeometry::Resolution::Resolved);
        EXPECT_EQ(result.source, EntryGeometry::Source::NotRequired);
        EXPECT_EQ(result.sourceStaffId, staffId);
        EXPECT_EQ(result.sourceStaffPositionSize, positionSize);
        ASSERT_EQ(result.notes.size(), entry->getEntry()->notes.size());
        std::vector<int64_t> positions;
        for (size_t index = 0; index < result.notes.size(); ++index) {
            EXPECT_EQ(result.notes[index].has_value(), NoteInfoPtr(entry, index).calcIsIncludedInVoicing());
            if (result.notes[index]) {
                EXPECT_EQ(result.notes[index]->staffId, staffId);
                ASSERT_TRUE(result.notes[index]->staffPosition);
                ASSERT_TRUE(result.notes[index]->verticalPosition);
                positions.push_back(*result.notes[index]->verticalPosition);
            }
        }
        ASSERT_FALSE(positions.empty());
        EXPECT_EQ(*std::max_element(positions.begin(), positions.end()), topPosition * positionSize);
        EXPECT_EQ(*std::min_element(positions.begin(), positions.end()),
            (partId == SCORE_PARTID ? bottomPosition : topPosition) * positionSize);
        EXPECT_FALSE(result.restStaffPosition);
    }
}

TEST(EntryGeometryTest, CrossStaffOriginOverridesPreserveStaffPositions)
{
    using namespace musx::dom;
    using namespace musx::util;
    const auto doc = loadEntryGeometryDocument("stemdirection.enigmaxml");
    constexpr StaffCmper staffId = 2;
    constexpr MeasCmper measureId = 1;
    constexpr size_t crossedQuarterIndex = 5;
    constexpr double originShift = 120;
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, staffId, measureId).createEntryFrame(0);
    const EntryInfoPtr entry(frame, crossedQuarterIndex);
    const auto saved = EntryGeometry::calc(entry);
    ASSERT_EQ(saved.resolution, EntryGeometry::Resolution::Resolved);
    const auto unhandled = EntryGeometry::calc(entry, [](const auto&, auto, const auto&) {
        return StaffOriginOffsetResolverResult{};
    });
    auto resolveOffset = [](double offset) -> StaffOriginOffsetResolver {
        return [offset](const auto&, auto, const auto&) {
            return StaffOriginOffsetResolverResult{StaffOriginOffsetResolverDecision::Offset, offset};
        };
    };
    const auto zero = EntryGeometry::calc(entry, resolveOffset(0));
    const auto shifted = EntryGeometry::calc(entry, resolveOffset(originShift));
    const auto unavailable = EntryGeometry::calc(entry, [](const auto&, auto, const auto&) {
        return StaffOriginOffsetResolverResult{StaffOriginOffsetResolverDecision::Unavailable, 0};
    });
    EXPECT_EQ(shifted.source, EntryGeometry::Source::Caller);
    EXPECT_EQ(unavailable.resolution, EntryGeometry::Resolution::MissingGeometry);
    bool sawCrossedNote = false;
    for (size_t index = 0; index < saved.notes.size(); ++index) {
        ASSERT_TRUE(saved.notes[index]);
        ASSERT_TRUE(unhandled.notes[index]);
        ASSERT_TRUE(zero.notes[index]);
        ASSERT_TRUE(shifted.notes[index]);
        ASSERT_TRUE(unavailable.notes[index]);
        EXPECT_EQ(saved.notes[index]->verticalPosition, unhandled.notes[index]->verticalPosition);
        EXPECT_EQ(shifted.notes[index]->staffPosition, saved.notes[index]->staffPosition);
        EXPECT_EQ(unavailable.notes[index]->staffPosition, saved.notes[index]->staffPosition);
        if (saved.notes[index]->staffId != staffId) {
            sawCrossedNote = true;
            ASSERT_TRUE(zero.notes[index]->verticalPosition);
            ASSERT_TRUE(shifted.notes[index]->verticalPosition);
            EXPECT_EQ(*shifted.notes[index]->verticalPosition, *zero.notes[index]->verticalPosition - originShift);
            EXPECT_FALSE(unavailable.notes[index]->verticalPosition);
        }
    }
    EXPECT_TRUE(sawCrossedNote);
}

TEST(EntryGeometryTest, RestPositionAndUnsupportedMirror)
{
    using namespace musx::dom;
    using musx::util::EntryGeometry;
    const auto doc = loadEntryGeometryDocument("stemdirection.enigmaxml");
    constexpr StaffCmper staffId = 1;
    constexpr MeasCmper measureId = 2;
    constexpr size_t floatingRestIndex = 6;
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, staffId, measureId).createEntryFrame(0);
    const EntryInfoPtr entry(frame, floatingRestIndex);
    ASSERT_TRUE(entry.calcDisplaysAsRest());
    const auto result = EntryGeometry::calc(entry);
    ASSERT_EQ(result.resolution, EntryGeometry::Resolution::Resolved);
    ASSERT_TRUE(result.restStaffPosition);
    EXPECT_EQ(result.restVerticalPosition, *result.restStaffPosition * result.sourceStaffPositionSize);
    for (const auto& note : result.notes) {
        EXPECT_FALSE(note);
    }
    auto hold = std::const_pointer_cast<details::GFrameHold>(
        doc->getDetails()->get<details::GFrameHold>(SCORE_PARTID, staffId, measureId));
    hold->mirrorFrame = true;
    EXPECT_EQ(EntryGeometry::calc(entry).resolution, EntryGeometry::Resolution::UnsupportedLegacyContext);
}

TEST(EntryGeometryTest, FractionalStaffScalingRoundsHalfEvpuAwayFromZero)
{
    using namespace musx::dom;
    using namespace musx::util;
    const auto doc = loadEntryGeometryDocument("stemdirection.enigmaxml");
    constexpr StaffCmper sourceStaffId = 2;
    constexpr MeasCmper measureId = 1;
    constexpr size_t crossedQuarterIndex = 5;
    constexpr int sourcePercent = 96;
    constexpr int targetPercent = 70;
    constexpr int testPosition = 2;
    constexpr int64_t roundedDistance = 18; // 2 * 12 * (70 / 96) = 17.5 EVPU.
    constexpr double originOffset = 96;
    constexpr int64_t normalizedOffset = 100;
    const auto frame = details::GFrameHoldContext(doc, SCORE_PARTID, sourceStaffId, measureId).createEntryFrame(0);
    const EntryInfoPtr entry(frame, crossedQuarterIndex);
    const NoteInfoPtr note(entry, 0);
    const auto targetStaffId = note.calcStaff();
    ASSERT_NE(targetStaffId, sourceStaffId);
    const auto system = std::const_pointer_cast<others::StaffSystem>(doc->calcSystemFromMeasure(SCORE_PARTID, measureId));
    ASSERT_TRUE(system);
    system->hasStaffScaling = true;
    for (const auto staffId : {sourceStaffId, targetStaffId}) {
        auto size = std::make_shared<details::StaffSize>(doc, SCORE_PARTID,
            EnigmaBase::ShareMode::None, system->getCmper(), staffId);
        size->staffPercent = staffId == sourceStaffId ? sourcePercent : targetPercent;
        doc->getDetails()->add(details::StaffSize::XmlNodeName, size);
    }
    const auto rawNote = std::const_pointer_cast<Note>(entry->getEntry()->notes.front());
    for (const int position : {testPosition, -testPosition}) {
        rawNote->harmLev += position - note.calcNoteProperties().staffPosition;
        const auto result = EntryGeometry::calc(entry, [](const auto&, auto, const auto&) {
            return StaffOriginOffsetResolverResult{StaffOriginOffsetResolverDecision::Offset, originOffset};
        });
        ASSERT_EQ(result.resolution, EntryGeometry::Resolution::Resolved);
        EXPECT_EQ(result.sourceStaffScale, Fraction::fromPercent(sourcePercent));
        ASSERT_TRUE(result.notes.front());
        EXPECT_EQ(result.notes.front()->staffPosition, position);
        EXPECT_EQ(result.notes.front()->verticalPosition,
            -normalizedOffset + (position < 0 ? -roundedDistance : roundedDistance));
    }
}
