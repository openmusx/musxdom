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
#include "musx/musx.h"
#include "test_utils.h"
#include "gtest/gtest.h"

using namespace musx::dom;
using musx::util::Slur;
using Contour = CurveContourDirection;

namespace {
class SlurTest : public ::testing::Test
{
protected:
    static constexpr Cmper staffId = 1;
    static constexpr MeasCmper measureId = 2;
    static constexpr Cmper shapeId = 999;
    DocumentPtr document;
    std::shared_ptr<others::SmartShape> shape;
    std::shared_ptr<others::Staff> staff;
    std::shared_ptr<const EntryFrame> frame;
    void SetUp() override
    {
        std::vector<char> xml;
        musxtest::readFile(musxtest::getInputPath() / "stemdirection.enigmaxml",
                           xml);
        document =
            musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(
                xml);
        staff = std::const_pointer_cast<others::Staff>(
            document->getOthers()->get<others::Staff>(SCORE_PARTID, staffId));
        staff->stemDirection = StemDirection::AlwaysUp;
        const details::GFrameHoldContext context(document, SCORE_PARTID, staffId,
                                                 measureId);
        frame = context.createEntryFrame(0);
        for (const auto& info : frame->getEntries()) {
            auto raw = std::const_pointer_cast<Entry>(info->getEntry());
            raw->freezeStemScore = false;
            raw->isHidden = false;
            raw->voice2 = raw->v2Launch = raw->graceNote = false;
        }
        auto layer = std::const_pointer_cast<others::LayerAttributes>(
            document->getOthers()->get<others::LayerAttributes>(SCORE_PARTID, 0));
        layer->freezeLayer = false;
        shape = std::make_shared<others::SmartShape>(
            document, SCORE_PARTID, EnigmaBase::ShareMode::All, shapeId);
        shape->shapeType = others::SmartShape::ShapeType::SlurAuto;
        shape->entryBased = true;
        shape->integrityCheck(shape);
        endpoint(shape->startTermSeg,
                 frame->getEntries().front()->getEntry()->getEntryNumber());
        endpoint(shape->endTermSeg,
                 frame->getEntries().back()->getEntry()->getEntryNumber());
    }
    void endpoint(const std::shared_ptr<others::SmartShape::TerminationSeg>& seg,
                  EntryNumber number)
    {
        seg->endPoint->staffId = staffId;
        seg->endPoint->measId = measureId;
        seg->endPoint->entryNumber = number;
    }
};

TEST_F(SlurTest, DefinitionIsIndependentOfEndpoints)
{
    shape->startTermSeg.reset();
    shape->endTermSeg.reset();
    shape->direction = ShapeDirection::Under;
    EXPECT_EQ(shape->calcDefinitionContourDirection(), Contour::Down);
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).reason,
              Slur::Reason::Definition);
    shape->shapeType = others::SmartShape::ShapeType::SlurUp;
    EXPECT_EQ(shape->calcDefinitionContourDirection(), Contour::Up);
    shape->shapeType = others::SmartShape::ShapeType::SlurAuto;
    shape->direction = ShapeDirection::Automatic;
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).resolution,
              Slur::Resolution::MissingEndpoint);
    shape->entryBased = false;
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).resolution,
              Slur::Resolution::AutomaticBeatAttached);
}

TEST_F(SlurTest, UniformStaffDirectionIsNotSlurForcing)
{
    const auto result = Slur::calcEffectiveDirection(*shape);
    EXPECT_EQ(result.direction, Contour::Down);
    EXPECT_EQ(result.reason, Slur::Reason::UniformStems);
}

TEST_F(SlurTest, LaterFreezeOverridesSpanDirection)
{
    auto last =
        std::const_pointer_cast<Entry>(frame->getEntries().back()->getEntry());
    last->freezeStemScore = true;
    last->upStemScore = true;
    const auto result = Slur::calcEffectiveDirection(*shape);
    EXPECT_EQ(result.direction, Contour::Up);
    EXPECT_EQ(result.reason, Slur::Reason::ContextualOverride);
    ASSERT_TRUE(result.sourceEntry);
    EXPECT_EQ(result.sourceEntry->getEntry()->getEntryNumber(),
              last->getEntryNumber());
}

TEST_F(SlurTest, TablaturePrecedesFrozenEntryAndNeedsNoGeometry)
{
    staff->notationStyle = others::Staff::NotationStyle::Tablature;
    staff->hideStems = true;
    auto first =
        std::const_pointer_cast<Entry>(frame->getEntries().front()->getEntry());
    first->freezeStemScore = true;
    first->upStemScore = false;
    Slur::Options options;
    options.geometryResolver =
        [](const EntryInfoPtr&) -> musx::util::Stem::GeometryResult {
        ADD_FAILURE() << "Tablature contour must not request geometry";
        return {};
    };
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape, options).direction,
              Contour::Up);
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape, options).reason,
              Slur::Reason::Tablature);
    staff->hideStems = false;
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape, options).direction,
              Contour::Down);
}

TEST_F(SlurTest, ExplicitSettingsAndDefaultCalculationDiffer)
{
    shape->direction = ShapeDirection::Over;
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).direction, Contour::Up);
    EXPECT_EQ(Slur::calcDefaultDirection(*shape).direction, Contour::Down);
}

TEST_F(SlurTest, MissingEntryNumberDoesNotUseBeatAssociation)
{
    shape->startTermSeg->endPoint->entryNumber = 0;
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).resolution,
              Slur::Resolution::MissingEndpoint);
}

TEST_F(SlurTest, SameEntryAndBackwardSpan)
{
    std::swap(shape->startTermSeg->endPoint->entryNumber,
              shape->endTermSeg->endPoint->entryNumber);
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).resolution,
              Slur::Resolution::InvalidSpan);
    shape->endTermSeg->endPoint->entryNumber =
        shape->startTermSeg->endPoint->entryNumber;
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).direction, Contour::Down);
}

TEST_F(SlurTest, AllSlurStylesUseTheSameRules)
{
    using Type = others::SmartShape::ShapeType;
    for (const auto type :
         {Type::SlurAuto, Type::DashSlurAuto, Type::DashContourSlurAuto}) {
        shape->shapeType = type;
        EXPECT_EQ(shape->calcDefinitionContourDirection(), Contour::Unspecified);
        EXPECT_EQ(Slur::calcEffectiveDirection(*shape).direction, Contour::Down);
    }
    for (const auto type :
         {Type::SlurUp, Type::DashSlurUp, Type::DashContourSlurUp}) {
        shape->shapeType = type;
        EXPECT_EQ(Slur::calcEffectiveDirection(*shape).direction, Contour::Up);
    }
    for (const auto type :
         {Type::SlurDown, Type::DashSlurDown, Type::DashContourSlurDown}) {
        shape->shapeType = type;
        EXPECT_EQ(Slur::calcEffectiveDirection(*shape).direction, Contour::Down);
    }
}

TEST_F(SlurTest, FirstFreezeWinsAndHiddenFreezeIsSkipped)
{
    auto first =
        std::const_pointer_cast<Entry>(frame->getEntries().front()->getEntry());
    auto last =
        std::const_pointer_cast<Entry>(frame->getEntries().back()->getEntry());
    first->freezeStemScore = last->freezeStemScore = true;
    first->upStemScore = false;
    last->upStemScore = true;
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).direction, Contour::Down);
    first->isHidden = true;
    const auto result = Slur::calcEffectiveDirection(*shape);
    EXPECT_EQ(result.direction, Contour::Up);
    EXPECT_TRUE(result.sourceEntry.isSameEntry(
        EntryInfoPtr(frame, frame->getEntries().size() - size_t{1})));
}

TEST_F(SlurTest, GraceFreezeDoesNotForceSlur)
{
    auto first =
        std::const_pointer_cast<Entry>(frame->getEntries().front()->getEntry());
    first->graceNote = true;
    first->freezeStemScore = true;
    first->upStemScore = true;
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).direction, Contour::Down);
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape).reason,
              Slur::Reason::UniformStems);
}

TEST_F(SlurTest, LayerFreezeForcesContour)
{
    auto layer = std::const_pointer_cast<others::LayerAttributes>(
        document->getOthers()->get<others::LayerAttributes>(SCORE_PARTID, 0));
    layer->freezeLayer = true;
    layer->freezeStemsUp = true;
    layer->onlyIfOtherLayersHaveNotes = false;
    const auto result = Slur::calcEffectiveDirection(*shape);
    EXPECT_EQ(result.direction, Contour::Up);
    EXPECT_EQ(result.reason, Slur::Reason::ContextualOverride);
}

TEST_F(SlurTest, MissingGeometryIsDistinctFromAutomatic)
{
    staff->stemDirection = StemDirection::Default;
    Slur::Options options;
    options.geometryResolver = [](const EntryInfoPtr&) {
        musx::util::Stem::GeometryResult result;
        result.decision =
            musx::util::StaffOriginOffsetResolverDecision::Unavailable;
        return result;
    };
    const auto result = Slur::calcEffectiveDirection(*shape, options);
    EXPECT_EQ(result.direction, Contour::Unspecified);
    EXPECT_EQ(result.resolution, Slur::Resolution::MissingGeometry);
    EXPECT_EQ(result.geometrySource, Slur::GeometrySource::Caller);
    shape->direction = ShapeDirection::Under;
    options.geometryResolver =
        [](const EntryInfoPtr&) -> musx::util::Stem::GeometryResult {
        ADD_FAILURE() << "Explicit contour must not request geometry";
        return {};
    };
    EXPECT_EQ(Slur::calcEffectiveDirection(*shape, options).direction,
              Contour::Down);
}

TEST_F(SlurTest, SplitStemsContributeBothDirections)
{
    auto last =
        std::const_pointer_cast<Entry>(frame->getEntries().back()->getEntry());
    last->doubleStem = true;
    const auto result = Slur::calcEffectiveDirection(*shape);
    EXPECT_EQ(result.direction, Contour::Up);
    EXPECT_EQ(result.reason, Slur::Reason::MixedStems);
}

TEST_F(SlurTest, GraceToFirstMainEntryHasPriorityOverItsFreeze)
{
    const auto firstInfo = frame->getEntries().front();
    const auto nextInfo = frame->getEntries().at(size_t{1});
    auto first = std::const_pointer_cast<Entry>(firstInfo->getEntry());
    auto next = std::const_pointer_cast<Entry>(nextInfo->getEntry());
    first->graceNote = true;
    first->upStemScore = true;
    first->freezeStemScore = true;
    next->freezeStemScore = true;
    next->upStemScore = false;
    next->notes.resize(size_t{1});
    endpoint(shape->endTermSeg, next->getEntryNumber());
    const auto result = Slur::calcEffectiveDirection(*shape);
    EXPECT_EQ(result.direction, Contour::Down);
    EXPECT_EQ(result.reason, Slur::Reason::GraceException);
}

TEST_F(SlurTest, CrossStaffSpanUsesOnlyEndpoints)
{
    constexpr Cmper otherStaffId = 3;
    constexpr MeasCmper otherMeasureId = 3;
    auto otherStaff = std::const_pointer_cast<others::Staff>(
        document->getOthers()->get<others::Staff>(SCORE_PARTID, otherStaffId));
    otherStaff->stemDirection = StemDirection::AlwaysUp;
    const details::GFrameHoldContext otherContext(document, SCORE_PARTID,
                                                  otherStaffId, otherMeasureId);
    const EntryInfoPtr other(otherContext.createEntryFrame(0), 0);
    for (const auto& info : other.getFrame()->getEntries()) {
        auto raw = std::const_pointer_cast<Entry>(info->getEntry());
        raw->freezeStemScore = false;
        raw->voice2 = raw->v2Launch = raw->graceNote = false;
        raw->splitStem = raw->doubleStem = false;
    }
    auto otherRaw = std::const_pointer_cast<Entry>(other->getEntry());
    otherRaw->freezeStemScore = false;
    otherRaw->voice2 = otherRaw->v2Launch = otherRaw->graceNote = false;
    endpoint(shape->endTermSeg, otherRaw->getEntryNumber());
    shape->endTermSeg->endPoint->staffId = otherStaffId;
    shape->endTermSeg->endPoint->measId = otherMeasureId;
    auto interior =
        std::const_pointer_cast<Entry>(frame->getEntries().back()->getEntry());
    interior->freezeStemScore = true;
    interior->upStemScore = true;
    const auto result = Slur::calcEffectiveDirection(*shape);
    const auto firstStem = musx::util::Stem::calcEffectiveDirection(EntryInfoPtr(frame, 0));
    const auto lastStem = musx::util::Stem::calcEffectiveDirection(other);
    ASSERT_TRUE(firstStem.primaryDirection);
    ASSERT_TRUE(lastStem.primaryDirection);
    EXPECT_NE(firstStem.primaryDirection, lastStem.primaryDirection);
    EXPECT_EQ(result.direction, Contour::Up);
    EXPECT_EQ(result.reason, Slur::Reason::MixedStems);
}

} // namespace
