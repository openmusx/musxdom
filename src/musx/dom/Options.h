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

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <stdexcept>

#include "music_theory/music_theory.hpp"
#include "BaseClasses.h"
#include "CommonClasses.h"
 // do not add other dom class dependencies. Use Implementations.h for implementations that need total class access.

namespace musx {
namespace dom {

namespace others {
class Staff;
} // namespace others

/**
 * @namespace musx::dom::options
 * @brief Classes in the @ref OptionsPool.
 */
namespace options {

/**
 * @class AccidentalOptions
 * @brief Options controlling the appearance and positioning of accidentals.
 *
 * This class is identified by the XML node name "accidentalOptions".
 */
class AccidentalOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit AccidentalOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    int minOverlap{};                       ///< "Minimum Vertical Spacing Between Accidentals (Measured in Steps)" (xml node is `<overlap>`)
    Evpu multiCharSpace{};                  ///< "Space Within Multi-Character Accidentals" (xml node is `<bacciAdd>`)
    bool crossLayerPositioning{};           ///< "Use Cross-Layer Accidental Positioning" (xml node is `<useNewAcciPositioning>`)
    Evpu startMeasureSepar{};               ///< "Adjustment at Start of Measure"--The sign is reversed in the Finale UI. (xml node is `<frontAcciSepar>`)
    Evpu acciNoteSpace{};                   ///< Space between accidentals and notes.
    Evpu acciAcciSpace{};                   ///< Space between accidentals.

    constexpr static std::string_view XmlNodeName = "accidentalOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<AccidentalOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class AlternateNotationOptions
 * @brief Options controlling the appearance of alternate notation.
 *
 * This class is identified by the XML node name "alternateNotationOptions".
 */
class AlternateNotationOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit AlternateNotationOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    Evpu halfSlashLift{};       ///< Baseline adjustment for half diamond in @ref Evpu.
    Evpu wholeSlashLift{};      ///< Baseline adjustment for whole diamond in @ref Evpu.
    Evpu dWholeSlashLift{};     ///< LBaseline adjustment for double whole diamond in @ref Evpu.
    Evpu halfSlashStemLift{};   ///< Stem connection value for half diamond in @ref Evpu.
    Evpu quartSlashStemLift{};  ///< Stem connection value for quarter slashes in @ref Evpu.
    Evpu quartSlashLift{};      ///< Baseline adjustment for quarter slashes in @ref Evpu.
    Evpu twoMeasNumLift{};      ///< Vertical offset for two-bar repeat numbers in @ref Evpu.

    constexpr static std::string_view XmlNodeName = "alternateNotationOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<AlternateNotationOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class AugmentationDotOptions
 * @brief Options controlling the appearance of augmentation dots.
 *
 * This class is identified by the XML node name "augmentationDotOptions".
 */
class AugmentationDotOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit AugmentationDotOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    Evpu dotUpFlagOffset{};     ///< "Horizontal Adjustment for Upstem Flags"
    Evpu dotOffset{};           ///< "Space Between Dots"
    bool adjMultipleVoices{};   ///< "Adjust Dots for Multiple Voices" (xml node is `<doDotDownAdjust>`)
    Evpu dotNoteOffset{};      ///< "Space Between Dot and Note" (xml node is `<dotFirstOffset>`)
    Evpu dotLift{};             ///< "Vertical Adjustment of Dot"
    /**
     * @brief Use legacy dot positioning on flipped stems.
     *
     * Finale 27.4 fixed a long-standing bug in how dots on flipped stems are positioned. Finale added
     * this option to avoid messing up documents in which the user manually worked around the bug.
     * Enabling it suppresses the 27.4 behavior. It is not represented in the Finale UI, except as
     * a question when you first open a document that it detects might contain the workaround.
     *
     * xml node is `<skip27_4DotAdjust>`
     */
    bool useLegacyFlippedStemPositioning{};

    constexpr static std::string_view XmlNodeName = "augmentationDotOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<AugmentationDotOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class BarlineOptions
 * @brief Options controlling the appearance of barlines.
 *
 * This class is identified by the XML node name "barlineOptions".
 */
class BarlineOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit BarlineOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    bool drawBarlines{};                    ///< "Display All Barlines"
    bool drawCloseSystemBarline{};          ///< "Close Barline at End of Each System"
    bool drawCloseFinalBarline{};           ///< "Close Barline at End of Piece"
    bool drawFinalBarlineOnLastMeas{};      ///< "Final Barline at End of Piece"
    bool drawDoubleBarlineBeforeKeyChanges{}; ///< "Double Barline Before Key Changes"
    bool drawLeftBarlineSingleStaff{};      ///< Left Barlines: "Display on Single Staves"
    bool drawLeftBarlineMultipleStaves{};   ///< Left Barlines: "Display on Multiple Staves"
    bool leftBarlineUsePrevStyle{};         ///< Left Barlines: "Default Style is Previous Measure's Right Barline"
    Efix thickBarlineWidth{};               ///< Width of thick barlines in @ref Efix.
    Efix barlineWidth{};                    ///< Width of thin barlines in @ref Efix.
    Efix doubleBarlineSpace{};              ///< Space between double barlines in @ref Efix.
    Efix finalBarlineSpace{};               ///< Space between final barlines in @ref Efix.
    Evpu barlineDashOn{};                   ///< Dash length for dashed barlines in @ref Evpu.
    Evpu barlineDashOff{};                  ///< Dash space for dashed barlines in @ref Evpu.

    constexpr static std::string_view XmlNodeName = "barlineOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<BarlineOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class BeamOptions
 * @brief Options controlling the appearance of beams.
 *
 * This class is identified by the XML node name "beamOptions".
 */
class BeamOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit BeamOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /// @enum FlattenStyle
    /// @brief Beaming style choices for when to flatten beams
    enum class FlattenStyle
    {
        OnEndNotes,         ///< Flatten based on end notes only (the legacy default)
        AlwaysFlat,         ///< Flatten all beams
        OnStandardNote,     ///< Flatten based on standard note
        OnExtremeNote       ///< Flatten based on extreme note
    };

    Evpu beamStubLength{};                   ///< "Broken Beam Length" in @ref Evpu.
    Evpu maxSlope{};                         ///< Maximum slope in @ref Evpu.
    Evpu beamSepar{};                        ///< Secondary beam separation in @ref Evpu. This is the distance between the top of one beam to the top of the next.
    Evpu maxFromMiddle{};                    ///< "Maximum Distance from Middle Staff Line" in @ref Evpu. (xml node is `<hmBeamTip>`)
    FlattenStyle beamingStyle{};             ///< Beaming style.
    bool extendBeamsOverRests{};             ///< "Extend Beams Over Rests" (xml node is `<incEdgeRestsInBeamGroups>`)
    bool incRestsInFourGroups{};             ///< "Include Rests when Beaming in Group of Four" (xml node is `<incRestsInClassicBeams>`)
    bool beamFourEighthsInCommonTime{};      ///< "Beam Four Eighth Notes Together in Common Time"
    bool beamThreeEighthsInCommonTime{};     ///< "Beam Three Eighth Notes Together Before/After An Eighth Rest"
    bool dispHalfStemsOnRests{};             ///< "Display Half-Stems for Beamed Rests" (xml node is `<doStemStubs>`)
    bool oldFinaleRestBeams{};               ///< Compatibility setting for legacy Finale files
    bool spanSpace{};                        ///< "Allow Primary Beam Within a Space"
    bool extendSecBeamsOverRests{};          ///< "Extend Secondary Beams Over Rests"
    Efix beamWidth{};                        ///< Thickness of beams in @ref Efix.

    constexpr static std::string_view XmlNodeName = "beamOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<BeamOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ChordOptions
 * @brief Options controlling chord symbol and fretboard rendering.
 *
 * This class is identified by the XML node name "chordOptions".
 */
class ChordOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit ChordOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /// @enum ChordAlignment
    /// @brief Horizontal alignment for chord symbols
    ///
    /// Only Left and Center are allowed. There is no Right alignment of chords.
    enum class ChordAlignment
    {
        Left,                       ///< this is the default and may not appear in the xml
        Center
    };

    /// @enum ChordStyle
    /// @brief Chord spelling style
    enum class ChordStyle
    {
        Standard,                   ///< this is the default and may not appear in the xml
        European,
        German,
        Roman,
        NashvilleA,
        NashvilleB,
        Solfeggio,                  // xml value is "solfeggio"
        Scandinavian
    };

    double fretPercent{};            ///< Percent size of fretboard relative to default. (100.0 is 100%)
    double chordPercent{};           ///< Percent size of chord symbol relative to default. (100.0 is 100%)
    Evpu chordSharpLift{};           ///< Vertical offset for sharp chord symbols.
    Evpu chordFlatLift{};            ///< Vertical offset for flat chord symbols.
    Evpu chordNaturalLift{};         ///< Vertical offset for natural chord symbols.
    bool showFretboards{};           ///< Display fretboards.
    Cmper fretStyleId{};             ///< Default @ref others::FretboardStyle ID.
    Cmper fretInstId{};              ///< Default @ref others::FretInstrument ID.
    bool multiFretItemsPerStr{};     ///< Allow multiple fret items per string.
    bool useFretboardFont{};         ///< Selects the fretboard font rather than group/style. Finale has no known UI for this and may never reference it. (XML node is `<useFretFont>`.)
    bool italicizeCapoChords{};      ///< Italicize capo chords.
    ChordAlignment chordAlignment{}; ///< Horizontal alignment for chord symbols.
    ChordStyle chordStyle{};         ///< Style of chord spelling.
    bool useSimpleChordSpelling{};   ///< Use simplified chord spelling.
    bool chordPlayback{};            ///< Enable chord symbol playback.

    constexpr static std::string_view XmlNodeName = "chordOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ChordOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ClefOptions
 * @brief Options for configuring clefs in a musical context.
 */
class ClefOptions : public OptionsBase {
public:
    /// @brief Information about a @ref ClefDef. (See @ref dom::ClefInfo.)
    using ClefInfo = dom::ClefInfo;

    /**
     * @brief Constructor
     * @param document A weak pointer to the document object.
     * @param partId Usually 0. This parameter is needed for the generic factory routine.
     * @param shareMode Usually `ShareMode::All`. This parameter is needed for the generic factory routine.
     *
     * Constructs a ClefOptions object that is associated with the provided document.
     */
    explicit ClefOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /**
     * @brief Represents a single clef definition.
     */
    class ClefDef : public ContainedClassBase
    {
    public:
        using ContainedClassBase::ContainedClassBase;

        int middleCPos{};               ///< Staff position of middle-C for this clef from reference staffline (usually the top). (xml node is `<adjust>`.)
        char32_t clefChar{};            ///< UTF-32 character code for the clef symbol.
        int staffPosition{};             ///< Staff position of the clef symbol's baseline, from reference staffline (usually the top). (xml node is `<clefYDisp>`)
        Efix baselineAdjust{};          ///< Additional baseline adjustment in Efix. (xml node is `<baseAdjust>`)
        Cmper shapeId{};                ///< Shape ID if the clef is represented as a shape. (xml node is `<shapeID>`)
        bool isShape{};                 ///< Indicates if the clef is a shape.
        bool scaleToStaffHeight{};      ///< Indicates if the shape should scale to staff height.
        bool useOwnFont{};              ///< Indicates if the clef has its own font.
        std::shared_ptr<FontInfo> font; ///< When `useOwnFont` is true, this is the clef's font. Otherwise `nullptr`.

        /// @brief Returns true if this is a blank clef.
        bool isBlank() const;

        /// @brief Calculates and returns the clef information.
        ///
        /// A clef whose glyph is a percussion clef is a percussion clef wherever the glyph sits. A clef's character is trusted to be
        /// a clef glyph, so a character that some symbol fonts use for another glyph still identifies a percussion clef. Pitched notes
        /// under a percussion clef sit where a clef would put them, and its octave is the octave of the glyph's position above middle C. For a glyph between the
        /// treble clef's middle C and the middle staff line, that is the number of whole octaves between the clef's middle C and a
        /// treble clef's.
        ///
        /// @param currStaff For most accurate results, pass the current @ref others::StaffComposite instance. If you supply it, staves with
        /// a notation style of tablature always return a tab clef and staves with a notation style of percussion always return a percussion
        /// clef, whatever the glyph. Otherwise, percussion clefs are identified by glyph, and clefs with other glyphs are identified by
        /// position, with percussion and tab clefs at the positions of the Finale SMuFL default file settings.
        /// @return See #ClefInfo.
        ClefInfo calcInfo(const MusxInstance<others::Staff>& currStaff = nullptr) const;


        /// @brief Calculate the font that applies to this clef, based on the options in @ref ClefDef.
        /// @return A shared pointer to the font instance used by this @ref ClefDef.
        /// @throws std::invalid_argument if not found.
        MusxInstance<FontInfo> calcFont() const;

        static const xml::XmlElementArray<ClefDef>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    ClefIndex defaultClef{};            ///< Default clef identifier. An index into `clefDefs`.
    int clefChangePercent{};            ///< Percentage adjustment for end-of-measure clef changes. 100 means no adjustment. (xml node is `<endMeasClefPercent>`)
    Evpu clefChangeOffset{};            ///< Offset from next barline for end-of-measure clef changes. (xml node is `<endMeasClefPosAdd>`).
    Evpu clefFrontSepar{};              ///< Spacing before clef. (xml node is `<clefFront>`)
    Evpu clefBackSepar{};               ///< Spacing after clef. (xml node is `<clefBack>`)
    bool showClefFirstSystemOnly{};     ///< "Display Clef Only on First System"
    Evpu clefKeySepar{};                ///< "Extra Space Between Clef and Key Signature" (xml node is `<clefKey>`)
    Evpu clefTimeSepar{};               ///< "Extra Space Between Clef and Time Signature" (xml node is `<clefTime>`)
    bool cautionaryClefChanges{};       ///< "Display Courtesy Clef a End of Staff System"

    /**
     * @brief Vector of clef definitions (@ref ClefDef).
     */
    std::vector<MusxInstance<ClefDef>> clefDefs;

    /// @brief Bounds-checked accessor function for #clefDefs.
    /// @param clefIndex The index to retrieve.
    /// @throws std::out_of_range if index is out of range.
    MusxInstance<ClefDef> getClefDef(ClefIndex clefIndex) const
    {
        MUSX_ASSERT_IF(clefIndex >= clefDefs.size()) {
            throw std::out_of_range("Clef index " + std::to_string(clefIndex) + " does not exist in document.");
        }
        return clefDefs[clefIndex];
    }

    /**
     * @brief The XML node name for this type.
     */
    constexpr static std::string_view XmlNodeName = "clefOptions";
    static const xml::XmlElementArray<ClefOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class FlagOptions
 * @brief Options controlling the appearance and adjustments of flags.
 *
 * This class is identified by the XML node name "flagOptions".
 */
class FlagOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit FlagOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    bool straightFlags{};                   ///< Whether to use straight flags.
    Efix upHAdj{};                          ///< Horizontal adjustment for upward flags in @ref Efix.
    Efix downHAdj{};                        ///< Horizontal adjustment for downward flags in @ref Efix.
    Efix upHAdj2{};                         ///< Horizontal adjustment for secondary upward flags in @ref Efix.
    Efix downHAdj2{};                       ///< Horizontal adjustment for secondary downward flags in @ref Efix.
    Efix upHAdj16{};                        ///< Horizontal adjustment for sixteenth upward flags in @ref Efix.
    Efix downHAdj16{};                      ///< Horizontal adjustment for sixteenth downward flags in @ref Efix.
    Efix eighthFlagHoist{};                 ///< Hoist value for eighth note flags in @ref Efix. (This value is not on Finale's UI page for Flag Options.)
    Efix stUpHAdj{};                        ///< Horizontal adjustment for straight upward flags in @ref Efix.
    Efix stDownHAdj{};                      ///< Horizontal adjustment for straight downward flags in @ref Efix.
    Efix upVAdj{};                          ///< Vertical adjustment for upward flags in @ref Efix.
    Efix downVAdj{};                        ///< Vertical adjustment for downward flags in @ref Efix.
    Efix upVAdj2{};                         ///< Vertical adjustment for secondary upward flags in @ref Efix. (xml node is `<upVadj2>`: this is probably a type somewhere in Finale)
    Efix downVAdj2{};                       ///< Vertical adjustment for secondary downward flags in @ref Efix.
    Efix upVAdj16{};                        ///< Vertical adjustment for sixteenth upward flags in @ref Efix.
    Efix downVAdj16{};                      ///< Vertical adjustment for sixteenth downward flags in @ref Efix.
    Efix stUpVAdj{};                        ///< Vertical adjustment for straight upward flags in @ref Efix.
    Efix stDownVAdj{};                      ///< Vertical adjustment for straight downward flags in @ref Efix.
    Evpu flagSpacing{};                     ///< "Flag Spacing" in @ref Evpu. (xml node is `<separ>`)
    Evpu secondaryGroupAdj{};               ///< "Secondary Group Adjust" in @ref Evpu. (xml node is `<extra>`)

    constexpr static std::string_view XmlNodeName = "flagOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<FlagOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class FontOptions
 * @brief An unordered map of default font settings for the document.
 *
 * The FontOptions class stores default font settings used throughout the document. It provides functionality
 * to manage various font attributes, such as size, style (bold, italic), and whether the font is hidden or fixed size.
 */
class FontOptions : public OptionsBase {
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the document object.
     * @param partId Usually 0. This parameter is needed for the generic factory routine.
     * @param shareMode Usually `ShareMode::All`. This parameter is needed for the generic factory routine.
     *
     * Constructs a FontOptions object that is associated with the provided document.
     */
    explicit FontOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /**
     * @enum FontType
     * @brief Types of fonts available for the document.
     *
     * The FontType enum represents different types of fonts used for musical and textual elements within the document.
     */
    enum class FontType
    {
        Music,             ///< Default music font.
        Key,               ///< Key signature font.
        Clef,              ///< Clef symbol font.
        Time,              ///< Time signature font.
        Chord,             ///< Chord name font.
        ChordAcci,         ///< Chord accidental font.
        Ending,            ///< Repeat ending number font.
        Tuplet,            ///< Tuplet font.
        TextBlock,         ///< Text block font.
        LyricVerse,        ///< Lyric verse font.
        LyricChorus,       ///< Lyric chorus font.
        LyricSection,      ///< Lyric section font.
        MultiMeasRest,     ///< Multi-measure rest font.
        Tablature,         ///< Tablature font.
        ChordSuffix,       ///< Chord suffix font.
        Expression,        ///< Expression font.
        Repeat,            ///< Repeat symbol font.
        Fretboard,         ///< Fretboard font.
        Flags,             ///< Flags font.
        Accis,             ///< Accidental symbols font.
        AltNotSlash,       ///< Alternate notation slash font.
        AltNotNum,         ///< Alternate notation number font.
        Rests,             ///< Rests notation font.
        ReptDots,          ///< Repeat dots font.
        Noteheads,         ///< Notehead symbols font.
        AugDots,           ///< Augmentation dots font.
        TimePlus,          ///< Additional time signature symbols font.
        Articulation,      ///< Articulation symbols font.
        Percussion,        ///< Percussion font.
        SmartShape8va,     ///< 8va smart shape font.
        MeasNumb,          ///< Measure number font.
        StaffNames,        ///< Staff names font.
        AbbrvStaffNames,   ///< Abbreviated staff names font.
        GroupNames,        ///< Group names font.
        SmartShape8vb,     ///< 8vb smart shape font.
        SmartShape15ma,    ///< 15ma smart shape font.
        SmartShape15mb,    ///< 15mb smart shape font.
        SmartShapeTrill,   ///< Trill smart shape font.
        SmartShapeWiggle,  ///< Wiggle smart shape font.
        AbbrvGroupNames,   ///< Abbreviated group names font.
        BendCurveFull,     ///< Full bend curve font.
        BendCurveWhole,    ///< Whole bend curve font.
        BendCurveFrac,     ///< Fractional bend curve font.
        TimeParts,         ///< Time signature parts font.
        TimePlusParts      ///< Additional time signature parts font.
    };

    /**
     * @brief Stores the default fonts for different elements.
     *
     * An unordered map that associates each `FontType` with its corresponding `FontInfo` settings.
     */
    std::unordered_map<FontType, MusxInstance<FontInfo>> fontOptions;

    /**
     * @brief get the `FontInfo` for a particular type, if the document has one
     * @param type the `FontType` to retrieve
     * @return a shared pointer to the font info for that type, or nullptr if the document has none
     */
    MusxInstance<FontInfo> getFontInfoOrNull(FontType type) const;

    /**
     * @brief get the `FontInfo` for a particular type from the document pool, if there is one
     * @param document the `Document` to search
     * @param type the `FontType` to retrieve
     * @return a shared pointer to the font info for that type, or nullptr if the document has no
     * options, no default fonts, or no font of that type
     */
    static MusxInstance<FontInfo> getFontInfoOrNull(const DocumentPtr& document, FontType type);

    /**
     * @brief get the `FontInfo` for a particular type
     * @param type the `FontType` to retrieve
     * @return a shared pointer to the font info for that type
     * @throws std::invalid_argument if the type is not found in the document
     */
    MusxInstance<FontInfo> getFontInfo(FontType type) const;

    /**
     * @brief get the `FontInfo` for a particular type from the document pool
     * @param document the `Document` to search
     * @param type the `FontType` to retrieve
     * @return a shared pointer to the font info for that type
     * @throws std::invalid_argument if the type is not found in the document
     */
    static MusxInstance<FontInfo> getFontInfo(const DocumentPtr& document, FontType type);

    /**
     * @brief The XML node name for this type.
     */
    constexpr static std::string_view XmlNodeName = "fontOptions";
    static const xml::XmlElementArray<FontOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class GraceNoteOptions
 * @brief Options controlling the appearance and behavior of grace notes.
 *
 * This class is identified by the XML node name "graceNoteOptions".
 */
class GraceNoteOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit GraceNoteOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    int tabGracePerc{};              ///< "Tablature Grace Note Size" (%)
    int gracePerc{};                 ///< "Grace Note Size" (%)
    Edu playbackDuration{};          ///< Playback duration for grace notes in @ref Edu.
    Evpu entryOffset{};              ///< "Grace Note Offset on Entry" in @ref Evpu. (xml node is `<graceBackup>`)
    bool slashFlaggedGraceNotes{};   ///< "Always Slash Flagged Grace Notes"
    Efix graceSlashWidth{};          ///< Width (thickness) of grace note slashes in @ref Efix.

    constexpr static std::string_view XmlNodeName = "graceNoteOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<GraceNoteOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class KeySignatureOptions
 * @brief Options controlling the appearance and behavior of key signatures.
 *
 * This class is identified by the XML node name "keySignatureOptions".
 */
class KeySignatureOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit KeySignatureOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    bool doKeyCancel{};                        ///< "Cancel Outgoing Key Signature"
    bool doCStart{};                           ///< "In C, Display Naturals at Start of Staff System"
    bool redisplayOnModeChange{};              ///< "Redisplay Key Signature If Only Mode is Changing" (xml node is `<doBankDiff>`)
    Evpu keyFront{};                           ///< "Space Before Key Signature"
    Evpu keyMid{};                             ///< "Space After Cancelled Key"
    Evpu keyBack{};                            ///< "Space After Key Signature"
    Evpu acciAdd{};                            ///< "Space Between Key Signature Accidentals"
    bool showKeyFirstSystemOnly{};             ///< "Show Key Signature Only on First System"
    Evpu keyTimeSepar{};                       ///< "Extra Space Between Key & Time Signatures" (xml node is `<keyTime>`)
    bool simplifyKeyHoldOctave{};              ///< "Preserve Octave When Simplifying Keys"
    bool cautionaryKeyChanges{};               ///< "Display Courtesy Key Signature at End of Staff System"
    bool doKeyCancelBetweenSharpsFlats{};      ///< "When Switching Between Sharps and Flats"

    constexpr static std::string_view XmlNodeName = "keySignatureOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<KeySignatureOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class LineCurveOptions
 * @brief Options controlling the appearance of line and curve elements.
 *
 * This class is identified by the XML node name "lineCurveOptions".
 */
class LineCurveOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit LineCurveOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    int bezierStep{};                    ///< Number of steps for Bézier curves.
    Efix enclosureWidth{};               ///< Default enclosure width in @ref Efix.
    bool enclosureRoundCorners{};        ///< Whether default enclosures have rounded corners.
    Efix enclosureCornerRadius{};        ///< Default rounded corner radius width for enclosures in @ref Efix.
    Efix staffLineWidth{};               ///< Width of staff lines in @ref Efix.
    Efix legerLineWidth{};               ///< Width of leger lines in @ref Efix.
    Evpu legerFrontLength{};             ///< Front length of leger lines in @ref Evpu.
    Evpu legerBackLength{};              ///< Back length of leger lines in @ref Evpu.
    Evpu restLegerFrontLength{};         ///< Front length of rest leger lines in @ref Evpu.
    Evpu restLegerBackLength{};          ///< Back length of rest leger lines in @ref Evpu.
    double psUlDepth{};                  ///< Depth of PostScript underline in 72 dpi points (fractional).
    double psUlWidth{};                  ///< Width of PostScript underline in 72 dpi points (fractional).
    EvpuFloat pathSlurTipWidth{};        ///< Width of slur tip path in fractional @ref EvpuFloat.

    constexpr static std::string_view XmlNodeName = "lineCurveOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<LineCurveOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class LyricOptions
 * @brief Options controlling lyric rendering in the musx file.
 *
 * This class is identified by the XML node name "lyricOptions".
 */
class LyricOptions : public OptionsBase {
public:
    /// @brief Constructor
    explicit LyricOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /// @enum SmartHyphenStart
    /// @brief When to start hyphenation
    enum class SmartHyphenStart {
        Always,     ///< "Always" (default value)
        Sometimes,  ///< "Only when the first syllable is attached to the second note or later"
        Never       ///< "Never"
    };

    /// @enum AutoNumberingAlign
    /// @brief Autonumbering type
    enum class AutoNumberingAlign {
        None,       ///< "Keep with First Syllable in Lyric" (default value)
        Align       ///< "Group Under the Same Note"
    };

    /// @enum WordExtConnectIndex
    /// @brief Word extension connection points
    enum class WordExtConnectIndex {
        LyricRightBottom,
        HeadRightLyrBaseline,
        SystemLeft,
        SystemRight,
        DotRightLyrBaseline,
        DurationLyrBaseline
    };

    /// @enum WordExtConnectStyleType
    /// @brief Word extension connection style categories
    enum class WordExtConnectStyleType {
        DefaultStart,
        DefaultEnd,
        SystemStart,
        SystemEnd,
        DottedEnd,
        DurationEnd,
        OneEntryEnd,
        ZeroLengthEnd,
        ZeroOffset
    };

    /// @enum SyllablePosStyleType
    /// @brief Lyric syllable position style types
    enum class SyllablePosStyleType {
        Default,        ///< "Others:" (always on, even though the `on` member may be false)
        WordExt,        ///< "Syllables with Word Extensions"
        First,          ///< "First Syllable in Lyric"
        SystemStart     ///< "Syllable at Start of System"
    };

    char32_t hyphenChar{};                         ///< "Hyphen Character"
    Evpu maxHyphenSeparation{};                    ///< "Maximum Space Between Hyphens"
    Evpu wordExtVertOffset{};                      ///< "Word Extension Vertical Offset from Baseline"
    Evpu wordExtHorzOffset{};                      ///< "Word Extension Horizontal Offset from Syllable"
    bool useSmartWordExtensions{};                 ///< "Use Smart Word Extensions"
    bool useAltHyphenFont{};                       ///< "Use Alternate Hyphen Font"
    std::shared_ptr<FontInfo> altHyphenFont;       ///< "Alternate Hyphen Font"
    bool useSmartHyphens{};                        ///< "Use Smart Hyphens"
    SmartHyphenStart smartHyphenStart{};           ///< "Smart Hyphen Start"
    bool wordExtNeedUnderscore{};                  ///< "Only Create on Lyrics With Underscores"
    Evpu wordExtMinLength{};                       ///< "Word Extension Minimum Length"
    bool wordExtOffsetToNotehead{};                ///< "Notehead Alignment: Align to Notehead" (if false: "Stretch to Note Duration")
    bool lyricUseEdgePunctuation{};                ///< reverse of "Ignore SyllableEdgePunctuation"
    std::string lyricPunctuationToIgnore;          ///< If this string is omitted in the xml, the default value is populated in #integrityCheck.
    bool showAutoNumbersOnVerses{};                ///< Auto-Number -> Verses
    bool showAutoNumbersOnChoruses{};              ///< Auto-Number -> Choruses
    bool showAutoNumbersOnSections{};              ///< Auto-Number -> Sections
    AutoNumberingAlign lyricAutoNumType{};         ///< "Automatic Lyric Numbers"
    Efix wordExtLineWidth{};                       ///< "Word Extension Line Width"

    /// @brief Syllable position style
    struct SyllablePosStyle {
        AlignJustify align{};                      ///< Horizontal alignment
        AlignJustify justify{};                    ///< Horizontal justification
        bool on{};                                 ///< Enabled state: ignore this value for `SyllablePosStyleType::Default`.
                                                   ///< The default values are the always-applicable fallback values.

        static const xml::XmlElementArray<SyllablePosStyle>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /// @brief Word extension connection style
    struct WordExtConnectStyle {
        WordExtConnectIndex connectIndex{};        ///< Connection point
        Evpu xOffset{};                            ///< Horizontal offset
        Evpu yOffset{};                            ///< Vertical offset

        static const xml::XmlElementArray<WordExtConnectStyle>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    std::unordered_map<SyllablePosStyleType, std::shared_ptr<SyllablePosStyle>> syllablePosStyles;     ///< Syllable positioning styles
    std::unordered_map<WordExtConnectStyleType, std::shared_ptr<WordExtConnectStyle>> wordExtConnectStyles; ///< Word extension connection styles

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OptionsBase::integrityCheck(ptrToThis);
        if (!altHyphenFont) {
            altHyphenFont = std::make_shared<FontInfo>(getDocument());
        }
        if (lyricPunctuationToIgnore.empty()) {
            lyricPunctuationToIgnore = ",.?!;:'\"“”‘’";  // this default punctuation set is taken from Document Without Libraries
        }
    }

    constexpr static std::string_view XmlNodeName = "lyricOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<LyricOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MiscOptions
 * @brief Options controlling miscellaneous settings.
 *
 * This class is identified by the XML node name "miscOptions".
 */
class MiscOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit MiscOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    bool showRepeatsForParts{};             ///< "Show Repeats for Parts"
    Edu pickupValue{};                      ///< The length of a pickup note on the first bar. This value is populated if you used
                                            ///< the "Document->Pickup Measure" menu to create the pickup. If the pickup was created in a legacy version of Finale,
                                            ///< the value is probably not populated. The pickup itself is implemented with a spacer, but #pickupValue specifies
                                            ///< the default rest value for empty staves in the pickup measure.
                                            ///< See @ref others::Frame for more information.
    bool keepWrittenOctaveInConcertPitch{}; ///< "Keep Octave Transposition in Concert Pitch" (xml node is `<retainOctaveTransInConcertPitch>`)
    bool showActiveLayerOnly{};             ///< "Show Active Layer Only" (xml node is `<showCurrentLayerOnly>`)
    bool consolidateRestsAcrossLayers{};    ///< "Consolidate Rests Across Layers" (xml node is `<combineRestsAcrossLayers>`)
    Evpu shapeDesignerDashLength{};         ///< Shape Designer dash length in @ref Evpu. (xml node is `<sdDashOn>`)
    Evpu shapeDesignerDashSpace{};          ///< Shape Designer dash space in @ref Evpu. (xml node is `<sdDashOff>`)
    Evpu restWidthAdjust{};                 ///< Purpose unknown.
    Evpu dblWholeVertAdjust{};              ///< Purpose unknown.
    bool alignMeasureNumbersWithBarlines{}; ///< "Align Measure Numbers With Barlines" (xml node is `<drawMeasureNumbersOverBarlines>`)

    constexpr static std::string_view XmlNodeName = "miscOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MiscOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MultimeasureRestOptions
 * @brief Options controlling the appearance and behavior of multimeasure rests.
 *
 * This class is identified by the XML node name "multimeasureRestOptions".
 */
class MultimeasureRestOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit MultimeasureRestOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    Evpu measWidth{};            ///< Space allocated per measure in multimeasure rest in @ref Evpu. (xml node is `<meaSpace>`)
    Evpu numAdjY{};              ///< Vertical adjustment for number in @ref Evpu. The Finale UI reverses the sign of this value. (xml node is `<numdec>`)
    Cmper shapeDef{};            ///< @ref Cmper of shape for multimeasure rest H-bar.
    int numStart{};              ///< Value for "Start Number At".
    int useSymsThreshold{};      ///< Numbers of bars rest below this number use symbols if `useSymbols` is checked. (xml node is `<threshold>`)
    Evpu symSpacing{};           ///< Spacing between symbols in @ref Evpu if `useSymbols` is checkd. (xml node is `<spacing>`)
    Evpu numAdjX{};              ///< Horizontal adjustment for number in @ref Evpu.
    Evpu startAdjust{};          ///< Start point adjustment for H-bar shape in @ref Evpu.
    Evpu endAdjust{};            ///< End point adjustment for H-bar shape in @ref Evpu.
    bool useSymbols{};           ///< Use character style for rests. (xml node is `<useCharRestStyle>`)
    bool noHorizontalStretch{};  ///< Inverse of "Stretch Horizontally" option.
    bool autoUpdateMmRests{};    ///< Automatically update multimeasure rests when the user changes these options.

    constexpr static std::string_view XmlNodeName = "multimeasureRestOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MultimeasureRestOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MusicSpacingOptions
 * @brief Options controlling music spacing.
 *
 * This class is identified by the XML node name "musicSpacingOptions".
 */
class MusicSpacingOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit MusicSpacingOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /// @enum ColUnisonsChoice
    /// @brief Options for how to handle unisons
    enum class ColUnisonsChoice
    {
        None,           ///< Do not avoid collisions with unisons. (The default.)
        DiffNoteheads,  ///< Avoid collisions with unisons that have different noteheads.
        All             ///< Avoid collisions with all unisons.
    };

    /// @enum GraceNoteSpacing
    /// @brief Options for how to handle grace note spacing
    enum class GraceNoteSpacing
    {
        ResetToEntry,   ///< Reset to the entry position. (The default.)
        Automatic,      ///< Automatically recompute grace note spacing. This activates `minDistGrace`. (xml value is "recomp")
        KeepCurrent     ///< Keep current grace note spacing. (xml value is "keep")
    };

    /// @enum ManualPositioning
    /// @brief Options for how to handle manual positioning
    enum class ManualPositioning
    {
        Clear,          ///< Clear manual positioning (the default).
        Ignore,         ///< Ignore manual positioning.
        Incorporate     ///< Incorporate manual positioning. (xml value is "incorp")
    };

    // layoutType appears to be cruft left over from legacy Finale. The only value seems to be "entry".
    Evpu minWidth{};                        ///< Minimum measure width in @ref Evpu.
    Evpu maxWidth{};                        ///< Maximum measure width in @ref Evpu.
    Evpu minDistance{};                     ///< Minimum distance between items in @ref Evpu.
    Evpu minDistTiedNotes{};                ///< Minimum distance between tied notes in @ref Evpu.
    bool avoidColNotes{};                   ///< Avoid collisions with notes.
    bool avoidColLyrics{};                  ///< Avoid collisions with lyrics.
    bool avoidColChords{};                  ///< Avoid collisions with chords.
    bool avoidColArtics{};                  ///< Avoid collisions with articulations.
    bool avoidColClefs{};                   ///< Avoid collisions with clefs.
    bool avoidColSeconds{};                 ///< Avoid collisions with seconds.
    bool avoidColStems{};                   ///< Avoid collisions with stems.
    ColUnisonsChoice avoidColUnisons{};     ///< Choice for how to handle collisions with unisons.
    bool avoidColLedgers{};                 ///< Avoid collisions with ledger lines.
    ManualPositioning manualPositioning{};  ///< Manual positioning choice.
    bool ignoreHidden{};                    ///< Ignore hidden notes. (This value is reversed in the Finale UI.)
    bool interpolateAllotments{};           ///< Interpolate allotments.
    bool usePrinter{};                      ///< "Use Fonts & Resolution From Printer" (may be a Windows-only option)
    bool useAllottmentTables{};             ///< Use allotment tables for spacing.
    Edu referenceDuration{};                ///< Reference duration for spacing in @ref Edu.
    Evpu referenceWidth{};                  ///< Reference width for spacing in @ref Evpu.
    double scalingFactor{};                 ///< Scaling factor for spacing.
    Evpu defaultAllotment{};                ///< Default allotment for spacing in @ref Evpu. (This value may not be settable in the Finale UI.)
    Evpu minDistGrace{};                    ///< Minimum distance for grace notes in @ref Evpu.
    GraceNoteSpacing graceNoteSpacing{};    ///< Choice for how to handle grace note spacing. (xml node is `<graceNoteManualPositioning>`)
    Evpu musFront{};                        ///< Additional spacing before the first note in @ref Evpu. (From Notes & Rests window.)
    Evpu musBack{};                         ///< Additional spacing after the last note in @ref Evpu. (From Notes & Rests window.)

    constexpr static std::string_view XmlNodeName = "musicSpacingOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MusicSpacingOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MusicSymbolOptions
 * @brief Specifies font-specific glyphs used to render common musical symbols.
 *
 * Each field stores a code point identifying the glyph to use for a particular
 * notation symbol. The font used to interpret a given value is determined by
 * the associated #FontOptions::FontType for that symbol category (e.g., noteheads,
 * accidentals, rests).
 *
 * - For SMuFL-compliant fonts, values are Unicode code points corresponding to
 *   SMuFL glyphs.
 * - For non-SMuFL music fonts (such as Maestro), values typically represent raw
 *   glyph indices rather than Unicode code points.
 *
 * Use #FontInfo::calcIsSymbolFont to determine whether a (non-SMuFL) font uses raw
 * glyph indices.
 *
 * This class corresponds to the XML element "musicSymbolOptions".
 */
class MusicSymbolOptions : public OptionsBase
{
public:
    /** @brief Constructor function */
    explicit MusicSymbolOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode)
    {
    }

    char32_t noteheadQuarter{};         ///< Code point for quarter notehead (#FontOptions::FontType::Noteheads)
    char32_t noteheadHalf{};            ///< Code point for half notehead (#FontOptions::FontType::Noteheads)
    char32_t noteheadWhole{};           ///< Code point for whole notehead (#FontOptions::FontType::Noteheads)
    char32_t noteheadDblWhole{};        ///< Code point for double whole notehead (#FontOptions::FontType::Noteheads)
    char32_t natural{};                 ///< Code point for natural sign (#FontOptions::FontType::Accis)
    char32_t flat{};                    ///< Code point for flat sign (#FontOptions::FontType::Accis)
    char32_t sharp{};                   ///< Code point for sharp sign (#FontOptions::FontType::Accis)
    char32_t dblFlat{};                 ///< Code point for double flat sign (#FontOptions::FontType::Accis)
    char32_t dblSharp{};                ///< Code point for double sharp sign (#FontOptions::FontType::Accis)
    char32_t parenNatural{};            ///< Code point for parenthesized natural (#FontOptions::FontType::Accis)
    char32_t parenFlat{};               ///< Code point for parenthesized flat (#FontOptions::FontType::Accis)
    char32_t parenSharp{};              ///< Code point for parenthesized sharp (#FontOptions::FontType::Accis)
    char32_t parenDblFlat{};            ///< Code point for parenthesized double flat (#FontOptions::FontType::Accis)
    char32_t parenDblSharp{};           ///< Code point for parenthesized double sharp (#FontOptions::FontType::Accis)
    char32_t chordNatural{};            ///< Code point for chord natural sign (#FontOptions::FontType::ChordAcci)
    char32_t chordFlat{};               ///< Code point for chord flat sign (#FontOptions::FontType::ChordAcci)
    char32_t chordSharp{};              ///< Code point for chord sharp sign (#FontOptions::FontType::ChordAcci)
    char32_t chordDblFlat{};            ///< Code point for chord double flat sign (#FontOptions::FontType::ChordAcci)
    char32_t chordDblSharp{};           ///< Code point for chord double sharp sign (#FontOptions::FontType::ChordAcci)
    char32_t keySigNatural{};           ///< Code point for key signature natural (#FontOptions::FontType::Key)
    char32_t keySigFlat{};              ///< Code point for key signature flat (#FontOptions::FontType::Key)
    char32_t keySigSharp{};             ///< Code point for key signature sharp (#FontOptions::FontType::Key)
    char32_t keySigDblFlat{};           ///< Code point for key signature double flat (#FontOptions::FontType::Key)
    char32_t keySigDblSharp{};          ///< Code point for key signature double sharp (#FontOptions::FontType::Key)
    char32_t restLonga{};               ///< Code point for longa rest (#FontOptions::FontType::Rests)
    char32_t restDblWhole{};            ///< Code point for double whole rest (#FontOptions::FontType::Rests)
    char32_t restWhole{};               ///< Code point for whole rest (#FontOptions::FontType::Rests)
    char32_t restHalf{};                ///< Code point for half rest (#FontOptions::FontType::Rests)
    char32_t restQuarter{};             ///< Code point for quarter rest (#FontOptions::FontType::Rests)
    char32_t restEighth{};              ///< Code point for eighth rest (#FontOptions::FontType::Rests)
    char32_t rest16th{};                ///< Code point for 16th rest (#FontOptions::FontType::Rests)
    char32_t rest32nd{};                ///< Code point for 32nd rest (#FontOptions::FontType::Rests)
    char32_t rest64th{};                ///< Code point for 64th rest (#FontOptions::FontType::Rests)
    char32_t rest128th{};               ///< Code point for 128th rest (#FontOptions::FontType::Rests)
    char32_t restDefMeas{};             ///< Code point for default measure rest (#FontOptions::FontType::Rests).
                                        ///< Finale uses this value only when it recognizes the glyph as a rest;
                                        ///< otherwise it uses #restWhole.
    char32_t oneBarRepeat{};            ///< Code point for one-bar repeat (#FontOptions::FontType::AltNotSlash)
    char32_t twoBarRepeat{};            ///< Code point for two-bar repeat (#FontOptions::FontType::AltNotSlash)
    char32_t slashBar{};                ///< Code point for slash bar (#FontOptions::FontType::AltNotSlash)
    char32_t quarterSlash{};            ///< Code point for quarter slash (#FontOptions::FontType::AltNotSlash)
    char32_t halfSlash{};               ///< Code point for half slash (#FontOptions::FontType::AltNotSlash)
    char32_t wholeSlash{};              ///< Code point for whole slash (#FontOptions::FontType::AltNotSlash)
    char32_t dblWholeSlash{};           ///< Code point for double whole slash (#FontOptions::FontType::AltNotSlash)
    char32_t timeSigPlus{};             ///< Code point for time signature plus sign (#FontOptions::FontType::TimePlus)
    char32_t timeSigPlusParts{};        ///< Code point for time signature plus (parts) (#FontOptions::FontType::TimePlusParts)
    char32_t timeSigAbrvCommon{};       ///< Code point for common time abbreviation (#FontOptions::FontType::Time)
    char32_t timeSigAbrvCut{};          ///< Code point for cut time abbreviation (#FontOptions::FontType::Time)
    char32_t timeSigAbrvCommonParts{};  ///< Code point for common time abbreviation (parts) (#FontOptions::FontType::TimeParts)
    char32_t timeSigAbrvCutParts{};     ///< Code point for cut time abbreviation (parts) (#FontOptions::FontType::TimeParts)
    char32_t augDot{};                  ///< Code point for augmentation dot (#FontOptions::FontType::AugDots)
    char32_t forwardRepeatDot{};        ///< Code point for forward repeat dot (#FontOptions::FontType::ReptDots)
    char32_t backRepeatDot{};           ///< Code point for backward repeat dot (#FontOptions::FontType::ReptDots)
    char32_t eightVaUp{};               ///< Code point for 8va (ottava) up (#FontOptions::FontType::SmartShape8va)
    char32_t eightVbDown{};             ///< Code point for 8vb (ottava) down (#FontOptions::FontType::SmartShape8vb)
    char32_t fifteenMaUp{};             ///< Code point for 15ma (quindicesima) up (#FontOptions::FontType::SmartShape15ma)
    char32_t fifteenMbDown{};           ///< Code point for 15mb (quindicesima) down (#FontOptions::FontType::SmartShape15mb)
    char32_t trillChar{};               ///< Code point for trill symbol (#FontOptions::FontType::SmartShapeTrill)
    char32_t wiggleChar{};              ///< Code point for wiggle (trill line) symbol (#FontOptions::FontType::SmartShapeWiggle)
    char32_t flagUp{};                  ///< Code point for upward flag (#FontOptions::FontType::Flags)
    char32_t flagDown{};                ///< Code point for downward flag (#FontOptions::FontType::Flags)
    char32_t flag16Up{};                ///< Code point for 16th note upward flag (#FontOptions::FontType::Flags)
    char32_t flag16Down{};              ///< Code point for 16th note downward flag (#FontOptions::FontType::Flags)
    char32_t flag2Up{};                 ///< Code point for 32nd note upward flag (#FontOptions::FontType::Flags)
    char32_t flag2Down{};               ///< Code point for 32nd note downward flag (#FontOptions::FontType::Flags)
    char32_t flagStraightUp{};          ///< Code point for straight upward flag (#FontOptions::FontType::Flags)
    char32_t flagStraightDown{};        ///< Code point for straight downward flag (#FontOptions::FontType::Flags)

    constexpr static std::string_view XmlNodeName = "musicSymbolOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MusicSymbolOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class NoteRestOptions
 * @brief Options controlling note/rest display and positioning.
 */
class NoteRestOptions : public OptionsBase {
public:
    /** @brief Constructor function. */
    explicit NoteRestOptions(const DocumentPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /**
     * @class NoteColor
     * @brief 16-bit per-channel note color (RGB) used by Finale.
     *
     * Instances are indexed according to 12-EDO pitch class, 0 - C, 1 - C#/Db, 2 - D, 3 - D#/Eb, ...
     */
    class NoteColor {
    public:
        uint16_t red{};         ///< Red component in Finale's 16-bit range.
        uint16_t green{};       ///< Green component in Finale's 16-bit range.
        uint16_t blue{};        ///< Blue component in Finale's 16-bit range.

        /// Required for musx::factory::FieldPopulator.
        static const xml::XmlElementArray<NoteColor>& xmlMappingArray();
    };

    bool doShapeNotes{};            ///< "Use Shape Notes" - the default set of note shapes is stored in Cmper 0 of @ref details::ShapeNote.
                                    ///< If true, this overrides the staff-level flag. However, if a staff or staff style has individual
                                    ///< note-shape settings, those take precedence over the default shapes.
    bool doCrossStaffNotes{};       ///< Inverse of "Display Cross-Staff Notes in Original Staff" (xml node is `<doCrossOver>`)
    Evpu drop8thRest{};             ///< Vertical 8th rest positioning from staff default line. (Usually the center line.)
    Evpu drop16thRest{};            ///< Vertical 16th rest positioning from staff default line. (Usually the center line.)
    Evpu drop32ndRest{};            ///< Vertical 32nd rest positioning from staff default line. (Usually the center line.)
    Evpu drop64thRest{};            ///< Vertical 64th rest positioning from staff default line. (Usually the center line.)
    Evpu drop128thRest{};           ///< Vertical 128th (and smaller) rest positioning from staff default line. (Usually the center line.)
    bool scaleManualPositioning{};  ///< "Scale Manual Positioning of Notes"
    bool drawOutline{};             ///< "Show border around colored noteheads"
    std::vector<std::shared_ptr<NoteColor>> noteColors{}; ///< Notehead colors, one per pitch-class.

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OptionsBase::integrityCheck(ptrToThis);
        const size_t currentSize = noteColors.size();
        if (currentSize < music_theory::STANDARD_12EDO_STEPS) {
            for (size_t x = currentSize; x < music_theory::STANDARD_12EDO_STEPS; x++) {
                noteColors.emplace_back(std::make_shared<NoteColor>());  // default values are all zero, so this is setting missing values to black color
            }
            MUSX_INTEGRITY_ERROR("Only " + std::to_string(currentSize) + " note colors provided. "
                + std::to_string(music_theory::STANDARD_12EDO_STEPS) + " were expected.");
        } else if (currentSize > music_theory::STANDARD_12EDO_STEPS) {
            noteColors.resize(music_theory::STANDARD_12EDO_STEPS);
            MUSX_INTEGRITY_ERROR(std::to_string(currentSize) + " note colors provided. Only "
                + std::to_string(music_theory::STANDARD_12EDO_STEPS) + " were expected.");
        }
    }

    constexpr static std::string_view XmlNodeName = "noteRestOptions";      ///< XML node name for this type.
    static const xml::XmlElementArray<NoteRestOptions>& xmlMappingArray();  ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class PageFormatOptions
 * @brief Options for page formatting in the document.
 *
 * This class provides configuration options for page formatting, including
 * scope adjustment, page format score, and page format parts.
 */
class PageFormatOptions : public OptionsBase {
public:
    /**
     * @enum AdjustPageScope
     * @brief Enum for the scope of page adjustments.
     */
    enum class AdjustPageScope
    {
        Current,        ///< Adjust the current page only.
        All,            ///< Adjust all pages.
        LeftOrRight,    ///< Adjust left or right pages.
        PageRange       ///< Adjust page range. (xml value is "range")
    };

    /**
     * @class PageFormat
     * @brief Represents the format settings for a page.
     */
    class PageFormat
    {
    public:
        Evpu pageHeight{};               ///< Height of the page.
        Evpu pageWidth{};                ///< Width of the page.
        int pagePercent{};               ///< Page scaling percentage (a value of 100 means no scaling).
        int sysPercent{};                ///< System scaling percentage (a value of 100 means no scaling).
        Evpu16ths rawStaffHeight{};      ///< Raw staff height (in 1/16 Evpu units). Note that this is different units than #others::StaffSystem::staffHeight.
        Evpu leftPageMarginTop{};        ///< Top margin for the left page. (Sign reversed in Finale UI.)
        Evpu leftPageMarginLeft{};       ///< Left margin for the left page.
        Evpu leftPageMarginBottom{};     ///< Bottom margin for the left page.
        Evpu leftPageMarginRight{};      ///< Right margin for the left page. (Sign reversed in Finale UI.)
        Evpu rightPageMarginTop{};       ///< Top margin for the right page. (Sign reversed in Finale UI.)
        Evpu rightPageMarginLeft{};      ///< Left margin for the right page.
        Evpu rightPageMarginBottom{};    ///< Bottom margin for the right page.
        Evpu rightPageMarginRight{};     ///< Right margin for the right page. (Sign reversed in Finale UI.)
        Evpu sysMarginTop{};             ///< System top margin. (Sign reversed in Finale UI.)
        Evpu sysMarginLeft{};            ///< System left margin.
        Evpu sysMarginBottom{};          ///< System bottom margin. This value is 96 (i.e., 1 5-line staff thickness) less than the U.I. value.
                                         ///< That means if you enter 0 in the Finale UI, this value is -96. The U.I. value is also sign reversed.
        Evpu sysMarginRight{};           ///< System bottom margin. (Sign reversed in Finale UI.)
        Evpu sysDistanceBetween{};       ///< Distance between systems. (Sign reversed in Finale UI.)
        Evpu firstPageMarginTop{};       ///< Top margin for the first page. (Sign reversed in Finale UI.)
        Evpu firstSysMarginTop{};        ///< Top margin for the first system. (Sign reversed in Finale UI.)
        Evpu firstSysMarginLeft{};       ///< Left margin for the first system.
        Evpu firstSysMarginDistance{};   ///< Distance between the first systems. (Sign reversed in Finale UI.)
        bool facingPages{};              ///< Whether to use the right page margin values.
        bool differentFirstSysMargin{};  ///< Whether to use the first system values.
        bool differentFirstPageMargin{}; ///< Whether to use the `firstPageMarginTop` value.

        /// @brief Calculates the page scaling as a #musx::util::Fraction where 1/1 means no scaling.
        util::Fraction calcPageScaling() const { return util::Fraction::fromPercent(pagePercent); }

        /// @brief Calculates the system scaling as a #musx::util::Fraction where 1/1 means no scaling.
        util::Fraction calcSystemScaling() const
        { return util::Fraction::fromPercent(sysPercent) * util::Fraction(rawStaffHeight, int(EVPU_PER_SPACE) * 4 * 16); }

        /// @brief Calculates the combined system & page scaling as a #musx::util::Fraction where 1/1 means no scaling.
        util::Fraction calcCombinedSystemScaling() const { return calcPageScaling() * calcSystemScaling(); }

        /**
         * @brief Default constructor for PageFormat.
         */
        PageFormat() = default;

        static const xml::XmlElementArray<PageFormat>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    // Properties
    AdjustPageScope adjustPageScope{};    ///< Scope of page adjustments.
    std::shared_ptr<PageFormat> pageFormatScore; ///< Page format for score settings.
    std::shared_ptr<PageFormat> pageFormatParts; ///< Page format for parts settings.
    bool avoidSystemMarginCollisions{};   ///< Whether to avoid system margin collisions.

    /** @brief Calculates the page format options for a specific part
     *
     * The options are mixed with actual page and system information for the specified part
     * to produce a set of page format options that matches the input @p partId if it uses consistent
     * page formatting on all its pages. This may be useful if a particular part uses different settings
     * than the other parts. An example might be a Piano/Vocal score with a different page size than the
     * other parts.
     *
     * @param partId [in] the part for which to calculate. Pass #SCORE_PARTID for the score.
     * @returns A pointer to a detached instance of @ref PageFormat that is a best approximation
     * to the settings for the input part.
     */
    MusxInstance<PageFormat> calcPageFormatForPart(Cmper partId) const;

    /**
     * @brief Constructor for PageFormatOptions.
     *
     * @param document A shared pointer to the document.
     * @param partId Usually 0. This parameter is needed for the generic factory routine.
     * @param shareMode Usually `ShareMode::All`. This parameter is needed for the generic factory routine.
     */
    explicit PageFormatOptions(const DocumentPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /**
     * @brief The XML node name for this type.
     */
    constexpr static std::string_view XmlNodeName = "pageFormatOptions";
    static const xml::XmlElementArray<PageFormatOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class PianoBraceBracketOptions
 * @brief Options controlling the appearance of piano braces and brackets.
 *
 * This class is identified by the XML node name "pianoBraceBracketOptions".
 */
class PianoBraceBracketOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit PianoBraceBracketOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    Evpu defBracketPos{};                   ///< "Group Brackets: Default Distance from Left Edge of Staff"
    EvpuFloat centerThickness{};            ///< Center thickness of braces in fractional @ref EvpuFloat.
    EvpuFloat tipThickness{};               ///< End thickness of braces in fractional @ref EvpuFloat. (xml node is `<endThickness>`)
    EvpuFloat outerBodyV{};                 ///< Vertical position of the outer wing in fractional @ref EvpuFloat. (xml node is `<outerWingY>`)
    EvpuFloat innerTipV{};                  ///< Vertical position of the inner tip in fractional @ref EvpuFloat. (xml node is `<innerTipY>`)
    EvpuFloat innerBodyV{};                 ///< Vertical position of the inner body in fractional @ref EvpuFloat. (xml node is `<innerWingY>`)
    EvpuFloat outerTipH{};                  ///< Horizontal position of the outer tip in fractional @ref EvpuFloat. (xml node is `<outerTipX>`)
    EvpuFloat outerTipV{};                  ///< Vertical position of the outer tip in fractional @ref EvpuFloat. (xml node is `<outerTipY>`)
    EvpuFloat outerBodyH{};                 ///< Horizontal positione of the outer body in fractional @ref EvpuFloat. (xml node is `<outerWingX>`)
    EvpuFloat width{};                      ///< Width of braces in fractional @ref EvpuFloat.
    EvpuFloat innerTipH{};                  ///< Horizontal position of the inner tip in fractional @ref EvpuFloat. (xml node is `<innerTipX>`)
    EvpuFloat innerBodyH{};                 ///< Horizontal position of the inner body in fractional @ref EvpuFloat. (xml node is `<innerWingX>`)

    constexpr static std::string_view XmlNodeName = "pianoBraceBracketOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<PianoBraceBracketOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class RepeatOptions
 * @brief Options controlling the appearance of repeats.
 *
 * This class is identified by the XML node name "repeatOptions".
 */
class RepeatOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit RepeatOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /// @enum WingStyle
    /// @brief Wing Styles from Document Options - Repeats
    enum class WingStyle
    {
        None,
        Curved,
        SingleLine,
        DoubleLine
    };

    /// @enum BackToBackStyle
    /// @brief Back-to-Back Styles from Document Options - Repeats
    enum class BackToBackStyle
    {
        Thin,
        Mixed,
        Thick
    };

    int maxPasses{};                       ///< Maximum number of passes for repeats.
    bool addPeriod{};                      ///< "Add Period After Number"
    Efix thickLineWidth{};                 ///< Heavy line thickness in @ref Efix.
    Efix thinLineWidth{};                  ///< Thin line thickness in @ref Efix.
    Efix lineSpace{};                      ///< Space between lines in @ref Efix.
    BackToBackStyle backToBackStyle{};     ///< Back-to-back style.
    Evpu forwardDotHPos{};                 ///< Horizontal separation of forward repeat dots in @ref Evpu.
    Evpu backwardDotHPos{};                ///< Horizontal separation of backward repeat dots in @ref Evpu.
    Evpu upperDotVPos{};                   ///< Vertical adjustment of the upper dot in @ref Evpu.
    Evpu lowerDotVPos{};                   ///< Vertical adjustment of the lower dot in @ref Evpu.
    WingStyle wingStyle{};                 ///< Wing style.
    Evpu afterClefSpace{};                 ///< Space after clef in @ref Evpu.
    Evpu afterKeySpace{};                  ///< Space after key signature in @ref Evpu.
    Evpu afterTimeSpace{};                 ///< Space after time signature in @ref Evpu.
    Evpu bracketHeight{};                  ///< Height of the bracket in @ref Evpu.
    Evpu bracketHookLen{};                 ///< Length of the bracket hook in @ref Evpu.
    Efix bracketLineWidth{};               ///< Width of the bracket line in @ref Efix.
    Evpu bracketStartInset{};              ///< Inset at the start of the bracket in @ref Evpu.
    Evpu bracketEndInset{};                ///< Inset at the end of the bracket in @ref Evpu.
    Evpu bracketTextHPos{};                ///< Horizontal position of bracket text in @ref Evpu.
    Evpu bracketTextVPos{};                ///< Vertical position of bracket text in @ref Evpu.
    Evpu bracketEndHookLen{};              ///< Length of the hook at the end of the bracket in @ref Evpu.
    bool bracketEndAnchorThinLine{};       ///< Whether the end of the bracket anchors to a thin line.
    bool showOnTopStaffOnly{};             ///< "Show On Top Staff Only"
    Cmper showOnStaffListNumber{};         ///< Staff list number (0 if none). @ref Cmper links to associated "repeatStaffList..." xml nodes.

    constexpr static std::string_view XmlNodeName = "repeatOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<RepeatOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class SmartShapeOptions
 * @brief Options controlling the appearance of smart shapes in the musx file.
 *
 * This class is identified by the XML node name "smartShapeOptions".
 */
class SmartShapeOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit SmartShapeOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /// @enum ConnectionIndex
    /// @brief Connection index values
    enum class ConnectionIndex
    {
        HeadLeftTop,
        HeadRightTop,
        HeadRightBottom,
        HeadLeftBottom,
        StemLeftTop,
        StemRightTop,
        StemRightBottom,
        StemLeftBottom,
        NoteLeftTop,
        NoteRightTop,
        NoteRightBottom,
        NoteLeftBottom,
        NoteLeftCenter,
        NoteRightCenter
    };

    /// @enum SlurConnectStyleType
    /// @brief Slur connection style types
    enum class SlurConnectStyleType
    {
        OverNoteStart,
        OverNoteEnd,
        OverStemStart,
        OverStemEnd,
        UnderNoteStart,
        UnderNoteEnd,
        UnderStemStart,
        UnderStemEnd,
        OverMixStemStart,
        OverMixStemEnd,
        OverStemGrace,
        OverStemPrincipal,
        UnderStemGrace,
        UnderStemPrincipal,
        UnderNoteGrace,
        UnderStemNotePrincipal,
        OverNoteGrace,
        OverStemNotePrincipal,
        OverBeamStart,
        OverBeamEnd,
        UnderBeamStart,
        UnderBeamEnd,
        OverMixFlagStart,
        OverFlagStart,
        UnderFlagStart,
        OverTabNumStart,
        OverTabNumEnd,
        UnderTabNumStart,
        UnderTabNumEnd
    };

    /// @enum TabSlideConnectStyleType
    /// @brief Tab slide connection style types
    enum class TabSlideConnectStyleType
    {
        DiffLevelPitchUpLineStart,
        DiffLevelPitchUpLineEnd,
        DiffLevelPitchUpSpaceStart,
        DiffLevelPitchUpSpaceEnd,
        DiffLevelPitchDownLineStart,
        DiffLevelPitchDownLineEnd,
        DiffLevelPitchDownSpaceStart,
        DiffLevelPitchDownSpaceEnd,
        SameLevelPitchUpLineStart,
        SameLevelPitchUpLineEnd,
        SameLevelPitchUpSpaceStart,
        SameLevelPitchUpSpaceEnd,
        SameLevelPitchDownLineStart,
        SameLevelPitchDownLineEnd,
        SameLevelPitchDownSpaceStart,
        SameLevelPitchDownSpaceEnd,
        SameLevelPitchSameStart,
        SameLevelPitchSameEnd
    };

    /// @enum GlissandoConnectStyleType
    /// @brief Glissando connection style types
    enum class GlissandoConnectStyleType
    {
        DefaultStart,
        DefaultEnd
    };

    /// @enum BendCurveConnectStyleType
    /// @brief Bend curve connection style types
    enum class BendCurveConnectStyleType
    {
        NoteStart,
        StaffEnd,
        StaffStart,
        NoteEnd,
        StaffToTopLineStart,
        StaffFromTopLineEnd,
        StaffEndOffset,
        StaffFromTopEndOffset
    };

    /// @enum SlurControlStyleType
    /// @brief Slur control style types
    enum class SlurControlStyleType
    {
        ShortSpan,
        MediumSpan,
        LongSpan,
        ExtraLongSpan
    };

    Evpu shortHairpinOpeningWidth{};         ///< "Short Hairpin Opening Width"
    Evpu crescHeight{};                      ///< "Crescendo Height"
    Evpu maximumShortHairpinLength{};       ///< "Maximum Short Hairpin Length"
    Efix crescLineWidth{};                  ///< "Crescendo/Decrescendo Line Width"
    Evpu hookLength{};                       ///< "Hook Length"
    Efix smartLineWidth{};                  ///< "Smart Line Width"
    bool showOctavaAsText{};                   ///< "Show Octava As Text"
    Evpu smartDashOn{};                      ///< "Smart Dash On Length"
    Evpu smartDashOff{};                     ///< "Smart Dash Off Length"
    bool crescHorizontal{};                    ///< "Horizontal Crescendo"
    ShapeDirection direction{};              ///< Default slur direction
    Evpu slurThicknessCp1X{};                ///< "Slur Thickness Control Point 1 X"
    Evpu slurThicknessCp1Y{};                ///< "Slur Thickness Control Point 1 Y"
    Evpu slurThicknessCp2X{};                ///< "Slur Thickness Control Point 2 X"
    Evpu slurThicknessCp2Y{};                ///< "Slur Thickness Control Point 2 Y"
    bool slurAvoidAccidentals{};               ///< "Slur Avoid Accidentals"
    Evpu slurAvoidStaffLinesAmt{};            ///< "Slur Avoid Staff Lines Amount"
    Efix maxSlurStretch{};                 ///< "Maximum Slur Stretch"
    Efix maxSlurLift{};                    ///< "Maximum Slur Lift"
    int slurSymmetry{};                   ///< "Slur Symmetry"
    bool useEngraverSlurs{};                   ///< "Use Engraver Slurs"
    Evpu slurLeftBreakHorzAdj{};              ///< "Slur Left Break Horizontal Adjustment"
    Evpu slurRightBreakHorzAdj{};            ///< "Slur Right Break Horizontal Adjustment"
    Evpu slurBreakVertAdj{};                 ///< "Slur Break Vertical Adjustment"
    bool slurAvoidStaffLines{};             ///< "Slur Avoid Staff Lines"
    Evpu slurPadding{};                      ///< "Slur Padding"
    Efix maxSlurAngle{};                   ///< "Maximum Slur Angle"
    Evpu slurAcciPadding{};                  ///< "Slur Accidental Padding"
    bool slurDoStretchFirst{};              ///< "Initial Adjustment: Stretch"
    bool slurStretchByPercent{};            ///< "As Percentage of Slur Length"
    int maxSlurStretchPercent{};          ///< "Maximum Slur Stretch Percentage"
    Evpu articAvoidSlurAmt{};                 ///< "Articulation Avoid Slur Amount"
    Cmper ssLineStyleCmpCustom{};              ///< Cmper of current custom line style. (@ref others::SmartShapeCustomLine)
    Cmper ssLineStyleCmpGlissando{};           ///< Cmper of current glissando line style. (@ref others::SmartShapeCustomLine)
    Cmper ssLineStyleCmpTabSlide{};            ///< Cmper of current tab slide line style. (@ref others::SmartShapeCustomLine)
    Cmper ssLineStyleCmpTabBendCurve{};        ///< Cmper of current tab bend line style. (@ref others::SmartShapeCustomLine)
    EvpuFloat smartSlurTipWidth{};            ///< "Smart Slur Tip Width"
    bool guitarBendUseParens{};                ///< "Guitar Bend Use Parentheses"
    bool guitarBendHideBendTo{};               ///< "Guitar Bend Hide Bend To"
    bool guitarBendGenText{};                  ///< "Guitar Bend Generate Text"
    bool guitarBendUseFull{};                  ///< "Guitar Bend Use Full"

    /// @brief Defined the connection offsets for each @ref ConnectionIndex value.
    struct ConnectionStyle {
        ConnectionIndex connectIndex{}; ///< Index of connection point
        Evpu xOffset{};                ///< Horizontal offset
        Evpu yOffset{};                ///< Vertical offset

        static const xml::XmlElementArray<ConnectionStyle>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /// @brief defines the slur contours for short, medium, long, and extra long slurs
    struct ControlStyle {
        Efix span{};                  ///< Length of the span
        Efix inset{};                 ///< Inset value
        Evpu height{};               ///< Height of the span

        static const xml::XmlElementArray<ControlStyle>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    std::unordered_map<SlurConnectStyleType, std::shared_ptr<ConnectionStyle>> slurConnectStyles; ///< Slur connections
    std::unordered_map<SlurControlStyleType, std::shared_ptr<ControlStyle>> slurControlStyles; ///< Slur contours
    std::unordered_map<TabSlideConnectStyleType, std::shared_ptr<ConnectionStyle>> tabSlideConnectStyles; ///< Tab slide connections
    std::unordered_map<GlissandoConnectStyleType, std::shared_ptr<ConnectionStyle>> glissandoConnectStyles; ///< Glissando connections
    std::unordered_map<BendCurveConnectStyleType, std::shared_ptr<ConnectionStyle>> bendCurveConnectStyles; ///< Benx curve connections

    constexpr static std::string_view XmlNodeName = "smartShapeOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<SmartShapeOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StaffOptions
 * @brief Options controlling the appearance and layout of staves.
 *
 * This class is identified by the XML node name "staffOptions".
 */
class StaffOptions : public OptionsBase {
using NamePositioning = others::NamePositioning;

public:
    /** @brief Constructor function */
    explicit StaffOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    Evpu staffSeparation{};         ///< Default topline-to-topline distance in @ref Evpu. (This value is sign-reversed in the Finale UI.)
    Evpu staffSeparIncr{};          ///< "Add Vertical Space" value for Setup Wizard.
    bool autoAdjustStaffSepar{};    ///< "The Default Value Topline-to-Topline Distance is a Preferred Value That Can Change if Necessary" (for Setup Wizard.)

    std::shared_ptr<NamePositioning> namePos;           ///< Default full name positioning for staves.
    std::shared_ptr<NamePositioning> namePosAbbrv;      ///< Default abbreviated name positioning for staves.
    std::shared_ptr<NamePositioning> groupNameFullPos;  ///< Default full name positioning for staff groups.
    std::shared_ptr<NamePositioning> groupNameAbbrvPos; ///< Default abbreviated name positioning for staff groups.

    constexpr static std::string_view XmlNodeName = "staffOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<StaffOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StemOptions
 * @brief Options controlling the appearance of stems.
 *
 * This class is identified by the XML node name "stemOptions".
 */
class StemOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit StemOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode)
    {
    }

    /**
     * @class StemConnection
     * @brief Information about one stemConnections element.
     */
    class StemConnection
    {
    public:
        Cmper fontId{};       ///< The font ID of the font this connection applies to. Zero means default music font. (xml nodename is `<font>`)
        char32_t symbol{};    ///< The codepoint of the symbol glyph in the font specified by #fontId.
        Efix upStemVert{};    ///< Upstem vertical adjustment.
        Efix downStemVert{};  ///< Downstem vertical adjustment.
        Efix upStemHorz{};    ///< Upstem horizontal adjustment.
        Efix downStemHorz{};  ///< Downstem horizontal adjustment.

        static const xml::XmlElementArray<StemConnection>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    Evpu halfStemLength{};     ///< Half stem length in @ref Evpu.
    Evpu stemLength{};         ///< Stem length in @ref Evpu.
    Evpu shortStemLength{};    ///< Short stem length in @ref Evpu. (xml node is `<stem2>`)
    Evpu revStemAdj{};         ///< Reverse stem adjustment in @ref Evpu.
    bool noReverseStems{};     ///< Set if "Display Reverse Stemming" is unchecked. (This bit is reversed in the Finale UI.)
    Efix stemWidth{};          ///< Stem width in @ref Efix.
    Efix stemOffset{};         ///< Stem offset in @ref Efix. (xml node is `<stemLift>`)
    bool useStemConnections{}; ///< "Use Stem Connections"

    /// @brief Array of stem connection definitions.
    ///
    /// The document factory builds this as a contiguous array up to the highest index
    /// encountered, inserting zero-initialized placeholders for any missing indices.
    ///
    /// @note Finale appears to treat the first @ref StemConnection with a zero @c symbol
    /// as a terminator and ignores that entry and all following ones. Nevertheless,
    /// Finale-produced files may still contain additional `<stemConnect>` elements after
    /// this point, sometimes with nonsensical or out-of-range values (e.g. 33554432).
    /// Such trailing entries are preserved here verbatim but are probably ignored
    /// by Finale.
    std::vector<std::shared_ptr<StemConnection>> stemConnections;

    constexpr static std::string_view XmlNodeName = "stemOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<StemOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/// @enum AccidentalInsertSymbolType
/// @brief Insert symbol types for accidentals
enum class AccidentalInsertSymbolType
{
    Sharp,
    Flat,
    Natural,
    DblSharp,
    DblFlat
};

/**
 * @class TextOptions
 * @brief Options controlling text rendering in the musx file. Many of these options are default values that are inserted
 * into Enigma tags when a page or measure text is created. Others are default values for the assignment. As such, once
 * a page or measure text has been created, it does not change even if these default values change.
 *
 * This class is identified by the XML node name "textOptions".
 */
class TextOptions : public OptionsBase
{
public:
    static constexpr int DEFAULT_LINE_SPACING_PERCENT = 100;

    /// @brief Constructor
    explicit TextOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode)
    {
    }

    /**
     * @brief Horizontal alignment options for page text positioning.
     */
    using HorizontalAlignment = AlignJustify;

    /**
     * @enum VerticalAlignment
     * @brief Vertical alignment options for page text positioning.
     */
    enum class VerticalAlignment
    {
        Top, // default value: leave as first (0) item
        Center,
        Bottom
    };

    /// @enum TextJustify
    /// @brief Text justification options
    enum class TextJustify {
        Left,           ///< "left"
        Center,         ///< "center"
        Right,          ///< "right"
        Full,           ///< "full"
        ForcedFull      ///< "forcedFull"
    };

    /// @brief "Line Spacing: Automatic" percent value.
    ///
    /// Line spacing is either a percent or an absolute distance, and the xml carries whichever applies.
    /// Exactly one of this and #textLineSpacingEvpu has a value once #integrityCheck has run.
    std::optional<int> textLineSpacingPercent;
    std::optional<Evpu> textLineSpacingEvpu;      ///< "Line Spacing: Set To" absolute value. (See #textLineSpacingPercent.)
    bool showTimeSeconds{};                       ///< "Include Seconds in Time Stamp"
    DateFormat dateFormat{};                      ///< "Date Format"
    int tabSpaces{};                              ///< "Use [x] Spaces in Place of One Tab Character"
    int textTracking{};                           ///< "Tracking" amount in EMs (1/1000 of the font size)
    Evpu textBaselineShift{};                     ///< "Baseline Shift" amount
    Evpu textSuperscript{};                       ///< "Superscript" amount
    bool textWordWrap{};                          ///< "Word Wrap"
    Evpu textPageOffset{};                        ///< "Page Offset"
    TextJustify textJustify{};                    ///< "Justification"
    bool textExpandSingleWord{};                  ///< "Expand Single Word"
    HorizontalAlignment textHorzAlign{};          ///< "Horizontal Alignment"
    VerticalAlignment textVertAlign{};            ///< "Vertical Alignment"
    bool textIsEdgeAligned{};                     ///< "Position from Page Edge"

    /// @brief Insert symbol information
    struct InsertSymbolInfo : public ContainedClassBase
    {
        using ContainedClassBase::ContainedClassBase;

        int trackingBefore{};                     ///< Tracking before in EMs (1/1000 font size units)
        int trackingAfter{};                      ///< Tracking after in EMs (1/1000 font size units)
        int baselineShiftPerc{};                  ///< Baseline shift percent
        std::shared_ptr<FontInfo> symFont;        ///< Symbol font (Percent-based size is a percent of the preceding font size in the Enigma string.)
        char32_t symChar{};                       ///< Symbol character

        void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
        {
            this->ContainedClassBase::integrityCheck(ptrToThis);
            if (!symFont) {
                symFont = std::make_shared<FontInfo>(getDocument(), /*sizeIsPercent*/ true);
                symFont->fontSize = 100;
                MUSX_INTEGRITY_ERROR("Text options accidental insert information is missing its font information.");
            }
        }

        static const xml::XmlElementArray<InsertSymbolInfo>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    std::unordered_map<AccidentalInsertSymbolType, std::shared_ptr<InsertSymbolInfo>> symbolInserts; ///< Insert symbol information map

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OptionsBase::integrityCheck(ptrToThis);
        if (textLineSpacingPercent && textLineSpacingEvpu) {
            textLineSpacingEvpu.reset();
            MUSX_INTEGRITY_ERROR("Text options specify both a percent and an absolute line spacing. "
                "The absolute value was discarded.");
        } else if (!textLineSpacingPercent && !textLineSpacingEvpu) {
            textLineSpacingPercent = DEFAULT_LINE_SPACING_PERCENT;
            MUSX_INTEGRITY_ERROR("Text options specify no line spacing. "
                + std::to_string(DEFAULT_LINE_SPACING_PERCENT) + " percent was assumed.");
        }
    }

    constexpr static std::string_view XmlNodeName = "textOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TextOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TieOptions
 * @brief Options controlling the appearance of ties.
 *
 * This class is identified by the XML node name "tieOptions".
 */
class TieOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit TieOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /**
     * @enum SecondsPlacement
     * @brief Enumeration for seconds placement options
     */
    enum class SecondsPlacement
    {
        None,                   ///< the default
        ShiftForSeconds         ///< xml value is "both"
    };

    /**
     * @enum ChordTieDirType
     * @brief Enumeration for chord tie direction type
     */
    enum class ChordTieDirType
    {
        OutsideInside,          ///< Legacy Finale 3.7 behavior (the default)
        StemReversal,           ///< Split at stem reversal point.
        SplitEvenly,            ///< Split evenly
        Unknown                 ///< Unknown (seen sometimes in upgraded legacy files)
    };

    /**
     * @enum MixedStemDirection
     * @brief Enumeration for mixed stem direction
     */
    enum class MixedStemDirection
    {
        Over,
        Under,
        OppositeFirst           ///< xml value is "start"
    };

    /**
     * @enum SpecialPosMode
     * @brief Enumeration for special position mode
     */
    enum class SpecialPosMode
    {
        None,       ///< the default
        Avoid       ///< avoid staff lines
    };

    /**
     * @enum InsetStyle
     * @brief Enumeration for inset styles
     */
    enum class InsetStyle
    {
        Fixed,                  ///< the default
        Percent
    };

    /**
     * @enum ControlStyleType
     * @brief Enumeration for tie control style types
     */
    enum class ControlStyleType
    {
        ShortSpan,
        MediumSpan,
        LongSpan,
        TieEnds
    };

    Evpu frontTieSepar{};                       ///< Separation for the front of ties
    Evpu thicknessRight{};                      ///< Right thickness of the tie
    Evpu thicknessLeft{};                       ///< Left thickness of the tie
    bool breakForTimeSigs{};                    ///< Break ties for time signatures
    bool breakForKeySigs{};                     ///< Break ties for key signatures
    Evpu breakTimeSigLeftHOffset{};             ///< Left horizontal offset for breaking time signature
    Evpu breakTimeSigRightHOffset{};            ///< Right horizontal offset for breaking time signature
    Evpu breakKeySigLeftHOffset{};              ///< Left horizontal offset for breaking key signature
    Evpu breakKeySigRightHOffset{};             ///< Right horizontal offset for breaking key signature
    Evpu sysBreakLeftHAdj{};                    ///< Left adjustment for system breaks
    Evpu sysBreakRightHAdj{};                   ///< Right adjustment for system breaks
    bool useOuterPlacement{};                   ///< Use outer placement for ties
    SecondsPlacement secondsPlacement{};        ///< Placement of seconds
    ChordTieDirType chordTieDirType{};          ///< Chord tie direction type
    bool chordTieDirOpposingSeconds{};          ///< Chord tie opposing seconds
    MixedStemDirection mixedStemDirection{};    ///< Mixed stem direction
    bool afterSingleDot{};                      ///< Special handling after a single dot
    bool afterMultipleDots{};                   ///< Special handling after multiple dots
    bool beforeAcciSingleNote{};                ///< Special handling before accidental single notes
    SpecialPosMode specialPosMode{};            ///< Whether to avoid staff lines. (This may have been called `specialPosMode` for planned future behaviors never implemented.)
    Evpu avoidStaffLinesDistance{};             ///< Distance to avoid staff lines
    InsetStyle insetStyle{};                    ///< Inset style for ties
    bool useInterpolation{};                    ///< "Interpolate Height Between Short and Long Span"
    bool useTieEndCtlStyle{};                   ///< Use tie end control style
    bool avoidStaffLinesOnly{};                 ///< Only avoid staff lines
    EvpuFloat tieTipWidth{};                    ///< Width of the tie tip

    /** @brief Struct for tie connect style */
    struct ConnectStyle {
        Evpu offsetX{}; ///< Horizontal offset
        Evpu offsetY{}; ///< Vertical offset

        static const xml::XmlElementArray<ConnectStyle>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /** @brief Tie connect styles */
    std::unordered_map<TieConnectStyleType, std::shared_ptr<ConnectStyle>> tieConnectStyles;

    /** @brief Struct for tie control style control points */
    struct ControlPoint {
        Efix insetRatio{}; ///< Inset ratio
        Evpu height{}; ///< Height
        Evpu insetFixed{}; ///< Fixed inset

        static const xml::XmlElementArray<ControlPoint>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /** @brief Struct for tie control style */
    struct ControlStyle {
        Evpu span{}; ///< Span length
        std::shared_ptr<ControlPoint> cp1; ///< Control point 1
        std::shared_ptr<ControlPoint> cp2; ///< Control point 2

        static const xml::XmlElementArray<ControlStyle>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /** @brief Tie control styles */
    std::unordered_map<ControlStyleType, std::shared_ptr<ControlStyle>> tieControlStyles;

    constexpr static std::string_view XmlNodeName = "tieOptions"; ///< The XML node name for this type
    static const xml::XmlElementArray<TieOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TimeSignatureOptions
 * @brief Options controlling the appearance and behavior of time signatures.
 *
 * This class is identified by the XML node name "timeSignatureOptions".
 */
class TimeSignatureOptions : public OptionsBase
{
public:
    /** @brief Constructor function */
    explicit TimeSignatureOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    Evpu timeUpperLift{};           ///< "Vertical Adjustment: Top Symbol, Score"
    Evpu timeFront{};               ///< "Space Before Time Signature, Score"
    Evpu timeBack{};                ///< "Space After Time Signature, Score"
    Evpu timeFrontParts{};          ///< "Space Before Time Signature, Parts"
    Evpu timeBackParts{};           ///< "Space After Time Signature, Parts"
    Evpu timeUpperLiftParts{};      ///< "Vertical Adjustment: Top Symbol, Parts"
    Evpu timeLowerLiftParts{};      ///< "Vertical Adjustment: Bottom Symbol, Parts"
    Evpu timeAbrvLiftParts{};       ///< "Vertical Adjustment: Abbreviated Symbol, Parts"
    bool timeSigDoAbrvCommon{};     ///< Abbreviate common time.
    bool timeSigDoAbrvCut{};        ///< Abbreviate cut time.
    int numCompositeDecimalPlaces{};///< "Decimal Places for Composite Meters" (xml node is `<defFloat>`)
    bool cautionaryTimeChanges{};   ///< "Display Courtesy Time Signature at End of Staff System"
    Evpu timeLowerLift{};           ///< "Vertical Adjustment: Bottom Symbol, Score"
    Evpu timeAbrvLift{};            ///< "Vertical Adjustment: Abbreviated Symbol, Score"

    constexpr static std::string_view XmlNodeName = "timeSignatureOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TimeSignatureOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TupletOptions
 * @brief Options controlling the appearance of tuplets.
 *
 * This class is identified by the XML node name "tupletOptions".
 */
class TupletOptions : public OptionsBase {
public:
    /** @brief Constructor function */
    explicit TupletOptions(const DocumentWeakPtr& document, Cmper partId = 0, ShareMode shareMode = ShareMode::All)
        : OptionsBase(document, partId, shareMode) {}

    /**
     * @enum AutoBracketStyle
     * @brief Auto-bracket style options.
     */
    enum class AutoBracketStyle
    {
        Always,             ///< "Always Use Specified Shape"--the default value
        UnbeamedOnly,       ///< "Bracket Unbeamed Notes Only"
        NeverBeamSide,      ///< "Never Bracket Beamed Notes on Beam Side"
    };

    /**
     * @enum NumberStyle
     * @brief Number style options.
     *
     * @note Finale appears to have implemented `RatioPlusBothNotes` and `RatioPlusDenominatorNote`
     * backwards from how they are named in the xml. It is probably a long-standing bug that was never worth fixing.
     * This DOM corrects the mistake by reversing their mapping in the enum mapping.
    */
    enum class NumberStyle
    {
        Nothing,                    ///< The default
        Number,                     ///< Number only
        UseRatio,                   ///< Ratio X:Y
        RatioPlusDenominatorNote,   ///< displays a note value only on the denominator (xml value is "ratioPlusBothNotes")
        RatioPlusBothNotes,         ///< display notes on both values (xml value is "ratioPlusDenNote")
    };

    /**
     * @enum PositioningStyle
     * @brief Positioning style options.
     */
    enum class PositioningStyle
    {
        Manual,     ///< The default
        BeamSide,   ///< Display the tuplet on the beam side
        NoteSide,   ///< Display the tuplet on the note side
        Above,      ///< Display the tuplet above the notes
        Below       ///< Display the tuplet below the notes
    };

    /**
     * @enum BracketStyle
     * @brief Bracket style options.
     */
    enum class BracketStyle
    {
        Nothing,    ///< No bracket (the default)
        Slur,       ///< Use a slur instead of a bracket.
        Bracket     ///< Use a bracket.
    };

    int displayNumber{};                    ///< The number of notes to display (xml node is `<symbolicNum>`)
    Edu displayDuration{};                  ///< The duration of each note to display (xml node is `<symbolicDur>`)
    int referenceNumber{};                  ///< The number of notes "in the time of" (xml node is `<refNum>`)
    Edu referenceDuration{};                ///< The duration of each note "in the time of" (xml node is `<refDur>`)
    bool alwaysFlat{};                      ///< "Always Flat" (xml node is `<flat>`)
    bool fullDura{};                        ///< "Bracket Full Duration"
    bool metricCenter{};                    ///< "Center Number Using Duration"
    bool avoidStaff{};                      ///< "Avoid Staff"
    AutoBracketStyle autoBracketStyle{};    ///< Autobracket style
    Evpu tupOffX{};                         ///< Horizontal offset.
    Evpu tupOffY{};                         ///< Vertical.
    Evpu brackOffX{};                       ///< Horizontal offset for brackets.
    Evpu brackOffY{};                       ///< Vertical offset for brackets.
    NumberStyle numStyle{};                 ///< Number style
    PositioningStyle posStyle{};            ///< Positioning style
    bool allowHorz{};                       ///< "Allow Horizontal Drag"
    bool ignoreHorzNumOffset{};             ///< "Ignore Horizontal Number Offset" (xml node is `<ignoreGlOffs>`)
    bool breakBracket{};                    ///< "Break Slur or Bracket"
    bool matchHooks{};                      ///< "Match Length of Hooks"
    bool useBottomNote{};                   ///< "Use Bottom Note" (xml node is `<noteBelow>`)
    BracketStyle brackStyle{};              ///< Bracket style.
    bool smartTuplet{};                     ///< "Engraver Tuplets"
    Evpu leftHookLen{};                     ///< Length of the left hook in the tuplet bracket. (This value is sign-reversed in the Finale UI.)
    Evpu leftHookExt{};                     ///< Extension of the left hook beyond the tuplet bracket.
    Evpu rightHookLen{};                    ///< Length of the right hook in the tuplet bracket. (This value is sign-reversed in the Finale UI.)
    Evpu rightHookExt{};                    ///< Extension of the right hook beyond the tuplet bracket.
    Evpu manualSlopeAdj{};                  ///< "Manual Slope Adjustment" in @ref Evpu. (xml node is `<slope>`)
    int tupMaxSlope{};                      ///< Maximum slope for automatic tuplet brackets in 10ths of an angular degree.
    Efix tupLineWidth{};                    ///< Line width for tuplet brackets in @ref Efix.
    Evpu tupNUpstemOffset{};                ///< Offset for upstem tuplet numbers.
    Evpu tupNDownstemOffset{};              ///< Offset for downstem tuplet numbers.

    constexpr static std::string_view XmlNodeName = "tupletOptions"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TupletOptions>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

} // namespace options
} // namespace dom
} // namespace musx
