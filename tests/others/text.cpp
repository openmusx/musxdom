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

constexpr static musxtest::string_view textXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <header>
    <headerData>
      <wordOrder>lo-endian</wordOrder>
      <textEncoding>Mac</textEncoding>
      <created>
        <year>2025</year>
        <month>6</month>
        <day>30</day>
        <modifiedBy/>
        <enigmaVersion>
          <major>27</major>
          <minor>4</minor>
          <devStatus>dev</devStatus>
        </enigmaVersion>
        <application>FIN</application>
        <platform>MAC</platform>
        <appVersion>
          <major>27</major>
          <minor>4</minor>
          <maint>1</maint>
          <devStatus>dev</devStatus>
          <build>146</build>
        </appVersion>
        <fileVersion>
          <major>27</major>
          <minor>4</minor>
          <devStatus>dev</devStatus>
        </fileVersion>
        <appRegion>US</appRegion>
      </created>
      <modified>
        <year>2025</year>
        <month>6</month>
        <day>30</day>
        <modifiedBy/>
        <enigmaVersion>
          <major>27</major>
          <minor>4</minor>
          <devStatus>dev</devStatus>
        </enigmaVersion>
        <application>FIN</application>
        <platform>MAC</platform>
        <appVersion>
          <major>27</major>
          <minor>4</minor>
          <maint>1</maint>
          <devStatus>dev</devStatus>
          <build>146</build>
        </appVersion>
        <fileVersion>
          <major>27</major>
          <minor>4</minor>
          <devStatus>dev</devStatus>
        </fileVersion>
        <appRegion>US</appRegion>
      </modified>
    </headerData>
  </header>
  <others>
    <fontName cmper="0">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Finale Maestro</name>
    </fontName>
    <fontName cmper="1">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Times</name>
    </fontName>
    <fontName cmper="2">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Helvetica</name>
    </fontName>
    <fontName cmper="3">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Seville</name>
    </fontName>
    <fontName cmper="4">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Jazz</name>
    </fontName>
    <fontName cmper="5">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Maestro Percussion</name>
    </fontName>
    <fontName cmper="6">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>JazzPerc</name>
    </fontName>
    <fontName cmper="7">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Broadway Copyist</name>
    </fontName>
    <fontName cmper="8">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Finale AlphaNotes</name>
    </fontName>
    <fontName cmper="9">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Times New Roman</name>
    </fontName>
    <fontName cmper="10">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Menlo</name>
    </fontName>
    <fontName cmper="11">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>New York</name>
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
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Finale Maestro</name>
    </fontName>
    <fontName cmper="14">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Patmm</name>
    </fontName>
    <fontName cmper="15">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Broadway Copyist Perc</name>
    </fontName>
    <fontName cmper="16">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>JazzText</name>
    </fontName>
    <fontName cmper="17">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>GraceNotes</name>
    </fontName>
    <fontName cmper="18">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Garamond</name>
    </fontName>
    <fontName cmper="19">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Bravura</name>
    </fontName>
    <fontName cmper="20">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Arial</name>
    </fontName>
    <fontName cmper="21">
      <charsetBank>Mac</charsetBank>
      <charsetVal>4095</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Petrucci</name>
    </fontName>
    <fontName cmper="22">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Palatino</name>
    </fontName>
    <fontName cmper="23">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Finale Lyrics</name>
    </fontName>
    <fontName cmper="27">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Geneva</name>
    </fontName>
    <fontName cmper="28">
      <charsetBank>Mac</charsetBank>
      <charsetVal>0</charsetVal>
      <pitch>0</pitch>
      <family>0</family>
      <name>Lucida Grande</name>
    </fontName>
  </others>)xml" // MSC doesn't like long string literals
  R"xml(<texts>
    <fileInfo type="title">My Piece</fileInfo>
    <fileInfo type="composer">L. BEETHOVEN</fileInfo>
    <fileInfo type="copyright">1823</fileInfo>
    <fileInfo type="description">GRM–####</fileInfo>
    <fileInfo type="lyricist">Johnny Göthe</fileInfo>
    <fileInfo type="arranger">Ferrucio Busoni</fileInfo>
    <fileInfo type="subtitle">Subtitle</fileInfo>
    <verse number="1">^fontid(23)^size(12)^nfx(0)The first verse.</verse>
    <chorus number="1">^fontid(11)^size(12)^nfx(0)The first chorus.</chorus>
    <section number="1">^fontid(11)^size(12)^nfx(0)The first sec-tion.</section>
    <blockText number="1">^fontid(9)^size(12)^nfx(0)SCORE</blockText>
    <blockText number="17">^fontid(9)^size(12)^nfx(0)^page(0)</blockText>
    <blockText number="18">^fontid(9)^size(12)^nfx(0)^title()</blockText>
    <blockText number="20">^fontid(9)^size(12)^nfx(0)^arranger()</blockText>
    <blockText number="21">^fontid(9)^size(18)^nfx(0)^subtitle()</blockText>
    <blockText number="22">^fontid(9)^size(12)^nfx(128)(SMuFL Default file for Beta Testing)</blockText>
    <blockText number="23">^fontid(0)^size(24)^nfx(0)</blockText>
    <blockText number="24">^fontid(1)^size(12)^nfx(67)^page(0)</blockText>
    <blockText number="25">^fontid(22)^size(8)^nfx(64)^description()</blockText>
    <blockText number="26">^fontid(1)^size(12)^nfx(1)^partname()</blockText>
    <blockText number="27">^fontid(1)^size(24)^nfx(65)^title()
^size(10)^nfx(66)for</blockText>
    <blockText number="28">^fontid(1)^size(12)^nfx(65)^composer() (^copyright())</blockText>
    <blockText number="29">^fontid(1)^size(9)^nfx(64)Licensed by ASCAP
One Lincoln Plaza
New York, NY  10023</blockText>
    <blockText number="30">^fontid(1)^size(9)^nfx(64)©^copyright() Great River Music
All rights reserved.
www.greatrivermusic.com</blockText>
    <blockText number="31">^fontid(1)^size(12)^nfx(1)^partname()</blockText>
    <blockText number="32">^fontid(1)^size(14)^nfx(2)Text block</blockText>
    <smartShapeText number="1">^fontid(9)^size(12)^nfx(2)Glissando</smartShapeText>
    <smartShapeText number="2">^fontid(1)^size(12)^nfx(2)Gliss.</smartShapeText>
    <smartShapeText number="3">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="4">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="5">^fontid(9)^size(12)^nfx(2)rit.</smartShapeText>
    <smartShapeText number="6">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="7">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="8">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="13">^fontid(2)^size(10)^nfx(0)A.H.</smartShapeText>
    <smartShapeText number="14">^fontid(2)^size(10)^nfx(0)A.H.</smartShapeText>
    <smartShapeText number="15">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="16">^fontid(2)^size(10)^nfx(0)Artificial Harmonic</smartShapeText>
    <smartShapeText number="17">^fontid(2)^size(10)^nfx(0)A.H.</smartShapeText>
    <smartShapeText number="18">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="19">^fontid(2)^size(10)^nfx(0)N.H.</smartShapeText>
    <smartShapeText number="20">^fontid(2)^size(10)^nfx(0)N.H.</smartShapeText>
    <smartShapeText number="21">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="22">^fontid(2)^size(10)^nfx(0)Natural Harmonic</smartShapeText>
    <smartShapeText number="23">^fontid(2)^size(10)^nfx(0)N.H.</smartShapeText>
    <smartShapeText number="24">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="25">^fontid(2)^size(10)^nfx(0)P.H.</smartShapeText>
    <smartShapeText number="26">^fontid(2)^size(10)^nfx(0)P.H.</smartShapeText>
    <smartShapeText number="27">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="28">^fontid(2)^size(10)^nfx(0)Pick Harmonics</smartShapeText>
    <smartShapeText number="29">^fontid(2)^size(10)^nfx(0)P.H.</smartShapeText>
    <smartShapeText number="30">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="31">^fontid(2)^size(10)^nfx(0)P.M.</smartShapeText>
    <smartShapeText number="32">^fontid(2)^size(10)^nfx(0)P.M.</smartShapeText>
    <smartShapeText number="33">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="34">^fontid(2)^size(10)^nfx(0)Palm Mute</smartShapeText>
    <smartShapeText number="35">^fontid(2)^size(10)^nfx(0)P.M.</smartShapeText>
    <smartShapeText number="36">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="37">^fontid(2)^size(10)^nfx(0)let ring</smartShapeText>
    <smartShapeText number="38">^fontid(2)^size(10)^nfx(0)let ring</smartShapeText>
    <smartShapeText number="39">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="40">^fontid(2)^size(10)^nfx(0)hold bend</smartShapeText>
    <smartShapeText number="41">^fontid(2)^size(10)^nfx(0)hold bend</smartShapeText>
    <smartShapeText number="42">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="43">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="44">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="45">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="46">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="47">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="48">^fontid(2)^size(6)^nfx(0)|</smartShapeText>
    <smartShapeText number="49">^fontid(9)^size(12)^nfx(2)rall.</smartShapeText>
    <smartShapeText number="50">^fontid(9)^size(12)^nfx(2)accel.</smartShapeText>
    <smartShapeText number="51">^fontid(9)^size(12)^nfx(2)cresc.</smartShapeText>
    <smartShapeText number="52">^fontid(9)^size(12)^nfx(2)dim.


</smartShapeText>
    <smartShapeText number="53">^fontid(9)^size(12)^nfx(2)decresc.</smartShapeText>
    <smartShapeText number="54">^fontid(2)^size(10)^nfx(0)H</smartShapeText>
    <smartShapeText number="55">^fontid(2)^size(10)^nfx(0)P</smartShapeText>
    <smartShapeText number="56">^fontid(2)^size(10)^nfx(0)B</smartShapeText>
    <smartShapeText number="57">^fontid(2)^size(10)^nfx(0)R</smartShapeText>
    <smartShapeText number="58">^fontid(2)^size(10)^nfx(0)S</smartShapeText>
    <smartShapeText number="59">^fontid(2)^size(10)^nfx(0)T</smartShapeText>
    <smartShapeText number="60">^fontid(13)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="61">^fontid(13)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="62">^fontid(13)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="63">^fontid(13)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="64">^fontid(0)^size(24)^nfx(0)</smartShapeText>
    <smartShapeText number="65">^fontid(0)^size(24)^nfx(0)^baseline(-12)^baseline(0)^baseline(-12)</smartShapeText>
    <smartShapeText number="66">^fontid(27)^size(8)^nfx(0)^baseline(-20)|</smartShapeText>
    <expression number="1">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="2">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="3">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="4">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="5">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="6">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="7">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="8">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="9">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="10">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="11">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="12">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="13">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="14">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="15">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="16">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="17">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="18">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="19">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="20">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="21">^fontTxt(Times New Roman,4096)^size(14)^nfx(2)subito ^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="22">^fontTxt(Times New Roman,4096)^size(14)^nfx(2)sub. ^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="150">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="180">^fontTxt(Patmm,4096)^size(14)^nfx(0)Tempo (∞=^value())</expression>
    <expression number="192">^fontTxt(Garamond,4096)^size(30)^nfx(1)^rehearsal()</expression>
    <expression number="193">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="194">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="195">^fontTxt(Times,4096)^size(10)^nfx(130)igpat</expression>
    <expression number="196">^fontTxt(Times,4096)^size(10)^nfx(130)skbeam</expression>
    <expression number="197">^fontTxt(Times,4096)^size(10)^nfx(130)skstem</expression>
    <expression number="199">^font(Times,4096)^size(14)^nfx(2)Expressions in this category are used by the Patterson Plugins Collection as a data store within your document. Please ignore them.</expression>
    <expression number="201">^fontTxt(Times,4096)^size(10)^nfx(130)skip spacing</expression>
    <expression number="203">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="204">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="205">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="206">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="207">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="208">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="209">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="210">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="211">^font(Times,4096)^size(14)^nfx(1)G.P.</expression>
    <expression number="212">^fontTxt(Times,4096)^size(10)^nfx(130)match score system</expression>
    <expression number="213">^fontTxt(Times,4096)^size(10)^nfx(130)score spacing</expression>
    <expression number="214">^fontTxt(Times,4096)^size(10)^nfx(130)igrc</expression>
    <expression number="215">^fontTxt(Times,4096)^size(10)^nfx(130)iclefs</expression>
    <expression number="216">^fontTxt(Times,4096)^size(12)^nfx(130)skip ties</expression>
    <expression number="217">bad ^fontTxt(Times,4096)^size(10)^nfx(130)skip tie ends</expression>
    <expression number="218">^fontTxt(TimesXXX,4096)^size(10)^nfx(130)skip trill-to</expression>
    <expression number="219">^font(Times,4096)^size(14)^nfx(2)22100:0: 0E000F001D000000EAFF0000</expression>
    <expression number="221">^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="222">^fontTxt(Times New Roman,4096)^size(14)^nfx(2)più ^fontMus(Font0,0)^size(24)^nfx(0)</expression>
    <expression number="223">^fontMus(Font0,0)^size(24)^nfx(0)^fontTxt(Times New Roman,4096)^size(14)^nfx(2) sempre</expression>
    <expression number="224">^fontMus(Font0,0)^size(24)^nfx(0)^fontTxt(Times New Roman,4096)^size(14)^nfx(2) sempre</expression>
    <bookmarkText number="1">Bookmark to Bar 12</bookmarkText>
  </texts>
</finale>
    )xml";

TEST(TextsTest, FileInfoText)
{
    using texts::FileInfoText;
    using Type = texts::FileInfoText::TextType;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    // Test Title
    auto fileInfo = texts->get<FileInfoText>(Cmper(Type::Title));
    ASSERT_TRUE(fileInfo);
    EXPECT_EQ(fileInfo->text, "My Piece");
    EXPECT_EQ(fileInfo->getTextType(), Type::Title);

    // Test Composer
    fileInfo = texts->get<FileInfoText>(Cmper(Type::Composer));
    ASSERT_TRUE(fileInfo);
    EXPECT_EQ(fileInfo->text, "L. BEETHOVEN");
    EXPECT_EQ(fileInfo->getTextType(), Type::Composer);

    // Test Copyright
    fileInfo = texts->get<FileInfoText>(Cmper(Type::Copyright));
    ASSERT_TRUE(fileInfo);
    EXPECT_EQ(fileInfo->text, "1823");
    EXPECT_EQ(fileInfo->getTextType(), Type::Copyright);

    // Test Description
    fileInfo = texts->get<FileInfoText>(Cmper(Type::Description));
    ASSERT_TRUE(fileInfo);
    EXPECT_EQ(fileInfo->text, "GRM–####");
    EXPECT_EQ(fileInfo->getTextType(), Type::Description);

    // Test Lyricist
    fileInfo = texts->get<FileInfoText>(Cmper(Type::Lyricist));
    ASSERT_TRUE(fileInfo);
    EXPECT_EQ(fileInfo->text, "Johnny Göthe");
    EXPECT_EQ(fileInfo->getTextType(), Type::Lyricist);

    // Test Arranger
    fileInfo = texts->get<FileInfoText>(Cmper(Type::Arranger));
    ASSERT_TRUE(fileInfo);
    EXPECT_EQ(fileInfo->text, "Ferrucio Busoni");
    EXPECT_EQ(fileInfo->getTextType(), Type::Arranger);

    // Test Subtitle
    fileInfo = texts->get<FileInfoText>(Cmper(Type::Subtitle));
    ASSERT_TRUE(fileInfo);
    EXPECT_EQ(fileInfo->text, "Subtitle");
    EXPECT_EQ(fileInfo->getTextType(), Type::Subtitle);

    // Test array
    EXPECT_EQ(texts->getArray<FileInfoText>().size(), 7);
    ASSERT_EQ(texts->getArray<FileInfoText>(Cmper(Type::Arranger)).size(), 1);
    EXPECT_EQ(texts->getArray<FileInfoText>(Cmper(Type::Arranger))[0]->getTextType(), Type::Arranger);
    EXPECT_EQ(texts->getArray<FileInfoText>(8).size(), 0);

    // Test ctor exception
    EXPECT_THROW(
        auto malformed = FileInfoText(doc, 8), // invalid text type
        std::invalid_argument
    );
}

TEST(TextsTest, LyricsText)
{
    using texts::LyricsVerse;
    using texts::LyricsChorus;
    using texts::LyricsSection;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    // Test Verse
    auto lyricsVerse = texts->get<LyricsVerse>(1);
    ASSERT_TRUE(lyricsVerse);
    EXPECT_EQ(lyricsVerse->text, "^fontid(23)^size(12)^nfx(0)The first verse.");
    EXPECT_FALSE(texts->get<LyricsVerse>(2));
    ASSERT_EQ(texts->getArray<LyricsVerse>().size(), 1);
    EXPECT_EQ(texts->getArray<LyricsVerse>()[0]->getTextNumber(), 1);
    EXPECT_EQ(texts->getArray<LyricsVerse>(2).size(), 0);

    // Test Chorus
    auto lyricsChorus = texts->get<LyricsChorus>(1);
    ASSERT_TRUE(lyricsChorus);
    EXPECT_EQ(lyricsChorus->text, "^fontid(11)^size(12)^nfx(0)The first chorus.");
    EXPECT_FALSE(texts->get<LyricsChorus>(2));
    ASSERT_EQ(texts->getArray<LyricsChorus>().size(), 1);
    EXPECT_EQ(texts->getArray<LyricsChorus>()[0]->getTextNumber(), 1);
    EXPECT_EQ(texts->getArray<LyricsChorus>(2).size(), 0);

    // Test Section
    auto lyricsSection = texts->get<LyricsSection>(1);
    ASSERT_TRUE(lyricsSection);
    EXPECT_EQ(lyricsSection->text, "^fontid(11)^size(12)^nfx(0)The first sec-tion.");
    EXPECT_FALSE(texts->get<LyricsSection>(2));
    ASSERT_EQ(texts->getArray<LyricsSection>().size(), 1);
    EXPECT_EQ(texts->getArray<LyricsSection>()[0]->getTextNumber(), 1);
    EXPECT_EQ(texts->getArray<LyricsSection>(2).size(), 0);
}

TEST(TextsTest, OtherText)
{
    using texts::BlockText;
    using texts::SmartShapeText;
    using texts::ExpressionText;
    using texts::BookmarkText;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    // Test block text
    auto blockText = texts->get<BlockText>(23);
    ASSERT_TRUE(blockText);
    EXPECT_EQ(blockText->text, "^fontid(0)^size(24)^nfx(0)");
    EXPECT_FALSE(texts->get<BlockText>(15));
    EXPECT_FALSE(texts->get<BlockText>(33));
    ASSERT_EQ(texts->getArray<BlockText>().size(), 16);
    EXPECT_EQ(texts->getArray<BlockText>()[12]->getTextNumber(), 29);
    EXPECT_EQ(texts->getArray<BlockText>(2).size(), 0);

    // Test smart shape text
    auto ssText = texts->get<SmartShapeText>(2);
    ASSERT_TRUE(ssText);
    EXPECT_EQ(ssText->text, "^fontid(1)^size(12)^nfx(2)Gliss.");
    EXPECT_FALSE(texts->get<SmartShapeText>(9));
    ASSERT_EQ(texts->getArray<SmartShapeText>().size(), 62);
    EXPECT_EQ(texts->getArray<SmartShapeText>()[8]->getTextNumber(), 13);
    EXPECT_EQ(texts->getArray<SmartShapeText>(10).size(), 0);

    // Test Section
    auto expText = texts->get<ExpressionText>(3);
    ASSERT_TRUE(expText);
    EXPECT_EQ(expText->text, "^fontMus(Font0,0)^size(24)^nfx(0)");
    EXPECT_FALSE(texts->get<ExpressionText>(101));
    ASSERT_EQ(texts->getArray<ExpressionText>().size(), 53);
    EXPECT_EQ(texts->getArray<ExpressionText>()[0]->getTextNumber(), 1);
    EXPECT_EQ(texts->getArray<ExpressionText>(225).size(), 0);

    // Test bookmark text
    auto bookmark = texts->get<BookmarkText>(1);
    ASSERT_TRUE(bookmark);
    EXPECT_EQ(bookmark->text, "Bookmark to Bar 12");
    EXPECT_FALSE(texts->get<BookmarkText>(2));
    ASSERT_EQ(texts->getArray<BookmarkText>().size(), 1);
    EXPECT_EQ(texts->getArray<BookmarkText>()[0]->getTextNumber(), 1);
    EXPECT_EQ(texts->getArray<BookmarkText>(2).size(), 0);
}

TEST(TextsText, EmbeddedNewLine)
{
    using texts::SmartShapeText;

    {
        auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(textXml);
        auto texts = doc->getTexts();
        ASSERT_TRUE(texts);
        auto text = texts->get<SmartShapeText>(52);
        ASSERT_TRUE(text);
        ASSERT_FALSE(text->text.empty());
        EXPECT_EQ(text->text[text->text.size() - 1], '\n') << "trailing newlines not preserved in tinyxml2";
    }

    {
        auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(textXml);
        auto texts = doc->getTexts();
        ASSERT_TRUE(texts);
        auto text = texts->get<SmartShapeText>(52);
        ASSERT_TRUE(text);
        ASSERT_FALSE(text->text.empty());
        EXPECT_EQ(text->text[text->text.size() - 1], '\n') << "trailing newlines not preserved in rapidxml";
    }

    {
        auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(textXml);
        auto texts = doc->getTexts();
        ASSERT_TRUE(texts);
        auto text = texts->get<SmartShapeText>(52);
        ASSERT_TRUE(text);
        ASSERT_FALSE(text->text.empty());
        EXPECT_EQ(text->text[text->text.size() - 1], '\n') << "trailing newlines not preserved in pugi";
    }
}

TEST(TextsTest, EnigmaComponents)
{
    constexpr auto constexpr_strlen = [](const auto& str) constexpr {
        return std::size(str) - 1;
    };

    size_t parsedSize = 0;
    auto components = musx::util::EnigmaString::parseComponents("^fontTxt(Times,4096)text", &parsedSize);
    EXPECT_EQ(components, std::vector<std::string>({ "fontTxt", "Times", "4096" }));
    EXPECT_EQ(parsedSize, constexpr_strlen("^fontTxt(Times,4096)"));

    components = musx::util::EnigmaString::parseComponents("^fontTxt((Times),(4096))", &parsedSize);
    EXPECT_EQ(components, std::vector<std::string>({ "fontTxt", "(Times)", "(4096)" }));
    EXPECT_EQ(parsedSize, constexpr_strlen("^fontTxt((Times),(4096))"));

    components = musx::util::EnigmaString::parseComponents("^size(10)size text", &parsedSize);
    EXPECT_EQ(components, std::vector<std::string>({ "size", "10" }));
    EXPECT_EQ(parsedSize, constexpr_strlen("^size(10)"));

    components = musx::util::EnigmaString::parseComponents("^some text", &parsedSize);
    EXPECT_EQ(components, std::vector<std::string>({ "some" }));
    EXPECT_EQ(parsedSize, constexpr_strlen("^some"));

    components = musx::util::EnigmaString::parseComponents("^^text", &parsedSize);
    EXPECT_EQ(components, std::vector<std::string>({ "^" }));
    EXPECT_EQ(parsedSize, constexpr_strlen("^^"));

    components = musx::util::EnigmaString::parseComponents("^fontTxt(Times(bad,4096)", &parsedSize);
    EXPECT_EQ(components, std::vector<std::string>({}));
    EXPECT_EQ(parsedSize, 0);
}

TEST(TextsTest, FontFromEnigma)
{
    using texts::ExpressionText;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    auto text = texts->get<ExpressionText>(216);
    ASSERT_TRUE(text);
    auto font = text->getRawTextCtx(text, SCORE_PARTID).parseFirstFontInfo();
    EXPECT_EQ(font->fontId, 1);
    EXPECT_EQ(font->fontSize, 12);
    EXPECT_FALSE(font->bold);
    EXPECT_TRUE(font->italic);
    EXPECT_FALSE(font->underline);
    EXPECT_FALSE(font->strikeout);
    EXPECT_FALSE(font->absolute);
    EXPECT_TRUE(font->hidden);

    text = texts->get<ExpressionText>(217);
    ASSERT_TRUE(text);
    font = text->getRawTextCtx(text, SCORE_PARTID).parseFirstFontInfo();
    EXPECT_FALSE(font);

    text = texts->get<ExpressionText>(218);
    ASSERT_TRUE(text);
    EXPECT_THROW(
        text->getRawTextCtx(text, SCORE_PARTID).parseFirstFontInfo(),
        std::invalid_argument
    );
}

TEST(TextsTest, ResolveEnigmaStyles)
{
    using texts::ExpressionText;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    auto text = texts->get<ExpressionText>(216);
    ASSERT_TRUE(text);

    std::vector<musx::util::EnigmaResolvedTextChunk> chunks;
    text->getRawTextCtx(text, SCORE_PARTID).parseEnigmaText([&](const std::string& chunkText, const musx::util::EnigmaStyles& styles) {
        chunks.push_back(musx::util::EnigmaTextChunk{ chunkText, styles }.resolve());
        return true;
    });
    doc.reset(); // the snapshot must not depend on the document

    ASSERT_EQ(chunks.size(), 1u);
    const auto& chunk = chunks.front();
    EXPECT_EQ(chunk.text, "skip ties");
    EXPECT_EQ(chunk.styles.font.name, "Times");
    EXPECT_EQ(chunk.styles.font.size, 12);
    EXPECT_FALSE(chunk.styles.font.sizeIsPercent);
    EXPECT_FALSE(chunk.styles.font.bold);
    EXPECT_TRUE(chunk.styles.font.italic);
    EXPECT_FALSE(chunk.styles.font.underline);
    EXPECT_FALSE(chunk.styles.font.strikeout);
    EXPECT_FALSE(chunk.styles.font.absolute);
    EXPECT_TRUE(chunk.styles.font.hidden);
    EXPECT_FALSE(chunk.styles.font.isSymbolFont);
    EXPECT_FALSE(chunk.styles.font.isSmufl);
    EXPECT_EQ(chunk.styles.categoryFont, musx::util::EnigmaStyles::CategoryTracking::TextFont);
    EXPECT_EQ(chunk.styles.baseline, 0);
    EXPECT_EQ(chunk.styles.superscript, 0);
    EXPECT_EQ(chunk.styles.tracking, 0);
}

TEST(TextsTest, ParseEnigmaTextLowLevel)
{
    using namespace musx::util;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    std::vector<std::pair<std::string, std::string>> output;

    auto recordChunk = [&](const std::string& text, const musx::util::EnigmaStyles&) -> bool {
        output.emplace_back("TEXT", text);
        return true;
    };

    auto handleNothing = [](const std::vector<std::string>&) -> std::optional<std::string> {
        return std::nullopt;
    };

    // Test 1: Escaped caret
    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "A ^^ B", recordChunk, handleNothing);
    std::vector<std::pair<std::string, std::string>> expected1 = {
        {"TEXT", "A ^ B"}
    };
    EXPECT_EQ(output, expected1);

    // Test 2: Unhandled command dumped raw
    output.clear();
    EnigmaString::EnigmaParsingOptions options;
    options.stripUnknownTags = false;
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^font(Times)X ^size(12)^foo(bar)^nfx(1) Y", recordChunk, handleNothing, options);
    std::vector<std::pair<std::string, std::string>> expected2 = {
        {"TEXT", "X "},
        {"TEXT", "^foo(bar)"},
        {"TEXT", " Y"}
    };
    EXPECT_EQ(output, expected2);

    // Test 3: A url command is stripped independently of the unknown-command policy
    output.clear();
    EnigmaString::EnigmaParsingOptions urlOptions;
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "Before ^url(0) after", recordChunk, handleNothing, urlOptions);
    std::vector<std::pair<std::string, std::string>> expectedUrlStripped = {
        {"TEXT", "Before  after"}
    };
    EXPECT_EQ(output, expectedUrlStripped);

    output.clear();
    urlOptions.stripUnknownTags = false;
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "Before ^url(0) after", recordChunk, handleNothing, urlOptions);
    EXPECT_EQ(output, expectedUrlStripped);

    // Test 4: Command is handled and replaced
    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "Before ^page(2) after",
        recordChunk,
        [](const std::vector<std::string>& parsed) -> std::optional<std::string> {
            if (parsed[0] == "page") return "3";
            return std::nullopt;
        });
    std::vector<std::pair<std::string, std::string>> expected3 = {
        {"TEXT", "Before 3 after"}
    };
    EXPECT_EQ(output, expected3);

    // Test 4: Invalid command → literal caret
    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "Broken ^ command", recordChunk, handleNothing);
    std::vector<std::pair<std::string, std::string>> expected4 = {
        {"TEXT", "Broken ^ command"}
    };
    EXPECT_EQ(output, expected4);

    // Test 5: Suppress command (returns empty std::nullopt)
    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "Hi ^suppress Bye", recordChunk, handleNothing);
    std::vector<std::pair<std::string, std::string>> expected5 = {
        {"TEXT", "Hi  Bye"},
    };
    EXPECT_EQ(output, expected5);

    // Test 6: Suppress valid command ^flat
    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "Hi ^flat() Bye", recordChunk,
        [](const std::vector<std::string>& parsed) -> std::optional<std::string> {
            if (parsed[0] == "flat") return "";
            return std::nullopt;
        }
    );
    std::vector<std::pair<std::string, std::string>> expected6 = {
        {"TEXT", "Hi  Bye"},
    };
    EXPECT_EQ(output, expected6);
}

TEST(TextsTest, CollectEnigmaTextChunks)
{
    using namespace musx::util;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    auto styledText = texts->get<texts::BlockText>(32);
    ASSERT_TRUE(styledText);
    auto chunks = styledText->getRawTextCtx(styledText, SCORE_PARTID).collectEnigmaTextChunks();
    ASSERT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0].text, "Text block");
    EXPECT_EQ(EnigmaString::plainTextFromChunks(chunks), "Text block");
    ASSERT_TRUE(chunks[0].styles.font);
    EXPECT_EQ(chunks[0].styles.font->fontId, 1);
    EXPECT_EQ(chunks[0].styles.font->fontSize, 14);
    EXPECT_EQ(chunks[0].styles.font->getEnigmaStyles(), 2);

    auto styleChangeText = texts->get<texts::BlockText>(27);
    ASSERT_TRUE(styleChangeText);
    chunks = styleChangeText->getRawTextCtx(styleChangeText, SCORE_PARTID).collectEnigmaTextChunks();
    ASSERT_EQ(chunks.size(), 2);
    EXPECT_EQ(chunks[0].text, "My Piece\n");
    EXPECT_EQ(chunks[1].text, "for");
    EXPECT_EQ(EnigmaString::plainTextFromChunks(chunks), "My Piece\nfor");
    ASSERT_TRUE(chunks[0].styles.font);
    ASSERT_TRUE(chunks[1].styles.font);
    EXPECT_NE(chunks[0].styles.font, chunks[1].styles.font);
    EXPECT_EQ(chunks[0].styles.font->fontSize, 24);
    EXPECT_EQ(chunks[0].styles.font->getEnigmaStyles(), 65);
    EXPECT_EQ(chunks[1].styles.font->fontSize, 10);
    EXPECT_EQ(chunks[1].styles.font->getEnigmaStyles(), 66);
    chunks[1].styles.font->setEnigmaStyles(0);
    EXPECT_EQ(chunks[0].styles.font->getEnigmaStyles(), 65);

    auto pageText = texts->get<texts::BlockText>(17);
    ASSERT_TRUE(pageText);
    chunks = pageText->getRawTextCtx(pageText, SCORE_PARTID).collectEnigmaTextChunks(
        [](const std::vector<std::string>& parsed) -> std::optional<std::string> {
            if (parsed[0] == "page") {
                return "custom page";
            }
            return std::nullopt;
        });
    ASSERT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0].text, "custom page");

    auto titleText = texts->get<texts::BlockText>(18);
    ASSERT_TRUE(titleText);
    chunks = titleText->getRawTextCtx(titleText, SCORE_PARTID).collectEnigmaTextChunks();
    ASSERT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0].text, "My Piece");
}

TEST(TextsTest, EnigmaAccidentalSubstitution)
{
    using EnigmaString = musx::util::EnigmaString;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(textXml);

    auto replaceAccidentalTags = [&](const std::string& text, EnigmaString::AccidentalStyle accidentalStyle = EnigmaString::AccidentalStyle::Ascii) -> std::string {
        std::string result;
        EnigmaString::EnigmaParsingOptions options(accidentalStyle);
        options.stripUnknownTags = false;
        EnigmaString::parseEnigmaText(doc, SCORE_PARTID, text, [&](const std::string& chunk, const musx::util::EnigmaStyles&) -> bool {
            result += chunk;
            return true;
        }, options);
        return result;
    };

    auto result = replaceAccidentalTags("^font(New York)^sharp()^natural()^flat()^^composer()"); //ascii default
    EXPECT_EQ(result, "#b^composer()");
    result = replaceAccidentalTags("^font(New York)^sharp()^natural()^flat()^^composer()", EnigmaString::AccidentalStyle::Smufl);
    std::string text = ""
                    + EnigmaString::fromU8(u8"\uE262")  // SMuFL sharp
                    + EnigmaString::fromU8(u8"\uE261")  // SMuFL natural
                    + EnigmaString::fromU8(u8"\uE260")  // SMuFL flat
                    + "^composer()";
    EXPECT_EQ(result, text);
    result = replaceAccidentalTags("^font(New York)^sharp()^natural()^flat()^^composer()", EnigmaString::AccidentalStyle::Unicode);
    EXPECT_EQ(result, "♯♮♭^composer()");
    result = EnigmaString::trimTags(result);
    EXPECT_EQ(result, "♯♮♭");
    result = EnigmaString::trimTags("^font(New York)^sharp()The composer tag is ^^composer()");
    EXPECT_EQ(result, "The composer tag is ^composer()");
}

TEST(TextsTest, EnigmaAccidentalParsingMaestro)
{
    using namespace musx::util;

    std::vector<char> acciXml;
    musxtest::readFile(musxtest::getInputPath() / "acci_inserts.enigmaxml", acciXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(acciXml);
    ASSERT_TRUE(doc);

    auto blockText = doc->getTexts()->get<texts::BlockText>(2);
    ASSERT_TRUE(blockText) << "block text 2 not found";

    using Results = std::tuple<std::string, int, Evpu, int, std::string>;
    std::vector<Results> expectedResults = {
        { "Times New Roman", 12, 0, 0, "test " },
        { "Maestro", 13, 16, 35, "#" },
        { "Maestro", 13, 9, 60, "b" },
        { "Maestro", 13, 16, 50, "n" },
        { "Times New Roman", 12, 0, 0, " test" },
    };

    size_t iterations = 0;
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, blockText->text, [&](const std::string& chunk, const EnigmaStyles& styles) {
        EXPECT_TRUE(iterations < expectedResults.size()) << "not enough expected results for interation " << iterations;
        if (iterations >= expectedResults.size()) {
            return false;
        }
        auto [expFont, expSize, expBaseline, expTracking, expChunk] = expectedResults[iterations];
        EXPECT_EQ(styles.font->getName(), expFont);
        EXPECT_EQ(styles.font->fontSize, expSize);
        EXPECT_EQ(styles.baseline, expBaseline);
        EXPECT_EQ(styles.tracking, expTracking);
        EXPECT_EQ(chunk, expChunk);
        iterations++;
        return true;
    });
    EXPECT_EQ(iterations, expectedResults.size()) << "text start/end plus 3 acci inserts";
}

TEST(TextsTest, EnigmaAccidentalParsingFinaleMaestro)
{
    using namespace musx::util;

    std::vector<char> acciXml;
    musxtest::readFile(musxtest::getInputPath() / "acci_smufl_inserts.enigmaxml", acciXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(acciXml);
    ASSERT_TRUE(doc);

    auto blockText = doc->getTexts()->get<texts::BlockText>(2);
    ASSERT_TRUE(blockText) << "block text 2 not found";

    using Results = std::tuple<std::string, int, Evpu, int, std::string>;
    std::vector<Results> expectedResults = {
        { "Times New Roman", 12, 0, 0, "test " },
        { "Finale Maestro", 13, 16, 35, EnigmaString::fromU8(u8"\uE262") },  // SMuFL sharp
        { "Finale Maestro", 13, 9, 60, EnigmaString::fromU8(u8"\uE260") },   // SMuFL flat
        { "Finale Maestro", 13, 16, 50, EnigmaString::fromU8(u8"\uE261") },  // SMuFL natural
        { "Times New Roman", 12, 0, 0, " test" },
    };

    size_t iterations = 0;
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, blockText->text, [&](const std::string& chunk, const EnigmaStyles& styles) {
        EXPECT_TRUE(iterations < expectedResults.size()) << "not enough expected results for iteration " << iterations;
        if (iterations >= expectedResults.size()) {
            return false;
        }
        auto [expFont, expSize, expBaseline, expTracking, expChunk] = expectedResults[iterations];
        EXPECT_EQ(styles.font->getName(), expFont);
        EXPECT_EQ(styles.font->fontSize, expSize);
        EXPECT_EQ(styles.baseline, expBaseline);
        EXPECT_EQ(styles.tracking, expTracking);
        EXPECT_EQ(chunk, expChunk);
        iterations++;
        return true;
    });
    EXPECT_EQ(iterations, expectedResults.size()) << "text start/end plus 3 acci inserts";
}

TEST(TextsTest, ParseEnigmaWithAccidentals)
{
    using namespace musx::util;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    std::string output;
    auto accumulateChunk = [&](const std::string& text, const musx::util::EnigmaStyles&) -> bool {
        output += text;
        return true;
    };

    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^font(New York)^sharp()^natural()^flat()^composer()", accumulateChunk, EnigmaString::AccidentalStyle::Ascii);
    EXPECT_EQ(output, "#bL. BEETHOVEN");
    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^font(New York)^sharp()^natural()^flat()^arranger()", accumulateChunk, EnigmaString::AccidentalStyle::Unicode);
    EXPECT_EQ(output, "♯♮♭Ferrucio Busoni");
}

TEST(TextsTest, ParseEnigmaInsertsBaseLevel)
{
    using namespace musx::util;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    std::string output;
    auto accumulateChunk = [&](const std::string& text, const musx::util::EnigmaStyles&) -> bool {
        output += text;
        return true;
    };

    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^title() ^subtitle() ^cprsym()^copyright()", accumulateChunk);
    EXPECT_EQ(output, "My Piece Subtitle @1823");

    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^composer() ^arranger() ^lyricist() ^description()", accumulateChunk);
    EXPECT_EQ(output, "L. BEETHOVEN Ferrucio Busoni Johnny Göthe GRM–####");

    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^date(0) | ^date(1) | ^date(2) | ^time(0) | ^time(1)", accumulateChunk);
    EXPECT_TRUE(musxtest::stringHasDigit(output));
    musxtest::g_endMessages << "parsed enigma dates/times: " << output << std::endl;

    output.clear();
    EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^fdate(0) | ^fdate(1) | ^fdate(2)", accumulateChunk);
    EXPECT_NE(output.find("2025"), std::string::npos);
    musxtest::g_endMessages << "parsed file dates: " << output << std::endl;
}

TEST(TextsTest, ParseEnigmaPerfTimeFromNotationMetadata)
{
    using namespace musx::util;

    constexpr musxtest::string_view minimalXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <header>
    <headerData>
      <wordOrder>lo-endian</wordOrder>
      <textEncoding>Mac</textEncoding>
    </headerData>
  </header>
</finale>
)xml";

    constexpr musxtest::string_view finaleMetadata = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<metadata version="27.4" xmlns="http://www.makemusic.com/2012/NotationMetadata">
  <fileInfo>
    <scoreDuration>273.0</scoreDuration>
    <creatorString>Finale 27.4.1.146 for Macintosh</creatorString>
  </fileInfo>
</metadata>
)xml";

    constexpr musxtest::string_view nonFinaleMetadata = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<metadata version="27.4" xmlns="http://www.makemusic.com/2012/NotationMetadata">
  <fileInfo>
    <scoreDuration>273.0</scoreDuration>
    <creatorString>MuseScore 4.4.0</creatorString>
  </fileInfo>
</metadata>
)xml";

    auto buildWithMetadata = [&](musxtest::string_view metadata) -> musx::dom::DocumentPtr {
        std::vector<char> metadataBuffer(metadata.begin(), metadata.end());
        musx::factory::DocumentFactory::CreateOptions options(
            std::filesystem::path(""),
            std::move(metadataBuffer),
            musx::factory::DocumentFactory::CreateOptions::EmbeddedGraphicFiles{});
        return musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(
            minimalXml.data(), minimalXml.size(), std::move(options));
    };

    auto getParsedText = [](const musx::dom::DocumentPtr& document, const std::string& input) -> std::string {
        std::string output;
        EnigmaString::parseEnigmaText(document, SCORE_PARTID, input, [&](const std::string& text, const musx::util::EnigmaStyles&) -> bool {
            output += text;
            return true;
        });
        return output;
    };

    auto finaleDoc = buildWithMetadata(finaleMetadata);
    ASSERT_TRUE(finaleDoc);
    ASSERT_TRUE(finaleDoc->getScoreDurationSeconds().has_value());
    EXPECT_DOUBLE_EQ(finaleDoc->getScoreDurationSeconds().value(), 273.0);
    EXPECT_EQ(getParsedText(finaleDoc, "^perftime(0)"), "4:33");
    EXPECT_EQ(getParsedText(finaleDoc, "^perftime(1)"), "00:04:33");
    EXPECT_EQ(getParsedText(finaleDoc, "^perftime(2)"), "00:04:33.000");
    EXPECT_EQ(getParsedText(finaleDoc, "^perftime(3)"), "04:33");
    EXPECT_EQ(getParsedText(finaleDoc, "X ^perftime(4) Y"), "X 4'33\" Y");
    EXPECT_EQ(getParsedText(finaleDoc, "^perftime(5)"), "4");

    auto nonFinaleDoc = buildWithMetadata(nonFinaleMetadata);
    ASSERT_TRUE(nonFinaleDoc);
    EXPECT_FALSE(nonFinaleDoc->getScoreDurationSeconds().has_value());
    EXPECT_EQ(getParsedText(nonFinaleDoc, "X ^perftime(4) Y"), "X  Y");
}

TEST(TextsTest, ParseEnigmaFontInfo)
{
    using namespace musx::util;

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(textXml);
    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    int iterationCount = 0;
    bool result = EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^font(Times)^size(13)^nfx(2)", [&](const std::string& chunk, const musx::util::EnigmaStyles& styles) {
        EXPECT_EQ(styles.font->fontId, 1);
        EXPECT_EQ(styles.font->fontSize, 13);
        EXPECT_EQ(styles.font->getEnigmaStyles(), 2);
        EXPECT_FALSE(styles.font->bold);
        EXPECT_TRUE(styles.font->italic);
        EXPECT_TRUE(chunk.empty());
        iterationCount++;
        return true;
    });
    EXPECT_EQ(iterationCount, 1) << "font should be reported even when no text";
    EXPECT_TRUE(result);

    iterationCount = 0;
    result = EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^fontid(2)^size(10)^nfx(2)text^nfx(0)", [&](const std::string& chunk, const musx::util::EnigmaStyles& styles) {
        if (iterationCount == 0) {
            EXPECT_EQ(styles.font->fontId, 2);
            EXPECT_EQ(styles.font->fontSize, 10);
            EXPECT_EQ(styles.font->getEnigmaStyles(), 2);
            EXPECT_FALSE(styles.font->bold);
            EXPECT_TRUE(styles.font->italic);
            EXPECT_EQ(chunk, "text");
        } else if (iterationCount == 1) {
            EXPECT_EQ(styles.font->fontId, 2);
            EXPECT_EQ(styles.font->fontSize, 10);
            EXPECT_EQ(styles.font->getEnigmaStyles(), 0);
            EXPECT_FALSE(styles.font->bold);
            EXPECT_FALSE(styles.font->italic);
            EXPECT_TRUE(chunk.empty());
        }
        iterationCount++;
        return true;
    });
    EXPECT_EQ(iterationCount, 2) << "trailing font change should be reported";
    EXPECT_TRUE(result);

    iterationCount = 0;
    result = EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "", [&](const std::string&, const musx::util::EnigmaStyles&) {
        iterationCount++;
        return true;
    });
    EXPECT_EQ(iterationCount, 0) << "nothing should be reported";
    EXPECT_TRUE(result);

    EnigmaString::EnigmaParsingOptions options;
    options.ignoreStyleTags = true;
    iterationCount = 0;
    result = EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^fontid(2)^size(10)^nfx(2)This is ^baseline(12)the text.^nfx(0)", [&](const std::string& chunk, const musx::util::EnigmaStyles&) {
        EXPECT_EQ(chunk, "This is the text.");
        iterationCount++;
        return true;
    }, options);
    EXPECT_EQ(iterationCount, 1) << "no font changes should be reported";
    EXPECT_TRUE(result);

    iterationCount = 0;
    result = EnigmaString::parseEnigmaText(doc, SCORE_PARTID, "^fontid(2)^size(10)^nfx(2)This is ^baseline(12)the text.^nfx(0)", [&](const std::string& chunk, const musx::util::EnigmaStyles&) {
        EXPECT_EQ(chunk, "This is ");
        iterationCount++;
        return false;
    });
    EXPECT_EQ(iterationCount, 1) << "only the first font change should be reported, due to early exit return";
    EXPECT_FALSE(result);
}

static void checkSyllable(const MusxInstance<texts::LyricsVerse>& lyr, size_t index, const std::string& expSyl, bool expBefore, bool expAfter, int underscores = 0)
{
    auto mutableText = const_cast<texts::LyricsVerse*>(lyr.get());
    mutableText->createSyllableInfo(lyr);
    ASSERT_GT(lyr->syllables.size(), index);
    const auto& syl = lyr->syllables[index];
    ASSERT_TRUE(syl);
    EXPECT_EQ(syl->syllable, expSyl);
    EXPECT_EQ(syl->hasHyphenBefore, expBefore);
    EXPECT_EQ(syl->hasHyphenAfter, expAfter);
    EXPECT_EQ(syl->strippedUnderscores, underscores);
}

TEST(TextsTest, LyricSyllableParsing)
{
    using texts::LyricsVerse;

    std::vector<char> syllXml;
    musxtest::readFile(musxtest::getInputPath() / "syllables.enigmaxml", syllXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(syllXml);
    ASSERT_TRUE(doc);

    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    auto lyrics = texts->getArray<LyricsVerse>();
    ASSERT_GE(lyrics.size(), 6);

    checkSyllable(lyrics[0], 0, "fi@#", false, false);
    checkSyllable(lyrics[0], 1, "na", false, false);
    checkSyllable(lyrics[0], 2, "le", false, false);

    checkSyllable(lyrics[1], 0, "fi", false, false);
    checkSyllable(lyrics[1], 1, "na.;—", false, false);
    checkSyllable(lyrics[1], 2, "le", false, false);

    checkSyllable(lyrics[2], 0, "fi", false, true);
    checkSyllable(lyrics[2], 1, "na", true, true);
    checkSyllable(lyrics[2], 2, "le", true, false);

    checkSyllable(lyrics[3], 0, "fi", false, false);
    checkSyllable(lyrics[3], 1, "na", false, false);
    checkSyllable(lyrics[3], 2, "le", false, false);

    EXPECT_TRUE(lyrics[4]->syllables.empty());

    checkSyllable(lyrics[5], 0, "finale", false, false);
    checkSyllable(lyrics[5], 1, "fi", false, true);
    checkSyllable(lyrics[5], 2, "na", true, true);
    checkSyllable(lyrics[5], 3, "le", true, false);
}

TEST(TextsTest, LyricSyllableLegacyWhitespace)
{
    using texts::LyricsVerse;

    std::vector<char> syllXml;
    musxtest::readFile(musxtest::getInputPath() / "syllables.enigmaxml", syllXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(syllXml);
    ASSERT_TRUE(doc);

    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);
    auto lyrics = texts->getArray<LyricsVerse>();
    ASSERT_FALSE(lyrics.empty());

    // Finale legacy lyric data uses these control bytes as spacing markers.
    auto mutableLyrics = const_cast<LyricsVerse*>(lyrics[0].get());
    mutableLyrics->text = "^fontid(1)^size(12)^nfx(0)soft-ly and \x01\x06\x01\x01sud-den-ly";

    checkSyllable(lyrics[0], 0, "soft", false, true);
    checkSyllable(lyrics[0], 1, "ly", true, false);
    checkSyllable(lyrics[0], 2, "and", false, false);
    checkSyllable(lyrics[0], 3, "sud", false, true);
    checkSyllable(lyrics[0], 4, "den", true, true);
    checkSyllable(lyrics[0], 5, "ly", true, false);
}

TEST(TextsTest, LyricSyllableUnderscoreParsing)
{
    using texts::LyricsVerse;

    std::vector<char> syllXml;
    musxtest::readFile(musxtest::getInputPath() / "wordexts.enigmaxml", syllXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(syllXml);
    ASSERT_TRUE(doc);

    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    auto lyrics = texts->getArray<LyricsVerse>();
    ASSERT_GE(lyrics.size(), 2);

    checkSyllable(lyrics[1], 0, "lets", false, false, 6);
    checkSyllable(lyrics[1], 1, "play", false, false, 2);
    checkSyllable(lyrics[1], 2, "ball", false, true, 5);
    checkSyllable(lyrics[1], 3, "ing", true, false, 1);

    {
        size_t nextIndex = 0;
        lyrics[1]->iterateStylesForSyllable(1, [&](const std::string& chunk, const musx::util::EnigmaStyles& styles) -> bool {
            EXPECT_LT(nextIndex, 1);
            if (nextIndex >= 1) return false;
            EXPECT_EQ(chunk, "play");
            EXPECT_EQ(styles.font->fontId, 11);
            nextIndex++;
            return true;
        });
    }

    {
        size_t nextIndex = 0;
        lyrics[1]->iterateStylesForSyllable(2, [&](const std::string& chunk, const musx::util::EnigmaStyles& styles) -> bool {
            EXPECT_LT(nextIndex, 1);
            if (nextIndex >= 1) return false;
            EXPECT_EQ(chunk, "ball");
            EXPECT_EQ(styles.font->fontId, 12);
            nextIndex++;
            return true;
        });
    }

    auto lyricOptions = doc->getOptions()->get<options::LyricOptions>();
    ASSERT_TRUE(lyricOptions);
    auto mutableOpts = const_cast<options::LyricOptions*>(lyricOptions.get());
    mutableOpts->useSmartWordExtensions = false;

    checkSyllable(lyrics[1], 0, "_lets__", false, true);
    checkSyllable(lyrics[1], 1, "_", true, false);
    checkSyllable(lyrics[1], 2, "___", false, false);
    checkSyllable(lyrics[1], 3, "play__", false, false);
    checkSyllable(lyrics[1], 4, "ball", false, true);
    checkSyllable(lyrics[1], 5, "___", true, false);
    checkSyllable(lyrics[1], 6, "__ing_", false, false);

    std::vector<std::string> expectedChunks = { "play", "_", "_" };
    std::vector<Cmper> expectedFontId = { 11, 1, 12 };
    {
        size_t nextIndex = 0;
        lyrics[1]->iterateStylesForSyllable(3, [&](const std::string& chunk, const musx::util::EnigmaStyles& styles) -> bool {
            EXPECT_LT(nextIndex, expectedChunks.size());
            if (nextIndex >= expectedChunks.size()) return false;
            EXPECT_EQ(chunk, expectedChunks[nextIndex]);
            EXPECT_EQ(styles.font->fontId, expectedFontId[nextIndex]);
            nextIndex++;
            return true;
        });
    }
}

TEST(TextsTest, LyricSyllableStyles)
{
    using texts::LyricsVerse;

    std::vector<char> syllXml;
    musxtest::readFile(musxtest::getInputPath() / "syllables.enigmaxml", syllXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(syllXml);
    ASSERT_TRUE(doc);

    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);

    auto lyrics = texts->getArray<LyricsVerse>();
    ASSERT_GE(lyrics.size(), 6);

    std::vector<std::string> expectedChunks = { "fi", "na", "le" };
    std::vector<bool> expectedItalics = { false, true, false };
    std::vector<Cmper> expectedFontId = { 1, 1, 1 };
    {
        size_t nextIndex = 0;
        lyrics[5]->iterateStylesForSyllable(0, [&](const std::string& chunk, const musx::util::EnigmaStyles& styles) -> bool {
            EXPECT_LT(nextIndex, expectedChunks.size());
            if (nextIndex >= expectedChunks.size()) return false;
            EXPECT_EQ(chunk, expectedChunks[nextIndex]);
            EXPECT_EQ(styles.font->italic, expectedItalics[nextIndex]);
            EXPECT_EQ(styles.font->fontId, expectedFontId[nextIndex]);
            nextIndex++;
            return true;
        });
    }
    {
        size_t nextIndex = 0;
        lyrics[5]->iterateStylesForSyllable(2, [&](const std::string& chunk, const musx::util::EnigmaStyles& styles) -> bool {
            EXPECT_LT(nextIndex, 1);
            if (nextIndex >= 1) return false;
            EXPECT_EQ(chunk, "na");
            EXPECT_EQ(styles.font->italic, false);
            EXPECT_EQ(styles.font->bold, false);
            EXPECT_EQ(styles.font->fontId, 11);
            nextIndex++;
            return true;
        });
    }
    {
        size_t nextIndex = 0;
        lyrics[5]->iterateStylesForSyllable(3, [&](const std::string& chunk, const musx::util::EnigmaStyles& styles) -> bool {
            EXPECT_LT(nextIndex, 1);
            if (nextIndex >= 1) return false;
            EXPECT_EQ(chunk, "le");
            EXPECT_EQ(styles.font->italic, false);
            EXPECT_EQ(styles.font->bold, false);
            EXPECT_EQ(styles.font->fontId, 1);
            nextIndex++;
            return true;
        });
    }
}

TEST(TextsTest, ExpressionsAndTitles)
{
    using namespace musx::util;
    std::string output;
    constexpr static Cmper kClarinetPartId = 1;

    std::vector<char> enigmaXml;
    musxtest::readFile(musxtest::getInputPath() / "enigma_strings.enigmaxml", enigmaXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(enigmaXml);
    ASSERT_TRUE(doc);

    auto expDef = doc->getOthers()->get<others::TextExpressionDef>(SCORE_PARTID, 1);
    ASSERT_TRUE(expDef) << "text expression def 1 does not exist.";

    output.clear();
    bool result = expDef->getRawTextCtx(SCORE_PARTID).parseEnigmaText([&](const std::string& chunk, const EnigmaStyles&) {
        output += chunk;
        return true;
    }, EnigmaString::AccidentalStyle::Unicode);
    EXPECT_TRUE(result);
    EXPECT_EQ(output, "Score value: 112 control: 64 pass: 3 pages: 2");

    output.clear();
    result = expDef->getRawTextCtx(kClarinetPartId).parseEnigmaText([&](const std::string& chunk, const EnigmaStyles&) {
        output += chunk;
        return true;
    }, EnigmaString::AccidentalStyle::Unicode);
    EXPECT_TRUE(result);
    EXPECT_EQ(output, "Clarinet in B♭ page: # value: 112 control: 64 pass: 3 pages: 1");

    auto measTexts = doc->getDetails()->getArray<details::MeasureTextAssign>(SCORE_PARTID, 1, 34);
    ASSERT_FALSE(measTexts.empty());

    const auto scorePageNum = doc->calcPageFromMeasure(SCORE_PARTID, measTexts[0]->getCmper2())->getCmper();
    EXPECT_EQ(scorePageNum, 2);
    const auto partPageNum = doc->calcPageFromMeasure(kClarinetPartId, measTexts[0]->getCmper2())->getCmper();
    EXPECT_EQ(partPageNum, 1);

    output.clear();
    result = measTexts[0]->getRawTextCtx(SCORE_PARTID).parseEnigmaText([&](const std::string& chunk, const EnigmaStyles&) {
        output += chunk;
        return true;
    }, EnigmaString::defaultInsertsCallback, EnigmaString::AccidentalStyle::Unicode);
    EXPECT_TRUE(result);
    EXPECT_EQ(output, "Score page: 2");

    output.clear();
    result = measTexts[0]->getRawTextCtx(kClarinetPartId).parseEnigmaText([&](const std::string& chunk, const EnigmaStyles&) {
        output += chunk;
        return true;
    }, EnigmaString::defaultInsertsCallback, EnigmaString::AccidentalStyle::Unicode);
    EXPECT_TRUE(result);
    EXPECT_EQ(output, "Clarinet in B♭ page: 1 page: 1");
}

TEST(TextsTest, LyricSyllableElisionRuns)
{
    using texts::LyricsVerse;

    // U+00A0 (non-breaking space) and U+203F (undertie): the only two characters Finale's own
    // MusicXML export treats as elision markers embedded in a syllable's text.
    constexpr const char* kNbsp = "\xC2\xA0";
    constexpr const char* kUndertie = "\xE2\x80\xBF";
    // U+2011 (non-breaking hyphen): not an elision marker; stays embedded as ordinary text.
    constexpr const char* kNonBreakingHyphen = "\xE2\x80\x91";

    std::vector<char> syllXml;
    musxtest::readFile(musxtest::getInputPath() / "syllables.enigmaxml", syllXml);
    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(syllXml);
    ASSERT_TRUE(doc);

    auto texts = doc->getTexts();
    ASSERT_TRUE(texts);
    auto lyrics = texts->getArray<LyricsVerse>();
    ASSERT_FALSE(lyrics.empty());

    auto mutableLyrics = const_cast<LyricsVerse*>(lyrics[0].get());

    // No embedded marker: one run, spanning the whole syllable, with the syllable's own hyphen
    // flags untouched.
    {
        mutableLyrics->text = "^fontid(1)^size(12)^nfx(0)plain";
        mutableLyrics->createSyllableInfo(lyrics[0]);
        ASSERT_EQ(lyrics[0]->syllables.size(), 1u);
        const auto runs = lyrics[0]->syllables[0]->calcElisionRuns();
        ASSERT_EQ(runs.size(), 1u);
        EXPECT_EQ(runs[0].text(), "plain");
        EXPECT_FALSE(runs[0].joinMarker.has_value());
        EXPECT_FALSE(runs[0].hasHyphenBefore);
        EXPECT_FALSE(runs[0].hasHyphenAfter);
    }

    // A non-breaking hyphen is not an elision trigger: it stays embedded as ordinary text.
    {
        mutableLyrics->text = std::string("^fontid(1)^size(12)^nfx(0)stre") + kNonBreakingHyphen + "ngth";
        mutableLyrics->createSyllableInfo(lyrics[0]);
        ASSERT_EQ(lyrics[0]->syllables.size(), 1u);
        const auto runs = lyrics[0]->syllables[0]->calcElisionRuns();
        ASSERT_EQ(runs.size(), 1u);
        EXPECT_EQ(runs[0].text(), std::string("stre") + kNonBreakingHyphen + "ngth");
    }

    // An embedded NBSP splits into two runs, joined by the NBSP codepoint itself (not a plain
    // ASCII space).
    {
        mutableLyrics->text = std::string("^fontid(1)^size(12)^nfx(0)and") + kNbsp + "a";
        mutableLyrics->createSyllableInfo(lyrics[0]);
        ASSERT_EQ(lyrics[0]->syllables.size(), 1u);
        const auto runs = lyrics[0]->syllables[0]->calcElisionRuns();
        ASSERT_EQ(runs.size(), 2u);
        EXPECT_EQ(runs[0].text(), "and");
        EXPECT_FALSE(runs[0].joinMarker.has_value());
        EXPECT_EQ(runs[1].text(), "a");
        ASSERT_TRUE(runs[1].joinMarker.has_value());
        EXPECT_EQ(*runs[1].joinMarker, char32_t(0x00A0));
    }

    // An embedded undertie behaves the same way, joined by the undertie codepoint.
    {
        mutableLyrics->text = std::string("^fontid(1)^size(12)^nfx(0)str") + kUndertie + "en" + kUndertie + "gth";
        mutableLyrics->createSyllableInfo(lyrics[0]);
        ASSERT_EQ(lyrics[0]->syllables.size(), 1u);
        const auto runs = lyrics[0]->syllables[0]->calcElisionRuns();
        ASSERT_EQ(runs.size(), 3u);
        EXPECT_EQ(runs[0].text(), "str");
        EXPECT_EQ(runs[1].text(), "en");
        EXPECT_EQ(runs[2].text(), "gth");
        ASSERT_TRUE(runs[1].joinMarker.has_value());
        EXPECT_EQ(*runs[1].joinMarker, char32_t(0x203F));
        ASSERT_TRUE(runs[2].joinMarker.has_value());
        EXPECT_EQ(*runs[2].joinMarker, char32_t(0x203F));
    }

    // Hyphen-edge redistribution: a three-syllable hyphen chain ("dai-ly-food"), each syllable
    // additionally split by an internal NBSP. Confirms a syllable's real leading hyphen can only
    // land on the first run, its real trailing hyphen only on the last, and a true "middle"
    // syllable ("ly", hyphenated on both sides) never produces a run flagged as both.
    {
        mutableLyrics->text = std::string("^fontid(1)^size(12)^nfx(0)d") + kNbsp + "ai-l" + kNbsp + "y-fo" + kNbsp + "od,";
        mutableLyrics->createSyllableInfo(lyrics[0]);
        ASSERT_EQ(lyrics[0]->syllables.size(), 3u);

        {
            const auto runs = lyrics[0]->syllables[0]->calcElisionRuns();
            ASSERT_EQ(runs.size(), 2u);
            EXPECT_EQ(runs[0].text(), "d");
            EXPECT_FALSE(runs[0].hasHyphenBefore);
            EXPECT_FALSE(runs[0].hasHyphenAfter);
            EXPECT_EQ(runs[1].text(), "ai");
            EXPECT_FALSE(runs[1].hasHyphenBefore);
            EXPECT_TRUE(runs[1].hasHyphenAfter);
        }
        {
            const auto runs = lyrics[0]->syllables[1]->calcElisionRuns();
            ASSERT_EQ(runs.size(), 2u);
            EXPECT_EQ(runs[0].text(), "l");
            EXPECT_TRUE(runs[0].hasHyphenBefore);
            EXPECT_FALSE(runs[0].hasHyphenAfter);
            EXPECT_EQ(runs[1].text(), "y");
            EXPECT_FALSE(runs[1].hasHyphenBefore);
            EXPECT_TRUE(runs[1].hasHyphenAfter);
        }
        {
            const auto runs = lyrics[0]->syllables[2]->calcElisionRuns();
            ASSERT_EQ(runs.size(), 2u);
            EXPECT_EQ(runs[0].text(), "fo");
            EXPECT_TRUE(runs[0].hasHyphenBefore);
            EXPECT_FALSE(runs[0].hasHyphenAfter);
            EXPECT_EQ(runs[1].text(), "od,");
            EXPECT_FALSE(runs[1].hasHyphenBefore);
            EXPECT_FALSE(runs[1].hasHyphenAfter);
        }
    }

    // Style changes within a run are preserved: reusing the multi-font "fi/na/le" syllable
    // (no elision markers, so one run) confirms the run's iterateStyles reports the same chunks
    // as the whole-syllable iterateStyles did before this feature existed.
    {
        auto lyrics2 = texts->getArray<LyricsVerse>();
        ASSERT_GE(lyrics2.size(), 6);
        const auto runs = lyrics2[5]->syllables[0]->calcElisionRuns();
        ASSERT_EQ(runs.size(), 1u);
        EXPECT_EQ(runs[0].text(), "finale");

        std::vector<std::string> expectedChunks = { "fi", "na", "le" };
        std::vector<bool> expectedItalics = { false, true, false };
        size_t nextIndex = 0;
        runs[0].iterateStyles([&](const std::string& chunk, const musx::util::EnigmaStyles& styles) -> bool {
            EXPECT_LT(nextIndex, expectedChunks.size());
            if (nextIndex >= expectedChunks.size()) return false;
            EXPECT_EQ(chunk, expectedChunks[nextIndex]);
            EXPECT_EQ(styles.font->italic, expectedItalics[nextIndex]);
            nextIndex++;
            return true;
        });
        EXPECT_EQ(nextIndex, expectedChunks.size());
    }
}

TEST(TextsTest, ImportTextRetainsConcreteTypeAndUsesIndependentNumbering)
{
    constexpr static musxtest::string_view sourceXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <texts>
        <blockText number="4">Copied block</blockText>
        <verse number="7">first second</verse>
    </texts>
</finale>
)xml";
    constexpr static musxtest::string_view targetXml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
    <texts>
        <blockText number="8">Existing block</blockText>
        <verse number="2">existing lyric</verse>
        <smartShapeText number="20">Independent type</smartShapeText>
    </texts>
</finale>
)xml";
    constexpr Cmper importedBlockId = 9;
    constexpr Cmper importedVerseId = 3;

    const auto source =
        musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(sourceXml);
    const auto target =
        musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(targetXml);
    const auto sourceBlock = source->getTexts()->get<texts::BlockText>(4);
    const auto sourceVerse = source->getTexts()->get<texts::LyricsVerse>(7);
    ASSERT_TRUE(sourceBlock);
    ASSERT_TRUE(sourceVerse);

    EXPECT_EQ(target->getTexts()->nextFreeCmper<texts::BlockText>(),
        std::optional<Cmper>(importedBlockId));
    EXPECT_EQ(target->getTexts()->nextFreeCmper<texts::LyricsVerse>(),
        std::optional<Cmper>(importedVerseId));

    EXPECT_EQ(texts::importTextInto(target, sourceBlock),
        std::optional<Cmper>(importedBlockId));
    EXPECT_EQ(texts::importTextInto(target, sourceVerse),
        std::optional<Cmper>(importedVerseId));

    const auto block = target->getTexts()->get<texts::BlockText>(importedBlockId);
    const auto verse = target->getTexts()->get<texts::LyricsVerse>(importedVerseId);
    ASSERT_TRUE(block);
    ASSERT_TRUE(verse);
    EXPECT_EQ(block->text, "Copied block");
    EXPECT_EQ(verse->text, "first second");
    EXPECT_EQ(block->getShareMode(), EnigmaBase::ShareMode::All);
    EXPECT_EQ(verse->getShareMode(), EnigmaBase::ShareMode::All);
    ASSERT_EQ(verse->syllables.size(), 2u);
    EXPECT_EQ(verse->syllables[0]->syllable, "first");
    EXPECT_EQ(verse->syllables[1]->syllable, "second");
}
