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

#include <sstream>

namespace accidental_tests {

using namespace musx::dom;

constexpr Cmper staffId = 1;
constexpr Cmper measureId = 1;
constexpr Cmper nextMeasureId = 2;
constexpr Cmper partId = 1;
constexpr LayerIndex otherLayer = 1;
constexpr size_t second = 1;
constexpr size_t third = 2;
constexpr int sharp = 1;
constexpr int flat = -1;
constexpr int doubleSharp = 2;
constexpr int octave = music_theory::STANDARD_DIATONIC_STEPS;
constexpr int fInG = -1;
constexpr int dInC = 1;
constexpr int eInC = 2;

struct NoteSpec
{
    int level{};
    int alteration{};
    std::string flags{};
};

struct EntrySpec
{
    std::vector<NoteSpec> notes{};
    std::string flags{};
    Evpu offset{};
    Edu duration{ Edu(NoteType::Quarter) };
    bool rest{};
};

DocumentPtr makeDocument(const std::vector<std::vector<EntrySpec>>& layers,
    const std::string& staffFlags = {}, const std::string& key = {},
    const std::string& extraOthers = {}, const std::string& extraDetails = {},
    PartVoicingPolicy policy = PartVoicingPolicy::Apply, bool includeClefs = true)
{
    std::ostringstream others;
    std::ostringstream details;
    std::ostringstream entries;
    EntryNumber nextEntry = 1;
    for (size_t layer = 0; layer < layers.size(); ++layer) {
        const auto frameId = layer + 1;
        others << "<frameSpec cmper=\"" << frameId << "\"><startEntry>" << nextEntry
               << "</startEntry><endEntry>" << nextEntry + layers[layer].size() - 1 << "</endEntry></frameSpec>";
        details << "<frame" << frameId << ">" << frameId << "</frame" << frameId << ">";
        for (size_t index = 0; index < layers[layer].size(); ++index, ++nextEntry) {
            const auto& entry = layers[layer][index];
            entries << "<entry entnum=\"" << nextEntry << "\" prev=\"" << (index ? nextEntry - 1 : 0)
                    << "\" next=\"" << (index + 1 < layers[layer].size() ? nextEntry + 1 : 0) << "\">"
                    << "<dura>" << entry.duration << "</dura><numNotes>" << entry.notes.size()
                    << "</numNotes><posi>" << entry.offset << "</posi><isValid/><noteDetail/>";
            if (!entry.rest) {
                entries << "<isNote/>";
            }
            entries << entry.flags;
            NoteNumber noteId = 1;
            for (const auto& note : entry.notes) {
                entries << "<note id=\"" << noteId++ << "\"><isValid/><harmLev>" << note.level
                        << "</harmLev><harmAlt>" << note.alteration << "</harmAlt>" << note.flags << "</note>";
            }
            entries << "</entry>";
        }
    }
    auto xml = std::string(R"xml(<finale><options>
        <clefOptions><clefDef index="0"><adjust>-10</adjust></clefDef></clefOptions>
        </options><others>
        <staffSpec cmper="1"><staffLines>5</staffLines><lineSpace>24</lineSpace>)xml")
        + staffFlags + "</staffSpec>"
        + "<measSpec cmper=\"1\"><beats>4</beats><divbeat>1024</divbeat><keySig>" + key + "</keySig></measSpec>"
        + "<measSpec cmper=\"2\"><beats>4</beats><divbeat>1024</divbeat><keySig>" + key + "</keySig></measSpec>"
        + "<partDef cmper=\"0\"/><partDef cmper=\"1\"><partOrder>1</partOrder></partDef>"
        + "<partGlobals cmper=\"65534\"/>"
        + "<instUsed cmper=\"0\" inci=\"0\"><inst>1</inst></instUsed>"
        + "<instUsed cmper=\"0\" inci=\"0\" part=\"1\" shared=\"false\"><inst>1</inst></instUsed>"
        + others.str() + extraOthers + "</others><details>"
        + "<gfhold cmper1=\"1\" cmper2=\"1\"><clefID>0</clefID>" + details.str() + "</gfhold>"
        + "<gfhold cmper1=\"1\" cmper2=\"2\"><clefID>0</clefID>" + details.str() + "</gfhold>"
        + extraDetails + "</details><entries>" + entries.str() + "</entries></finale>";
    if (!includeClefs) {
        const auto start = xml.find("<clefOptions>");
        const auto end = xml.find("</clefOptions>") + std::string("</clefOptions>").size();
        xml.erase(start, end - start);
    }
    return musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml, policy);
}

NoteInfoPtr noteAt(const DocumentPtr& document, size_t entryIndex = 0, size_t noteIndex = 0,
    LayerIndex layer = 0, Cmper requestedPartId = SCORE_PARTID, Cmper requestedMeasureId = measureId,
    musx::util::Fraction timeOffset = 0)
{
    const details::GFrameHoldContext context(document, requestedPartId, staffId, requestedMeasureId, timeOffset);
    return NoteInfoPtr(EntryInfoPtr(context.createEntryFrame(layer), entryIndex), noteIndex);
}

void expectSequence(const DocumentPtr& document, std::initializer_list<bool> expected,
    LayerIndex layer = 0, PitchMode mode = PitchMode::CurrentView)
{
    size_t index = 0;
    for (const bool displayed : expected) {
        const auto note = noteAt(document, index++, 0, layer);
        ASSERT_TRUE(note);
        EXPECT_EQ(note.calcAccidentalIsDisplayed(mode), displayed) << "entry " << index << " layer " << layer;
    }
}

TEST(Accidentals, RepetitionCancellationAndOctavesIgnoreSavedAutomaticFlags)
{
    const auto document = makeDocument({ {
        { { { 0, sharp } } },
        { { { 0, sharp, "<showAcci/>" } } },
        { { { 0, 0 } } },
        { { { 0, 0, "<showAcci/>" } } },
        { { { octave, sharp } } },
        { { { 0, doubleSharp } } },
        { { { 0, sharp } } }
    } });
    expectSequence(document, { true, false, true, false, true, true, true });
    EXPECT_FALSE(noteAt(document)->showAcci);
    EXPECT_TRUE(noteAt(document, second)->showAcci);
    EXPECT_TRUE(noteAt(document, 0, 0, 0, SCORE_PARTID, nextMeasureId).calcAccidentalIsDisplayed());
}

TEST(Accidentals, FrozenSettingsParenthesesAndDeferredInitialAccidental)
{
    for (const std::string suppress : { "<freezeAcci/>", "<tieEnd/>", "<tieEnd/><parenAcci/>" }) {
        const auto document = makeDocument({ {
            { { { 0, sharp, suppress } } }, { { { 0, sharp } } }, { { { 0, sharp } } }
        } });
        expectSequence(document, { false, true, false });
    }
    const auto document = makeDocument({ {
        { { { 0, 0, "<freezeAcci/><showAcci/>" } } },
        { { { 0, 0, "<parenAcci/>" } } },
        { { { 0, sharp, "<freezeAcci/><parenAcci/>" } } },
        { { { 0, sharp, "<tieEnd/><freezeAcci/><showAcci/>" } } }
    } });
    expectSequence(document, { true, true, false, true });
}

TEST(Accidentals, KeyBaselineAndHiddenKeyModes)
{
    const std::vector<std::vector<EntrySpec>> layers{ {
        { { { fInG, 0 } } }, { { { fInG, flat } } }, { { { fInG, 0 } } }
    } };
    expectSequence(makeDocument(layers, {}, "<key>1</key>"), { false, true, true });
    expectSequence(makeDocument(layers, "<hideKeySigs/>", "<key>1</key>"), { false, true, true });
    expectSequence(makeDocument(layers, {}, "<key>1</key><hideKeySigShowAccis/>"), { true, true, true });
    expectSequence(makeDocument(layers, {}, "<key>1</key><keyless/>"), { true, true, true });
}

TEST(Accidentals, SharedHistoryAndLayerRedisplay)
{
    const std::vector<std::vector<EntrySpec>> layers{
        { { { { 0, sharp } } }, { { { eInC, 0 } } }, { { { 0, sharp } } } },
        { { { { dInC, 0 } } }, { { { 0, sharp } } }, { { { 0, sharp } } } }
    };
    const auto shared = makeDocument(layers);
    expectSequence(shared, { true, false, false });
    expectSequence(shared, { false, false, false }, otherLayer);
    const auto redisplay = makeDocument(layers, "<redisplayLayerAccis/>");
    expectSequence(redisplay, { true, false, false });
    expectSequence(redisplay, { false, true, false }, otherLayer);
}

TEST(Accidentals, SimultaneousUnisonsAndConflictingAlterations)
{
    const auto unisons = makeDocument({ { { { { 0, sharp }, { 0, sharp } } } } });
    EXPECT_TRUE(noteAt(unisons).calcAccidentalIsDisplayed());
    EXPECT_FALSE(noteAt(unisons, 0, second).calcAccidentalIsDisplayed());
    const auto conflicts = makeDocument({ {
        { { { 0, 0 }, { 0, sharp } } }, { { { 0, sharp } } }, { { { 0, sharp } } }
    } });
    EXPECT_TRUE(noteAt(conflicts).calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(conflicts, 0, second).calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(conflicts, second).calcAccidentalIsDisplayed());
    EXPECT_FALSE(noteAt(conflicts, third).calcAccidentalIsDisplayed());
}

TEST(Accidentals, SimultaneousCourtesyPriorityAndTies)
{
    const auto courtesy = makeDocument({ { { { { 0, sharp },
        { 0, sharp, "<freezeAcci/><showAcci/><parenAcci/>" } } } } });
    EXPECT_FALSE(noteAt(courtesy).calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(courtesy, 0, second).calcAccidentalIsDisplayed());
    const auto forced = makeDocument({ { { { { 0, sharp, "<freezeAcci/><showAcci/>" },
        { 0, sharp, "<freezeAcci/><showAcci/>" } } } } });
    EXPECT_TRUE(noteAt(forced).calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(forced, 0, second).calcAccidentalIsDisplayed());
    const auto ties = makeDocument({ { { { { 0, sharp, "<tieEnd/>" }, { 0, sharp } } } } },
        "<redisplayLayerAccis/>");
    EXPECT_FALSE(noteAt(ties).calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(ties, 0, second).calcAccidentalIsDisplayed());
}

TEST(Accidentals, HiddenEntriesContributeHistoryAndRestsDoNot)
{
    const auto document = makeDocument({ {
        { { { 0, sharp } }, "<ignore/>" },
        { { { 0, sharp } } },
        { { { 0, 0 } }, {}, 0, Edu(NoteType::Quarter), true },
        { { { 0, sharp } } }
    } });
    expectSequence(document, { false, false, false, false });
    for (const std::string flags : { "<notationStyle>percussion</notationStyle>",
        "<notationStyle>tab</notationStyle>", "<altNotation>rhythmic</altNotation>",
        "<altNotation>blank</altNotation>" }) {
        expectSequence(makeDocument({ { { { { 0, sharp, "<freezeAcci/><showAcci/>" } } } } }, flags), { false });
    }
}

TEST(Accidentals, GracePositionsAndManualOffsets)
{
    constexpr Evpu earlyGrace = -24;
    constexpr Evpu lateGrace = -12;
    const auto document = makeDocument({ {
        { { { 0, sharp } }, "<graceNote/>", earlyGrace, Edu(NoteType::Eighth) },
        { { { 0, 0 } }, "<graceNote/>", lateGrace, Edu(NoteType::Eighth) },
        { { { 0, sharp } } }
    } });
    expectSequence(document, { true, true, true });
    const auto shifted = makeDocument({
        { { { { 0, sharp } }, {}, lateGrace } },
        { { { { 0, sharp } }, {}, earlyGrace } }
    });
    EXPECT_FALSE(noteAt(shifted).calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(shifted, 0, 0, otherLayer).calcAccidentalIsDisplayed());
}

TEST(Accidentals, VoiceTwoUsesElapsedPosition)
{
    const auto document = makeDocument({ {
        { { { dInC, 0 } }, "<controller/>", 0, Edu(NoteType::Half) },
        { { { 0, sharp } }, "<v2/>" },
        { { { 0, sharp } }, "<v2/>" },
        { { { 0, sharp } } }
    } });
    expectSequence(document, { false, true, false, false });
}

TEST(Accidentals, CachePopulatesWholeMeasureAcrossWrappersAndLayers)
{
    const auto document = makeDocument({
        { { { { 0, sharp } } }, { { { 0, 0 } } } },
        { { { { dInC, sharp } } } }
    });
    const auto& cache = document->getAccidentalCache();
    EXPECT_TRUE(cache.measures.empty());
    document->clearAccidentalCache();
    EXPECT_EQ(&cache, &document->getAccidentalCache());
    const auto first = noteAt(document);
    EXPECT_TRUE(first.calcAccidentalIsDisplayed());
    EXPECT_FALSE(cache.measures.empty());
    std::const_pointer_cast<Note>(noteAt(document, second).operator->())->harmAlt = sharp;
    std::const_pointer_cast<Note>(noteAt(document, 0, 0, otherLayer).operator->())->harmAlt = 0;
    // Queries through new frames and wrappers reuse the complete pre-edit snapshot, including false values.
    EXPECT_TRUE(noteAt(document, second).calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(document, 0, 0, otherLayer).calcAccidentalIsDisplayed());
    document->clearAccidentalCache();
    EXPECT_EQ(&cache, &document->getAccidentalCache());
    EXPECT_TRUE(cache.measures.empty());
    EXPECT_FALSE(noteAt(document, second).calcAccidentalIsDisplayed());
    EXPECT_FALSE(noteAt(document, 0, 0, otherLayer).calcAccidentalIsDisplayed());
    std::const_pointer_cast<Note>(noteAt(document, second).operator->())->harmAlt = 0;
    EXPECT_FALSE(noteAt(document, second).calcAccidentalIsDisplayed());
    document->clearAccidentalCache();
    EXPECT_TRUE(noteAt(document, second).calcAccidentalIsDisplayed());
    EXPECT_TRUE(first.calcAccidentalIsDisplayed());
}

TEST(Accidentals, PitchModeAndPartCachesRemainSeparate)
{
    const auto document = makeDocument({ { { { { 0, 0 } } } } },
        "<transposition><chromatic><diatonic>0</diatonic><alteration>1</alteration></chromatic></transposition>", {},
        "<partGlobals cmper=\"65534\" part=\"1\" shared=\"false\"><showTransposed/></partGlobals>");
    const auto score = noteAt(document);
    const auto part = noteAt(document, 0, 0, 0, partId);
    EXPECT_FALSE(score.calcAccidentalIsDisplayed());
    EXPECT_TRUE(score.calcAccidentalIsDisplayed(PitchMode::Written));
    EXPECT_FALSE(score.calcAccidentalIsDisplayed(PitchMode::Concert));
    EXPECT_TRUE(part.calcAccidentalIsDisplayed());
    EXPECT_FALSE(part.calcAccidentalIsDisplayed(PitchMode::Concert));
    EXPECT_FALSE(score.calcAccidentalIsDisplayed());
}

TEST(Accidentals, PartVoicingExcludesOtherLayersFromHistory)
{
    const std::vector<std::vector<EntrySpec>> layers{
        { { { { 0, sharp } } }, { { { dInC, 0 } } } },
        { { { { dInC, 0 } } }, { { { 0, sharp } } } }
    };
    const std::string voicing = "<voicingDef cmper=\"1\" part=\"1\" shared=\"false\">"
        "<enabled/><singleLayer>1</singleLayer></voicingDef>";
    const auto document = makeDocument(layers, {}, {}, voicing);
    EXPECT_FALSE(noteAt(document, second, 0, otherLayer).calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(document, second, 0, otherLayer, partId).calcAccidentalIsDisplayed());
    const auto ignored = makeDocument(layers, {}, {}, voicing, {}, PartVoicingPolicy::Ignore);
    EXPECT_FALSE(noteAt(ignored, second, 0, otherLayer, partId).calcAccidentalIsDisplayed());
}

TEST(Accidentals, CrossStaffPlacementKeepsOwningStaffHistory)
{
    const auto document = makeDocument({ {
        { { { 0, sharp, "<crossStaff/>" } }, "<crossStaff/>" }, { { { 0, sharp } } }
    } }, {}, {}, "<staffSpec cmper=\"2\"><staffLines>5</staffLines></staffSpec>",
        "<crossStaff entnum=\"1\" inci=\"0\"><noteID>1</noteID><instrument>2</instrument></crossStaff>");
    expectSequence(document, { true, false });
}

TEST(Accidentals, CachedResultsDoNotRetainDocument)
{
    DocumentWeakPtr weak;
    {
        const auto document = makeDocument({ { { { { 0, sharp } } } } });
        weak = document;
        EXPECT_TRUE(noteAt(document).calcAccidentalIsDisplayed());
    }
    EXPECT_TRUE(weak.expired());
}

TEST(Accidentals, KeySignatureTranspositionUsesWrittenBaseline)
{
    const auto document = makeDocument({ { { { { eInC, 0 } } }, { { { eInC, sharp } } } } },
        "<transposition><keysig><interval>1</interval><adjust>2</adjust></keysig></transposition>");
    expectSequence(document, { false, true }, 0, PitchMode::Written);
    expectSequence(document, { false, true }, 0, PitchMode::Concert);
    const auto hidden = makeDocument({ { { { { eInC, 0 } } } } },
        "<transposition><keysig><interval>1</interval><adjust>2</adjust></keysig></transposition><hideKeySigsShowAccis/>");
    EXPECT_FALSE(noteAt(hidden).calcAccidentalIsDisplayed(PitchMode::Concert));
    EXPECT_TRUE(noteAt(hidden).calcAccidentalIsDisplayed(PitchMode::Written));
}

TEST(Accidentals, EnharmonicSpellingKeepsSeparateDiatonicHistory)
{
    const auto document = makeDocument({ {
        { { { 0, sharp } } }, { { { dInC, flat } } }, { { { 0, sharp } } }
    } });
    expectSequence(document, { true, true, false });
    const auto respelled = makeDocument({ {
        { { { 0, sharp } } }, { { { 0, sharp } } }, { { { 0, sharp } } }
    } }, {}, {}, {}, "<noteAlter entnum=\"2\" inci=\"0\"><noteID>1</noteID><enharmonic/></noteAlter>");
    expectSequence(respelled, { true, true, false });
}

TEST(Accidentals, StaffStyleAtEntryControlsNotationVisibility)
{
    const auto document = makeDocument({ {
        { { { 0, sharp } } }, { { { 0, 0 } } }, { { { 0, sharp } } }
    } }, "<hasStyles/>", {},
        "<staffStyle cmper=\"1\"><staffLines>5</staffLines><altNotation>rhythmic</altNotation><altLayer>0</altLayer>"
        "<mask><altNotation/></mask></staffStyle>"
        "<staffStyleAssign cmper=\"1\" inci=\"0\"><style>1</style><startMeas>1</startMeas>"
        "<endMeas>1</endMeas><startEdu>1024</startEdu><endEdu>2047</endEdu></staffStyleAssign>");
    expectSequence(document, { true, false, true });
}

TEST(Accidentals, VoicingExcludesIndividualChordNotes)
{
    const auto document = makeDocument({ {
        { { { 0, sharp }, { eInC, 0 } } }, { { { 0, sharp } } }
    } }, {}, {}, "<voicingDef cmper=\"1\" part=\"1\" shared=\"false\"><enabled/>"
        "<voicingType>useMultipleLayers</voicingType><singleVoiceType>selected</singleVoiceType>"
        "<select1st/><selectSingleNote/></voicingDef>");
    EXPECT_TRUE(noteAt(document).calcAccidentalIsDisplayed());
    EXPECT_FALSE(noteAt(document, 0, 0, 0, partId).calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(document, second, 0, 0, partId).calcAccidentalIsDisplayed());
}

TEST(Accidentals, CustomKeyAlterationUnits)
{
    std::vector<char> xml;
    musxtest::readFile(musxtest::getInputPath() / "keysigs.enigmaxml", xml);
    const auto document = musx::factory::DocumentFactory::create<musx::xml::tinyxml2::Document>(xml);
    constexpr Cmper nonlinearMeasure = 5;
    constexpr Cmper edoMajorMeasure = 6;
    constexpr Cmper edoMinorMeasure = 7;
    // E F G A B C D E, each at its first occurrence in its octave.
    const std::vector<std::pair<Cmper, std::vector<bool>>> cases{
        { nonlinearMeasure, { true, false, false, true, false, false, false, false } },
        { edoMajorMeasure, { false, false, true, true, true, true, true, true } },
        { edoMinorMeasure, { true, true, true, false, false, true, true, true } }
    };
    for (const auto& [measure, expected] : cases) {
        for (size_t index = 0; index < expected.size(); ++index) {
            const auto note = noteAt(document, index, 0, 0, SCORE_PARTID, measure);
            ASSERT_TRUE(note);
            EXPECT_EQ(note.calcAccidentalIsDisplayed(PitchMode::Concert), expected[index])
                << "measure " << measure << " note " << index;
        }
    }
}

TEST(Accidentals, TimeOffsetContextsRemainSeparate)
{
    const auto document = makeDocument({ { { { { 0, sharp } } }, { { { 0, 0 } } } } });
    EXPECT_TRUE(noteAt(document).calcAccidentalIsDisplayed());
    const auto offset = musx::util::Fraction::fromEdu(Edu(NoteType::Quarter));
    EXPECT_TRUE(noteAt(document, second, 0, 0, SCORE_PARTID, measureId, offset).calcAccidentalIsDisplayed());
}

TEST(Accidentals, FailedCalculationDoesNotPublishPartialResults)
{
    const auto document = makeDocument({
        { { { { 0, sharp } }, {}, 0, Edu(NoteType::Quarter), true } },
        { { { { dInC, sharp } } } }
    }, {}, {}, {}, {}, PartVoicingPolicy::Apply, false);
    const auto first = noteAt(document);
    EXPECT_THROW((void)first.calcAccidentalIsDisplayed(), std::invalid_argument);
    const auto clefs = std::make_shared<options::ClefOptions>(document);
    clefs->clefDefs.emplace_back(std::make_shared<options::ClefOptions::ClefDef>(clefs));
    document->getOptions()->add(options::ClefOptions::XmlNodeName, clefs);
    std::const_pointer_cast<Entry>(first.getEntryInfo()->getEntry())->isNote = true;
    EXPECT_TRUE(first.calcAccidentalIsDisplayed());
    EXPECT_TRUE(noteAt(document, 0, 0, otherLayer).calcAccidentalIsDisplayed());
}

TEST(Accidentals, SimultaneousRedisplayStillResolvesUnisonsTogether)
{
    const auto document = makeDocument({
        { { { { 0, sharp } } }, { { { 0, sharp } } } },
        { { { { dInC, 0 } } }, { { { 0, sharp } } } }
    }, "<redisplayLayerAccis/>");
    expectSequence(document, { true, false });
    expectSequence(document, { false, false }, otherLayer);
}

} // namespace accidental_tests
