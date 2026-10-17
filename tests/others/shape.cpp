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
#include <filesystem>
#include <typeindex>
#include <vector>

#include "gtest/gtest.h"
#include "musx/musx.h"
#include "test_utils.h"

using namespace musx::dom;

constexpr static musxtest::string_view shapeXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <shapeData cmper="5">
            <data>0</data>
            <data>2</data>
            <data>0</data>
            <data>2</data>
            <data>28</data>
            <data>2</data>
            <data>1000</data>
            <data>1000</data>
            <data>0</data>
            <data>0</data>
            <data>0</data>
            <data>0</data>
            <data>0</data>
            <data>256</data>
            <data>18</data>
            <data>0</data>
            <data>28</data>
            <data>0</data>
        </shapeData>
        <shapeDef cmper="6">
            <instList>7</instList>
            <dataList>5</dataList>
            <shapeType>articulation</shapeType>
        </shapeDef>
        <shapeList cmper="7">
            <instruct>
                <numData>11</numData>
                <tag>startObject</tag>
            </instruct>
            <instruct>
                <numData>2</numData>
                <tag>rMoveTo</tag>
            </instruct>
            <instruct>
                <numData>1</numData>
                <tag>lineWidth</tag>
            </instruct>
            <instruct>
                <numData>2</numData>
                <tag>setDash</tag>
            </instruct>
            <instruct>
                <numData>2</numData>
                <tag>rLineTo</tag>
            </instruct>
            <instruct>
                <tag>stroke</tag>
            </instruct>
        </shapeList>
    </others>
</finale>
    )xml";

constexpr static musxtest::string_view insufficientShapeDataXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <shapeData cmper="5">
            <data>1</data>
        </shapeData>
        <shapeDef cmper="6">
            <instList>7</instList>
            <dataList>5</dataList>
        </shapeDef>
        <shapeList cmper="7">
            <instruct>
                <numData>1</numData>
                <tag>lineWidth</tag>
            </instruct>
            <instruct>
                <numData>1</numData>
                <tag>lineWidth</tag>
            </instruct>
        </shapeList>
    </others>
</finale>
    )xml";

constexpr static musxtest::string_view verticalLineRightHooksXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <shapeData cmper="19">
            <data>20</data><data>3</data><data>902</data><data>-8</data><data>928</data><data>-122</data>
            <data>1000</data><data>390</data><data>0</data><data>24</data><data>288</data>
            <data>8</data><data>3</data><data>128</data><data>-24</data><data>0</data>
            <data>256</data><data>0</data><data>-288</data><data>128</data><data>24</data><data>0</data>
            <data>20</data><data>3</data><data>902</data><data>-8</data><data>928</data><data>-122</data>
            <data>1000</data><data>390</data><data>0</data><data>24</data><data>288</data>
            <data>8</data><data>3</data><data>128</data><data>-24</data><data>0</data>
            <data>256</data><data>0</data><data>-288</data><data>128</data><data>24</data><data>0</data>
        </shapeData>
        <shapeData cmper="21">
            <data>24</data><data>0</data><data>-2</data><data>2</data><data>24</data><data>-107</data>
            <data>1000</data><data>1000</data><data>0</data><data>0</data><data>0</data>
            <data>0</data><data>0</data><data>256</data><data>18</data><data>0</data>
            <data>0</data><data>0</data><data>-24</data><data>0</data><data>0</data><data>-105</data>
            <data>24</data><data>1</data>
        </shapeData>
        <shapeData cmper="23">
            <data>20</data><data>10</data><data>3321</data><data>-9</data><data>3347</data><data>-374</data>
            <data>1000</data><data>1262</data><data>0</data><data>24</data><data>288</data>
            <data>8</data><data>3</data><data>128</data><data>-24</data><data>0</data>
            <data>256</data><data>0</data><data>-288</data><data>128</data><data>24</data><data>0</data>
            <data>20</data><data>10</data><data>3321</data><data>-9</data><data>3347</data><data>-374</data>
            <data>1000</data><data>1262</data><data>0</data><data>24</data><data>288</data>
            <data>8</data><data>3</data><data>128</data><data>-24</data><data>0</data>
            <data>256</data><data>0</data><data>-288</data><data>128</data><data>24</data><data>0</data>
        </shapeData>
        <shapeData cmper="43">
            <data>20</data><data>21</data><data>3918</data><data>39</data><data>3944</data><data>-769</data>
            <data>1000</data><data>2799</data><data>0</data><data>24</data><data>288</data>
            <data>8</data><data>3</data><data>128</data><data>-24</data><data>0</data>
            <data>256</data><data>0</data><data>-288</data><data>128</data><data>24</data><data>0</data>
            <data>20</data><data>21</data><data>3918</data><data>39</data><data>3944</data><data>-769</data>
            <data>1000</data><data>2799</data><data>0</data><data>24</data><data>288</data>
            <data>8</data><data>3</data><data>128</data><data>-24</data><data>0</data>
            <data>256</data><data>0</data><data>-288</data><data>128</data><data>24</data><data>0</data>
        </shapeData>
        <shapeDef cmper="127"><instList>19</instList><dataList>19</dataList><shapeType>expression</shapeType></shapeDef>
        <shapeDef cmper="128"><instList>21</instList><dataList>21</dataList><shapeType>articulation</shapeType></shapeDef>
        <shapeDef cmper="129"><instList>23</instList><dataList>23</dataList><shapeType>expression</shapeType></shapeDef>
        <shapeDef cmper="130"><instList>43</instList><dataList>43</dataList><shapeType>expression</shapeType></shapeDef>
        <shapeList cmper="19">
            <instruct><numData>11</numData><tag>startObject</tag></instruct><instruct><numData>1</numData><tag>bracket</tag></instruct>
            <instruct><numData>1</numData><tag>vertMode</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><tag>stroke</tag></instruct>
            <instruct><numData>11</numData><tag>startObject</tag></instruct><instruct><numData>1</numData><tag>bracket</tag></instruct>
            <instruct><numData>1</numData><tag>vertMode</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><tag>stroke</tag></instruct>
        </shapeList>
        <shapeList cmper="21">
            <instruct><numData>11</numData><tag>startObject</tag></instruct><instruct><numData>2</numData><tag>rMoveTo</tag></instruct>
            <instruct><numData>1</numData><tag>lineWidth</tag></instruct><instruct><numData>2</numData><tag>setDash</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>2</numData><tag>rLineTo</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>2</numData><tag>rLineTo</tag></instruct>
            <instruct><tag>stroke</tag></instruct>
        </shapeList>
        <shapeList cmper="23">
            <instruct><numData>11</numData><tag>startObject</tag></instruct><instruct><numData>1</numData><tag>bracket</tag></instruct>
            <instruct><numData>1</numData><tag>vertMode</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><tag>stroke</tag></instruct>
            <instruct><numData>11</numData><tag>startObject</tag></instruct><instruct><numData>1</numData><tag>bracket</tag></instruct>
            <instruct><numData>1</numData><tag>vertMode</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><tag>stroke</tag></instruct>
        </shapeList>
        <shapeList cmper="43">
            <instruct><numData>11</numData><tag>startObject</tag></instruct><instruct><numData>1</numData><tag>bracket</tag></instruct>
            <instruct><numData>1</numData><tag>vertMode</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><tag>stroke</tag></instruct>
            <instruct><numData>11</numData><tag>startObject</tag></instruct><instruct><numData>1</numData><tag>bracket</tag></instruct>
            <instruct><numData>1</numData><tag>vertMode</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><numData>1</numData><tag>lineWidth</tag></instruct>
            <instruct><numData>2</numData><tag>rLineTo</tag></instruct><instruct><tag>stroke</tag></instruct>
        </shapeList>
    </others>
</finale>
)xml";

TEST(ShapeDefTest, PopulateFields)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(shapeXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto shapeDef = others->get<others::ShapeDef>(SCORE_PARTID, 6);
    ASSERT_TRUE(shapeDef) << "ShapeDef 6 not found but does exist";
    EXPECT_EQ(shapeDef->shapeType, others::ShapeDef::ShapeType::Articulation);
    EXPECT_EQ(shapeDef->instructionList, 7);
    EXPECT_EQ(shapeDef->dataList, 5);

    auto shapeData = others->get<others::ShapeData>(SCORE_PARTID, 5);
    ASSERT_TRUE(shapeData) << "ShapeData 5 not found but does exist";
    ASSERT_EQ(shapeData->values.size(), 18);
    EXPECT_EQ(shapeData->values[0], 0);
    EXPECT_EQ(shapeData->values[1], 2);
    EXPECT_EQ(shapeData->values[2], 0);
    EXPECT_EQ(shapeData->values[3], 2);
    EXPECT_EQ(shapeData->values[4], 28);
    EXPECT_EQ(shapeData->values[5], 2);
    EXPECT_EQ(shapeData->values[6], 1000);
    EXPECT_EQ(shapeData->values[7], 1000);

    auto shapeList = others->get<others::ShapeInstructionList>(SCORE_PARTID, 7);
    ASSERT_TRUE(shapeList) << "ShapeInstructionList 7 not found but does exist";
    ASSERT_EQ(shapeList->instructions.size(), 6);
    EXPECT_EQ(shapeList->instructions[0]->type, ShapeDefInstructionType::StartObject);
    EXPECT_EQ(shapeList->instructions[0]->numData, 11);
    EXPECT_EQ(shapeList->instructions[1]->type, ShapeDefInstructionType::RMoveTo);
    EXPECT_EQ(shapeList->instructions[1]->numData, 2);
    EXPECT_EQ(shapeList->instructions[2]->type, ShapeDefInstructionType::LineWidth);
    EXPECT_EQ(shapeList->instructions[2]->numData, 1);
    EXPECT_EQ(shapeList->instructions[3]->type, ShapeDefInstructionType::SetDash);
    EXPECT_EQ(shapeList->instructions[3]->numData, 2);
    EXPECT_EQ(shapeList->instructions[4]->type, ShapeDefInstructionType::RLineTo);
    EXPECT_EQ(shapeList->instructions[4]->numData, 2);
    EXPECT_EQ(shapeList->instructions[5]->type, ShapeDefInstructionType::Stroke);
}

namespace musx::dom::others {
class ShapeDefTestAccessor
{
public:
    static bool iterateInstructions(
        const MusxInstance<ShapeDef>& shape,
        std::function<bool(ShapeDefInstructionType, std::vector<int>)> callback)
    {
        return shape->iterateInstructions(std::move(callback));
    }
};
} // namespace musx::dom::others

TEST(ShapeDefTest, InterateInstructions)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(shapeXml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    auto shapeDef = others->get<others::ShapeDef>(SCORE_PARTID, 6);
    ASSERT_TRUE(shapeDef) << "ShapeDef 6 not found but does exist";
    EXPECT_EQ(shapeDef->shapeType, others::ShapeDef::ShapeType::Articulation);
    EXPECT_EQ(shapeDef->instructionList, 7);
    EXPECT_EQ(shapeDef->dataList, 5);

    auto shapeData = others->get<others::ShapeData>(SCORE_PARTID, shapeDef->dataList);
    ASSERT_TRUE(shapeData) << "ShapeData " << shapeDef->dataList << " not found but does exist";
    ASSERT_EQ(shapeData->values.size(), 18);

    auto shapeList = others->get<others::ShapeInstructionList>(SCORE_PARTID, shapeDef->instructionList);
    ASSERT_TRUE(shapeList) << "ShapeInstructionList " << shapeDef->instructionList << " not found but does exist";
    ASSERT_EQ(shapeList->instructions.size(), 6);

    size_t nextIndex = 0;
    size_t nextDataIndex = 0;

    musx::dom::others::ShapeDefTestAccessor::iterateInstructions(shapeDef, [&](ShapeDefInstructionType type, std::vector<int> data) {
        EXPECT_LT(nextIndex, shapeList->instructions.size()) << "next index exceeds size of instructions array";
        if (nextIndex >= shapeList->instructions.size()) {
            return false;
        }
        EXPECT_LE(nextDataIndex + data.size(), shapeData->values.size()) << "next data index exceeds size of data array";
        if (nextDataIndex + data.size() > shapeData->values.size()) {
            return false;
        }
        EXPECT_EQ(type, shapeList->instructions[nextIndex]->type);
        EXPECT_EQ(data.size(), shapeList->instructions[nextIndex]->numData);
        for (size_t x = 0; x < data.size(); x++) {
            EXPECT_EQ(data[x], shapeData->values[nextDataIndex + x]);
        }
        nextIndex++;
        nextDataIndex += data.size();
        return true;
    });
    EXPECT_EQ(nextIndex, shapeList->instructions.size()) << "iteration value " << nextIndex
        << " does not equal number of instructions " << shapeList->instructions.size();
}

TEST(ShapeDefTest, IterateInstructionsRejectsInsufficientDataBeforeCallback)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(insufficientShapeDataXml);
    auto shapeDef = doc->getOthers()->get<others::ShapeDef>(SCORE_PARTID, 6);
    ASSERT_TRUE(shapeDef);

    size_t callbackCount = 0;
    EXPECT_THROW(
        musx::dom::others::ShapeDefTestAccessor::iterateInstructions(shapeDef,
            [&](ShapeDefInstructionType, std::vector<int>) {
                ++callbackCount;
                return true;
            }),
        musx::dom::integrity_error);
    EXPECT_EQ(callbackCount, 0);
}

TEST(ShapeDefTest, RecognizeShapes)
{
    std::vector<char> enigmaXml;
    musxtest::readFile(musxtest::getInputPath() / "shape_recognize.enigmaxml", enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(enigmaXml);
    ASSERT_TRUE(doc);

    constexpr size_t EXPECTED_VALUES = 11;
    constexpr std::array<Cmper, EXPECTED_VALUES> expectedCmpers = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    constexpr std::array<KnownShapeDefType, EXPECTED_VALUES> expectedTypes =
    {
        KnownShapeDefType::Unrecognized,
        KnownShapeDefType::Unrecognized,
        KnownShapeDefType::Unrecognized,
        KnownShapeDefType::Unrecognized,
        KnownShapeDefType::TenutoMark,
        KnownShapeDefType::Blank,
        KnownShapeDefType::SlurTieCurveRight,
        KnownShapeDefType::SlurTieCurveLeft,
        KnownShapeDefType::Unrecognized,
        KnownShapeDefType::Unrecognized,
        KnownShapeDefType::Unrecognized,
    };

    auto shapes = doc->getOthers()->getArray<others::ShapeDef>(SCORE_PARTID);
    ASSERT_EQ(shapes.size(), EXPECTED_VALUES);
    size_t x = 0;
    for (const auto& shape : shapes) {
        ASSERT_LT(x, shapes.size());
        EXPECT_EQ(shape->getCmper(), expectedCmpers[x]) << "shape cmper " << shape->getCmper() << " is not expected value " << expectedCmpers[x];
        EXPECT_EQ(shape->recognize(), expectedTypes[x]) << "recognized shape type is not the expected value for cmper " << shape->getCmper();
        x++;
    }
}

TEST(ShapeDefTest, RecognizeResolvedEmptyInstructionListAsBlank)
{
    constexpr musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <shapeDef cmper="1">
            <instList>9</instList>
            <dataList>9</dataList>
        </shapeDef>
        <shapeList cmper="9"/>
    </others>
</finale>
)xml";
    auto document = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(document);
    const auto shape = document->getOthers()->get<others::ShapeDef>(SCORE_PARTID, 1);
    ASSERT_TRUE(shape);
    EXPECT_EQ(shape->instructionList, 9);
    EXPECT_EQ(shape->dataList, 9);
    EXPECT_TRUE(shape->isBlank());
    EXPECT_EQ(shape->recognize(), KnownShapeDefType::Blank);
}

TEST(ShapeDefTest, MissingInstructionListIsVerboseAndNotBlank)
{
    constexpr musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <shapeDef cmper="1">
            <instList>9</instList>
            <dataList>9</dataList>
        </shapeDef>
    </others>
</finale>
)xml";
    auto document = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(document);
    const auto shape = document->getOthers()->get<others::ShapeDef>(SCORE_PARTID, 1);
    ASSERT_TRUE(shape);

    auto previousLogger = musx::util::Logger::getCallback();
    struct LoggerRestorer {
        musx::util::Logger::LogCallback callback;
        ~LoggerRestorer() { musx::util::Logger::setCallback(std::move(callback)); }
    } loggerRestorer{previousLogger};
    std::vector<std::pair<musx::util::Logger::LogLevel, std::string>> diagnostics;
    musx::util::Logger::setCallback([&](musx::util::Logger::LogLevel level, const std::string& message) {
        diagnostics.emplace_back(level, message);
    });

    EXPECT_FALSE(shape->isBlank());
    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].first, musx::util::Logger::LogLevel::Verbose);
    EXPECT_EQ(diagnostics[0].second, "ShapeDef 1 references missing instruction list 9.");
}

TEST(ShapeDefTest, RecognizeVerticalLineRightHooks)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(verticalLineRightHooksXml);
    ASSERT_TRUE(doc);

    for (const auto cmper : {Cmper{127}, Cmper{128}, Cmper{129}, Cmper{130}}) {
        const auto shape = doc->getOthers()->get<others::ShapeDef>(SCORE_PARTID, cmper);
        ASSERT_TRUE(shape) << "missing shapeDef " << cmper;
        EXPECT_EQ(shape->recognize(), KnownShapeDefType::VerticalLineRightHooks)
            << "recognized shape type is not the expected value for cmper " << cmper;
    }
}

TEST(ShapeDefTest, RecognizeOnlyExpectedShapesInPattersonDefault)
{
    const std::vector<std::pair<Cmper, KnownShapeDefType>> expectedRecognized = {
        {10, KnownShapeDefType::SnapPizzicatoAbove},
        {11, KnownShapeDefType::SnapPizzicatoBelow},
        {12, KnownShapeDefType::BuzzPizzicato},
        {29, KnownShapeDefType::FingernailPizzCurveUp},
        {30, KnownShapeDefType::FingernailPizzCurveDown},
        {91, KnownShapeDefType::PedalArrowheadDown},
        {92, KnownShapeDefType::PedalArrowheadUp},
        {93, KnownShapeDefType::PedalArrowheadShortUpDownLongUp},
        {94, KnownShapeDefType::PedalArrowheadLongUpDownShortUp},
        {95, KnownShapeDefType::Blank},
        {96, KnownShapeDefType::SnapPizzicatoAbove},
        {101, KnownShapeDefType::FingernailPizzCurveDown},
        {110, KnownShapeDefType::BuzzPizzicato},
        {111, KnownShapeDefType::SnapPizzicatoBelow},
        {112, KnownShapeDefType::SnapPizzicatoAbove},
        {120, KnownShapeDefType::PedalArrowheadDown},
        {121, KnownShapeDefType::PedalArrowheadUp},
        {122, KnownShapeDefType::PedalArrowheadLongUpDownShortUp},
        {123, KnownShapeDefType::PedalArrowheadShortUpDownLongUp},
        {124, KnownShapeDefType::TenutoMark},
        {125, KnownShapeDefType::SlurTieCurveRight},
        {126, KnownShapeDefType::SlurTieCurveRight},
    };

    std::vector<char> enigmaXml;
    musxtest::readFile(musxtest::getInputPath() / "reference" / "PattersonDefault.enigmaxml", enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(enigmaXml);
    ASSERT_TRUE(doc);

    auto shapes = doc->getOthers()->getArray<others::ShapeDef>(SCORE_PARTID);
    ASSERT_EQ(shapes.size(), 143);

    for (const auto& shape : shapes) {
        const auto expected = std::find_if(expectedRecognized.begin(), expectedRecognized.end(),
            [&](const auto& entry) { return entry.first == shape->getCmper(); });
        const auto expectedType = expected != expectedRecognized.end()
            ? expected->second
            : KnownShapeDefType::Unrecognized;
        EXPECT_EQ(shape->recognize(), expectedType)
            << "unexpected recognition result for PattersonDefault shapeDef " << shape->getCmper();
    }
}

namespace {
struct PedalArrowheadNegativeCase
{
    std::filesystem::path path;
    std::array<Cmper, 4> arrowCmpers;
    std::array<Cmper, 3> cmpers;
};

void verifyPedalArrowheads(
    const std::filesystem::path& path,
    const std::array<Cmper, 4>& arrowCmpers,
    const std::array<Cmper, 3>& negativeCmpers)
{
    std::vector<char> enigmaXml;
    musxtest::readFile(path, enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(enigmaXml);
    ASSERT_TRUE(doc) << "unable to parse " << path.string();

    constexpr std::array<KnownShapeDefType, 4> expectedTypes = {
        KnownShapeDefType::PedalArrowheadDown,
        KnownShapeDefType::PedalArrowheadUp,
        KnownShapeDefType::PedalArrowheadShortUpDownLongUp,
        KnownShapeDefType::PedalArrowheadLongUpDownShortUp,
    };

    std::vector<KnownShapeDefType> recognizedTypes;
    recognizedTypes.reserve(arrowCmpers.size());
    for (const auto cmper : arrowCmpers) {
        const auto shape = doc->getOthers()->get<others::ShapeDef>(SCORE_PARTID, cmper);
        ASSERT_TRUE(shape) << "missing arrowhead shapeDef " << cmper << " in " << path.string();
        EXPECT_EQ(shape->shapeType, others::ShapeDef::ShapeType::Arrowhead)
            << "arrowhead shapeDef " << cmper << " has unexpected type in " << path.string();
        const auto recognized = shape->recognize();
        recognizedTypes.push_back(recognized);
    }
    std::sort(recognizedTypes.begin(), recognizedTypes.end());
    std::array<KnownShapeDefType, 4> expectedSorted = expectedTypes;
    std::sort(expectedSorted.begin(), expectedSorted.end());
    EXPECT_EQ(recognizedTypes.size(), expectedSorted.size()) << "recognized arrowhead count mismatch in " << path.string();
    for (size_t i = 0; i < expectedSorted.size(); ++i) {
        EXPECT_EQ(recognizedTypes[i], expectedSorted[i]) << "recognized arrowhead set mismatch in " << path.string();
    }

    for (const auto cmper : negativeCmpers) {
        const auto shape = doc->getOthers()->get<others::ShapeDef>(SCORE_PARTID, cmper);
        ASSERT_TRUE(shape) << "missing non-arrowhead shapeDef " << cmper << " in " << path.string();
        EXPECT_EQ(shape->recognize(), KnownShapeDefType::Unrecognized)
            << "expected no recognized shape type for cmper " << cmper << " in " << path.string();
    }
}

void verifyOnlyExpectedRecognizedShapes(
    const std::filesystem::path& path,
    const std::vector<std::pair<Cmper, KnownShapeDefType>>& expectedRecognized)
{
    std::vector<char> enigmaXml;
    musxtest::readFile(path, enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(enigmaXml);
    ASSERT_TRUE(doc) << "unable to parse " << path.string();

    for (const auto& shape : doc->getOthers()->getArray<others::ShapeDef>(SCORE_PARTID)) {
        const auto expected = std::find_if(expectedRecognized.begin(), expectedRecognized.end(),
            [&](const auto& entry) { return entry.first == shape->getCmper(); });
        const auto expectedType = expected != expectedRecognized.end()
            ? expected->second
            : KnownShapeDefType::Unrecognized;
        EXPECT_EQ(shape->recognize(), expectedType)
            << "unexpected recognition result for shapeDef " << shape->getCmper() << " in " << path.string();
    }
}

} // namespace

TEST(ShapeDefTest, RecognizePedalArrowheads)
{
    const auto inputRoot = musxtest::getInputPath() / "reference";
    const std::array<PedalArrowheadNegativeCase, 6> cases = {{
        {inputRoot / "FinaleMaestroFontDefaultMac.enigmaxml", {91, 92, 93, 94}, {81, 82, 83}},
        {inputRoot / "MaestroFontDefaultWin.enigmaxml", {91, 92, 93, 94}, {81, 82, 83}},
        {inputRoot / "Finale Broadway Font Default.enigmaxml", {97, 98, 99, 100}, {87, 88, 89}},
        {inputRoot / "Finale Jazz Font Default.enigmaxml", {44, 45, 46, 47}, {34, 35, 36}},
        {inputRoot / "Handwritten Default.enigmaxml", {96, 97, 98, 99}, {86, 87, 88}},
        {inputRoot / "Jazz Font Default.enigmaxml", {43, 44, 45, 46}, {33, 34, 35}},
    }};

    for (const auto& entry : cases) {
        verifyPedalArrowheads(entry.path, entry.arrowCmpers, entry.cmpers);
    }
}

TEST(ShapeDefTest, RecognizeBroadwayFontDefaultShapes)
{
    const auto path = musxtest::getInputPath() / "reference" / "Finale Broadway Font Default.enigmaxml";
    verifyOnlyExpectedRecognizedShapes(path, {
        {20, KnownShapeDefType::SnapPizzicatoAbove},
        {21, KnownShapeDefType::SnapPizzicatoAbove},
        {22, KnownShapeDefType::SnapPizzicatoBelow},
        {23, KnownShapeDefType::SnapPizzicatoBelow},
        {24, KnownShapeDefType::BuzzPizzicato},
        {25, KnownShapeDefType::BuzzPizzicato},
        {42, KnownShapeDefType::FingernailPizzCurveUp},
        {43, KnownShapeDefType::FingernailPizzCurveDown},
        {97, KnownShapeDefType::PedalArrowheadDown},
        {98, KnownShapeDefType::PedalArrowheadUp},
        {99, KnownShapeDefType::PedalArrowheadShortUpDownLongUp},
        {100, KnownShapeDefType::PedalArrowheadLongUpDownShortUp},
        {101, KnownShapeDefType::Blank},
        {121, KnownShapeDefType::Blank},
        {122, KnownShapeDefType::Blank},
        {123, KnownShapeDefType::Blank},
    });
}

TEST(ShapeDefTest, RecognizeMaestroFontDefaultWinShapes)
{
    const auto path = musxtest::getInputPath() / "reference" / "MaestroFontDefaultWin.enigmaxml";
    verifyOnlyExpectedRecognizedShapes(path, {
        {10, KnownShapeDefType::SnapPizzicatoAbove},
        {11, KnownShapeDefType::SnapPizzicatoBelow},
        {12, KnownShapeDefType::BuzzPizzicato},
        {29, KnownShapeDefType::FingernailPizzCurveUp},
        {30, KnownShapeDefType::FingernailPizzCurveDown},
        {91, KnownShapeDefType::PedalArrowheadDown},
        {92, KnownShapeDefType::PedalArrowheadUp},
        {93, KnownShapeDefType::PedalArrowheadShortUpDownLongUp},
        {94, KnownShapeDefType::PedalArrowheadLongUpDownShortUp},
        {95, KnownShapeDefType::Blank},
        {96, KnownShapeDefType::SnapPizzicatoAbove},
        {101, KnownShapeDefType::FingernailPizzCurveDown},
        {110, KnownShapeDefType::BuzzPizzicato},
        {111, KnownShapeDefType::SnapPizzicatoBelow},
        {112, KnownShapeDefType::SnapPizzicatoAbove},
    });
}

TEST(ShapeDefTest, RecognizeJazzFontDefaultShapes)
{
    const auto inputRoot = musxtest::getInputPath() / "reference";
    const std::vector<std::pair<Cmper, KnownShapeDefType>> finaleJazzExpected = {
        {39, KnownShapeDefType::Blank},
        {44, KnownShapeDefType::PedalArrowheadDown},
        {45, KnownShapeDefType::PedalArrowheadUp},
        {46, KnownShapeDefType::PedalArrowheadLongUpDownShortUp},
        {47, KnownShapeDefType::PedalArrowheadShortUpDownLongUp},
        {49, KnownShapeDefType::Blank},
    };
    const std::vector<std::pair<Cmper, KnownShapeDefType>> jazzExpected = {
        {38, KnownShapeDefType::Blank},
        {43, KnownShapeDefType::PedalArrowheadDown},
        {44, KnownShapeDefType::PedalArrowheadUp},
        {45, KnownShapeDefType::PedalArrowheadLongUpDownShortUp},
        {46, KnownShapeDefType::PedalArrowheadShortUpDownLongUp},
        {48, KnownShapeDefType::Blank},
    };

    verifyOnlyExpectedRecognizedShapes(inputRoot / "Finale Jazz Font Default.enigmaxml", finaleJazzExpected);
    verifyOnlyExpectedRecognizedShapes(inputRoot / "Jazz Font Default.enigmaxml", jazzExpected);
}

TEST(ShapeDefTest, CalculateWidths)
{
    std::vector<char> enigmaXml;
    musxtest::readFile(musxtest::getInputPath() / "shape_recognize.enigmaxml", enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(enigmaXml);
    ASSERT_TRUE(doc);

    auto shapes = doc->getOthers()->getArray<others::ShapeDef>(SCORE_PARTID);
    ASSERT_EQ(shapes.size(), 11);

    constexpr std::array<std::optional<Evpu>, 11> expectedWidths = {
        std::nullopt,          // cmper 1: text shape
        std::nullopt,          // cmper 2: text shape
        std::nullopt,          // cmper 3: no bounding box
        std::nullopt,          // cmper 4: text shape
        Evpu{28},              // cmper 5: tenuto
        Evpu{0},               // cmper 6: blank shape
        Evpu{60},              // cmper 7: tie slur right
        Evpu{60},              // cmper 8: tie slur left
        Evpu{170},             // cmper 9: scaled slur
        Evpu{142},             // cmper 10: leftward slur
        Evpu{56},              // cmper 11: line too wide to be tenuto
    };

    for (size_t i = 0; i < shapes.size(); ++i) {
        const auto width = shapes[i]->calcWidth();
        EXPECT_EQ(width, expectedWidths[i]) << "width mismatch for cmper " << shapes[i]->getCmper();
    }
}

TEST(ShapeDefTest, CalculateSlurContourDirection)
{
    std::vector<char> enigmaXml;
    musxtest::readFile(musxtest::getInputPath() / "shape_recognize.enigmaxml", enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(enigmaXml);
    ASSERT_TRUE(doc);

    auto shapes = doc->getOthers()->getArray<others::ShapeDef>(SCORE_PARTID);
    ASSERT_EQ(shapes.size(), 11);

    using Contour = musx::dom::CurveContourDirection;
    constexpr std::array<Contour, 11> expectedDirections = {
        Contour::Unspecified,   // 1
        Contour::Unspecified,   // 2
        Contour::Unspecified,   // 3
        Contour::Unspecified,   // 4
        Contour::Unspecified,   // 5
        Contour::Unspecified,   // 6
        Contour::Up,            // 7
        Contour::Down,          // 8
        Contour::Up,            // 9
        Contour::Down,          // 10
        Contour::Unspecified    // 11
    };

    for (size_t i = 0; i < shapes.size(); ++i) {
        const auto direction = shapes[i]->calcSlurContour();
        EXPECT_EQ(direction, expectedDirections[i]) << "contour mismatch for cmper " << shapes[i]->getCmper();
    }
}

constexpr static musxtest::string_view shapeImportSourceXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <fontName cmper="9">
            <charsetBank>Mac</charsetBank>
            <charsetVal>4095</charsetVal>
            <pitch>0</pitch>
            <family>0</family>
            <name>Seville</name>
        </fontName>
        <shapeData cmper="3">
            <data>9</data>
            <data>24</data>
            <data>1</data>
            <data>57</data>
        </shapeData>
        <shapeDef cmper="2">
            <instList>4</instList>
            <dataList>3</dataList>
            <shapeType>clef</shapeType>
        </shapeDef>
        <shapeList cmper="4">
            <instruct>
                <numData>3</numData>
                <tag>setFont</tag>
            </instruct>
            <instruct>
                <numData>1</numData>
                <tag>drawChar</tag>
            </instruct>
        </shapeList>
    </others>
</finale>
    )xml";

constexpr static musxtest::string_view shapeImportTargetXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <fontName cmper="1">
            <charsetBank>Mac</charsetBank>
            <charsetVal>0</charsetVal>
            <pitch>0</pitch>
            <family>0</family>
            <name>Times New Roman</name>
        </fontName>
        <shapeData cmper="8">
            <data>0</data>
        </shapeData>
        <shapeDef cmper="6">
            <instList>7</instList>
            <dataList>8</dataList>
            <shapeType>articulation</shapeType>
        </shapeDef>
        <shapeList cmper="7">
            <instruct>
                <numData>1</numData>
                <tag>lineWidth</tag>
            </instruct>
        </shapeList>
    </others>
</finale>
    )xml";

TEST(ShapeTest, ImportShapeDefCopiesListsAndRemapsItsFont)
{
    auto source = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(shapeImportSourceXml);
    auto target = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(shapeImportTargetXml);

    auto shape = source->getOthers()->get<others::ShapeDef>(SCORE_PARTID, 2);
    ASSERT_TRUE(shape);

    std::vector<std::type_index> importedTypes;
    const auto imported = others::importShapeDefInto(target, shape,
        [&importedTypes](const EnigmaBase& object) { importedTypes.emplace_back(typeid(object)); });
    ASSERT_TRUE(imported);
    EXPECT_EQ(importedTypes.size(), 4u);
    EXPECT_EQ(std::count(importedTypes.begin(), importedTypes.end(),
                  std::type_index(typeid(others::FontDefinition))),
        1);
    EXPECT_EQ(std::count(importedTypes.begin(), importedTypes.end(),
                  std::type_index(typeid(others::ShapeInstructionList))),
        1);
    EXPECT_EQ(std::count(importedTypes.begin(), importedTypes.end(),
                  std::type_index(typeid(others::ShapeData))),
        1);
    EXPECT_EQ(std::count(importedTypes.begin(), importedTypes.end(),
                  std::type_index(typeid(others::ShapeDef))),
        1);
    // Each pool numbers independently, so the copy takes the next free cmper in each.
    EXPECT_EQ(*imported, 7);

    auto copied = target->getOthers()->get<others::ShapeDef>(SCORE_PARTID, *imported);
    ASSERT_TRUE(copied);
    EXPECT_EQ(copied->shapeType, others::ShapeDef::ShapeType::Clef);
    EXPECT_EQ(copied->instructionList, 8);
    EXPECT_EQ(copied->dataList, 9);
    // The source numbered its lists 4 and 3; carrying those across would have named the target's
    // own unrelated shapeList 4 and shapeData 3.
    EXPECT_NE(copied->instructionList, shape->instructionList);
    EXPECT_NE(copied->dataList, shape->dataList);

    // Seville is absent from the target, so it is cloned; the setFont data must name the new
    // cmper rather than the source's 9.
    bool sawFont = false;
    copied->iterateInstructions([&](const ShapeDefInstruction::Decoded& decoded) {
        if (const auto setFont = std::get_if<ShapeDefInstruction::SetFont>(&decoded.data)) {
            sawFont = true;
            EXPECT_NE(setFont->font.fontId, 9);
            EXPECT_EQ(setFont->font.getName(), "Seville");
            EXPECT_EQ(setFont->font.fontSize, 24);
            EXPECT_TRUE(setFont->font.bold);
        }
        return true;
    });
    EXPECT_TRUE(sawFont);

    // The instruction stream and the non-font data are copied verbatim.
    auto copiedData = target->getOthers()->get<others::ShapeData>(SCORE_PARTID, copied->dataList);
    ASSERT_TRUE(copiedData);
    ASSERT_EQ(copiedData->values.size(), 4u);
    EXPECT_EQ(copiedData->values[1], 24);
    EXPECT_EQ(copiedData->values[3], 57);
}

constexpr static musxtest::string_view shapeWithGraphicXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <others>
        <shapeData cmper="1">
            <data>100</data>
            <data>200</data>
            <data>12</data>
        </shapeData>
        <shapeDef cmper="1">
            <instList>1</instList>
            <dataList>1</dataList>
            <shapeType>expression</shapeType>
        </shapeDef>
        <shapeList cmper="1">
            <instruct>
                <numData>3</numData>
                <tag>extGraphic</tag>
            </instruct>
        </shapeList>
        <shapeDef cmper="2">
            <shapeType>other</shapeType>
        </shapeDef>
    </others>
</finale>
    )xml";

TEST(ShapeTest, ImportShapeDefRefusesAShapeNamingAGraphic)
{
    auto source = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(shapeWithGraphicXml);
    auto target = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(shapeImportTargetXml);

    auto shape = source->getOthers()->get<others::ShapeDef>(SCORE_PARTID, 1);
    ASSERT_TRUE(shape);

    // The graphic is named by a cmper that means nothing in the target, and graphics are not
    // copied yet, so the shape cannot be imported faithfully.
    EXPECT_FALSE(others::importShapeDefInto(target, shape));

    // Nothing partial is left behind: the target keeps only the shape it already had.
    EXPECT_EQ(target->getOthers()->getArray<others::ShapeDef>(SCORE_PARTID).size(), 1u);
    EXPECT_EQ(target->getOthers()->getArray<others::ShapeData>(SCORE_PARTID).size(), 1u);
    EXPECT_EQ(target->getOthers()->getArray<others::ShapeInstructionList>(SCORE_PARTID).size(), 1u);
}

TEST(ShapeTest, ImportShapeDefCopiesABlankShapeWithoutLists)
{
    auto source = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(shapeWithGraphicXml);
    auto target = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(shapeImportTargetXml);

    auto blank = source->getOthers()->get<others::ShapeDef>(SCORE_PARTID, 2);
    ASSERT_TRUE(blank);
    ASSERT_TRUE(blank->isBlank());

    const auto imported = others::importShapeDefInto(target, blank);
    ASSERT_TRUE(imported);
    auto copied = target->getOthers()->get<others::ShapeDef>(SCORE_PARTID, *imported);
    ASSERT_TRUE(copied);
    EXPECT_TRUE(copied->isBlank());
    EXPECT_EQ(copied->instructionList, 0);
    EXPECT_EQ(copied->dataList, 0);
    // A shape that draws nothing owns no lists, so none were created.
    EXPECT_EQ(target->getOthers()->getArray<others::ShapeData>(SCORE_PARTID).size(), 1u);
}

TEST(ShapeTest, NextFreeCmperIsPerType)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(shapeImportTargetXml);
    auto others = doc->getOthers();

    // Each type numbers independently: this document holds shapeDef 6, shapeList 7, shapeData 8
    // and fontName 1, so no single answer serves all four.
    EXPECT_EQ(others->nextFreeCmper<others::ShapeDef>(SCORE_PARTID), std::optional<Cmper>(Cmper(7)));
    EXPECT_EQ(others->nextFreeCmper<others::ShapeInstructionList>(SCORE_PARTID), std::optional<Cmper>(Cmper(8)));
    EXPECT_EQ(others->nextFreeCmper<others::ShapeData>(SCORE_PARTID), std::optional<Cmper>(Cmper(9)));
    EXPECT_EQ(others->nextFreeCmper<others::FontDefinition>(SCORE_PARTID), std::optional<Cmper>(Cmper(2)));

    // An empty pool starts at 1, leaving 0 for whatever sentinel meaning the type gives it.
    EXPECT_EQ(others->nextFreeCmper<others::TextExpressionDef>(SCORE_PARTID), std::optional<Cmper>(Cmper(1)));
}

TEST(ShapeTest, NextFreeCmperReusesTheLowestGapOnlyWhenTheTopIsTaken)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(shapeImportTargetXml);
    auto others = doc->getOthers();
    const auto addDefinition = [&](Cmper cmper) {
        others->add(others::TextExpressionDef::XmlNodeName,
            std::make_shared<others::TextExpressionDef>(doc, SCORE_PARTID, EnigmaBase::ShareMode::All, cmper));
    };

    // A gap below the highest identifier is not reused while the space above it is free.
    addDefinition(1);
    addDefinition(3);
    EXPECT_EQ(others->nextFreeCmper<others::TextExpressionDef>(SCORE_PARTID), std::optional<Cmper>(Cmper(4)));

    // With the highest possible identifier taken, the lowest unused one above zero is returned.
    constexpr Cmper maxCmper = (std::numeric_limits<Cmper>::max)();
    addDefinition(maxCmper);
    EXPECT_EQ(others->nextFreeCmper<others::TextExpressionDef>(SCORE_PARTID), std::optional<Cmper>(Cmper(2)));
    addDefinition(2);
    EXPECT_EQ(others->nextFreeCmper<others::TextExpressionDef>(SCORE_PARTID), std::optional<Cmper>(Cmper(4)));
}
