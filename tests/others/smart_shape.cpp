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

#include <algorithm>
#include <typeindex>
#include <vector>

#include "gtest/gtest.h"
#include "musx/musx.h"
#include "test_utils.h"

 using namespace musx::dom;

TEST(SmartShape, Populate)
{
    constexpr static musxtest::string_view xml = R"xml(
    <?xml version="1.0" encoding="UTF-8"?>
    <finale>
        <others>
            <smartShape cmper="1">
                <shapeType>slurAuto</shapeType>
                <entryBased/>
                <rotate/>
                <yBreakType>opposite</yBreakType>
                <startTermSeg>
                    <endPt>
                        <inst>1</inst>
                        <meas>1</meas>
                        <entryNum>5</entryNum>
                    </endPt>
                    <endPtAdj>
                        <y>24</y>
                        <on/>
                        <contextDir>over</contextDir>
                        <contextEntCnct>headRightTop</contextEntCnct>
                    </endPtAdj>
                    <breakAdj>
                        <x>-124</x>
                        <y>12</y>
                        <on/>
                    </breakAdj>
                </startTermSeg>
                <endTermSeg>
                    <endPt>
                        <inst>1</inst>
                        <meas>1</meas>
                        <entryNum>6</entryNum>
                    </endPt>
                    <endPtAdj>
                        <x>-1</x>
                        <y>37</y>
                        <on/>
                        <contextDir>over</contextDir>
                        <contextEntCnct>headRightTop</contextEntCnct>
                    </endPtAdj>
                    <breakAdj>
                        <x>256</x>
                        <y>-72</y>
                        <on/>
                    </breakAdj>
                </endTermSeg>
                <dir>over</dir>
                <fullCtlPtAdj>
                    <startCtlPtX>573</startCtlPtX>
                    <startCtlPtY>35</startCtlPtY>
                    <endCtlPtX>-559</endCtlPtX>
                    <endCtlPtY>64</endCtlPtY>
                    <on/>
                    <contextDir>over</contextDir>
                </fullCtlPtAdj>
            </smartShape>
            <smartShape cmper="2">
                <shapeType>glissando</shapeType>
                <entryBased/>
                <yBreakType>opposite</yBreakType>
                <startTermSeg>
                    <endPt>
                        <inst>1</inst>
                        <meas>1</meas>
                        <entryNum>5</entryNum>
                    </endPt>
                    <endPtAdj>
                        <x>27</x>
                        <y>14</y>
                        <on/>
                        <contextEntCnct>noteRightCenter</contextEntCnct>
                    </endPtAdj>
                </startTermSeg>
                <endTermSeg>
                    <endPt>
                        <inst>1</inst>
                        <meas>1</meas>
                        <entryNum>6</entryNum>
                    </endPt>
                    <endPtAdj>
                        <x>-25</x>
                        <y>9</y>
                        <on/>
                        <contextEntCnct>noteLeftCenter</contextEntCnct>
                    </endPtAdj>
                </endTermSeg>
                <startNoteID>1</startNoteID>
                <endNoteID>2</endNoteID>
                <lineStyleID>3</lineStyleID>
            </smartShape>
            <smartShape cmper="3">
                <shapeType>cresc</shapeType>
                <startTermSeg>
                    <endPt>
                        <inst>1</inst>
                        <meas>1</meas>
                    </endPt>
                    <endPtAdj>
                        <x>24</x>
                        <y>-185</y>
                        <on/>
                    </endPtAdj>
                </startTermSeg>
                <endTermSeg>
                    <endPt>
                        <inst>1</inst>
                        <meas>1</meas>
                        <edu>2048</edu>
                    </endPt>
                    <endPtAdj>
                        <x>-22</x>
                        <y>-131</y>
                        <on/>
                    </endPtAdj>
                </endTermSeg>
                <fullCtlPtAdj>
                    <startCtlPtY>40</startCtlPtY>
                    <on/>
                </fullCtlPtAdj>
            </smartShape>
        </others>
    </finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    {
        auto smartShape = others->get<others::SmartShape>(SCORE_PARTID, 1);
        ASSERT_TRUE(smartShape) << "SmartShape with cmper 1 not found";

        EXPECT_EQ(smartShape->shapeType, others::SmartShape::ShapeType::SlurAuto);
        EXPECT_EQ(smartShape->direction, ShapeDirection::Over);
        EXPECT_TRUE(smartShape->entryBased);
        EXPECT_EQ(smartShape->startNoteId, 0);
        EXPECT_EQ(smartShape->endNoteId, 0);
        EXPECT_EQ(smartShape->lineStyleId, 0);

        ASSERT_TRUE(smartShape->startTermSeg);
        ASSERT_TRUE(smartShape->startTermSeg->endPoint);
        EXPECT_EQ(smartShape->startTermSeg->endPoint->staffId, 1);
        EXPECT_EQ(smartShape->startTermSeg->endPoint->measId, 1);
        EXPECT_EQ(smartShape->startTermSeg->endPoint->entryNumber, 5);

        ASSERT_TRUE(smartShape->startTermSeg->endPointAdj);
        EXPECT_EQ(smartShape->startTermSeg->endPointAdj->vertOffset, 24);
        EXPECT_TRUE(smartShape->startTermSeg->endPointAdj->active);

        ASSERT_TRUE(smartShape->endTermSeg);
        ASSERT_TRUE(smartShape->endTermSeg->endPoint);
        EXPECT_EQ(smartShape->endTermSeg->endPoint->staffId, 1);
        EXPECT_EQ(smartShape->endTermSeg->endPoint->measId, 1);
        EXPECT_EQ(smartShape->endTermSeg->endPoint->entryNumber, 6);

        ASSERT_TRUE(smartShape->endTermSeg->endPointAdj);
        EXPECT_EQ(smartShape->endTermSeg->endPointAdj->horzOffset, -1);
        EXPECT_EQ(smartShape->endTermSeg->endPointAdj->vertOffset, 37);
        EXPECT_TRUE(smartShape->endTermSeg->endPointAdj->active);

        ASSERT_TRUE(smartShape->startTermSeg->breakAdj);
        EXPECT_EQ(smartShape->startTermSeg->breakAdj->horzOffset, -124);
        EXPECT_EQ(smartShape->startTermSeg->breakAdj->vertOffset, 12);
        EXPECT_TRUE(smartShape->startTermSeg->breakAdj->active);

        ASSERT_TRUE(smartShape->endTermSeg->breakAdj);
        EXPECT_EQ(smartShape->endTermSeg->breakAdj->horzOffset, 256);
        EXPECT_EQ(smartShape->endTermSeg->breakAdj->vertOffset, -72);
        EXPECT_TRUE(smartShape->endTermSeg->breakAdj->active);
    }
    {
        auto smartShape = others->get<others::SmartShape>(SCORE_PARTID, 2);
        ASSERT_TRUE(smartShape) << "SmartShape with cmper 2 not found";

        EXPECT_EQ(smartShape->shapeType, others::SmartShape::ShapeType::Glissando);
        EXPECT_TRUE(smartShape->entryBased);
        EXPECT_EQ(smartShape->startNoteId, 1);
        EXPECT_EQ(smartShape->endNoteId, 2);
        EXPECT_EQ(smartShape->lineStyleId, 3);

        ASSERT_TRUE(smartShape->startTermSeg->breakAdj);
        EXPECT_FALSE(smartShape->startTermSeg->breakAdj->active);
        ASSERT_TRUE(smartShape->endTermSeg->breakAdj);
        EXPECT_FALSE(smartShape->endTermSeg->breakAdj->active);
    }
    {
        auto smartShape = others->get<others::SmartShape>(SCORE_PARTID, 3);
        ASSERT_TRUE(smartShape) << "SmartShape with cmper 3 not found";

        EXPECT_EQ(smartShape->shapeType, others::SmartShape::ShapeType::Crescendo);
        EXPECT_FALSE(smartShape->entryBased);
        EXPECT_EQ(smartShape->startTermSeg->endPoint->eduPosition, 0);
        EXPECT_EQ(smartShape->endTermSeg->endPoint->eduPosition, 2048);

        ASSERT_TRUE(smartShape->startTermSeg->breakAdj);
        ASSERT_TRUE(smartShape->endTermSeg->breakAdj);
    }
}

TEST(SmartShape, PartiallySharedTerminationSegments)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <frameSpec cmper="1" inci="0">
      <startEntry>1</startEntry>
      <endEntry>1</endEntry>
    </frameSpec>
    <measSpec cmper="1">
      <beats>4</beats>
      <divbeat>1024</divbeat>
      <hasSmartShape/>
    </measSpec>
    <smartShape cmper="3034">
      <shapeType>octaveUp</shapeType>
      <startTermSeg>
        <endPt>
          <inst>1</inst>
          <meas>1</meas>
        </endPt>
        <endPtAdj>
          <x>12</x>
          <on/>
        </endPtAdj>
      </startTermSeg>
      <endTermSeg>
        <endPt>
          <inst>1</inst>
          <meas>1</meas>
          <edu>2048</edu>
        </endPt>
      </endTermSeg>
      <hidden/>
    </smartShape>
    <smartShape cmper="3034" part="1" shared="true">
      <startTermSeg>
        <endPt>
          <edu>0</edu>
        </endPt>
      </startTermSeg>
      <endTermSeg>
        <endPt>
          <edu>2048</edu>
        </endPt>
      </endTermSeg>
      <hidden>
        <offInPart/>
      </hidden>
    </smartShape>
    <smartShapeMeasMark cmper="1" inci="0">
      <shapeNum>3034</shapeNum>
    </smartShapeMeasMark>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
    </staffSpec>
  </others>
  <details>
    <gfhold cmper1="1" cmper2="1">
      <clefID>0</clefID>
      <frame1>1</frame1>
    </gfhold>
  </details>
  <entries>
    <entry entnum="1" prev="0" next="0">
      <dura>1024</dura>
      <numNotes>0</numNotes>
      <isValid/>
      <floatRest/>
      <sorted/>
    </entry>
  </entries>
</finale>
)xml";

    auto document = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(document);

    const auto scoreShape = document->getOthers()->get<others::SmartShape>(SCORE_PARTID, 3034);
    const auto partShape = document->getOthers()->get<others::SmartShape>(1, 3034);
    ASSERT_TRUE(scoreShape);
    ASSERT_TRUE(partShape);

    EXPECT_EQ(partShape->getRequestedPartId(), 1);
    EXPECT_FALSE(partShape->hidden);

    ASSERT_TRUE(partShape->startTermSeg);
    ASSERT_TRUE(partShape->startTermSeg->endPoint);
    EXPECT_EQ(partShape->startTermSeg->endPoint->staffId, 1);
    EXPECT_EQ(partShape->startTermSeg->endPoint->measId, 1);
    EXPECT_EQ(partShape->startTermSeg->endPoint->eduPosition, 0);

    ASSERT_TRUE(partShape->endTermSeg);
    ASSERT_TRUE(partShape->endTermSeg->endPoint);
    EXPECT_EQ(partShape->endTermSeg->endPoint->staffId, 1);
    EXPECT_EQ(partShape->endTermSeg->endPoint->measId, 1);
    EXPECT_EQ(partShape->endTermSeg->endPoint->eduPosition, 2048);

    ASSERT_TRUE(partShape->startTermSeg->endPointAdj);
    EXPECT_EQ(partShape->startTermSeg->endPointAdj->horzOffset, 12);
    EXPECT_TRUE(partShape->startTermSeg->endPointAdj->active);

    EXPECT_NE(partShape->startTermSeg.get(), scoreShape->startTermSeg.get());
    EXPECT_NE(partShape->startTermSeg->endPoint.get(), scoreShape->startTermSeg->endPoint.get());
    EXPECT_NE(partShape->startTermSeg->endPointAdj.get(), scoreShape->startTermSeg->endPointAdj.get());
    EXPECT_EQ(partShape->startTermSeg->endPoint->getParent<others::SmartShape>()->getRequestedPartId(), 1);
    EXPECT_EQ(partShape->endTermSeg->endPoint->getParent<others::SmartShape>()->getRequestedPartId(), 1);

    EXPECT_TRUE(scoreShape->hidden);
    EXPECT_EQ(scoreShape->startTermSeg->endPoint->staffId, 1);
    EXPECT_EQ(scoreShape->startTermSeg->endPoint->measId, 1);
    EXPECT_EQ(scoreShape->endTermSeg->endPoint->staffId, 1);
    EXPECT_EQ(scoreShape->endTermSeg->endPoint->measId, 1);

    const auto sharedPartShape = document->getOthers()->get<others::SmartShape>(2, 3034);
    ASSERT_TRUE(sharedPartShape);
    EXPECT_EQ(sharedPartShape->getRequestedPartId(), 2);
    EXPECT_TRUE(sharedPartShape->hidden);
    EXPECT_NE(sharedPartShape->startTermSeg.get(), scoreShape->startTermSeg.get());
    EXPECT_NE(sharedPartShape->startTermSeg->endPoint.get(), scoreShape->startTermSeg->endPoint.get());
    EXPECT_NE(sharedPartShape->endTermSeg.get(), scoreShape->endTermSeg.get());
    EXPECT_NE(sharedPartShape->endTermSeg->endPoint.get(), scoreShape->endTermSeg->endPoint.get());
    EXPECT_EQ(sharedPartShape->startTermSeg->endPoint->getParent<others::SmartShape>()->getRequestedPartId(), 2);
    EXPECT_EQ(sharedPartShape->endTermSeg->endPoint->getParent<others::SmartShape>()->getRequestedPartId(), 2);

    const details::GFrameHoldContext partContext(document, 1, 1, 1);
    ASSERT_TRUE(partContext);
    const auto entryFrame = partContext.createEntryFrame(0);
    ASSERT_TRUE(entryFrame);
    ASSERT_EQ(entryFrame->getEntries().size(), 1);
    const EntryInfoPtr entryInfo(entryFrame, 0);
    EXPECT_TRUE(partShape->startTermSeg->endPoint->calcAssociatedEntry(true));
    EXPECT_TRUE(partShape->calcAppliesTo(entryInfo));

    const details::GFrameHoldContext sharedPartContext(document, 2, 1, 1);
    ASSERT_TRUE(sharedPartContext);
    const auto sharedPartEntryFrame = sharedPartContext.createEntryFrame(0);
    ASSERT_TRUE(sharedPartEntryFrame);
    const EntryInfoPtr sharedPartEntryInfo(sharedPartEntryFrame, 0);
    EXPECT_TRUE(sharedPartShape->startTermSeg->endPoint->calcAssociatedEntry(true));
    EXPECT_TRUE(sharedPartShape->calcAppliesTo(sharedPartEntryInfo));
}

TEST(SmartShapeCustomLine, Populate)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <ssLineStyle cmper="1">
            <lineStyle>char</lineStyle>
            <charParams>
                <lineChar>126</lineChar>
                <fontID>4</fontID>
                <fontSize>24</fontSize>
                <baselineShiftEms>-88</baselineShiftEms>
            </charParams>
            <lineCapStartType>arrowheadPreset</lineCapStartType>
            <lineCapEndType>arrowheadCustom</lineCapEndType>
            <lineCapStartArrowID>3</lineCapStartArrowID>
            <lineCapEndArrowID>6</lineCapEndArrowID>
            <makeHorz/>
            <lineAfterLeftStartText/>
            <lineBeforeRightEndText/>
            <lineAfterLeftContText/>
            <leftStartRawTextID>1</leftStartRawTextID>
            <leftContRawTextID>2</leftContRawTextID>
            <rightEndRawTextID>3</rightEndRawTextID>
            <centerFullRawTextID>4</centerFullRawTextID>
            <centerAbbrRawTextID>5</centerAbbrRawTextID>
            <leftStartX>7</leftStartX>
            <leftStartY>11</leftStartY>
            <leftContX>13</leftContX>
            <leftContY>17</leftContY>
            <rightEndX>19</rightEndX>
            <rightEndY>23</rightEndY>
            <centerFullX>29</centerFullX>
            <centerFullY>31</centerFullY>
            <centerAbbrX>37</centerAbbrX>
            <centerAbbrY>41</centerAbbrY>
            <lineStartX>1</lineStartX>
            <lineStartY>5</lineStartY>
            <lineEndX>3</lineEndX>
            <lineEndY>5</lineEndY>
            <lineContX>2</lineContX>
        </ssLineStyle>
        <ssLineStyle cmper="2">
            <lineStyle>solid</lineStyle>
            <solidParams>
                <lineWidth>115</lineWidth>
            </solidParams>
            <lineCapStartType>hook</lineCapStartType>
            <lineCapEndType>hook</lineCapEndType>
            <lineCapStartHookLength>192</lineCapStartHookLength>
            <lineCapEndHookLength>-320</lineCapEndHookLength>
        </ssLineStyle>
        <ssLineStyle cmper="3">
            <lineStyle>dashed</lineStyle>
            <dashedParams>
                <lineWidth>115</lineWidth>
                <dashOn>1216</dashOn>
                <dashOff>1728</dashOff>
            </dashedParams>
            <lineCapEndType>arrowheadPreset</lineCapEndType>
            <lineCapEndArrowID>1</lineCapEndArrowID>
        </ssLineStyle>
    </others>
</finale>
)xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    {
        auto line = others->get<others::SmartShapeCustomLine>(SCORE_PARTID, 1);
        ASSERT_TRUE(line);
        EXPECT_EQ(line->lineStyle, others::SmartShapeCustomLine::LineStyle::Char);
        ASSERT_TRUE(line->charParams);
        EXPECT_EQ(line->charParams->lineChar, 126);
        ASSERT_TRUE(line->charParams->font);
        EXPECT_EQ(line->charParams->font->fontId, 4);
        EXPECT_EQ(line->charParams->font->fontSize, 24);
        EXPECT_EQ(line->charParams->baselineShiftEms, -88);
        EXPECT_EQ(line->lineCapStartType, others::SmartShapeCustomLine::LineCapType::ArrowheadPreset);
        EXPECT_EQ(line->lineCapEndType, others::SmartShapeCustomLine::LineCapType::ArrowheadCustom);
        EXPECT_EQ(line->lineCapStartArrowId, 3);
        EXPECT_EQ(line->lineCapEndArrowId, 6);
        EXPECT_TRUE(line->makeHorz);
        EXPECT_TRUE(line->lineAfterLeftStartText);
        EXPECT_TRUE(line->lineBeforeRightEndText);
        EXPECT_TRUE(line->lineAfterLeftContText);
        EXPECT_EQ(line->leftStartRawTextId, 1);
        EXPECT_EQ(line->leftContRawTextId, 2);
        EXPECT_EQ(line->rightEndRawTextId, 3);
        EXPECT_EQ(line->centerFullRawTextId, 4);
        EXPECT_EQ(line->centerAbbrRawTextId, 5);
        EXPECT_EQ(line->leftStartX, 7);
        EXPECT_EQ(line->leftStartY, 11);
        EXPECT_EQ(line->leftContX, 13);
        EXPECT_EQ(line->leftContY, 17);
        EXPECT_EQ(line->rightEndX, 19);
        EXPECT_EQ(line->rightEndY, 23);
        EXPECT_EQ(line->centerFullX, 29);
        EXPECT_EQ(line->centerFullY, 31);
        EXPECT_EQ(line->centerAbbrX, 37);
        EXPECT_EQ(line->centerAbbrY, 41);
        EXPECT_EQ(line->lineStartX, 1);
        EXPECT_EQ(line->lineStartY, 5);
        EXPECT_EQ(line->lineEndX, 3);
        EXPECT_EQ(line->lineEndY, 5);
        EXPECT_EQ(line->lineContX, 2);
    }
    {
        auto line = others->get<others::SmartShapeCustomLine>(SCORE_PARTID, 2);
        ASSERT_TRUE(line);
        EXPECT_EQ(line->lineStyle, others::SmartShapeCustomLine::LineStyle::Solid);
        ASSERT_TRUE(line->solidParams);
        EXPECT_EQ(line->solidParams->lineWidth, 115);
        EXPECT_EQ(line->lineCapStartType, others::SmartShapeCustomLine::LineCapType::Hook);
        EXPECT_EQ(line->lineCapEndType, others::SmartShapeCustomLine::LineCapType::Hook);
        EXPECT_EQ(line->lineCapStartHookLength, 192);
        EXPECT_EQ(line->lineCapEndHookLength, -320);
    }
    {
        auto line = others->get<others::SmartShapeCustomLine>(SCORE_PARTID, 3);
        ASSERT_TRUE(line);
        EXPECT_EQ(line->lineStyle, others::SmartShapeCustomLine::LineStyle::Dashed);
        ASSERT_TRUE(line->dashedParams);
        EXPECT_EQ(line->dashedParams->lineWidth, 115);
        EXPECT_EQ(line->dashedParams->dashOn, 1216);
        EXPECT_EQ(line->dashedParams->dashOff, 1728);
        EXPECT_EQ(line->lineCapEndType, others::SmartShapeCustomLine::LineCapType::ArrowheadPreset);
        EXPECT_EQ(line->lineCapEndArrowId, 1);
    }
}

TEST(SmartShapeCustomLine, ImportCopiesDependenciesAndUsesShareModeAll)
{
    constexpr static musxtest::string_view sourceXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <fontName cmper="4">
            <charsetBank>Mac</charsetBank>
            <charsetVal>0</charsetVal>
            <pitch>0</pitch>
            <family>0</family>
            <name>Line Font</name>
        </fontName>
        <shapeDef cmper="6">
            <shapeType>other</shapeType>
        </shapeDef>
        <ssLineStyle cmper="2">
            <lineStyle>char</lineStyle>
            <charParams>
                <lineChar>126</lineChar>
                <fontID>4</fontID>
                <fontSize>24</fontSize>
                <efx>
                    <bold/>
                    <italic/>
                </efx>
                <baselineShiftEms>-88</baselineShiftEms>
            </charParams>
            <lineCapStartType>arrowheadCustom</lineCapStartType>
            <lineCapEndType>arrowheadCustom</lineCapEndType>
            <lineCapStartArrowID>6</lineCapStartArrowID>
            <lineCapEndArrowID>6</lineCapEndArrowID>
            <makeHorz/>
            <leftStartRawTextID>3</leftStartRawTextID>
            <leftContRawTextID>4</leftContRawTextID>
            <rightEndRawTextID>3</rightEndRawTextID>
            <leftStartX>7</leftStartX>
            <lineCapStartHookLength>192</lineCapStartHookLength>
        </ssLineStyle>
    </others>
    <texts>
        <smartShapeText number="3">Alpha</smartShapeText>
        <smartShapeText number="4">Beta</smartShapeText>
    </texts>
</finale>
)xml";
    constexpr static musxtest::string_view targetXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <fontName cmper="1">
            <charsetBank>Mac</charsetBank>
            <charsetVal>0</charsetVal>
            <pitch>0</pitch>
            <family>0</family>
            <name>Target Font</name>
        </fontName>
        <shapeDef cmper="7">
            <shapeType>other</shapeType>
        </shapeDef>
        <ssLineStyle cmper="4">
            <lineStyle>solid</lineStyle>
            <solidParams>
                <lineWidth>115</lineWidth>
            </solidParams>
        </ssLineStyle>
    </others>
    <texts>
        <smartShapeText number="9">Existing</smartShapeText>
    </texts>
</finale>
)xml";
    constexpr Cmper sourceLineId = 2;
    constexpr Cmper importedFontId = 2;
    constexpr Cmper importedLineId = 5;
    constexpr Cmper importedShapeId = 8;
    constexpr Cmper importedAlphaTextId = 10;
    constexpr Cmper importedBetaTextId = 11;

    auto source =
        musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(sourceXml);
    auto target =
        musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(targetXml);
    const auto sourceLine =
        source->getOthers()->get<others::SmartShapeCustomLine>(SCORE_PARTID, sourceLineId);
    ASSERT_TRUE(sourceLine);

    std::vector<std::type_index> importedTypes;
    const auto imported = others::importSmartShapeCustomLineInto(target, sourceLine,
        [&importedTypes](const EnigmaBase& object) {
            importedTypes.emplace_back(typeid(object));
        });
    ASSERT_EQ(imported, std::optional<Cmper>(importedLineId));
    EXPECT_EQ(importedTypes.size(), 5u);
    EXPECT_EQ(std::count(importedTypes.begin(), importedTypes.end(),
                  std::type_index(typeid(others::FontDefinition))), 1);
    EXPECT_EQ(std::count(importedTypes.begin(), importedTypes.end(),
                  std::type_index(typeid(others::ShapeDef))), 1);
    EXPECT_EQ(std::count(importedTypes.begin(), importedTypes.end(),
                  std::type_index(typeid(texts::SmartShapeText))), 2);
    EXPECT_EQ(std::count(importedTypes.begin(), importedTypes.end(),
                  std::type_index(typeid(others::SmartShapeCustomLine))), 1);
    const auto line =
        target->getOthers()->get<others::SmartShapeCustomLine>(SCORE_PARTID, *imported);
    ASSERT_TRUE(line);
    EXPECT_EQ(line->getSourcePartId(), SCORE_PARTID);
    EXPECT_EQ(line->getShareMode(), EnigmaBase::ShareMode::All);
    EXPECT_EQ(line->lineStyle, others::SmartShapeCustomLine::LineStyle::Char);
    ASSERT_TRUE(line->charParams);
    ASSERT_TRUE(line->charParams->font);
    EXPECT_EQ(line->charParams->lineChar, 126);
    EXPECT_EQ(line->charParams->font->fontId, importedFontId);
    EXPECT_EQ(line->charParams->font->getName(), "Line Font");
    EXPECT_EQ(line->charParams->font->fontSize, 24);
    EXPECT_TRUE(line->charParams->font->bold);
    EXPECT_TRUE(line->charParams->font->italic);
    EXPECT_EQ(line->charParams->baselineShiftEms, -88);
    EXPECT_EQ(line->leftStartX, 7);
    EXPECT_EQ(line->lineCapStartHookLength, 192);

    EXPECT_EQ(line->lineCapStartArrowId, importedShapeId);
    EXPECT_EQ(line->lineCapEndArrowId, importedShapeId);
    const auto shape =
        target->getOthers()->get<others::ShapeDef>(SCORE_PARTID, importedShapeId);
    ASSERT_TRUE(shape);
    EXPECT_EQ(shape->getSourcePartId(), SCORE_PARTID);
    EXPECT_EQ(shape->getShareMode(), EnigmaBase::ShareMode::All);

    const auto font =
        target->getOthers()->get<others::FontDefinition>(SCORE_PARTID, importedFontId);
    ASSERT_TRUE(font);
    EXPECT_EQ(font->getSourcePartId(), SCORE_PARTID);
    EXPECT_EQ(font->getShareMode(), EnigmaBase::ShareMode::All);

    EXPECT_EQ(line->leftStartRawTextId, importedAlphaTextId);
    EXPECT_EQ(line->rightEndRawTextId, importedAlphaTextId);
    EXPECT_EQ(line->leftContRawTextId, importedBetaTextId);
    const auto alpha =
        target->getTexts()->get<texts::SmartShapeText>(importedAlphaTextId);
    const auto beta =
        target->getTexts()->get<texts::SmartShapeText>(importedBetaTextId);
    ASSERT_TRUE(alpha);
    ASSERT_TRUE(beta);
    EXPECT_EQ(alpha->text, "Alpha");
    EXPECT_EQ(beta->text, "Beta");
    EXPECT_EQ(alpha->getShareMode(), EnigmaBase::ShareMode::All);
    EXPECT_EQ(beta->getShareMode(), EnigmaBase::ShareMode::All);
}

TEST(CenterShape, Populate)
{
    constexpr static musxtest::string_view xml = R"xml(
    <?xml version="1.0" encoding="UTF-8"?>
    <finale>
      <details>
        <centerShape cmper1="1" cmper2="7">
          <startBreakAdj>
            <x>37</x>
            <y>-199</y>
            <on/>
          </startBreakAdj>
          <endBreakAdj>
            <x>-68</x>
            <y>-199</y>
            <on/>
          </endBreakAdj>
          <ctlPtAdj>
            <startCtlPtY>136</startCtlPtY>
            <on/>
          </ctlPtAdj>
        </centerShape>
      </details>
    </finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    auto centerShape = details->get<details::CenterShape>(SCORE_PARTID, 1, 7);
    ASSERT_TRUE(centerShape) << "CenterShape with cmper1=1, cmper2=7 not found";

    ASSERT_TRUE(centerShape->startBreakAdj);
    EXPECT_EQ(centerShape->startBreakAdj->horzOffset, 37);
    EXPECT_EQ(centerShape->startBreakAdj->vertOffset, -199);
    EXPECT_TRUE(centerShape->startBreakAdj->active);

    ASSERT_TRUE(centerShape->endBreakAdj);
    EXPECT_EQ(centerShape->endBreakAdj->horzOffset, -68);
    EXPECT_EQ(centerShape->endBreakAdj->vertOffset, -199);
    EXPECT_TRUE(centerShape->endBreakAdj->active);
}

TEST(CenterShape, PartiallySharedAdjustments)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <details>
    <centerShape cmper1="1" cmper2="7">
      <startBreakAdj>
        <x>37</x>
        <y>-199</y>
        <on/>
      </startBreakAdj>
      <endBreakAdj>
        <x>-68</x>
        <y>-201</y>
        <on/>
      </endBreakAdj>
      <ctlPtAdj>
        <startCtlPtX>11</startCtlPtX>
        <startCtlPtY>136</startCtlPtY>
        <endCtlPtX>22</endCtlPtX>
        <endCtlPtY>44</endCtlPtY>
        <on/>
      </ctlPtAdj>
    </centerShape>
    <centerShape cmper1="1" cmper2="7" part="1" shared="true">
      <startBreakAdj>
        <x>50</x>
      </startBreakAdj>
      <ctlPtAdj>
        <startCtlPtY>200</startCtlPtY>
      </ctlPtAdj>
    </centerShape>
  </details>
</finale>
)xml";

    const auto document = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(document);

    const auto scoreShape = document->getDetails()->get<details::CenterShape>(SCORE_PARTID, 1, 7);
    const auto partShape = document->getDetails()->get<details::CenterShape>(1, 1, 7);
    ASSERT_TRUE(scoreShape);
    ASSERT_TRUE(partShape);

    EXPECT_EQ(partShape->getRequestedPartId(), 1);

    ASSERT_TRUE(partShape->startBreakAdj);
    EXPECT_EQ(partShape->startBreakAdj->horzOffset, 50);
    EXPECT_EQ(partShape->startBreakAdj->vertOffset, -199);
    EXPECT_TRUE(partShape->startBreakAdj->active);

    ASSERT_TRUE(partShape->endBreakAdj);
    EXPECT_EQ(partShape->endBreakAdj->horzOffset, -68);
    EXPECT_EQ(partShape->endBreakAdj->vertOffset, -201);
    EXPECT_TRUE(partShape->endBreakAdj->active);

    ASSERT_TRUE(partShape->ctlPtAdj);
    EXPECT_EQ(partShape->ctlPtAdj->startCtlPtX, 11);
    EXPECT_EQ(partShape->ctlPtAdj->startCtlPtY, 200);
    EXPECT_EQ(partShape->ctlPtAdj->endCtlPtX, 22);
    EXPECT_EQ(partShape->ctlPtAdj->endCtlPtY, 44);
    EXPECT_TRUE(partShape->ctlPtAdj->active);

    EXPECT_NE(partShape->startBreakAdj.get(), scoreShape->startBreakAdj.get());
    EXPECT_NE(partShape->endBreakAdj.get(), scoreShape->endBreakAdj.get());
    EXPECT_NE(partShape->ctlPtAdj.get(), scoreShape->ctlPtAdj.get());
    EXPECT_EQ(partShape->startBreakAdj->getParent<details::CenterShape>()->getRequestedPartId(), 1);
    EXPECT_EQ(partShape->endBreakAdj->getParent<details::CenterShape>()->getRequestedPartId(), 1);
    EXPECT_EQ(partShape->ctlPtAdj->getParent<details::CenterShape>()->getRequestedPartId(), 1);

    EXPECT_EQ(scoreShape->startBreakAdj->horzOffset, 37);
    EXPECT_EQ(scoreShape->ctlPtAdj->startCtlPtY, 136);

    const auto sharedPartShape = document->getDetails()->get<details::CenterShape>(2, 1, 7);
    ASSERT_TRUE(sharedPartShape);
    EXPECT_EQ(sharedPartShape->getRequestedPartId(), 2);
    EXPECT_NE(sharedPartShape->startBreakAdj.get(), scoreShape->startBreakAdj.get());
    EXPECT_NE(sharedPartShape->endBreakAdj.get(), scoreShape->endBreakAdj.get());
    EXPECT_NE(sharedPartShape->ctlPtAdj.get(), scoreShape->ctlPtAdj.get());
    EXPECT_EQ(sharedPartShape->startBreakAdj->getParent<details::CenterShape>()->getRequestedPartId(), 2);
    EXPECT_EQ(sharedPartShape->endBreakAdj->getParent<details::CenterShape>()->getRequestedPartId(), 2);
    EXPECT_EQ(sharedPartShape->ctlPtAdj->getParent<details::CenterShape>()->getRequestedPartId(), 2);
}

TEST(SmartShapes, IndependentTimeSigs)
{
    using Fraction = musx::util::Fraction;

    std::vector<char> enigmaXml;
    musxtest::readFile(musxtest::getInputPath() / "independent_timesig.enigmaxml", enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(enigmaXml);
    ASSERT_TRUE(doc);

    {
        auto ss = doc->getOthers()->get<others::SmartShape>(SCORE_PARTID, 1);
        ASSERT_TRUE(ss) << "failed to load SmartShape 1";
        EXPECT_EQ(ss->shapeType, others::SmartShape::ShapeType::SlurAuto);
        EXPECT_EQ(ss->startTermSeg->endPoint->calcPosition(), Fraction(1, 4));
        EXPECT_EQ(ss->startTermSeg->endPoint->calcGlobalPosition(), Fraction(1, 6));
        EXPECT_TRUE(ss->startTermSeg->endPoint->getMeasureAssignment());
        EXPECT_TRUE(ss->startTermSeg->endPoint->getEntryAssignment());
        EXPECT_EQ(ss->endTermSeg->endPoint->calcPosition(), Fraction(5, 8));
        EXPECT_EQ(ss->endTermSeg->endPoint->calcGlobalPosition(), Fraction(5, 12));
        EXPECT_TRUE(ss->endTermSeg->endPoint->getMeasureAssignment());
        EXPECT_TRUE(ss->endTermSeg->endPoint->getEntryAssignment());
    }

    {
        auto ss = doc->getOthers()->get<others::SmartShape>(SCORE_PARTID, 4);
        ASSERT_TRUE(ss) << "failed to load SmartShape 4";
        EXPECT_EQ(ss->shapeType, others::SmartShape::ShapeType::Crescendo);
        EXPECT_EQ(ss->startTermSeg->endPoint->calcPosition(), Fraction(1, 4));
        EXPECT_EQ(ss->startTermSeg->endPoint->calcGlobalPosition(), Fraction(1, 6));
        EXPECT_TRUE(ss->startTermSeg->endPoint->getMeasureAssignment());
        EXPECT_FALSE(ss->startTermSeg->endPoint->getEntryAssignment());
        EXPECT_EQ(ss->endTermSeg->endPoint->calcPosition(), Fraction(5, 8));
        EXPECT_EQ(ss->endTermSeg->endPoint->calcGlobalPosition(), Fraction(5, 12));
        EXPECT_TRUE(ss->endTermSeg->endPoint->getMeasureAssignment());
        EXPECT_FALSE(ss->endTermSeg->endPoint->getEntryAssignment());
    }
}

TEST(SmartShapes, EntriesInShapeTest)
{
    std::vector<char> enigmaXml;
    musxtest::readFile(musxtest::getInputPath() / "slur.enigmaxml", enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(enigmaXml);
    ASSERT_TRUE(doc);

    auto slur = doc->getOthers()->get<others::SmartShape>(SCORE_PARTID, 1);
    ASSERT_TRUE(slur);
    EXPECT_EQ(slur->shapeType, others::SmartShape::ShapeType::SlurAuto);

    auto scrollView = doc->getScrollViewStaves(SCORE_PARTID);
    EXPECT_GE(scrollView.size(), 2);

    {
        std::vector<EntryNumber> expectedEntries = { 5, 6, 7 };
        size_t x = 0;
        slur->iterateEntries([&](const EntryInfoPtr& entryInfo) {
            EXPECT_LT(x, expectedEntries.size());
            if (x >= expectedEntries.size()) return false;
            EXPECT_EQ(entryInfo->getEntry()->getEntryNumber(), expectedEntries[x]);
            x++;
            return true;
        });
    }
    {
        std::vector<EntryNumber> expectedEntries = { 5, 6, 7, 9, 10 };
        size_t x = 0;
        scrollView.iterateEntries(0, 1, slur->createGlobalMusicRange(), [&](const EntryInfoPtr& entryInfo) {
            EXPECT_LT(x, expectedEntries.size());
            if (x >= expectedEntries.size()) return false;
            EXPECT_EQ(entryInfo->getEntry()->getEntryNumber(), expectedEntries[x]);
            x++;
            return true;
        });
    }
    {
        std::vector<EntryNumber> expectedEntries = { 9, 10, 5, 6, 7 };
        size_t x = 0;
        scrollView.iterateEntries(1, 0, slur->createGlobalMusicRange(), [&](const EntryInfoPtr& entryInfo) {
            EXPECT_LT(x, expectedEntries.size());
            if (x >= expectedEntries.size()) return false;
            EXPECT_EQ(entryInfo->getEntry()->getEntryNumber(), expectedEntries[x]);
            x++;
            return true;
        });
    }
}

TEST(SmartShapes, EndpointNotes)
{
    std::vector<char> enigmaXml;
    musxtest::readFile(musxtest::getInputPath() / "trill-to.enigmaxml", enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(enigmaXml);
    ASSERT_TRUE(doc);

    auto shape = doc->getOthers()->get<others::SmartShape>(SCORE_PARTID, 1);
    ASSERT_TRUE(shape);
    EXPECT_EQ(shape->shapeType, others::SmartShape::ShapeType::TabSlide);
    EXPECT_TRUE(shape->entryBased);
    EXPECT_EQ(shape->startNoteId, 1);
    EXPECT_EQ(shape->endNoteId, 1);

    const auto startNote = shape->calcStartNote();
    ASSERT_TRUE(startNote);
    EXPECT_EQ(startNote.getEntryInfo()->getEntry()->getEntryNumber(), shape->startTermSeg->endPoint->entryNumber);
    EXPECT_EQ(startNote->getNoteId(), shape->startNoteId);

    const auto endNote = shape->calcEndNote();
    ASSERT_TRUE(endNote);
    EXPECT_EQ(endNote.getEntryInfo()->getEntry()->getEntryNumber(), shape->endTermSeg->endPoint->entryNumber);
    EXPECT_EQ(endNote->getNoteId(), shape->endNoteId);

    // The shape spans two entries, so the two ends are different notes.
    EXPECT_NE(startNote.getEntryInfo()->getEntry()->getEntryNumber(),
        endNote.getEntryInfo()->getEntry()->getEntryNumber());
}

TEST(SmartShapes, EndpointNotesRequireEntryAttachment)
{
    std::vector<char> enigmaXml;
    musxtest::readFile(musxtest::getInputPath() / "independent_timesig.enigmaxml", enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(enigmaXml);
    ASSERT_TRUE(doc);

    // A beat-attached shape has no note association, even where calcAssociatedEntry would find a
    // nearby entry.
    auto shape = doc->getOthers()->get<others::SmartShape>(SCORE_PARTID, 4);
    ASSERT_TRUE(shape);
    EXPECT_FALSE(shape->entryBased);
    EXPECT_EQ(shape->startTermSeg->endPoint->entryNumber, 0);
    EXPECT_EQ(shape->endTermSeg->endPoint->entryNumber, 0);

    EXPECT_FALSE(shape->calcStartNote());
    EXPECT_FALSE(shape->calcEndNote());
}

TEST(SmartShapes, LyricShapes)
{
    constexpr static musxtest::string_view xml = R"xml(
    <?xml version="1.0" encoding="UTF-8"?>
    <finale>
        <others>
            <smartShape cmper="1">
                <shapeType>hyphen</shapeType>
            </smartShape>
            <smartShape cmper="2">
                <shapeType>wordExt</shapeType>
            </smartShape>
            <smartShape cmper="3">
                <shapeType>slurAuto</shapeType>
            </smartShape>
            <smartShape cmper="4">
                <shapeType>smartLine</shapeType>
            </smartShape>
        </others>
    </finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    const auto isLyricShape = [&](Cmper shapeId) {
        auto shape = others->get<others::SmartShape>(SCORE_PARTID, shapeId);
        EXPECT_TRUE(shape) << "SmartShape " << shapeId << " not found";
        return shape && shape->calcIsLyricShape();
    };
    EXPECT_TRUE(isLyricShape(1));
    EXPECT_TRUE(isLyricShape(2));
    EXPECT_FALSE(isLyricShape(3));
    EXPECT_FALSE(isLyricShape(4));
}

TEST(SmartShapes, VerticalPlacementForBeatAttached)
{
    constexpr static musxtest::string_view xml = R"xml(
    <?xml version="1.0" encoding="UTF-8"?>
    <finale>
        <others>
            <measSpec cmper="1">
                <beats>4</beats>
                <divbeat>1024</divbeat>
            </measSpec>
            <staffSpec cmper="1">
                <staffLines>5</staffLines>
                <lineSpace>24</lineSpace>
            </staffSpec>
            <smartShape cmper="1">
                <shapeType>solidLine</shapeType>
                <startTermSeg>
                    <endPt><inst>1</inst><meas>1</meas></endPt>
                    <endPtAdj><y>24</y><on/></endPtAdj>
                </startTermSeg>
                <endTermSeg>
                    <endPt><inst>1</inst><meas>1</meas><edu>1024</edu></endPt>
                </endTermSeg>
            </smartShape>
            <smartShape cmper="2">
                <shapeType>solidLine</shapeType>
                <startTermSeg>
                    <endPt><inst>1</inst><meas>1</meas></endPt>
                    <endPtAdj><y>-120</y><on/></endPtAdj>
                </startTermSeg>
                <endTermSeg>
                    <endPt><inst>1</inst><meas>1</meas><edu>1024</edu></endPt>
                    <endPtAdj><y>-200</y><on/></endPtAdj>
                </endTermSeg>
            </smartShape>
            <smartShape cmper="3">
                <shapeType>solidLine</shapeType>
                <startTermSeg>
                    <endPt><inst>1</inst><meas>1</meas></endPt>
                    <endPtAdj><y>-48</y><on/></endPtAdj>
                </startTermSeg>
                <endTermSeg>
                    <endPt><inst>1</inst><meas>1</meas><edu>1024</edu></endPt>
                    <endPtAdj><y>-48</y><on/></endPtAdj>
                </endTermSeg>
            </smartShape>
            <smartShape cmper="4">
                <shapeType>solidLine</shapeType>
                <startTermSeg>
                    <endPt><inst>1</inst><meas>1</meas></endPt>
                    <endPtAdj><y>-200</y><on/></endPtAdj>
                </startTermSeg>
                <endTermSeg>
                    <endPt><inst>1</inst><meas>1</meas><edu>1024</edu></endPt>
                </endTermSeg>
            </smartShape>
            <smartShape cmper="5">
                <shapeType>slurAuto</shapeType>
                <entryBased/>
                <startTermSeg>
                    <endPt><inst>1</inst><meas>1</meas><entryNum>5</entryNum></endPt>
                </startTermSeg>
                <endTermSeg>
                    <endPt><inst>1</inst><meas>1</meas><entryNum>6</entryNum></endPt>
                </endTermSeg>
            </smartShape>
        </others>
    </finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);

    auto placementFor = [&](Cmper shapeCmper) {
        auto shape = doc->getOthers()->get<others::SmartShape>(SCORE_PARTID, shapeCmper);
        EXPECT_TRUE(shape) << "smart shape " << shapeCmper;
        return shape->calcVerticalPlacementForBeatAttached();
    };

    // Both endpoints at or above the top staff line.
    EXPECT_EQ(placementFor(1), VerticalPlacement::Above);
    // Both endpoints below the bottom staff line (bottom line is at -96 for a 5-line staff).
    EXPECT_EQ(placementFor(2), VerticalPlacement::Below);
    // Both endpoints in the middle of the staff: neither above nor below.
    EXPECT_EQ(placementFor(3), VerticalPlacement::Float);
    // One endpoint below the bottom line, the other at the top line.
    EXPECT_EQ(placementFor(4), VerticalPlacement::Float);
    // Entry-based shapes have no beat-attached placement.
    EXPECT_EQ(placementFor(5), VerticalPlacement::NotApplicable);
}
