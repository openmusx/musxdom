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

TEST(GraphicsFileMetadataTest, PopulateAllFileInfoClasses)
{
    // XML moved internal to the test and renamed `xml`
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <fileAlias cmper="1">
      <length>548</length>
      <aliasHandle>00000000240202000000520E504744207461206172447669006500000000000000000000000043CF76592B480000010095BE5412617253707465654B547978652E746470006600000000000000000000000000000000000000000000000000000000000000000000000000000000000000007100DB03DAE19EFB0000000000000000FFFFFFFF0000000900000000000000000000000000000D00532D756F637220656946656C007310000800000043CFC69F0000110008000000DBE1FE4F000001001800010095BE010057BE01006CBA0100E6B2010054260000E82A020073004752205061446174442069726576523A626F726520746F46646C7265003A6F5265627472003A6946616E656C3AC44F0063726568747361723A6C56006E657574657220736E6F742065684220756C6666003A532D756F637220656946656C3A735400617253707465654B547978652E74647000660E002600120054007200610070005300650074004B006500790054006500780074002E007000640066000F001E000E005200470050002000440061007400610020004400720069007600650012006000522F626F726520746F46646C7265522F626F72652F746946616E656C92C64F2F63726568747361722F6C6556746E727573656F20206E687420656C4266752F66532D756F637220656946656C2F737254706165534B74796565547478702E666413001700562F6C6F6D757365522F5047442074612061724476690065FFFF0000</aliasHandle>
    </fileAlias>
    <fileAlias cmper="2">
      <length>544</length>
      <aliasHandle>00000000200202000000520E504744207461206172447669006500000000000000000000000043CF76592B480000010095BE50117265756373736F694B6E7965702E6664000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000007100AB03DAE115F60000000000000000FFFFFFFF0000000900000000000000000000000000000D00532D756F637220656946656C007310000800000043CFC69F0000110008000000DBE1754A000001001800010095BE010057BE01006CBA0100E6B2010054260000E82A020072004752205061446174442069726576523A626F726520746F46646C7265003A6F5265627472003A6946616E656C3AC44F0063726568747361723A6C56006E657574657220736E6F742065684220756C6666003A532D756F637220656946656C3A7350007265756373736F694B6E7965702E66640E0024001100500065007200630075007300730069006F006E004B00650079002E007000640066000F001E000E005200470050002000440061007400610020004400720069007600650012005F00522F626F726520746F46646C7265522F626F72652F746946616E656C92C64F2F63726568747361722F6C6556746E727573656F20206E687420656C4266752F66532D756F637220656946656C2F7365506372737569736E6F654B2E796470006613001700562F6C6F6D757365522F5047442074612061724476690065FFFF0000</aliasHandle>
    </fileAlias>
    <fileDesc cmper="1">
      <version>256</version>
      <pathType>macAlias</pathType>
      <pathID>1</pathID>
    </fileDesc>
    <fileDesc cmper="2">
      <version>256</version>
      <pathType>macAlias</pathType>
      <pathID>2</pathID>
    </fileDesc>
    <filePath cmper="1">
      <path>TrapSetKeyText.pdf</path>
    </filePath>
    <filePath cmper="2">
      <path>PercussionKey.pdf</path>
    </filePath>
    <fileURLBookmark cmper="1">
      <length>1644</length>
      <urlBookmarkData>6F626B6F066C0000000010040030000000000000000000000000000000000000000000000000000000000000000000000574000000040000030300000200000000070000010100006F56756C656D0073000E00000101000047522050614461744420697265760000000D0000010100006F526562747246206C6F65640072000000060000010100006F5265627472000000080000010100006946616E656C92C6000A000001010000724F6863736572746C61000000150000010100006556746E727573656F20206E687420656C42667500660000000D000001010000532D756F637220656946656C0073000000120000010100007254706165534B74796565547478702E6664000000240000060100000010000000200000003800000050000000600000007000000084000000A4000000BC0000000800000304000066820000000000000008000003040000000200000000000000080000030400002AE8000000000000000800000304000026540001000000000008000003040000B2E60001000000000008000003040000BA6C0001000000000008000003040000BE570001000000000008000003040000BE95000100000000000800000304000003DB00710000000000240000060100000104000001140000012400000134000001440000015400000164000001740000018400000008000004000000C541EBB2007F000000180000020100000001000000000000000F000000000000000000000000000000230000090100006966656C2F3A2F2F6F56756C656D2F73475225503032614461743225443069726576002F0008000003040000C0006CFC03A300000008000004000000B84126CE00C6000000240000010100003535443544333945452D32412D4537333030382D31462D314545454336363933333937340018000002010000010100000001000013EF00000001000000000000000000000017000001010000562F6C6F6D757365522F504744207461206172447669006500080000090100006966656C2F3A2F2F000C000001010000614D6963746E736F2068444800080000030400009000968200E700000008000004000000C7415A2800B3000000240000010100004545423532453735442D46372D3433344333422D30352D313133343730464335454644340018000002010000008100000001000013EF00000001000000000000000000000001000001010000002F000000600000FFFEFFFFF000000000000000000700002002000003380000000000002005000002A80000000000002010000002B80000000000002011000002EC0000000000002012000002CC0000000000002013000002DC0000000000002020000003180000000000000004000003030000F0000000000400000303000000000000000400000303000000010000002800000601000003AC000003B8000003C4000003B8000003B8000003B8000003B8000003B8000003B8000003B80000012900000201000062616539646134303462623933303535663662386134343831643066646232653132396331316665663164636364356131613635653764633632396638626330303B3B303030303030303030303B3030303030303B303030303030303030303B30303030303030303030303032303B306F632E6D70616C702E6570612D706173646E6F622E7865726461772D69726574303B3B313130303030306331303B30303030303030303730303164333B6264302F3B6F76756C656D2F736772207061646174642069726576722F626F726520746F66646C7265722F626F72652F746946616E656C92C66F2F63726568747361722F6C6576746E727573656F20206E687420656C6266752F66732D756F637220656966656C2F737274706165736B74796565547478702E666400000000002F000001010000534E5255424C6F6F6D6B7261516B61756172746E6E694D65756F746E6465654E7774726F566B6C6F6D757365654B0079000000000501000000C00000FFFEFFFF0001000003440000000F00001004000000D80000000000001005000001940000000000001010000001D00000000000001040000001C00000000000002000000003D00000000000002002000002880000000000002005000001F000000000000020100000002000000000000020110000023C00000000000020120000021C00000000000020130000022C000000000000202000000268000000000000D01000000004000000000000F0800000040000000000000005348000056C000000000000</urlBookmarkData>
    </fileURLBookmark>
    <fileURLBookmark cmper="2">
      <length>1640</length>
      <urlBookmarkData>6F626B6F06680000000010040030000000000000000000000000000000000000000000000000000000000000000000000570000000040000030300000200000000070000010100006F56756C656D0073000E00000101000047522050614461744420697265760000000D0000010100006F526562747246206C6F65640072000000060000010100006F5265627472000000080000010100006946616E656C92C6000A000001010000724F6863736572746C61000000150000010100006556746E727573656F20206E687420656C42667500660000000D000001010000532D756F637220656946656C00730000001100000101000065506372737569736E6F654B2E7964700066000000240000060100000010000000200000003800000050000000600000007000000084000000A4000000BC0000000800000304000066820000000000000008000003040000000200000000000000080000030400002AE8000000000000000800000304000026540001000000000008000003040000B2E60001000000000008000003040000BA6C0001000000000008000003040000BE570001000000000008000003040000BE95000100000000000800000304000003AB00710000000000240000060100000104000001140000012400000134000001440000015400000164000001740000018400000008000004000000C541E8B280BA000000180000020100000001000000000000000F000000000000000000000000000000230000090100006966656C2F3A2F2F6F56756C656D2F73475225503032614461743225443069726576002F0008000003040000C0006CFC03A300000008000004000000B84126CE00C6000000240000010100003535443544333945452D32412D4537333030382D31462D314545454336363933333937340018000002010000010100000001000013EF00000001000000000000000000000017000001010000562F6C6F6D757365522F504744207461206172447669006500080000090100006966656C2F3A2F2F000C000001010000614D6963746E736F2068444800080000030400009000968200E700000008000004000000C7415A2800B3000000240000010100004545423532453735442D46372D3433344333422D30352D313133343730464335454644340018000002010000008100000001000013EF00000001000000000000000000000001000001010000002F000000600000FFFEFFFFF000000000000000000700002002000003380000000000002005000002A80000000000002010000002B80000000000002011000002EC0000000000002012000002CC0000000000002013000002DC0000000000002020000003180000000000000004000003030000F0000000000400000303000000000000000400000303000000010000002800000601000003AC000003B8000003C4000003B8000003B8000003B8000003B8000003B8000003B8000003B80000012800000201000032336531336539616535353363633337366564363032633965356337346166666165373839383161613630623334343661343861343237373836653332646336303B3B303030303030303030303B3030303030303B303030303030303030303B30303030303030303030303032303B306F632E6D70616C702E6570612D706173646E6F622E7865726461772D69726574303B3B313130303030306331303B30303030303030303730303161333B6264302F3B6F76756C656D2F736772207061646174642069726576722F626F726520746F66646C7265722F626F72652F746966616E656C92C66F2F63726568747361722F6C6576746E727573656F20206E687420656C6266752F66732D756F637220656966656C2F7365706372737569736E6F656B2E7964700066002F000001010000534E5255424C6F6F6D6B7261516B61756172746E6E694D65756F746E6465654E7774726F566B6C6F6D757365654B0079000000000501000000C00000FFFEFFFF0001000003440000000F00001004000000D80000000000001005000001940000000000001010000001D00000000000001040000001C00000000000002000000003D00000000000002002000002880000000000002005000001F000000000000020100000002000000000000020110000023C00000000000020120000021C00000000000020130000022C000000000000202000000268000000000000D01000000004000000000000F08000000400000000000000053080000568000000000000</urlBookmarkData>
    </fileURLBookmark>
  </others>
</finale>
)xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    // ---------- FileAlias ----------
    {
        auto a1 = others->get<others::FileAlias>(SCORE_PARTID, 1);
        ASSERT_TRUE(a1) << "FileAlias with cmper 1 not found";
        EXPECT_EQ(a1->length, a1->aliasHandle.size());
        EXPECT_EQ(a1->length, static_cast<size_t>(548));
        EXPECT_EQ(a1->aliasHandle[4], 0x24);

        auto a2 = others->get<others::FileAlias>(SCORE_PARTID, 2);
        ASSERT_TRUE(a2) << "FileAlias with cmper 2 not found";
        EXPECT_EQ(a2->length, a2->aliasHandle.size());
        EXPECT_EQ(a2->length, static_cast<size_t>(544));
        EXPECT_EQ(a2->aliasHandle[5], 0x02);
    }

    // ---------- FileDescription ----------
    {
        auto d1 = others->get<others::FileDescription>(SCORE_PARTID, 1);
        ASSERT_TRUE(d1) << "FileDescription with cmper 1 not found";
        EXPECT_EQ(d1->version, static_cast<uint32_t>(256));
        EXPECT_EQ(d1->pathType, others::FileDescription::PathType::MacAlias);
        EXPECT_EQ(d1->pathId, 1);

        auto d2 = others->get<others::FileDescription>(SCORE_PARTID, 2);
        ASSERT_TRUE(d2) << "FileDescription with cmper 2 not found";
        EXPECT_EQ(d2->version, static_cast<uint32_t>(256));
        EXPECT_EQ(d2->pathType, others::FileDescription::PathType::MacAlias);
        EXPECT_EQ(d2->pathId, 2);
    }

    // ---------- FilePath ----------
    {
        auto p1 = others->get<others::FilePath>(SCORE_PARTID, 1);
        ASSERT_TRUE(p1) << "FilePath with cmper 1 not found";
        EXPECT_EQ(p1->path, std::string("TrapSetKeyText.pdf"));

        auto p2 = others->get<others::FilePath>(SCORE_PARTID, 2);
        ASSERT_TRUE(p2) << "FilePath with cmper 2 not found";
        EXPECT_EQ(p2->path, std::string("PercussionKey.pdf"));
    }

    // ---------- FileUrlBookmark ----------
    {
        auto b1 = others->get<others::FileUrlBookmark>(SCORE_PARTID, 1);
        ASSERT_TRUE(b1) << "FileUrlBookmark with cmper 1 not found";
        EXPECT_EQ(b1->length, b1->urlBookmarkData.size());
        EXPECT_EQ(b1->length, static_cast<size_t>(1644));
        EXPECT_EQ(b1->urlBookmarkData[2], 0x6B);

        auto b2 = others->get<others::FileUrlBookmark>(SCORE_PARTID, 2);
        ASSERT_TRUE(b2) << "FileUrlBookmark with cmper 2 not found";
        EXPECT_EQ(b2->length, b2->urlBookmarkData.size());
        EXPECT_EQ(b2->length, static_cast<size_t>(1640));
        EXPECT_EQ(b1->urlBookmarkData[4], 0x06);
    }
}

TEST(PageGraphicAssignTest, PopulateFields)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <pageGraphicAssign cmper="4" inci="0">
      <version>256</version>
      <left>13</left>
      <bottom>-956</bottom>
      <width>495</width>
      <height>256</height>
      <fDescID>2</fDescID>
      <displayType>one</displayType>
      <displayHidden/>
      <halign>center</halign>
      <valign>top</valign>
      <posFrom>margins</posFrom>
      <fixedPerc/>
      <startPage>4</startPage>
      <endPage>4</endPage>
      <savedRecord/>
      <origWidth>556</origWidth>
      <origHeight>324</origHeight>
      <rightPgHAlign>right</rightPgHAlign>
      <rightPgVAlign>top</rightPgVAlign>
      <rightPgPosFrom>margins</rightPgPosFrom>
      <rightPgFixedPerc/>
      <rightPgLeft>17</rightPgLeft>
      <rightPgBottom>-956</rightPgBottom>
      <graphicCmper>2</graphicCmper>
    </pageGraphicAssign>
    <pageGraphicAssign cmper="4" inci="0" part="1" shared="true">
      <displayHidden>
        <offInPart/>
      </displayHidden>
    </pageGraphicAssign>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    // Score assignment
    auto g1 = others->get<others::PageGraphicAssign>(SCORE_PARTID, 4, 0);
    ASSERT_TRUE(g1) << "PageGraphicAssign (cmper=4, inci=0) not found for SCORE_PARTID but does exist";

    EXPECT_EQ(g1->version, 256u);
    EXPECT_EQ(g1->left, 13);
    EXPECT_EQ(g1->bottom, -956);
    EXPECT_EQ(g1->width, 495);
    EXPECT_EQ(g1->height, 256);
    EXPECT_EQ(g1->fDescId, 2);
    EXPECT_EQ(g1->displayType, others::PageGraphicAssign::PageAssignType::One);
    EXPECT_TRUE(g1->hidden);
    EXPECT_EQ(g1->hAlign, others::PageGraphicAssign::HorizontalAlignment::Center);
    EXPECT_EQ(g1->vAlign, others::PageGraphicAssign::VerticalAlignment::Top);
    EXPECT_EQ(g1->posFrom, others::PageGraphicAssign::PositionFrom::Margins);
    EXPECT_TRUE(g1->fixedPerc);
    EXPECT_EQ(g1->startPage, 4);
    EXPECT_EQ(g1->endPage, 4);
    EXPECT_TRUE(g1->savedRecord);
    EXPECT_EQ(g1->origWidth, 556);
    EXPECT_EQ(g1->origHeight, 324);
    EXPECT_EQ(g1->rightPgHAlign, others::PageGraphicAssign::HorizontalAlignment::Right);
    EXPECT_EQ(g1->rightPgVAlign, others::PageGraphicAssign::VerticalAlignment::Top);
    EXPECT_EQ(g1->rightPgPosFrom, others::PageGraphicAssign::PositionFrom::Margins);
    EXPECT_TRUE(g1->rightPgFixedPerc);
    EXPECT_EQ(g1->rightPgLeft, 17);
    EXPECT_EQ(g1->rightPgBottom, -956);
    EXPECT_EQ(g1->graphicCmper, 2);

    // Part 1 assignment (shared="true"): inherits from score, overridden fields apply
    auto g2 = others->get<others::PageGraphicAssign>(1, 4, 0);
    ASSERT_TRUE(g2) << "PageGraphicAssign (cmper=4, inci=0) not found for part 1 but does exist";

    // Inherited fields
    EXPECT_EQ(g2->version, g1->version);
    EXPECT_EQ(g2->left, g1->left);
    EXPECT_EQ(g2->bottom, g1->bottom);
    EXPECT_EQ(g2->width, g1->width);
    EXPECT_EQ(g2->height, g1->height);
    EXPECT_EQ(g2->fDescId, g1->fDescId);
    EXPECT_EQ(g2->displayType, g1->displayType);
    EXPECT_EQ(g2->hAlign, g1->hAlign);
    EXPECT_EQ(g2->vAlign, g1->vAlign);
    EXPECT_EQ(g2->posFrom, g1->posFrom);
    EXPECT_EQ(g2->fixedPerc, g1->fixedPerc);
    EXPECT_EQ(g2->startPage, g1->startPage);
    EXPECT_EQ(g2->endPage, g1->endPage);
    EXPECT_EQ(g2->savedRecord, g1->savedRecord);
    EXPECT_EQ(g2->origWidth, g1->origWidth);
    EXPECT_EQ(g2->origHeight, g1->origHeight);
    EXPECT_EQ(g2->rightPgHAlign, g1->rightPgHAlign);
    EXPECT_EQ(g2->rightPgVAlign, g1->rightPgVAlign);
    EXPECT_EQ(g2->rightPgPosFrom, g1->rightPgPosFrom);
    EXPECT_EQ(g2->rightPgFixedPerc, g1->rightPgFixedPerc);
    EXPECT_EQ(g2->rightPgLeft, g1->rightPgLeft);
    EXPECT_EQ(g2->rightPgBottom, g1->rightPgBottom);
    EXPECT_EQ(g2->graphicCmper, g1->graphicCmper);

    // Overridden field
    EXPECT_FALSE(g2->hidden); // displayHidden with <offInPart/> -> false in the part
}

TEST(MeasureGraphicAssignTest, PopulateFields)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <details>
    <measGraphicAssign cmper1="1" cmper2="2" inci="0">
      <version>256</version>
      <left>184</left>
      <bottom>82</bottom>
      <width>121</width>
      <height>16</height>
      <fDescID>1</fDescID>
      <displayType>one</displayType>
      <displayHidden/>
      <savedRecord/>
      <origWidth>133</origWidth>
      <origHeight>18</origHeight>
      <graphicCmper>1</graphicCmper>
    </measGraphicAssign>
  </details>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);

    auto details = doc->getDetails();
    ASSERT_TRUE(details);

    auto assign = details->get<details::MeasureGraphicAssign>(SCORE_PARTID, 1, 2, 0);
    ASSERT_TRUE(assign);

    EXPECT_EQ(assign->version, 256u);
    EXPECT_EQ(assign->left, 184);
    EXPECT_EQ(assign->bottom, 82);
    EXPECT_EQ(assign->width, 121);
    EXPECT_EQ(assign->height, 16);
    EXPECT_EQ(assign->fDescId, 1);
    EXPECT_TRUE(assign->hidden);
    EXPECT_TRUE(assign->savedRecord);
    EXPECT_EQ(assign->origWidth, 133);
    EXPECT_EQ(assign->origHeight, 18);
    EXPECT_EQ(assign->graphicCmper, 1);
}

TEST(ShapeGraphicAssignTest, PopulateFields)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <shapeGraphicAssign cmper="1" inci="0">
      <version>256</version>
      <left>728</left>
      <bottom>-580</bottom>
      <width>336</width>
      <height>168</height>
      <fDescID>1</fDescID>
      <displayType>one</displayType>
      <halign>left</halign>
      <valign>top</valign>
      <posFrom>paper</posFrom>
      <fixedPerc/>
      <savedRecord/>
      <origWidth>336</origWidth>
      <origHeight>168</origHeight>
      <graphicCmper>0</graphicCmper>
    </shapeGraphicAssign>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);

    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    // Score assignment (partId = SCORE_PARTID)
    auto g = others->get<others::ShapeGraphicAssign>(SCORE_PARTID, 1, 0);
    ASSERT_TRUE(g) << "ShapeGraphicAssign (cmper=1, inci=0) not found for SCORE_PARTID but does exist";

    EXPECT_EQ(g->version, 256u);
    EXPECT_EQ(g->left, 728);
    EXPECT_EQ(g->bottom, -580);
    EXPECT_EQ(g->width, 336);
    EXPECT_EQ(g->height, 168);
    EXPECT_EQ(g->fDescId, 1);

    // Invariants displayType/posFrom are intentionally not modeled on the class.
    // We still verify alignment & flags populated as expected.
    EXPECT_EQ(g->hAlign, others::ShapeGraphicAssign::HorizontalAlignment::Left);
    EXPECT_EQ(g->vAlign, others::ShapeGraphicAssign::VerticalAlignment::Top);

    // Legacy assignments may omit graphicCmper and are then identified by their own
    // comparator. The lookup must include the class's mandatory zero incidence.
    EXPECT_EQ(others::ShapeGraphicAssign::findForGraphic(doc, SCORE_PARTID, 1), g);

    // Boolean-presence nodes
    EXPECT_TRUE(g->fixedPerc);
    EXPECT_TRUE(g->savedRecord);

    // Not present in XML -> default false
    EXPECT_FALSE(g->hidden);

    EXPECT_EQ(g->origWidth, 336);
    EXPECT_EQ(g->origHeight, 168);
    EXPECT_EQ(g->graphicCmper, 0);
}

// The score has three pages and no leading blank page; the part has one leading blank page and three
// more, so page assignment ID 2 is page 1 of the score and page 2 of the part.
TEST(PageGraphicAssignTest, PageNumbersApplyOddEvenToEveryAssignment)
{
    constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <pageSpec cmper="1"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>1</firstSystem></pageSpec>
    <pageSpec cmper="2"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>2</firstSystem></pageSpec>
    <pageSpec cmper="3"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>3</firstSystem></pageSpec>
    <pageSpec cmper="1" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>-1</firstSystem></pageSpec>
    <pageSpec cmper="2" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>1</firstSystem></pageSpec>
    <pageSpec cmper="3" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>2</firstSystem></pageSpec>
    <pageSpec cmper="4" part="1" shared="false"><height>3168</height><width>2448</width><percent>100</percent><firstSystem>3</firstSystem></pageSpec>
    <partDef cmper="0"><partOrder>0</partOrder><copies>1</copies></partDef>
    <partDef cmper="1"><partOrder>1</partOrder><copies>1</copies></partDef>
    <pageGraphicAssign cmper="0" inci="0">
      <displayType>even</displayType>
      <startPage>1</startPage>
    </pageGraphicAssign>
    <pageGraphicAssign cmper="2" inci="0">
      <displayType>one</displayType>
      <startPage>2</startPage>
      <endPage>2</endPage>
    </pageGraphicAssign>
    <pageGraphicAssign cmper="3" inci="0">
      <displayType>odd</displayType>
      <startPage>3</startPage>
      <endPage>3</endPage>
    </pageGraphicAssign>
  </others>
</finale>
    )xml";

    auto doc = musx::factory::DocumentFactory::create<musx::xml::rapidxml::Document>(xml);
    ASSERT_TRUE(doc);
    const auto others = doc->getOthers();

    const auto evenPages = others->get<others::PageGraphicAssign>(SCORE_PARTID, 0, 0);
    ASSERT_TRUE(evenPages);
    EXPECT_EQ(evenPages->calcPageNumbers(SCORE_PARTID), std::vector<PageCmper>({2}));
    EXPECT_EQ(evenPages->calcPageNumbers(1), std::vector<PageCmper>({2, 4}));
    EXPECT_TRUE(evenPages->calcAppearsOnPage(1, 4));

    const auto singlePage = others->get<others::PageGraphicAssign>(SCORE_PARTID, 2, 0);
    ASSERT_TRUE(singlePage);
    EXPECT_TRUE(singlePage->calcMatchesOddEven(1)) << "a single-page graphic admits any page";
    EXPECT_TRUE(singlePage->calcMatchesOddEven(2));
    EXPECT_EQ(singlePage->calcPageNumbers(SCORE_PARTID), std::vector<PageCmper>({1}));
    EXPECT_EQ(singlePage->calcPageNumbers(1), std::vector<PageCmper>({2}));

    // The assignment ID is page 2 of the score, which is even, and page 3 of the part.
    const auto oddSinglePage = others->get<others::PageGraphicAssign>(SCORE_PARTID, 3, 0);
    ASSERT_TRUE(oddSinglePage);
    EXPECT_TRUE(oddSinglePage->calcPageNumbers(SCORE_PARTID).empty());
    EXPECT_EQ(oddSinglePage->calcPageNumbers(1), std::vector<PageCmper>({3}));
}
