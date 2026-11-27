/*
 * Copyright (C) 2026, Robert Patterson
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

#include <string>

#include "gtest/gtest.h"
#include "musx/musx.h"
#include "test_utils.h"

using namespace musx::dom;
using namespace musx::util;

namespace {

/// @brief The knobs Finale can use to suppress a fretboard, plus the root that selects the diagram.
struct ChordSetup
{
    bool showFretboards{true};      ///< options::ChordOptions::showFretboards
    bool hideFretboards{false};     ///< others::Staff::hideFretboards
    bool showFretboard{true};       ///< details::ChordAssign::showFretboard
    bool useFretboardFont{false};   ///< details::ChordAssign::useFretboardFont
    int rootScaleNum{0};            ///< Selects which of the group's twelve diagrams is used.
    bool useFretboardStyle{true};   ///< details::ChordAssign::fbStyleId names the style when true, or is zero.
};

/// @brief Emits one diagram, padding the group to the twelve that FretboardGroup requires.
std::string diagram(int pitchClassIndex, const std::string& body)
{
    return "<fretboard cmper1=\"10\" cmper2=\"" + std::to_string(pitchClassIndex) + "\">" + body + "</fretboard>";
}

/// @brief Builds a one-chord document whose fretboard group carries three diagrams of interest.
/// @details The document is in C major with no transposition, so the chord's scale degree maps to a pitch
/// class and then to a diagram index: degree 0 (C) selects diagram 3, degree 1 (D) diagram 5, and degree 2
/// (E) diagram 7. The remaining diagrams exist only to satisfy the group's twelve-diagram requirement.
std::string makeXml(const ChordSetup& setup)
{
    const auto flag = [](bool value, const char* name) {
        return value ? std::string("<") + name + "/>" : std::string();
    };

    // Diagram 3: every string played, a finger number, and a barre, on a diagram starting at fret 6.
    std::string diagrams = diagram(3,
        "<numFrets>4</numFrets><fretNum>6</fretNum><showNum/><numFretCells>6</numFretCells><numFretBarres>1</numFretBarres>"
        "<cell><string>6</string><fret>1</fret><shape>closed</shape></cell>"
        "<cell><string>5</string><fret>3</fret><shape>closed</shape></cell>"
        "<cell><string>4</string><fret>3</fret><shape>closed</shape><fingerNum>3</fingerNum></cell>"
        "<cell><string>3</string><fret>1</fret><shape>closed</shape></cell>"
        "<cell><string>2</string><fret>1</fret><shape>closed</shape></cell>"
        "<cell><string>1</string><fret>1</fret><shape>closed</shape></cell>"
        "<barre><fret>0</fret><startString>1</startString><endString>6</endString></barre>");
    // Diagram 5: every string has a cell, but one is explicitly muted.
    diagrams += diagram(5,
        "<numFrets>4</numFrets><fretNum>1</fretNum><numFretCells>6</numFretCells>"
        "<cell><string>6</string><fret>0</fret><shape>muted</shape></cell>"
        "<cell><string>5</string><fret>3</fret><shape>closed</shape></cell>"
        "<cell><string>4</string><fret>2</fret><shape>closed</shape></cell>"
        "<cell><string>3</string><fret>0</fret><shape>open</shape></cell>"
        "<cell><string>2</string><fret>1</fret><shape>closed</shape></cell>"
        "<cell><string>1</string><fret>0</fret><shape>open</shape></cell>");
    // Diagram 7: strings 5 and 6 have no cell at all and no muted marker.
    diagrams += diagram(7,
        "<numFrets>4</numFrets><fretNum>1</fretNum><numFretCells>4</numFretCells>"
        "<cell><string>4</string><fret>2</fret><shape>closed</shape></cell>"
        "<cell><string>3</string><fret>2</fret><shape>closed</shape></cell>"
        "<cell><string>2</string><fret>1</fret><shape>closed</shape></cell>"
        "<cell><string>1</string><fret>0</fret><shape>open</shape></cell>");
    for (int index = 0; index < 12; index++) {
        if (index != 3 && index != 5 && index != 7) {
            diagrams += diagram(index, "<numFrets>4</numFrets><fretNum>1</fretNum>");
        }
    }

    return
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<finale>"
        "<options><chordOptions>" + flag(setup.showFretboards, "showFretboards") + "</chordOptions></options>"
        "<others>"
        "<staffSpec cmper=\"1\"><staffLines>5</staffLines>" + flag(setup.hideFretboards, "hideFretboards") + "</staffSpec>"
        "<measSpec cmper=\"1\"><beats>4</beats><divbeat>1024</divbeat></measSpec>"
        "<fretStyle cmper=\"2\"><name>Test Style</name><fretNumText>fr.</fretNumText></fretStyle>"
        "<fretGroup cmper=\"10\" inci=\"0\"><fretInstID>1</fretInstID><name>Test Group</name></fretGroup>"
        "<fretInst cmper=\"1\"><numFrets>21</numFrets><numStrings>6</numStrings><name>Standard Guitar</name>"
        "<string><pitch>64</pitch></string><string><pitch>59</pitch></string><string><pitch>55</pitch></string>"
        "<string><pitch>50</pitch></string><string><pitch>45</pitch></string><string><pitch>40</pitch></string>"
        "</fretInst>"
        "</others>"
        "<details>"
        "<chordAssign cmper1=\"1\" cmper2=\"1\" inci=\"0\">"
        "<suffix>10</suffix>" + std::string(setup.useFretboardStyle ? "<fbStyleID>2</fbStyleID>" : "") + "<rootScaleNum>" + std::to_string(setup.rootScaleNum) + "</rootScaleNum>"
        + flag(setup.showFretboard, "showFretboard") + flag(setup.useFretboardFont, "useFretFont") +
        "</chordAssign>"
        + diagrams +
        "</details>"
        "</finale>";
}

std::optional<FretboardDisplayData> calcFor(const ChordSetup& setup)
{
    const auto xml = makeXml(setup);
    const auto doc = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    EXPECT_TRUE(doc);
    if (!doc) {
        return std::nullopt;
    }
    const auto chord = doc->getDetails()->get<details::ChordAssign>(SCORE_PARTID, 1, 1, 0);
    EXPECT_TRUE(chord);
    const auto measure = doc->getOthers()->get<others::Measure>(SCORE_PARTID, 1);
    EXPECT_TRUE(measure);
    if (!chord || !measure) {
        return std::nullopt;
    }
    return calcFretboardDisplayData(chord, measure->createKeySignature(StaffCmper(1)),
        KeySignature::KeyContext::Written);
}

} // namespace

TEST(FretboardDisplayDataTest, ResolvesCellsAndBarresInDisplayFrets)
{
    const auto fretboard = calcFor(ChordSetup{});
    ASSERT_TRUE(fretboard);
    EXPECT_EQ(fretboard->stringCount, 6);
    EXPECT_EQ(fretboard->fretCount, 4);
    EXPECT_EQ(fretboard->fretboardNumber, 6);
    EXPECT_TRUE(fretboard->showFretboardNumber);
    EXPECT_EQ(fretboard->fretNumberText, std::optional<std::string>("fr."));
    EXPECT_EQ(fretboard->unplayedStringDisplay, UnplayedStringDisplay::None);

    // Cells keep Finale's source order, shifted from the diagram-relative fret to the display fret.
    ASSERT_EQ(fretboard->cells.size(), static_cast<size_t>(6));
    const auto expectCell = [&](size_t index, int string, int fret, int finger) {
        EXPECT_EQ(fretboard->cells[index].string, string) << "cell " << index;
        EXPECT_EQ(fretboard->cells[index].fret, fret) << "cell " << index;
        EXPECT_EQ(fretboard->cells[index].finger, finger) << "cell " << index;
    };
    expectCell(0, 6, 6, 0);
    expectCell(1, 5, 8, 0);
    expectCell(2, 4, 8, 3);
    expectCell(3, 3, 6, 0);
    expectCell(4, 2, 6, 0);
    expectCell(5, 1, 6, 0);

    // The barre is stored as fret 0, meaning the diagram's first fret. That is the cell coordinate's fret 1,
    // which on a diagram starting at fret 6 displays as fret 6, coinciding with the cells on strings 1 and 6.
    ASSERT_EQ(fretboard->barres.size(), static_cast<size_t>(1));
    EXPECT_EQ(fretboard->barres[0].fret, 6);
    EXPECT_EQ(fretboard->barres[0].startString, 1);
    EXPECT_EQ(fretboard->barres[0].endString, 6);
}

TEST(FretboardDisplayDataTest, ReportsUnplayedStrings)
{
    auto setup = ChordSetup{};

    // Diagram 5 covers every string but marks one of them muted.
    setup.rootScaleNum = 1;
    const auto muted = calcFor(setup);
    ASSERT_TRUE(muted);
    EXPECT_EQ(muted->unplayedStringDisplay, UnplayedStringDisplay::Muted);

    // Diagram 7 leaves strings 5 and 6 out entirely, with no muted marker to show for them.
    setup.rootScaleNum = 2;
    const auto blank = calcFor(setup);
    ASSERT_TRUE(blank);
    EXPECT_EQ(blank->unplayedStringDisplay, UnplayedStringDisplay::Blank);

    // A diagram whose first fret is 1 needs no shift, so open strings and stopped frets pass through.
    ASSERT_EQ(blank->cells.size(), static_cast<size_t>(4));
    EXPECT_EQ(blank->cells[0].fret, 2);
    EXPECT_EQ(blank->cells[3].fret, 0);
}

TEST(FretboardDisplayDataTest, SuppressedByChordOptions)
{
    auto setup = ChordSetup{};
    setup.showFretboards = false;
    EXPECT_FALSE(calcFor(setup));
}

TEST(FretboardDisplayDataTest, SuppressedByStaff)
{
    auto setup = ChordSetup{};
    setup.hideFretboards = true;
    EXPECT_FALSE(calcFor(setup));
}

TEST(FretboardDisplayDataTest, SuppressedByChordAssignment)
{
    auto setup = ChordSetup{};
    setup.showFretboard = false;
    EXPECT_FALSE(calcFor(setup));

    // A fretboard rendered from a font carries no structured diagram to report.
    setup = ChordSetup{};
    setup.useFretboardFont = true;
    EXPECT_FALSE(calcFor(setup));
}

TEST(FretboardDisplayDataTest, OmitsFretNumberTextWithoutStyle)
{
    auto setup = ChordSetup{};
    setup.useFretboardStyle = false;
    const auto fretboard = calcFor(setup);
    ASSERT_TRUE(fretboard);
    EXPECT_FALSE(fretboard->fretNumberText);
}
