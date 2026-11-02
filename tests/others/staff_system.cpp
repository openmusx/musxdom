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

constexpr static musxtest::string_view staffSystemXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <staffSystemSpec cmper="1">
      <startMeas>1</startMeas>
      <endMeas>4</endMeas>
      <horzPercent>12050</horzPercent>
      <ssysPercent>70</ssysPercent>
      <staffHeight>6144</staffHeight>
      <top>-12</top>
      <left>576</left>
      <right>-13</right>
      <bottom>-110</bottom>
      <hasStaffScaling/>
      <placeEndSpaceBeforeBarline/>
      <scaleVert/>
      <scaleContentOnly/>
      <distanceToPrev>-490</distanceToPrev>
      <extraStartSystemSpace>24</extraStartSystemSpace>
      <extraEndSystemSpace>36</extraEndSystemSpace>
    </staffSystemSpec>
  </others>
</finale>
)xml";

TEST(StaffSystemTest, PopulateFields)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(staffSystemXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto staffSystem = others->get<others::StaffSystem>(SCORE_PARTID, 1);
    ASSERT_TRUE(staffSystem) << "StaffSystem with cmper 1 not found";

    EXPECT_EQ(staffSystem->startMeas, 1);
    EXPECT_EQ(staffSystem->endMeas, 4);
    EXPECT_DOUBLE_EQ(staffSystem->horzPercent, 120.50); // Divided by 100.0 in XML mapping.
    EXPECT_EQ(staffSystem->ssysPercent, 70);
    EXPECT_EQ(staffSystem->staffHeight, 6144);
    EXPECT_EQ(staffSystem->top, -12);
    EXPECT_EQ(staffSystem->left, 576);
    EXPECT_EQ(staffSystem->right, -13);
    EXPECT_EQ(staffSystem->bottom, -110);
    EXPECT_TRUE(staffSystem->hasStaffScaling);
    EXPECT_TRUE(staffSystem->placeEndSpaceBeforeBarline);
    EXPECT_TRUE(staffSystem->scaleVert);
    EXPECT_TRUE(staffSystem->holdMargins); // Corresponds to `<scaleContentOnly>`.
    EXPECT_EQ(staffSystem->distanceToPrev, -490);
    EXPECT_EQ(staffSystem->extraStartSystemSpace, 24);
    EXPECT_EQ(staffSystem->extraEndSystemSpace, 36);
}

constexpr static musxtest::string_view systemStartIndexXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <measSpec cmper="1"/>
    <measSpec cmper="2"/>
    <staffSystemSpec cmper="1"><startMeas>2</startMeas></staffSystemSpec>
    <staffSystemSpec cmper="2"><startMeas>7</startMeas><endMeas>0</endMeas></staffSystemSpec>
    <staffSystemSpec cmper="3"><startMeas>4</startMeas><endMeas>2</endMeas></staffSystemSpec>
    <staffSystemSpec cmper="4"><startMeas>7</startMeas><endMeas>9</endMeas></staffSystemSpec>
    <staffSystemSpec cmper="5"/>
    <staffSystemSpec cmper="1" part="1" shared="false"><startMeas>1</startMeas></staffSystemSpec>
    <staffSystemSpec cmper="2" part="1" shared="false"><startMeas>4</startMeas></staffSystemSpec>
    <partDef cmper="0"/>
    <partDef cmper="1"/>
    <partDef cmper="2"/>
  </others>
</finale>
)xml";

TEST(StaffSystemTest, SystemStartIndexWithoutPageLayout)
{
    const auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(systemStartIndexXml);
    constexpr MeasCmper kBeforeFirstMeasureId = 1;
    constexpr MeasCmper kFirstMeasureId = 2;
    constexpr MeasCmper kBetweenStartsMeasureId = 3;
    constexpr MeasCmper kMiddleMeasureId = 4;
    constexpr MeasCmper kLastMeasureId = 7;
    constexpr MeasCmper kBeyondPieceMeasureId = 9;
    constexpr SystemCmper kFirstSystemId = 1;
    constexpr SystemCmper kMiddleSystemId = 3;
    constexpr SystemCmper kLastSystemId = 4;
    constexpr Cmper kUnknownPartId = 3;
    constexpr Cmper kEmptyPartId = 2;

    const auto score = doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, SCORE_PARTID);
    ASSERT_TRUE(score);
    EXPECT_FALSE(score->isLayoutCalculated());
    EXPECT_EQ(doc->calcSystemFromMeasure(SCORE_PARTID, 0), nullptr);
    EXPECT_EQ(doc->calcSystemFromMeasure(SCORE_PARTID, kBeforeFirstMeasureId), nullptr);
    EXPECT_EQ(doc->calcSystemFromMeasure(kUnknownPartId, kFirstMeasureId), nullptr);
    EXPECT_EQ(doc->calcSystemFromMeasure(kEmptyPartId, kFirstMeasureId), nullptr);
    EXPECT_EQ(score->calcSystemFromMeasure(0), nullptr);
    EXPECT_EQ(score->calcSystemFromMeasure(kBeforeFirstMeasureId), nullptr);
    const auto emptyPart = doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, kEmptyPartId);
    ASSERT_TRUE(emptyPart);
    EXPECT_EQ(emptyPart->calcSystemFromMeasure(kFirstMeasureId), nullptr);

    const std::pair<MeasCmper, SystemCmper> expected[] = {
        {kFirstMeasureId, kFirstSystemId},
        {kBetweenStartsMeasureId, kFirstSystemId},
        {kMiddleMeasureId, kMiddleSystemId},
        {kLastMeasureId, kLastSystemId},
        {kBeyondPieceMeasureId, kLastSystemId}
    };
    for (const auto& [measureId, systemId] : expected) {
        const auto system = doc->calcSystemFromMeasure(SCORE_PARTID, measureId);
        ASSERT_TRUE(system) << "measure " << measureId;
        EXPECT_EQ(system->getCmper(), systemId) << "measure " << measureId;
        EXPECT_EQ(score->calcSystemFromMeasure(measureId), system);
    }
}

TEST(StaffSystemTest, SystemStartIndexUsesRequestedPartWithSharedMeasures)
{
    const auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(systemStartIndexXml);
    constexpr Cmper kPartId = 1;
    constexpr MeasCmper kMeasureId = 1;
    constexpr MeasCmper kNextStartMeasureId = 4;
    constexpr SystemCmper kFirstSystemId = 1;
    constexpr SystemCmper kNextSystemId = 2;
    const auto measure = doc->getOthers()->get<others::Measure>(kPartId, kMeasureId);
    ASSERT_TRUE(measure);
    EXPECT_EQ(measure->getSourcePartId(), SCORE_PARTID);
    EXPECT_EQ(measure->getRequestedPartId(), kPartId);
    EXPECT_EQ(doc->calcSystemFromMeasure(SCORE_PARTID, measure->getCmper()), nullptr);
    const auto system = doc->calcSystemFromMeasure(measure->getRequestedPartId(), measure->getCmper());
    ASSERT_TRUE(system);
    EXPECT_EQ(system->getCmper(), kFirstSystemId);
    EXPECT_EQ(system->getRequestedPartId(), kPartId);
    const auto nextSystem = doc->calcSystemFromMeasure(kPartId, kNextStartMeasureId);
    ASSERT_TRUE(nextSystem);
    EXPECT_EQ(nextSystem->getCmper(), kNextSystemId);
    EXPECT_EQ(nextSystem->getRequestedPartId(), kPartId);
    const auto part = doc->getOthers()->get<others::PartDefinition>(SCORE_PARTID, kPartId);
    ASSERT_TRUE(part);
    EXPECT_EQ(part->getRequestedPartId(), SCORE_PARTID);
    EXPECT_EQ(part->calcSystemFromMeasure(kMeasureId), system);
    EXPECT_EQ(part->calcSystemFromMeasure(kNextStartMeasureId), nextSystem);
}

TEST(StaffSystemTest, SystemStartIndexDoesNotInheritScoreSystems)
{
    constexpr musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <staffSystemSpec cmper="1"><startMeas>1</startMeas></staffSystemSpec>
    <partDef cmper="0"/>
    <partDef cmper="1"/>
  </others>
</finale>
)xml";
    constexpr Cmper kPartId = 1;
    constexpr MeasCmper kMeasureId = 1;
    const auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    EXPECT_TRUE(doc->calcSystemFromMeasure(SCORE_PARTID, kMeasureId));
    EXPECT_EQ(doc->calcSystemFromMeasure(kPartId, kMeasureId), nullptr);
}
