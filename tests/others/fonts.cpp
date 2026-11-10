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

constexpr static musxtest::string_view fontProperties = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <options>
    <fontOptions>
      <font type="music">
        <fontID>13</fontID>
        <fontSize>24</fontSize>
      </font>
      <font type="ending">
        <fontID>1</fontID>
        <fontSize>12</fontSize>
        <efx>
          <bold/>
          <italic/>
          <underline/>
          <strikeout/>
          <absolute/>
          <hidden/>
        </efx>
      </font>
    </fontOptions>
  </options>
  <others>
    <fontName cmper="1">
      <charsetBank>Mac</charsetBank>
      <charsetVal>1</charsetVal>
      <pitch>2</pitch>
      <family>3</family>
      <name>Times</name>
    </fontName>
  </others>
</finale>
    )xml";

constexpr static musxtest::string_view unknownSmuflFontProperties = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <options>
    <fontOptions>
      <font type="music"/>
      <font type="noteheads"/>
      <font type="accis"/>
      <font type="rests"/>
    </fontOptions>
    <musicSymbolOptions>
      <noteheadQuarter>57508</noteheadQuarter>
      <natural>57953</natural>
      <restQuarter>58597</restQuarter>
    </musicSymbolOptions>
  </options>
  <others>
    <fontName cmper="0">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Unknown SMuFL Font</name>
    </fontName>
    <fontName cmper="1">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Unknown Nondefault Font</name>
    </fontName>
    <fontName cmper="2">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Unknown SMuFL Font</name>
    </fontName>
  </others>
</finale>
    )xml";

constexpr static musxtest::string_view windowsMissingFont = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <header>
    <headerData>
      <created>
        <platform>WIN</platform>
      </created>
    </headerData>
  </header>
  <options>
    <fontOptions>
      <font type="music">
        <fontID>13</fontID>
      </font>
    </fontOptions>
  </options>
</finale>
    )xml";

constexpr static musxtest::string_view knownSmuflFontNameSpellings = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <fontName cmper="1">
      <name>Finale Maestro</name>
    </fontName>
    <fontName cmper="2">
      <name>FinaleMaestro</name>
    </fontName>
    <fontName cmper="3">
      <name>finale maestro text</name>
    </fontName>
    <fontName cmper="4">
      <name>BRAVURATEXT</name>
    </fontName>
    <fontName cmper="5">
      <name>Times New Roman</name>
    </fontName>
    <fontName cmper="6">
      <name>Maestro</name>
    </fontName>
  </others>
</finale>
    )xml";

TEST(FontTest, NormalizesFontNamesWithAsciiRules)
{
    EXPECT_EQ(normalizeFontName(" \t\n\v\f\rFinale Maestro"), "finalemaestro");
    EXPECT_EQ(normalizeFontName("Maestro, Times"), "maestro,times");
    EXPECT_EQ(normalizeFontName("ÉLAN"), "Élan");
    EXPECT_EQ(normalizeFontName("finalemaestro"), "finalemaestro");
}

TEST(FontTest, MatchesKnownSmuflFontNamesIgnoringSpacesAndCase)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(knownSmuflFontNameSpellings);

    const auto isSmufl = [&doc](Cmper fontId) {
        auto fontInfo = std::make_shared<FontInfo>(doc);
        fontInfo->fontId = fontId;
        return fontInfo->calcIsSMuFL();
    };

    EXPECT_TRUE(isSmufl(1));    // family name as spelled
    EXPECT_TRUE(isSmufl(2));    // PostScript-style spelling with spaces removed
    EXPECT_TRUE(isSmufl(3));    // lowercase
    EXPECT_TRUE(isSmufl(4));    // uppercase and space-free
    EXPECT_FALSE(isSmufl(5));   // not a music font
    EXPECT_FALSE(isSmufl(6));   // legacy (non-SMuFL) Finale music font
}

TEST(FontTest, FontInfoPropertiesTest)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(fontProperties);
    auto options = doc->getOptions();
    ASSERT_TRUE(options);
    auto fontOptions = options->get<options::FontOptions>();
    ASSERT_TRUE(fontOptions);
    EXPECT_THROW(
        fontOptions->getFontInfo(options::FontOptions::FontType::AbbrvStaffNames),
        std::invalid_argument
    );
    auto fontInfo = fontOptions->getFontInfo(options::FontOptions::FontType::Ending);
    ASSERT_TRUE(fontInfo);
    EXPECT_EQ(fontInfo->fontId, 1);
    EXPECT_EQ(fontInfo->fontSize, 12);
    EXPECT_TRUE(fontInfo->bold);
    EXPECT_TRUE(fontInfo->italic);
    EXPECT_TRUE(fontInfo->underline);
    EXPECT_TRUE(fontInfo->strikeout);
    EXPECT_TRUE(fontInfo->absolute);
    EXPECT_TRUE(fontInfo->hidden);
    EXPECT_EQ(fontInfo->getName(), "Times");
}

TEST(FontTest, FontDefinitionProperties)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(fontProperties);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);
    auto fontDef = others->get<others::FontDefinition>(SCORE_PARTID, 1);
    ASSERT_TRUE(fontDef);
    EXPECT_EQ(fontDef->charsetBank, others::FontDefinition::CharacterSetBank::MacOS);
    EXPECT_EQ(fontDef->charsetVal, 1);
    EXPECT_EQ(fontDef->pitch, 2);
    EXPECT_EQ(fontDef->family, 3);
    EXPECT_EQ(fontDef->name, "Times");
}

TEST(FontTest, FontInfoNoName)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(fontProperties);
    auto fontInfo = options::FontOptions::getFontInfo(doc, options::FontOptions::FontType::Music);
    ASSERT_TRUE(fontInfo);
    EXPECT_EQ(fontInfo->fontId, 13);
    EXPECT_EQ(fontInfo->fontSize, 24);
    EXPECT_FALSE(fontInfo->bold);
    EXPECT_FALSE(fontInfo->italic);
    EXPECT_FALSE(fontInfo->underline);
    EXPECT_FALSE(fontInfo->strikeout);
    EXPECT_FALSE(fontInfo->absolute);
    EXPECT_FALSE(fontInfo->hidden);
    const auto missingFont = doc->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 13);
    ASSERT_TRUE(missingFont);
    EXPECT_EQ(missingFont->charsetBank, others::FontDefinition::CharacterSetBank::MacOS);
    EXPECT_EQ(fontInfo->getName(), "Missing Font (13)");
}

TEST(FontTest, MissingFontUsesWindowsHeaderPlatform)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(windowsMissingFont);
    const auto missingFont = doc->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 13);
    ASSERT_TRUE(missingFont);
    EXPECT_EQ(missingFont->charsetBank, others::FontDefinition::CharacterSetBank::Windows);
}

TEST(FontTest, DetectsUnknownDefaultSmuflFontFromMusicSymbols)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(unknownSmuflFontProperties);
    auto fontOptions = doc->getOptions()->get<options::FontOptions>();
    ASSERT_TRUE(fontOptions);
    auto fontInfo = fontOptions->getFontInfo(options::FontOptions::FontType::Music);
    ASSERT_TRUE(fontInfo);

    EXPECT_TRUE(fontInfo->calcIsSMuFL());
}

TEST(FontTest, DoesNotInferUnknownNondefaultFontFromMusicSymbols)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(unknownSmuflFontProperties);
    auto fontInfo = std::make_shared<FontInfo>(doc);
    fontInfo->fontId = 1;

    EXPECT_FALSE(fontInfo->calcIsSMuFL());
}

TEST(FontTest, DetectsExplicitFontWithDefaultMusicFontName)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(unknownSmuflFontProperties);
    auto fontInfo = std::make_shared<FontInfo>(doc);
    fontInfo->fontId = 2;

    EXPECT_TRUE(fontInfo->calcIsSMuFL());
}

TEST(FontTest, ChangingFontIdUsesSeparateSmuflCacheEntry)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(unknownSmuflFontProperties);
    auto fontInfo = std::make_shared<FontInfo>(doc);
    fontInfo->fontId = 1;
    ASSERT_FALSE(fontInfo->calcIsSMuFL());

    fontInfo->setFontIdByName("Unknown SMuFL Font");

    EXPECT_TRUE(fontInfo->calcIsSMuFL());
}

constexpr static musxtest::string_view defaultMusicFontProperties = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <fontName cmper="0">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Finale Maestro</name>
    </fontName>
    <fontName cmper="12">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Maestro</name>
    </fontName>
    <fontName cmper="13">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Finale Maestro</name>
    </fontName>
  </others>
</finale>
    )xml";

TEST(FontTest, SameTypefaceMatchesDefaultMusicFontIdWithConcreteId)
{
    // Font id 0 is the default music font. The same typeface also exists under a concrete id, which
    // is how Finale records a font option that was set explicitly rather than left on the default.
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(defaultMusicFontProperties);

    auto defaultMusic = std::make_shared<FontInfo>(doc);
    defaultMusic->fontId = 0;
    auto concrete = std::make_shared<FontInfo>(doc);
    concrete->fontId = 13;

    EXPECT_TRUE(defaultMusic->calcIsSameTypeface(*concrete));
    EXPECT_TRUE(concrete->calcIsSameTypeface(*defaultMusic));
}

TEST(FontTest, SameTypefaceRejectsFontPinnedToADifferentTypeface)
{
    // A font option pinned to legacy Maestro keeps that typeface after the document default music
    // font is changed to Finale Maestro.
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(defaultMusicFontProperties);

    auto defaultMusic = std::make_shared<FontInfo>(doc);
    defaultMusic->fontId = 0;
    auto pinned = std::make_shared<FontInfo>(doc);
    pinned->fontId = 12;

    EXPECT_FALSE(defaultMusic->calcIsSameTypeface(*pinned));
}

TEST(FontTest, IsSameComparesTypefaceSizeAndEffects)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(defaultMusicFontProperties);

    auto defaultMusic = std::make_shared<FontInfo>(doc);
    defaultMusic->fontId = 0;
    defaultMusic->fontSize = 24;

    auto concrete = std::make_shared<FontInfo>(doc);
    concrete->fontId = 13;
    concrete->fontSize = 24;

    EXPECT_TRUE(defaultMusic->isSame(*concrete));

    concrete->fontSize = 18;
    EXPECT_FALSE(defaultMusic->isSame(*concrete));

    concrete->fontSize = 24;
    concrete->bold = true;
    EXPECT_FALSE(defaultMusic->isSame(*concrete));
}

TEST(FontTest, SameTypefaceDoesNotCompareNamesOfTwoConcreteIds)
{
    // Only the default music font is duplicated by design. Two distinct non-zero ids that happen to
    // share a name indicate a malformed document and are not treated as the same typeface.
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(unknownSmuflFontProperties);

    // Ids 0 and 2 both name "Unknown SMuFL Font"; id 1 names a different font.
    auto sentinel = std::make_shared<FontInfo>(doc);
    sentinel->fontId = 0;
    auto duplicate = std::make_shared<FontInfo>(doc);
    duplicate->fontId = 2;
    auto other = std::make_shared<FontInfo>(doc);
    other->fontId = 1;

    EXPECT_TRUE(sentinel->calcIsSameTypeface(*duplicate));
    EXPECT_FALSE(other->calcIsSameTypeface(*duplicate));
}

constexpr static musxtest::string_view importTargetProperties = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <fontName cmper="0">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Jazz</name>
    </fontName>
    <fontName cmper="3">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Times New Roman</name>
    </fontName>
  </others>
</finale>
    )xml";

constexpr static musxtest::string_view importSourceProperties = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <fontName cmper="0">
      <charsetBank>Win</charsetBank>
      <charsetVal>2</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Maestro</name>
    </fontName>
    <fontName cmper="1">
      <charsetBank>Win</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>1</pitch>
      <family>2</family>
      <name>  Times   New Roman </name>
    </fontName>
    <fontName cmper="4">
      <charsetBank>Win</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Seville</name>
    </fontName>
  </others>
</finale>
    )xml";

TEST(FontTest, ImportFontDefinitionMatchesByNameNotCmper)
{
    auto target = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importTargetProperties);
    auto source = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importSourceProperties);
    const auto before = target->getOthers()->getArray<others::FontDefinition>(SCORE_PARTID).size();

    auto times = source->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 1);
    ASSERT_TRUE(times);
    // The same typeface sits at cmper 1 in the source and 3 in the target, and the source spells
    // it with extra whitespace, so only a normalized comparison finds it.
    EXPECT_EQ(importFontDefinitionInto(target, times), std::optional<Cmper>(Cmper(3)));
    // Matching reuses what is there; nothing is added.
    EXPECT_EQ(target->getOthers()->getArray<others::FontDefinition>(SCORE_PARTID).size(), before);
}

TEST(FontTest, ImportFontDefinitionClonesWhenAbsentAndKeepsSpelling)
{
    auto target = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importTargetProperties);
    auto source = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importSourceProperties);

    auto seville = source->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 4);
    ASSERT_TRUE(seville);
    const auto imported = importFontDefinitionInto(target, seville);
    ASSERT_TRUE(imported);
    EXPECT_EQ(*imported, 4); // one past the target's highest existing cmper, which is 3

    auto added = target->getOthers()->get<others::FontDefinition>(SCORE_PARTID, *imported);
    ASSERT_TRUE(added);
    EXPECT_EQ(added->name, "Seville");
    EXPECT_EQ(added->charsetBank, others::FontDefinition::CharacterSetBank::Windows);
    EXPECT_EQ(added->charsetVal, 0);

    // A second import of the same typeface finds the copy rather than making another.
    EXPECT_EQ(importFontDefinitionInto(target, seville), imported);
}

TEST(FontTest, ImportFontDefinitionPassesZeroThroughWithoutMatchingByName)
{
    auto target = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importTargetProperties);
    auto source = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importSourceProperties);

    auto music = source->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0);
    ASSERT_TRUE(music);
    // Id 0 names the destination's own music font. Returning the source's typeface, or matching
    // "Maestro" into the target, would change what every element storing 0 renders as.
    EXPECT_EQ(importFontDefinitionInto(target, music), std::optional<Cmper>(Cmper(0)));
    auto zero = target->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0);
    ASSERT_TRUE(zero);
    EXPECT_EQ(zero->name, "Jazz");
}

TEST(FontTest, ImportFontDefinitionCreatesZeroWhenTargetLacksIt)
{
    auto target = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(fontProperties);
    auto source = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importSourceProperties);

    // This target defines only cmper 1, so the returned id would otherwise name nothing.
    EXPECT_FALSE(target->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0));

    auto music = source->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0);
    ASSERT_TRUE(music);
    EXPECT_EQ(importFontDefinitionInto(target, music), std::optional<Cmper>(Cmper(0)));

    auto created = target->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0);
    ASSERT_TRUE(created);
    EXPECT_EQ(created->name, "Maestro");
}

constexpr static musxtest::string_view targetWithMusicFontButNoZero = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <options>
    <fontOptions>
      <font type="music">
        <fontID>5</fontID>
        <fontSize>24</fontSize>
      </font>
    </fontOptions>
  </options>
  <others>
    <fontName cmper="5">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Broadway Copyist</name>
    </fontName>
  </others>
</finale>
    )xml";

TEST(FontTest, ImportFontDefinitionTakesZeroFromTargetsOwnMusicFont)
{
    auto target = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(targetWithMusicFontButNoZero);
    auto source = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importSourceProperties);

    auto music = source->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0);
    ASSERT_TRUE(music);
    EXPECT_EQ(music->name, "Maestro");

    EXPECT_EQ(importFontDefinitionInto(target, music), std::optional<Cmper>(Cmper(0)));

    // The target's own FontOptions name Broadway Copyist as its music font, so that is what id 0
    // stands for here. Taking the source's Maestro would have changed the typeface of everything
    // in the target that stores 0.
    auto created = target->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0);
    ASSERT_TRUE(created);
    EXPECT_EQ(created->name, "Broadway Copyist");
}

TEST(FontTest, ImportFontDefinitionFallbackAlsoGivesTheTypefaceANonZeroCmper)
{
    // This target has no FontOptions, so it cannot say what its music font is, and no definition
    // at 0. The source's own default music font is the only remaining answer.
    auto target = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(fontProperties);
    auto source = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importSourceProperties);
    EXPECT_FALSE(target->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0));

    auto music = source->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0);
    ASSERT_TRUE(music);
    EXPECT_EQ(importFontDefinitionInto(target, music), std::optional<Cmper>(Cmper(0)));

    auto created = target->getOthers()->get<others::FontDefinition>(SCORE_PARTID, 0);
    ASSERT_TRUE(created);
    EXPECT_EQ(created->name, "Maestro");

    // Id 0 tracks whatever the music font later becomes and is never selected by matching, so the
    // typeface must also be addressable under a concrete cmper.
    auto fonts = target->getOthers()->getArray<others::FontDefinition>(SCORE_PARTID);
    const auto named = std::count_if(fonts.begin(), fonts.end(),
        [](const MusxInstance<others::FontDefinition>& font) {
            return font->getCmper() != 0 && normalizeFontName(font->name) == "maestro";
        });
    EXPECT_EQ(named, 1);
}

TEST(FontTest, GetFontInfoOrNullReportsAbsenceInsteadOfThrowing)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(fontProperties);
    auto fontOptions = doc->getOptions()->get<options::FontOptions>();
    ASSERT_TRUE(fontOptions);

    // Present: both forms agree with the throwing accessor.
    auto music = fontOptions->getFontInfoOrNull(options::FontOptions::FontType::Music);
    ASSERT_TRUE(music);
    EXPECT_EQ(music->fontId, 13);
    EXPECT_EQ(options::FontOptions::getFontInfoOrNull(doc, options::FontOptions::FontType::Music)->fontId, 13);
    EXPECT_EQ(fontOptions->getFontInfo(options::FontOptions::FontType::Music)->fontId, 13);

    // Absent: null rather than an exception, while the throwing accessor still throws.
    EXPECT_FALSE(fontOptions->getFontInfoOrNull(options::FontOptions::FontType::Tablature));
    EXPECT_FALSE(options::FontOptions::getFontInfoOrNull(doc, options::FontOptions::FontType::Tablature));
    EXPECT_THROW(static_cast<void>(fontOptions->getFontInfo(options::FontOptions::FontType::Tablature)),
        std::invalid_argument);
}

TEST(FontTest, GetFontInfoOrNullHandlesADocumentWithNoFontOptions)
{
    // A document whose options pool has no FontOptions at all must not throw from the static form.
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(importTargetProperties);
    EXPECT_FALSE(doc->getOptions()->get<options::FontOptions>());
    EXPECT_FALSE(options::FontOptions::getFontInfoOrNull(doc, options::FontOptions::FontType::Music));
}

constexpr static musxtest::string_view drawnCharacterFontProperties = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <fontName cmper="1">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Maestro</name>
    </fontName>
    <fontName cmper="2">
      <charsetBank>Win</charsetBank>
      <charsetVal>2</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Jazz</name>
    </fontName>
    <fontName cmper="3">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Finale Maestro</name>
    </fontName>
    <fontName cmper="4">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Times</name>
    </fontName>
  </others>
</finale>
)xml";

TEST(FontTest, DrawnCharacterRemovesWindowsSymbolOffset)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(drawnCharacterFontProperties);
    const auto fontWithId = [&](Cmper fontId) {
        auto fontInfo = std::make_shared<FontInfo>(doc);
        fontInfo->fontId = fontId;
        return fontInfo;
    };
    constexpr char32_t windowsWhole = 0xF077;
    constexpr char32_t windowsFirst = 0xF020;
    constexpr char32_t windowsLast = 0xF0FF;

    // A non-SMuFL symbol font draws the character without the offset, whichever platform recorded the font.
    const auto macSymbolFont = fontWithId(1);
    EXPECT_EQ(macSymbolFont->calcDrawnCharacter(windowsWhole), U'w');
    EXPECT_EQ(macSymbolFont->calcDrawnCharacter(windowsFirst), U' ');
    EXPECT_EQ(macSymbolFont->calcDrawnCharacter(windowsLast), char32_t(0xFF));
    EXPECT_EQ(macSymbolFont->calcDrawnCharacter(U'w'), U'w');
    EXPECT_EQ(macSymbolFont->calcDrawnCharacter(windowsFirst - 1), windowsFirst - 1);
    EXPECT_EQ(macSymbolFont->calcDrawnCharacter(windowsLast + 1), windowsLast + 1);
    EXPECT_EQ(fontWithId(2)->calcDrawnCharacter(windowsWhole), U'w');

    // A SMuFL font and a text font keep every character as stored.
    EXPECT_EQ(fontWithId(3)->calcDrawnCharacter(windowsWhole), windowsWhole);
    EXPECT_EQ(fontWithId(4)->calcDrawnCharacter(windowsWhole), windowsWhole);
}
