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
#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include "musx/util/Logger.h"
#include "musx/util/Fraction.h"

#include "BaseClasses.h"
#include "CommonClasses.h"
 // do not add other dom class dependencies. Use .cpp files for implementations that need total class access.

namespace musx {
namespace dom {

namespace details {
class Baseline;
class BaselineNoInci;
class ShapeNoteBase;
}

namespace others {

class StaffStyle;
/**
 * @class Staff
 * @brief Represents the definition of a Finale staff.
 *
 * The cmper is the staff ID. This class is identified by the XML node name "staffSpec".
 */
class Staff : public OthersBase
{
public:
    /**
     * @enum AutoNumberingStyle
     * @brief Enum for auto-numbering style. Auto-numbering is based on #instUuid.
     */
    enum class AutoNumberingStyle
    {
        ArabicSuffix,       ///< Arabic numeral suffix (default). May not appear in xml.
        RomanSuffix,        ///< Roman numeral suffix.
        OrdinalPrefix,      ///< Ordinal number prefix: 1st 2nd 3rd, ...
        AlphaSuffix,        ///< Alphabetic suffix.
        ArabicPrefix        ///< Arabic numeral prefix (with dot): 1. 2. 3. ...
    };

    /**
     * @enum NotationStyle
     * @brief Enum for notation style.
     */
    enum class NotationStyle
    {
        Standard,
        Percussion,
        Tablature
    };

    /**
     * @enum HideMode
     * @brief Enum for hide mode.
     */
    enum class HideMode
    {
        None,           ///< Do not hide
        Cutaway,        ///< Cutaway score
        ScoreParts,     ///< Collapse in score and parts
        Score           ///< Collapse in score only
    };

    /**
     * @enum AlternateNotation
     * @brief Enum for alternate notation styles.
     */
    enum class AlternateNotation
    {
        Normal,         ///< Normal Notation (default)
        SlashBeats,     ///< Slash Notation (on beats)
        Rhythmic,       ///< Rhythmic Notation
        OneBarRepeat,   ///< One Bar Repeat(s)
        TwoBarRepeat,   ///< Two Bar Repeat(s)
        BlankWithRests, ///< Blank Notation with Rests
        Blank           ///< Blank Notation
    };

    /**
     * @class KeySigTransposition
     * @brief Represents key signature transposition details.
     */
    class KeySigTransposition
    {
    public:
        int interval{};     ///< The diatonic transposition interval.
        int adjust{};       ///< The adjustment to the number of sharps or flats in the key signature.

        static const xml::XmlElementArray<KeySigTransposition>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /**
     * @class ChromaticTransposition
     * @brief Represents chromatic transposition details.
     */
    class ChromaticTransposition
    {
    public:
        int alteration{};   ///< The alteration that defines the chromatic interval (in chromatic half-steps). See @ref music_theory::Transposer for more information.
        int diatonic{};     ///< The diatonic interval. See @ref music_theory::Transposer for more information.

        static const xml::XmlElementArray<ChromaticTransposition>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /**
     * @class Transposition
     * @brief Encapsulates transposition options for a staff.
     *
     * Contains flags (such as setToClef and noKeyOpt) and shared pointers
     * to embedded transposition types: key signature and chromatic.
     */
    class Transposition : public ContainedClassBase
    {
    public:
        using  ContainedClassBase::ContainedClassBase;

        bool setToClef{};               ///< If true, forces the clef in #Staff::transposedClef.
        bool noSimplifyKey{};           ///< Inverse of "Simplify Key" (xml node is `<noKeyOpt>`)

        /// Shared pointer to the key signature transposition details, if any.
        std::shared_ptr<KeySigTransposition> keysig;

        /// Shared pointer to the chromatic transposition details, if any.
        std::shared_ptr<ChromaticTransposition> chromatic;

        /// @brief Returns true if the input has the same contents as this.
        bool isSame(const Transposition& other) const;

        void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
        {
            ContainedClassBase::integrityCheck(ptrToThis);
            Cmper thisCmper = 0;
            if (const auto parent = getParent<others::Staff>()) {
                thisCmper = parent->getCmper();
            }
            if (!keysig && !chromatic) {
                MUSX_INTEGRITY_ERROR("Transposition settings " + std::to_string(thisCmper) + " has neither a key signature nor chromatic settings.");
            } else if (keysig && chromatic) {
                MUSX_INTEGRITY_ERROR("Transposition settings " + std::to_string(thisCmper) + " has both key signature and chromatic settings.");
            }
        }

        static const xml::XmlElementArray<Transposition>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /** @brief Constructor function */
    explicit Staff(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // WARNING: Any fields added here must have a mask added in StaffStyle (if it does not already exist)
    //          and must be added to StaffComposite::applyStyle.

    // Public properties corresponding to the XML structure
    NotationStyle notationStyle{};  ///< Standard, percussion, or tablature
    std::shared_ptr<FontInfo> noteFont; ///< The custom font to use for noteheads. Guaranteed non-null by #integrityCheck if #useNoteFont is `true`.
    bool useNoteShapes{};           ///< Indicates that note shapes should be used. It is ignored is no staff- or staffstyle-specific note shapes exist.
    bool useNoteFont{};             ///< Indicates if #noteFont should be used.
    ClefIndex defaultClef{};        ///< Index of default clef for the staff.
    ClefIndex transposedClef{};     ///< Index of transposed clef for the staff. Only used if #Transposition::setToClef is true.
    std::optional<int> staffLines{}; ///< Number of lines in the staff (if no custom staff)
    std::optional<std::vector<int>> customStaff; ///< A list of stafflines from 0..26 where a standard 5-line staff is values 11, 12, 13, 14, 15.
    Evpu lineSpace{};               ///< Distance between staff lines.
    std::string instUuid;           ///< Unique identifier for the type of instrument.
    int capoPos{};                  ///< "Capo Position" (Tablature Staff Attributes)
    int lowestFret{};               ///< "Default Lowest Fret" (Tablature Staff Attributes)
    bool floatKeys{};               ///< "Independent Key Signature"
    bool floatTime{};               ///< "Independent Time Signature"
    bool blineBreak{};              ///< "Break Barlines Between Staves"
    bool rbarBreak{};               ///< "Break Repeat Barlines Between Staves"
    bool hasStyles{};               ///< Indicates that this staff has staff style assignments
    bool showNameInParts{};         ///< "Display Staff Name in Parts" (xml node is `<showNameParts>`)
    bool showNoteColors{};          ///< "Color Noteheads" (Score Manager)
    // No fixture has a partially shared Staff (part-specific staffSpec records are fully unshared).
    // If one is found, Transposition must be copied and rebound before overlaying its nested fields.
    // See PartContextRebinder<details::CenterShape> and the CenterShape field mappings for the pattern.
    std::shared_ptr<Transposition> transposition; ///< Transposition details, if non-null.
    bool hideNameInScore{};         ///< Inverse of "Display Staff Name in Score" (xml node is `<hideStfNameInScore>`)
    Evpu botBarlineOffset{};        ///< Offset for the bottom barline.
    AlternateNotation altNotation{};///< Alternate notation on the staff
    LayerIndex altLayer{};          ///< Alternate notation Apply To Layer index (0..3)
    bool altHideArtics{};           ///< Hide Articulations in alternate notation (in Apply-To Layer)
    bool altHideLyrics{};           ///< Hide Lyrics in alternate notation (in Apply-To Layer)
    bool altHideSmartShapes{};      ///< Hide Smart Shapes (in Apply-To layer: probably only affects entry-attached shapes)
    bool altRhythmStemsUp{};        ///< Show stems up in alternate Rhythmic Notation mode
    bool altSlashDots{};            ///< Show dots on beat slashes in compound meter
    bool altHideOtherNotes{};       ///< Hide notes in other layers in alternate notation
    bool altHideOtherArtics{};      ///< Hide articulations in other layers in alternate notation
    bool altHideExpressions{};      ///< Hide Expressions in alternate notation (in Apply-To Layer)
    bool altHideOtherLyrics{};      ///< Hide lyrics in other layers in alternate notation
    bool altHideOtherSmartShapes{}; ///< Hide Smart Shapes in other layers in alternate notation
    bool altHideOtherExpressions{}; ///< Hide Expressions in other layers in alternate notation
    bool hideRepeatBottomDot{};     ///< Inverse of "Bottom Repeat Dot" in Staff Setup dialog
    bool flatBeams{};               ///< "Flat Beams"
    bool hideFretboards{};          ///< Inverse of "Display Fretboards"
    bool blankMeasure{};            ///< Inverse of "Display Rests in Empty Measures"
    bool hideRepeatTopDot{};        ///< Inverse of "Top Repeat Dot" in Staff Setup dialog
    bool hideLyrics{};              ///< Inverse of "Display Lyrics"
    bool noOptimize{};              ///< Inverse of "Allow Hiding When Empty"
    Evpu topBarlineOffset{};        ///< Offset for the top barline.
    bool hideMeasNums{};            ///< Inverse of "Display Measure Numbers"
    bool hideRepeats{};             ///< Inverse of "Display Endings and Text Repeats"
    bool hideBarlines{};            ///< Inverse of "Display Barlines"
    bool hideRptBars{};             ///< Inverse of "Display Repeat Bars"
    bool hideKeySigs{};             ///< Inverse of "Display Key Signatures"
    bool hideTimeSigs{};            ///< Inverse of "Display Time Signatures in Score"
    bool hideClefs{};               ///< Inverse of "Display Clefs"
    bool hideStaffLines{};          ///< Inverse of "Display Staff Lines"
    bool hideChords{};              ///< Inverse of "Display Chords"
    bool noKey{};                   ///< "Ignore Key Signatures"
    Evpu dwRestOffset{};            ///< Offset for downstem rests.
    Evpu wRestOffset{};             ///< Offset for whole rests.
    Evpu hRestOffset{};             ///< Offset for half rests.
    Evpu otherRestOffset{};         ///< Offset for other rests.
    bool hideRests{};               ///< Inverse of "Display Rests"
    bool hideTies{};                ///< Inverse of "Display Ties"
    bool hideDots{};                ///< Inverse of "Display Augmentation Dots"
    int stemReversal{};             ///< Stem reversal value.
    Cmper fullNameTextId{};         ///< Full name @ref TextBlock ID. (xml node is `<fullName>`)
    Cmper abbrvNameTextId{};        ///< Abbreviated name @ref TextBlock ID. (xml node is `<abbrvName>`)
    Evpu botRepeatDotOff{};         ///< Offset for bottom repeat dots.
    Evpu topRepeatDotOff{};         ///< Offset for top repeat dots.
    Efix vertTabNumOff{};           ///< Vertical offset for tab number. (Tablature Staff Attributes)
    bool showTabClefAllSys{};       ///< Inverse of "Show Clef Only On First Measure" (Tablature Staff Attributes)
    bool useTabLetters{};           ///< "Use Letters" (Tablature Staff Attributes)
    bool breakTabLinesAtNotes{};    ///< "Break Tablature Lines At Numbers" (Tablature Staff Attributes)
    bool hideTuplets{};             ///< Inverse of "Show Tuplets" (Tablature Staff Attributes)
    Cmper fretInstId{};             ///< Cmper of @ref FretInstrument for TAB strings. (Tablature Staff Attributes)
    bool hideStems{};               ///< Inverse of "Display Stems"
    StemDirection stemDirection{};  ///< stem direction for staff (xml node is `<stemDir>`)
    bool hideBeams{};               ///< Inverse of "Show Beams"
    bool stemStartFromStaff{};      ///< Vertical offset location for notehead end of stems.
                                    ///< - Offset from notehead: false
                                    ///< - Offset from staff: true
    bool stemsFixedEnd{};           ///< "Use Vertical Offset For Beam End Of Stems (Offset From Staff)"
    bool stemsFixedStart{};         ///< "Use Vertical Offset For Notehead End Of Stems"
    Efix horzStemOffUp{};           ///< Horizontal Stem Offsets: Up Stem
    Efix horzStemOffDown{};         ///< Horizontal Stem Offsets: Down Stem
    Efix vertStemStartOffUp{};      ///< Vertical Stem Offsets For Notehead End: Up Stem
    Efix vertStemStartOffDown{};    ///< Vertical Stem Offsets For Notehead End: Down Stem
    Efix vertStemEndOffUp{};        ///< Vertical Stem Offsets For Stem End: Up Stem
    Efix vertStemEndOffDown{};      ///< Vertical Stem Offsets For Stem End: Down Stem
    HideMode hideMode{};            ///< "Force Hide Staff" option
    bool redisplayLayerAccis{};     ///< "Redisplay Accidentals in Other Layers Within Measures"
    bool hideTimeSigsInParts{};     ///< Inverse of "Display Time Signatures in Parts"
    AutoNumberingStyle autoNumbering{}; ///< Autonumbering style if #useAutoNumbering is true. (xml node is `<autoNum>`)
    bool useAutoNumbering{};        ///< Whether names should be auto-numbered. (xml node is `<useAutoNum>`)
    bool hideKeySigsShowAccis{};    ///< "Hide Key Signature and Show Accidentals" transposition option.

    // The following values are not in xml but computed by the factory.

    Cmper noteShapesId{};           ///< Calculated cmper for note shapes. If not overridden by a staff style, it is the same as the
                                    ///< staff cmper or zero if default. (Populated by in #calcAllRuntimeValues.)
    bool noteShapesFromStyle{};     ///< True if #noteShapesId is for a staff style. (Determines which note shapes class to retrieve.)
    Cmper fullNamePosId{};          ///< Calculated cmper for full name position id. If not overridden by a staff style, it is the
                                    ///< same as the staff cmper or zero if default. (Populated by in #calcAllRuntimeValues.)
    bool fullNamePosFromStyle{};    ///< True if #fullNamePosId is for a staff style. (Determines which full name pos class to retrieve.)
                                    ///< Populated by in #calcAllRuntimeValues.
    Cmper abrvNamePosId{};          ///< Calculated cmper for abbreviated name position id. If not overridden by a staff style, it is the
                                    ///< same as the staff cmper or zero if default. (Populated by in #calcAllRuntimeValues.)
    bool abrvNamePosFromStyle{};    ///< True if #abrvNamePosId is for a staff style. (Determines which abrv name pos class to retrieve.)
                                    ///< Populated by in #calcAllRuntimeValues.
    Cmper multiStaffInstId{};       ///< Calculated cmper for @ref MultiStaffInstrumentGroup, if any. This value is not in the xml.
                                    ///< It is set by the factory with the Resolver function for @ref MultiStaffInstrumentGroup.
    std::optional<int> autoNumberValue; ///< Calculated autonumbering value. It is computed by #calcAllAutoNumberValues.
    std::optional<Cmper> percussionMapId; ///< Calculated percussion map Id for a percussion staff. (Populated by in #calcAllRuntimeValues.)

    /// @brief Returns the full staff name without Enigma tags. If the full name contains part-specific tags (rare), they are inserted for the score.
    /// @param accidentalStyle The style for accidental subsitution in names like "Clarinet in Bb".
    std::string getFullName(util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii) const;

    /// @brief Returns the abbreviated staff name without Enigma tags. If the abbreviated name contains part-specific tags (rare), they are inserted for the score.
    /// @param accidentalStyle The style for accidental subsitution in names like "Clarinet in Bb".
    std::string getAbbreviatedName(util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii) const;

    /// @brief Returns true if the staff shows its full or abbreviated name for the current requested partId.
    bool calcShowName() const;

    /// @brief Returns the @ref MultiStaffInstrumentGroup for this staff if it is part of one in the data. Otherwise nullptr.
    MusxInstance<MultiStaffInstrumentGroup> getMultiStaffInstGroup() const;

    /// @brief Returns the @ref details::StaffGroup for this staff if it is part of a multistaff instrument (as defined in #Document::getInstruments).
    /// @param forPartId The part id to use.
    MusxInstance<details::StaffGroup> getMultiStaffInstVisualGroup(Cmper forPartId) const;

    /// @brief Returns the parsing context for the full name.
    /// @param forPartId The part id to use for partname and page inserts
    /// @param preferStaffName When true, use the staff name if there is one (rather than the multi-instrument group name)
    util::EnigmaParsingContext getFullInstrumentNameCtx(Cmper forPartId, bool preferStaffName = false) const;

    /// @brief Returns the parsing context for the abbreviated name.
    /// @param forPartId The part id to use for partname and page inserts
    /// @param preferStaffName When true, use the staff name if there is one (rather than the multi-instrument group name)
    util::EnigmaParsingContext getAbbreviatedInstrumentNameCtx(Cmper forPartId, bool preferStaffName = false) const;

    /// @brief Returns the full instrument name for this staff without Enigma tags and with autonumbering (if any)
    /// @note Ordinal prefix numbering is currently supported only for English.
    /// @param accidentalStyle The style for accidental subsitution in names like "Clarinet in Bb".
    /// @param preferStaffName When true, use the staff name if there is one (rather than the multi-instrument group name)
    std::string getFullInstrumentName(util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii, bool preferStaffName = false) const;

    /// @brief Returns the abbreviated instrument name for this staff without Enigma tags and with autonumbering (if any)
    /// @note Ordinal prefix numbering is currently supported only for English.
    /// @param accidentalStyle The style for accidental subsitution in names like "Clarinet in Bb".
    /// @param preferStaffName When true, use the staff name if there is one (rather than the multi-instrument group name)
    std::string getAbbreviatedInstrumentName(util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii, bool preferStaffName = false) const;

    /// @brief Returns the first nonempty playback route name assigned to this staff.
    ///
    /// Playback routes are searched by notation layer in ascending order, followed by chord-symbol
    /// playback and MIDI-expression playback. Disabled and soloed playback streams remain eligible.
    /// @return The playback route name, or an empty string if no assigned route has a nonempty name.
    std::string getPlaybackRouteName() const;

    /// @brief Returns true if the staff shows its full or abbreviated instrument name for the current requested partId.
    bool calcShowInstrumentName() const;

    /// @brief Returns the note shape record in effect for this staff instance
    MusxInstance<details::ShapeNoteBase> getNoteShapes() const;

    /// @brief Returns the full name positioning in effect for this staff instance
    MusxInstance<NamePositioning> getFullNamePosition() const;

    /// @brief Returns the abbreviated name positioning in effect for this staff instance
    MusxInstance<NamePositioning> getAbbreviatedNamePosition() const;

    /// @brief Returns if names should be shown for the specified part
    bool showNamesForPart(Cmper partId) const
    { return partId == SCORE_PARTID ? !hideNameInScore : showNameInParts; }

    /**
     * @brief Get the auto-numbering value for this staff, if applicable.
     *
     * Calculates #autoNumberValue for every staff based on the occurrences of the instrument UUID
     * and in each staff instance. If numbering is not applicable (e.g., auto numbering
     * is disabled, the UUID is empty, or there is only one instance), #autoNumberValue has no value.
     *
     * This function is called by the DocumentFactory when the document is created.
     */
    static void calcAllAutoNumberValues(const DocumentPtr& document);

    /**
     * @brief Populate runtime values for all staves or staffstyles, such as percussion map Id if any.
     *
     * This function is called by the DocumentFactory when the document is created.
     */
    template <typename SubType>
    static void calcAllRuntimeValues(const DocumentPtr& document);

    /// @brief Calculate the auto numbering prefix or suffix, if needed.
    /// @return A std::pair containing
    ///     - A std::string that is either the prefix or suffix.
    ///     - A bool that if true indicates the string is a prefix and if false a suffix.
    std::pair<std::string, bool> calcAutoNumberingAffix() const;

    /// @brief Add auto numbering as a prefix or suffix, if needed
    /// @param plainName The name (full or abbreviated) to which to add the auto numbering
    /// @return Auto numbered name.
    std::string addAutoNumbering(const std::string& plainName) const;

    /// @brief Returns the clef in this staff at the specified location
    /// @param measureId The measure of the location
    /// @param position The Edu elapsed time with the measure
    /// @param forWrittenPitch If true, return the transposing clef if there is one
    ClefIndex calcClefIndexAt(MeasCmper measureId, Edu position, bool forWrittenPitch = false) const;

    /// @brief Returns the first clef in this staff
    ClefIndex calcFirstClefIndex(bool forWrittenPitch = false) const
    { return calcClefIndexAt(1, 0, forWrittenPitch); }

    /// @brief Returns the first clef in the specified staff in the document
    /// @param document the document to search
    /// @param partId the linked part to search
    /// @param staffCmper the staff cmper to search
    static ClefIndex calcFirstClefIndex(const DocumentPtr& document, Cmper partId, StaffCmper staffCmper);

    /**
     * @brief A clef change attached to a staff at a measure location.
     *
     * The position is expressed as a staff-level fraction of a whole note, matching Finale's clef-list
     * positioning convention. Callers that need global score time should apply the staff's time stretch.
     */
    struct ClefChange
    {
        ClefIndex clefIndex{};                  ///< The Finale clef index.
        util::Fraction position{};              ///< The staff-level position in the measure.
        ShowClefMode showClefMode{};            ///< The Finale clef display mode.
        bool afterBarline{};                    ///< True if a measure-start clef is placed after the barline.
        Evpu yEvpuPos{};                        ///< Vertical EVPU adjustment for mid-measure clefs.
        int percent{};                          ///< Clef scaling percent, where 100 is unscaled.
        int xEvpuOffset{};                      ///< Horizontal EVPU adjustment for mid-measure clefs.
    };

    /// @brief Iterate clef changes in this staff at the given measure.
    /// @param measureId The measure to inspect.
    /// @param forWrittenPitch If true, use an active transposed clef as Finale's written clef.
    /// @param iterator The callback function. Return false to stop iteration.
    /// @return True if iteration completed. False if @p iterator returned false and exited early.
    bool iterateClefChangesAtMeasure(
        MeasCmper measureId,
        bool forWrittenPitch,
        std::function<bool(const ClefChange&)> iterator) const;

    /// @brief Calculates the number of staff lines on this staff.
    int calcNumberOfStafflines() const;

    /// @brief Calculates the middle staff position. For staves with even numbers of lines, it is the middle space.
    int calcMiddleStaffPosition() const;

    /// @brief Returns the configured rest staff-step offset for an Edu value.
    [[nodiscard]] Evpu calcRestOffset(Edu edu) const;

    /// @brief Calculates barline endpoint offsets relative to the staff center.
    /// @return A std::pair containing
    ///     - the top barline endpoint relative to staff center, in EVPU
    ///     - the bottom barline endpoint relative to staff center, in EVPU
    /// @note This uses the staff-position sign convention: positive is above/up and negative is below/down.
    [[nodiscard]] std::pair<Evpu, Evpu> calcBarlineOffsetsFromCenter() const;

    /// @brief Calculates the baseline zero position for this staff, relative to the reference line, before any displacements are applied.
    ///
    /// This function reproduces Finale’s observed behavior but may yield inaccurate results in unusual configurations of custom staff lines.
    /// In the most common cases, Finale defines the baseline zero position as the midpoint between the reference line and the
    /// bottom staff line, which is the value computed by this function.
    Evpu calcBaselineZeroPosition() const;

    /// @brief Returns the baseline position for the staff at the given system. The staff should be the staff at the beginning of the system.
    /// @tparam BaselineType The type of baseline. (@ref details::BaselineChords, @ref details::BaselineExpressionsAbove, etc.: not the `BaselineSystem` variants)
    /// @param systemId The system Cmper for which to get the per-system baseline offset. Use #Document::calcScrollViewCmper for Scroll View values.
    /// @return The accumulated position of the baseline, relative to the staff's reference line.
    template<typename BaselineType,
             std::enable_if_t<std::is_base_of<details::BaselineNoInci, BaselineType>::value, int> = 0>
    Evpu calcBaselinePosition(SystemCmper systemId) const
    {
        return calcBaselinePositionImpl<BaselineType>(systemId, std::nullopt);
    }

    /// @brief Returns the baseline position for the staff at the given system. The staff should be the staff at the beginning of the system.
    /// @tparam BaselineType The type of baseline. (@ref details::BaselineChords, @ref details::BaselineExpressionsAbove, etc.: not the `BaselineSystem` variants)
    /// @param systemId The system Cmper for which to get the per-system baseline offset. Use #Document::calcScrollViewCmper for Scroll View values.
    /// @param lyricNumber The Cmper of the lyric text block to find.
    /// @return The accumulated position of the baseline, relative to the staff's reference line.
    template<typename BaselineType,
             std::enable_if_t<std::is_base_of<details::Baseline, BaselineType>::value &&
                              !std::is_base_of<details::BaselineNoInci, BaselineType>::value, int> = 0>
    Evpu calcBaselinePosition(SystemCmper systemId, Cmper lyricNumber) const
    {
        return calcBaselinePositionImpl<BaselineType>(systemId, lyricNumber);
    }

    /// @brief Creates a vector of @ref LyricsLineInfo for the current requested part.
    /// @param systemId The system for which to create the lyrics line information/
    /// @return Lyrics line information in order by descending baseline position (relative to this staff's reference line).
    std::vector<LyricsLineInfo> createLyricsLineInfo(SystemCmper systemId) const;

    /// @brief Calculates the position of the top staff line, relative to the reference line.
    int calcTopLinePosition() const;

    /// @brief Calculates the Evpu position of the top staff line, relative to the reference line.
    Evpu calcTopLineEvpu() const
    { return static_cast<Evpu>(calcTopLinePosition() * static_cast<int>(EVPU_PER_STAFF_POSITION)); }

    /// @brief Calculates the position of the top staff line, relative to the reference line.
    int calcBottomLinePosition() const;

    /// @brief Calculates the Evpu position of the bottom staff line, relative to the reference line.
    Evpu calcBottomLineEvpu() const
    { return static_cast<Evpu>(calcBottomLinePosition() * static_cast<int>(EVPU_PER_STAFF_POSITION)); }

    /// @brief Return true if this staff has an instrument assigned.
    bool hasInstrumentAssigned() const;

    /// @brief Calculates the transposition interval for this staff or staff composite
    /// @return A pair of int containing
    ///     - the diatonic displacement interval (positive up, negative down)
    ///     - the alteration in chromatic halfsteps
    /// Downward intervals reverse the sign of the chromatic alteration.
    std::pair<int, int> calcTranspositionInterval() const;

    /// @brief Gets a list of all parts that contain this staff
    /// @param includeScore If true, include the score in the list. (Defaults to true)
    MusxInstanceList<PartDefinition> getContainingParts(bool includeScore = true) const;

    /// @brief Finds the first part that contains this staff, not including the score.
    /// @return The first part that contains this staff or nullptr if none.
    MusxInstance<PartDefinition> firstContainingPart() const;

    /// @brief Returns true if the given layer is hidden by alternate notation.
    ///
    /// Blank, blank-with-rests, one-bar repeat, two-bar repeat, and slash notation all replace the
    /// layer's entries, and Finale counts the layer as having no notes. Rhythmic notation does not:
    /// it restyles the entries rather than replacing them, and the layer still counts as having
    /// notes. (Observed behavior.)
    ///
    /// @param forLayerIndex 0-based layer index (0..3)
    bool calcAlternateNotationHidesEntries(LayerIndex forLayerIndex) const;

    /// @brief Iterate all entries in the document by staff and then measure. This function wraps MusxInstanceList<StaffUsed>::iterateEntries.
    /// @param iterator The callback function.
    /// @return True if iteration completed. False if the @p iterator returned false and exited early.
    bool iterateEntries(std::function<bool(const EntryInfoPtr&)> iterator) const;

    /// @brief Returns true if both instances have the same notation style settings. This is helpful for determining if
    /// there is an instrument change.
    bool calcIsSameNotationStyle(const Staff& other) const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        OthersBase::integrityCheck(ptrToThis);
        if (!staffLines && !customStaff) {
            MUSX_INTEGRITY_ERROR("Staff or StaffStyle " + std::to_string(getCmper()) + " has neither a standard nor a custom staff definition.");
        } else if (staffLines && customStaff) {
            MUSX_INTEGRITY_ERROR("Staff or StaffStyle " + std::to_string(getCmper()) + " has both a standard and a custom staff definition.");
        }
        if (customStaff) { // guarantee ascending order of staves.
            std::sort(customStaff.value().begin(), customStaff.value().end(),
                [](const auto& a, const auto& b) { return a < b; });
        }
        if (transposition) {
            if (!transposition->chromatic && !transposition->keysig) {
                MUSX_INTEGRITY_ERROR("Staff or StaffStyle " + std::to_string(getCmper()) + " has transposition with neither keysig nor chromatic transposition defined.");
            } else if (transposition->chromatic && transposition->keysig) {
                MUSX_INTEGRITY_ERROR("Staff or StaffStyle " + std::to_string(getCmper()) + " has transposition with both keysig and chromatic transposition defined.");
            }
        }
        if (useNoteFont && !noteFont) {
            noteFont = std::make_shared<FontInfo>(getDocument()); // do this first to avoid unreachable code warning, since MUSX_INTEGRITY_ERROR might throw
            MUSX_INTEGRITY_ERROR("Staff or StaffStyle " + std::to_string(getCmper()) + " specifies to use a custom notehead font, but no custom font was provided.");
        }
    }

    constexpr static std::string_view XmlNodeName = "staffSpec"; ///< The XML node name for this type.
    static const xml::XmlElementArray<Staff>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.

private:
    template <typename NamePositionType>
    MusxInstance<NamePositioning> getNamePosition() const;

    template<typename BaselineType>
    Evpu calcBaselinePositionImpl(SystemCmper systemId, std::optional<Cmper> lyricNumber) const;
};

/**
 * @class StaffStyle
 * @brief Represents a Finale staff style.
 *
 * The cmper is a 1-based staff style ID (not necessarily sequential).
 * This class is identified by the XML node name "staffStyle".
 */
class StaffStyle : public Staff
{
protected:
    /** @brief protected constructor for @ref StaffComposite. This constructor must be followed by a call to createMasks. */
    explicit StaffStyle(const MusxInstance<Staff>& staff)
        : Staff(*staff) {}

    /// @brief Separate creator for masks.
    /// @param ptrToSelf This allows both external subclass creation (StaffComposite) and shared_from_this
    void createMasks(const MusxInstance<EnigmaBase>& ptrToSelf)
    {
        masks = std::make_shared<Masks>(ptrToSelf);
    }

public:
    /** @brief Constructor function */
    explicit StaffStyle(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : Staff(document, partId, shareMode, cmper) {}

    /// @brief lists the masks that deterimine if this staff style overrides the staff settings
    class Masks : public ContainedClassBase
    {
    public:
        using ContainedClassBase::ContainedClassBase;

        bool floatNoteheadFont{};       ///< overrides notehead font settings
        bool useNoteShapes{};           ///< overrides #Staff::useNoteShapes
        bool flatBeams{};               ///< overrides #Staff::flatBeams
        bool blankMeasureRest{};        ///< overrides #Staff::blankMeasure
        bool noOptimize{};              ///< overrides #Staff::noOptimize
        bool notationStyle{};           ///< overrides notations style
        bool defaultClef{};             ///< overrides #Staff::defaultClef
        bool staffType{};               ///< overrides staff properties (see #StaffComposite::applyStyle)
        bool transposition{};           ///< overrides transposition fields
        bool blineBreak{};              ///< overrides #Staff::blineBreak
        bool rbarBreak{};               ///< overrides #Staff::rbarBreak
        bool negMnumb{};                ///< overrides #Staff::hideMeasNums
        bool negRepeat{};               ///< overrides #Staff::hideRepeats
        bool negNameScore{};            ///< overrides #Staff::hideNameInScore
        bool hideBarlines{};            ///< overrides #Staff::hideBarlines
        bool fullName{};                ///< overrides #Staff::fullNameTextId
        bool abrvName{};                ///< overrides #Staff::abbrvNameTextId
        bool floatKeys{};               ///< overrides #Staff::floatKeys
        bool floatTime{};               ///< overrides #Staff::floatTime
        bool hideRptBars{};             ///< overrides #Staff::hideRptBars
        bool negKey{};                  ///< overrides #Staff::hideKeySigs
        bool negTime{};                 ///< overrides #Staff::hideTimeSigs (in Score)
        bool negClef{};                 ///< overrides #Staff::hideClefs
        bool hideStaff{};               ///< overrides #Staff::hideMode
        bool noKey{};                   ///< overrides #Staff::noKey
        bool fullNamePos{};             ///< overrides presence, absence of @ref NamePositionStyleFull instance.
        bool abrvNamePos{};             ///< overrides presence, absence of @ref NamePositionStyleAbbreviated instance.
        bool altNotation{};             ///< overrides alternate notation properties (see #StaffComposite::applyStyle)
        bool showTies{};                ///< overrides #Staff::hideTies
        bool showDots{};                ///< overrides #Staff::hideDots
        bool showRests{};               ///< overrides #Staff::hideRests
        bool showStems{};               ///< overrides stem properties (see #StaffComposite::applyStyle)
        bool hideChords{};              ///< overrides #Staff::hideChords
        bool hideFretboards{};          ///< overrides #Staff::hideFretboards
        bool hideLyrics{};              ///< overrides #Staff::hideLyrics
        bool showNameParts{};           ///< overrides #Staff::showNameInParts
        bool showNoteColors{};          ///< overrides #Staff::showNoteColors
        bool hideStaffLines{};          ///< overrides #Staff::hideStaffLines
        bool redisplayLayerAccis{};     ///< overrides #Staff::redisplayLayerAccis
        bool negTimeParts{};            ///< overrides #Staff::hideTimeSigsInParts
        bool hideKeySigsShowAccis{};    ///< overrides #Staff::hideKeySigsShowAccis

        static const xml::XmlElementArray<Masks>& xmlMappingArray();    ///< Required for musx::factory::FieldPopulator.
    };

    std::string styleName;              ///< name of staff style
    bool copyable{};                    ///< whether the staff style assignments for this style should be copied when copy/pasting music
    bool addToMenu{};                   ///< add this staff style to the context menu for staff styles
    // No fixture has a part-specific StaffStyle. If partial sharing is observed, both Masks and the
    // inherited Transposition will need part-context rebinding and copy-overlay population.
    // See PartContextRebinder<details::CenterShape> and the CenterShape field mappings for the pattern.
    std::shared_ptr<Masks> masks;       ///< override masks: guaranteed to exist by #integrityCheck, which is called by the factory
                                        ///< (xml node is `<mask>`)

    /// @brief Determines whether this staff style represents or includes an instrument change.
    ///
    /// Beginning with Finale 2012, a dedicated group of staff style masks was reserved exclusively
    /// for instrument changes. These masks always appear together and never occur in staff styles
    /// used for other purposes. The masks are `notationStyle`, `defaultClef`, `showNoteColors`, and
    /// `hideKeySigsShowAccis`. If the #instUuid is a real instrument (rather than Blank or Unknown),
    /// that also indicates an instrument change.
    ///
    /// Earlier Finale files may not follow this convention. They could potentially contain
    /// staff styles where one or more instrument-change masks are set independently. To handle both
    /// modern and legacy cases, this function tests whether *any* of the instrument-change masks is
    /// active in the current staff style.
    ///
    /// In addition, there is a set of masks that may be included in an instrument change staff style
    /// or also may be included in a regular staff style. These are `staffType`, `transposition`, `fullName`,
    /// and `abrvName`. This function ignores these optional masks.
    ///
    /// @note If the current instance is a @ref StaffComposite, it incorporates all masks in effect at the
    /// music location for which it was created. For this reason, testing for any instrument-change
    /// masks (rather than all) remains useful even in modern files.
    /// @return `true` if one or more instrument-change masks are set in the current his staff style;
    ///         `false` otherwise.
    bool containsInstrumentChange() const;

    /// @brief Finds a subset from all StaffStyle instances that overlap with the specified
    /// metric position on a given staff in a give linked part or score.
    /// @param document The document to search
    /// @param partId The linked part id to search
    /// @param staffId The staff to search
    /// @param measId The MeasCmper of the measure position
    /// @param eduPosition The Edu position within the measure specified by @p measId.
    static MusxInstanceList<StaffStyle> findAllOverlappingStyles(const DocumentPtr& document,
        Cmper partId, StaffCmper staffId, MeasCmper measId, Edu eduPosition);

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        if (!masks) {
            // Finale allows creation of staff styles with no masks, so this is just a verbose comment
            util::Logger::log(util::Logger::LogLevel::Verbose, "StaffStyle " + styleName
                + " (" + std::to_string(getCmper()) + ") does not override anything.");
            createMasks(ptrToThis);
        }
        if (useNoteFont && !masks->floatNoteheadFont && !noteFont) {
            useNoteFont = false; // silence integrity check in Staff.
        }
        Staff::integrityCheck(ptrToThis);
    }

    constexpr static std::string_view XmlNodeName = "staffStyle"; ///< The XML node name for this type.
    static const xml::XmlElementArray<StaffStyle>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StaffStyleAssign
 * @brief Represents an assignment
 *
 * @note Staff style assignments use staff EDU ranges. The end measure can be greater than
 * the number of measures in the document.
 *
 * The cmper is the staff ID. This class is identified by the XML node name "staffStyleAssign".
 */
class StaffStyleAssign : public EnigmaMusicRange
{
public:
    /** @brief Constructor function */
    explicit StaffStyleAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : EnigmaMusicRange(document, partId, shareMode, cmper, inci) {}

    Cmper styleId{};        ///< The cmper of the assigned @ref StaffStyle.

    /// @brief Returns the @ref StaffStyle instance for this assignment
    /// @return Can return nullptr only in the case when there is an itegrity error
    /// @throws musx::dom::integrity_error if compiled to throw integrity errors
    MusxInstance<StaffStyle> getStaffStyle() const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        EnigmaMusicRange::integrityCheck(ptrToThis);
        if (!styleId) {
            MUSX_INTEGRITY_ERROR(std::string("Staff style assignment has no staff style id:")
                + " Part " + std::to_string(getSourcePartId())
                + " Staff " + std::to_string(getCmper())
            );
        }
    }

    constexpr static std::string_view XmlNodeName = "staffStyleAssign"; ///< The XML node name for this type.
    static const xml::XmlElementArray<StaffStyleAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StaffComposite
 * @brief Represents a composite of an underlying @ref Staff instance with any applicable @ref StaffStyle instances applied.
 *
 * Instances of StaffComposite are not part of the document structure and cannot be created directly.
 * Use #createCurrent to create an instance.
 */
class StaffComposite : public StaffStyle
{
private:
    /** @brief private constructor. must be followed by a call to createMasks. */
    explicit StaffComposite(const MusxInstance<Staff>& staff, MeasCmper measId, Edu eduPosition)
        : StaffStyle(staff), m_measureId(measId), m_eduPosition(eduPosition) {}

    /// @brief Modifies the current StaffComposite instance with all applicable values from the @ref StaffStyle.
    /// @param staffStyle The @ref StaffStyle to apply.
    void applyStyle(const MusxInstance<StaffStyle>& staffStyle);

    const MeasCmper m_measureId;
    const Edu m_eduPosition;
    bool m_instUuidChanged{};

public:
    /// @brief Calculates the current staff at the specified metric position by applying all relevant staff styles,
    ///
    /// Note that the Finale app has logic to assure that no two assigments modify the same staff properties at the same metric
    /// position. If this occurs despite Finale's code, the last one processed is the one that takes effect.
    ///
    /// @param document The document to search
    /// @param partId The ID of the linked part or score
    /// @param staffId The @ref Staff cmper of the base staff for which to apply staff styles
    /// @param measId The measure location to search
    /// @param eduPosition The Edu position within the measure to search
    /// @return The composite result or null if @p staffId is not valid.
    static MusxInstance<StaffComposite> createCurrent(const DocumentPtr& document, Cmper partId, StaffCmper staffId, MeasCmper measId, Edu eduPosition);

    /// @brief Returns the measure this staff composite was created with.
    MeasCmper getMeasureId() const { return m_measureId; }

    /// @brief Returns the Edu position this staff composite was created with.
    Edu getEduPosition() const { return m_eduPosition; }

    /// @brief Returns if this composite staff includes an instrument uuid changed by a staff style.
    bool getInstrumentUuidChanged() const { return m_instUuidChanged; }

    /// @brief Returns the clef in this staff at the location of the composite staff.
    /// @param forWrittenPitch If true, return the transposing clef if there is one.
    ClefIndex calcClefIndex(bool forWrittenPitch = false) const
    { return this->calcClefIndexAt(m_measureId, m_eduPosition, forWrittenPitch); }

    /// @brief Returns the underlying staff without any staff styles applied.
    MusxInstance<others::Staff> getRawStaff() const;
};

} // namespace others
} // namespace dom
} // namespace musx
