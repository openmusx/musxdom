#include "gtest/gtest.h"
#include "musx/musx.h"
#include "test_utils.h"

constexpr static musxtest::string_view clefOptionsXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <options>
    <clefOptions>
      <defaultClef>4</defaultClef>
      <endMeasClefPercent>75</endMeasClefPercent>
      <endMeasClefPosAdd>-12</endMeasClefPosAdd>
      <clefFront>24</clefFront>
      <clefBack>1</clefBack>
      <showClefFirstSystemOnly/>
      <clefKey>2</clefKey>
      <clefTime>3</clefTime>
      <cautionaryClefChanges/>
      <clefDef index="0">
        <adjust>-10</adjust>
        <clefChar>57424</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="1">
        <adjust>-4</adjust>
        <clefChar>57436</clefChar>
        <clefYDisp>-4</clefYDisp>
      </clefDef>
      <clefDef index="2">
        <adjust>-2</adjust>
        <clefChar>57436</clefChar>
        <clefYDisp>-2</clefYDisp>
      </clefDef>
      <clefDef index="3">
        <adjust>2</adjust>
        <clefChar>57442</clefChar>
        <clefYDisp>-2</clefYDisp>
      </clefDef>
      <clefDef index="4">
        <adjust>-10</adjust>
        <clefChar>57450</clefChar>
        <clefYDisp>-4</clefYDisp>
        <baseAdjust>12</baseAdjust>
        <font>
          <fontID>10</fontID>
          <fontSize>24</fontSize>
        </font>
        <useOwnFont/>
      </clefDef>
      <clefDef index="5">
        <adjust>-3</adjust>
        <clefChar>57426</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="6">
        <adjust>9</adjust>
        <clefChar>57444</clefChar>
        <clefYDisp>-2</clefYDisp>
      </clefDef>
      <clefDef index="7">
        <clefChar>57442</clefChar>
        <clefYDisp>-4</clefYDisp>
      </clefDef>
      <clefDef index="8">
        <adjust>-12</adjust>
        <clefChar>57424</clefChar>
        <clefYDisp>-8</clefYDisp>
      </clefDef>
      <clefDef index="9">
        <clefChar>57436</clefChar>
      </clefDef>
      <clefDef index="10">
        <adjust>-6</adjust>
        <clefChar>57436</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="11">
        <adjust>-8</adjust>
        <clefChar>57436</clefChar>
        <clefYDisp>-8</clefYDisp>
      </clefDef>
      <clefDef index="12">
        <adjust>-10</adjust>
        <clefChar>57449</clefChar>
        <clefYDisp>-4</clefYDisp>
      </clefDef>
      <clefDef index="13">
        <adjust>-17</adjust>
        <clefChar>57427</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="14">
        <adjust>-5</adjust>
        <clefChar>57445</clefChar>
        <clefYDisp>-2</clefYDisp>
      </clefDef>
      <clefDef index="15">
        <adjust>-10</adjust>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="16">
        <adjust>-10</adjust>
        <shapeID>2</shapeID>
        <isShape/>
        <scaleToStaffHeight/>
      </clefDef>
      <clefDef index="17">
        <adjust>-10</adjust>
        <shapeID>3</shapeID>
        <isShape/>
        <scaleToStaffHeight/>
      </clefDef>
    </clefOptions>
  </options>
</finale>
)xml";

using namespace musx::dom;

TEST(ClefOptionsTest, ClefDefPropertiesTest)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(clefOptionsXml);
    auto options = doc->getOptions();
    ASSERT_TRUE(options);
    auto clefOptions = options->get<options::ClefOptions>();
    ASSERT_TRUE(clefOptions);

    // Test properties
    EXPECT_EQ(clefOptions->defaultClef, 4);
    EXPECT_EQ(clefOptions->clefChangePercent, 75);
    EXPECT_EQ(clefOptions->clefChangeOffset, -12);
    EXPECT_EQ(clefOptions->clefFrontSepar, 24);
    EXPECT_EQ(clefOptions->clefBackSepar, 1);
    EXPECT_TRUE(clefOptions->showClefFirstSystemOnly);
    EXPECT_EQ(clefOptions->clefKeySepar, 2);
    EXPECT_EQ(clefOptions->clefTimeSepar, 3);
    EXPECT_TRUE(clefOptions->cautionaryClefChanges);
    ASSERT_EQ(clefOptions->clefDefs.size(), 18);

    // Test ClefDef index 0
    const auto& clefDef0 = clefOptions->clefDefs[0];
    EXPECT_EQ(clefDef0->middleCPos, -10);
    EXPECT_EQ(clefDef0->clefChar, 0xe050);
    EXPECT_EQ(clefDef0->staffPosition, -6);
    EXPECT_FALSE(clefDef0->isShape);
    EXPECT_FALSE(clefDef0->scaleToStaffHeight);

    // Test ClefDef index 4
    const auto& clefDef4 = clefOptions->clefDefs[4];
    EXPECT_EQ(clefDef4->middleCPos, -10);
    EXPECT_EQ(clefDef4->clefChar, 0xe06a);
    EXPECT_EQ(clefDef4->staffPosition, -4);
    EXPECT_EQ(clefDef4->baselineAdjust, 12);
    ASSERT_TRUE(clefDef4->font);
    EXPECT_EQ(clefDef4->font->fontId, 10);
    EXPECT_EQ(clefDef4->font->fontSize, 24);
    EXPECT_TRUE(clefDef4->useOwnFont);
    EXPECT_FALSE(clefDef4->isShape);
    EXPECT_FALSE(clefDef4->scaleToStaffHeight);

    // Test ClefDef index 16
    const auto& clefDef16 = clefOptions->clefDefs[16];
    EXPECT_EQ(clefDef16->middleCPos, -10);
    EXPECT_EQ(clefDef16->shapeId, 2);
    EXPECT_TRUE(clefDef16->isShape);
    EXPECT_TRUE(clefDef16->scaleToStaffHeight);
}

TEST(ClefOptionsTest, OutOfSeqenceClefDef)
{
  constexpr static musxtest::string_view outOfSequence = R"xml(
    <?xml version="1.0" encoding="UTF-8"?>
    <finale>
      <options>
        <clefOptions>
          <defaultClef>4</defaultClef>
          <endMeasClefPercent>75</endMeasClefPercent>
          <endMeasClefPosAdd>-12</endMeasClefPosAdd>
          <clefFront>24</clefFront>
          <clefBack>1</clefBack>
          <showClefFirstSystemOnly/>
          <clefKey>2</clefKey>
          <clefTime>3</clefTime>
          <cautionaryClefChanges/>
          <clefDef index="2">
            <adjust>-10</adjust>
            <clefChar>57424</clefChar>
            <clefYDisp>-6</clefYDisp>
          </clefDef>
        </clefOptions>
      </options>
    </finale>
    )xml";

    EXPECT_THROW(
        auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(outOfSequence),
        std::invalid_argument
    );
}

TEST(ClefOptionsTest, Only2ClefDefs)
{
    constexpr static musxtest::string_view only2Defs = R"xml(
    <?xml version="1.0" encoding="UTF-8"?>
    <finale>
      <options>
        <clefOptions>
          <clefDef index="0">
            <adjust>-10</adjust>
            <clefChar>57424</clefChar>
            <clefYDisp>-6</clefYDisp>
          </clefDef>
          <clefDef index="1">
            <adjust>-4</adjust>
            <clefChar>57436</clefChar>
            <clefYDisp>-4</clefYDisp>
          </clefDef>
        </clefOptions>
      </options>
    </finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(only2Defs);
    auto options = doc->getOptions();
    ASSERT_TRUE(options);
    auto clefOptions = options->get<options::ClefOptions>();
    ASSERT_TRUE(clefOptions);
    EXPECT_EQ(clefOptions->clefDefs.size(), 2);
}

TEST(ClefOptionsTest, ClefInfoTest)
{
    std::vector<char> smuflDefaultClefs;
    musxtest::readFile(musxtest::getInputPath() / "smuflclefs.enigmaxml", smuflDefaultClefs);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(smuflDefaultClefs);
    ASSERT_TRUE(doc);

    auto options = doc->getOptions();
    ASSERT_TRUE(options);
    auto clefOptions = options->get<options::ClefOptions>();
    ASSERT_TRUE(clefOptions);
    ASSERT_EQ(clefOptions->clefDefs.size(), 18);

    static const std::vector<options::ClefOptions::ClefInfo> expectedValues = {
        { music_theory::ClefType::G, 0 },
        { music_theory::ClefType::C, 0 },
        { music_theory::ClefType::C, 0 },
        { music_theory::ClefType::F, 0 },
        { music_theory::ClefType::Percussion2, 0 },
        { music_theory::ClefType::G, -1 },
        { music_theory::ClefType::F, -1 },
        { music_theory::ClefType::F, 0 },
        { music_theory::ClefType::G, 0 },
        { music_theory::ClefType::C, 0 },
        { music_theory::ClefType::C, 0 },
        { music_theory::ClefType::C, 0 },
        { music_theory::ClefType::Percussion1, 0 },
        { music_theory::ClefType::G, 1 },
        { music_theory::ClefType::F, 1 },
        { music_theory::ClefType::G, 0 },
        { music_theory::ClefType::Tab, 0 },
        { music_theory::ClefType::TabSerif, 0 },
   };

    for (size_t x = 0; x < clefOptions->clefDefs.size(); x++) {
        const auto& def = clefOptions->clefDefs[x];
        auto [clefType, octave] = def->calcInfo();
        EXPECT_EQ(clefType, expectedValues[x].first);
        EXPECT_EQ(octave, expectedValues[x].second);
        EXPECT_EQ(def->isBlank(), x == 15) << "x is " << x;
    }
}

TEST(ClefOptionsTest, LegactClefDefs)
{
constexpr static musxtest::string_view legacyClefDefsXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <options>
    <clefOptions>
      <endMeasClefPercent>75</endMeasClefPercent>
      <endMeasClefPosAdd>-8</endMeasClefPosAdd>
      <clefFront>24</clefFront>
      <cautionaryClefChanges/>
      <clefDef index="0">
        <adjust>-10</adjust>
        <clefChar>38</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="1">
        <adjust>-4</adjust>
        <clefChar>66</clefChar>
        <clefYDisp>-4</clefYDisp>
      </clefDef>
      <clefDef index="2">
        <adjust>-2</adjust>
        <clefChar>66</clefChar>
        <clefYDisp>-2</clefYDisp>
      </clefDef>
      <clefDef index="3">
        <adjust>2</adjust>
        <clefChar>63</clefChar>
        <clefYDisp>-2</clefYDisp>
      </clefDef>
      <clefDef index="4">
        <adjust>-10</adjust>
        <clefChar>214</clefChar>
        <clefYDisp>-4</clefYDisp>
      </clefDef>
      <clefDef index="5">
        <adjust>-3</adjust>
        <clefChar>86</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="6">
        <adjust>9</adjust>
        <clefChar>116</clefChar>
        <clefYDisp>-2</clefYDisp>
      </clefDef>
      <clefDef index="7">
        <clefChar>63</clefChar>
        <clefYDisp>-4</clefYDisp>
      </clefDef>
      <clefDef index="8">
        <adjust>-12</adjust>
        <clefChar>38</clefChar>
        <clefYDisp>-8</clefYDisp>
      </clefDef>
      <clefDef index="9">
        <clefChar>66</clefChar>
      </clefDef>
      <clefDef index="10">
        <adjust>-6</adjust>
        <clefChar>66</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="11">
        <adjust>-8</adjust>
        <clefChar>66</clefChar>
        <clefYDisp>-8</clefYDisp>
      </clefDef>
      <clefDef index="12">
        <adjust>-10</adjust>
        <clefChar>139</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="13">
        <adjust>-17</adjust>
        <clefChar>160</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="14">
        <adjust>-5</adjust>
        <clefChar>230</clefChar>
        <clefYDisp>-2</clefYDisp>
      </clefDef>
      <clefDef index="15">
        <adjust>-10</adjust>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="16">
        <adjust>-10</adjust>
        <shapeID>4</shapeID>
        <isShape/>
        <scaleToStaffHeight/>
      </clefDef>
      <clefDef index="17">
        <adjust>-10</adjust>
        <shapeID>5</shapeID>
        <isShape/>
        <scaleToStaffHeight/>
      </clefDef>
    </clefOptions>
   </options>
</finale>
)xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(legacyClefDefsXml);
    auto options = doc->getOptions();
    ASSERT_TRUE(options);
    auto clefOptions = options->get<options::ClefOptions>();
    ASSERT_TRUE(clefOptions);

    for (size_t x = 0; x < clefOptions->clefDefs.size(); x++) {
        if (!clefOptions->clefDefs[x]->isShape) {
            EXPECT_EQ(clefOptions->clefDefs[x]->isBlank(), x == 15) << "Blank clef status does not match for clef index " << x;
        }
    }
}
TEST(ClefOptionsTest, PercussionClefGlyphs)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <options>
    <clefOptions>
      <clefDef index="0">
        <adjust>-10</adjust>
        <clefChar>38</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="1">
        <adjust>-10</adjust>
        <clefChar>139</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="2">
        <adjust>-10</adjust>
        <clefChar>47</clefChar>
        <clefYDisp>-4</clefYDisp>
      </clefDef>
      <clefDef index="3">
        <adjust>-3</adjust>
        <clefChar>139</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="4">
        <adjust>-17</adjust>
        <clefChar>214</clefChar>
        <clefYDisp>-4</clefYDisp>
      </clefDef>
      <clefDef index="5">
        <adjust>-10</adjust>
        <clefChar>61487</clefChar>
        <clefYDisp>-6</clefYDisp>
      </clefDef>
      <clefDef index="6">
        <adjust>-10</adjust>
        <clefChar>57449</clefChar>
        <clefYDisp>-6</clefYDisp>
        <useOwnFont/>
        <font>
          <fontID>1</fontID>
          <fontSize>24</fontSize>
        </font>
      </clefDef>
      <clefDef index="7">
        <adjust>-3</adjust>
        <clefChar>139</clefChar>
        <clefYDisp>-4</clefYDisp>
      </clefDef>
      <clefDef index="8">
        <adjust>-10</adjust>
        <clefChar>139</clefChar>
        <clefYDisp>-6</clefYDisp>
        <useOwnFont/>
        <font>
          <fontID>2</fontID>
          <fontSize>24</fontSize>
        </font>
      </clefDef>
    </clefOptions>
    <fontOptions>
      <font type="clef">
        <fontSize>24</fontSize>
      </font>
    </fontOptions>
  </options>
  <others>
    <fontName cmper="0">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Maestro</name>
    </fontName>
    <fontName cmper="1">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Finale Maestro</name>
    </fontName>
    <fontName cmper="2">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Jazz</name>
    </fontName>
  </others>
</finale>
)xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    ASSERT_TRUE(doc);
    auto clefOptions = doc->getOptions()->get<options::ClefOptions>();
    ASSERT_TRUE(clefOptions);

    static const std::vector<options::ClefOptions::ClefInfo> expectedValues = {
        { music_theory::ClefType::G, 0 },           // treble glyph on the G line
        { music_theory::ClefType::Percussion1, 0 }, // percussion glyph on the G line, middle C as for treble
        { music_theory::ClefType::Percussion1, 0 }, // percussion glyph on the middle line, middle C as for treble
        { music_theory::ClefType::Percussion1, -1 },// middle C an octave higher than for treble
        { music_theory::ClefType::Percussion2, 1 }, // middle C an octave lower than for treble
        { music_theory::ClefType::Percussion1, 0 }, // Windows symbol-font code point
        { music_theory::ClefType::Percussion1, 0 }, // SMuFL glyph on the G line
        { music_theory::ClefType::Percussion1, -1 },// middle line, middle C an octave higher than for treble
        { music_theory::ClefType::Percussion1, 0 }, // a clef's character is trusted to be a clef glyph, even where Jazz has another glyph at 139
    };
    ASSERT_EQ(clefOptions->clefDefs.size(), expectedValues.size());
    for (size_t x = 0; x < expectedValues.size(); x++) {
        EXPECT_EQ(clefOptions->clefDefs[x]->calcInfo(), expectedValues[x]) << "clef index " << x;
    }
}
