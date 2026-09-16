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
#include <stdexcept>
#include <map>
#include <tuple>

#include "musx/util/EnigmaString.h"
#include "musx/util/Logger.h"
#include "musx/util/Fraction.h"
#include "musx/util/PseudoTieUtils.h"
#include "musx/dom/PercussionNoteType.h"

#include "BaseClasses.h"
#include "CommonClasses.h"
#include "Options.h"
 // do not add other dom class dependencies. Use Implementations.cpp for implementations that need total class access.

namespace musx {
namespace utils {
enum class PseudoTieMode;
} // namespace utils

namespace dom {

class Entry;
class NoteInfoPtr;

enum class KnownShapeDefType;

namespace details {
class FretboardDiagram;
class GFrameHold;
class IndependentStaffDetails;
class StaffGroup;
}

/**
 * @namespace musx::dom::others
 * @brief Classes in the @ref OthersPool.
 */
namespace others {

class ShapeDef;
class Staff;
class StaffComposite;
class StaffList;
class StaffListCategoryScore;
class StaffListCategoryParts;
class StaffListRepeatScore;
class StaffListRepeatParts;
class StaffListRepeatScoreForced;
class StaffListRepeatPartsForced;

template<class ScoreList, class PartsList, class ScoreForcedList, class PartsForcedList>
class StaffListSet;

/// @brief The type of @ref StaffListSet used by @ref MarkingCategory.
using CategoryStaffListSet = StaffListSet<StaffListCategoryScore, StaffListCategoryParts, void, void>;

/// @brief The type of @ref StaffListSet used by @ref RepeatBack, @ref RepeatEndingStart, and @ref TextRepeatAssign.
using RepeatStaffListSet = StaffListSet<StaffListRepeatScore, StaffListRepeatParts, StaffListRepeatScoreForced, StaffListRepeatPartsForced>;

/**
 * @class AcciAmountFlats
 * @brief Lists the aleration values of each nth flat in a custom key signature.
 * Normally these values are all set to the number of EDO divisions in a chromatic half-step.
 *
 * Note that while flats are numbered from 1-7, this table is indexed 0-6.
 *
 * AcciAmountFlats is primarily useful with microtone systems that use standard key
 * signatures. Typically linear modes in 12-EDO will not have this table, but larger EDOs
 * need to specify acci alteration amounts with magnitude larger than 1.
 *
 * The cmper is the value returned by #KeySignature::getKeyMode. The built-in major and minor
 * cmpers (0 and 1) ignore this table, and it should not be present for those values. (However, with Finale
 * anything is possible.)
 *
 * This class is identified by the XML node name "acciAmountFlats".
 */
class AcciAmountFlats : public OthersArray<int, 7>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using OthersArray::OthersArray;

    constexpr static std::string_view XmlNodeName = "acciAmountFlats"; ///< The XML node name for this type.
    static const xml::XmlElementArray<AcciAmountFlats>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class AcciAmountSharps
 * @brief Lists the aleration values of each nth sharp in a custom key signature.
 * Normally these values are all set to the number of EDO divisions in a chromatic half-step.
 *
 * Note that while flats are numbered from 1-7, this table is indexed 0-6.
 *
 * AcciAmountSharps is required for non-linear key signatures. It specifies whether a slot
 * is a sharp or a flat with a positive or negative value. The first zero value in the table
 * terminates the sequence of accidentals in the key signature.
 *
 * AcciAmountSharps is also useful with microtone systems that use standard key signatures.
 * Typically linear modes in 12-EDO will not have this table, but EDOs larger than 19-EDO
 * may need to specify acci alteration amounts with magnitude larger than 1 (equal to the number
 * of EDO divisions in a chromatic half-step.)
 *
 * The cmper is the value returned by #KeySignature::getKeyMode. The built-in major and minor
 * cmpers (0 and 1) ignore this table, and it should not be present for those values. (However, with Finale
 * anything is possible.)
 *
 * This class is identified by the XML node name "acciAmountFlats".
 */
class AcciAmountSharps : public OthersArray<int, 7>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using OthersArray::OthersArray;

    constexpr static std::string_view XmlNodeName = "acciAmountSharps"; ///< The XML node name for this type.
    static const xml::XmlElementArray<AcciAmountSharps>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class AcciOrderFlats
 * @brief Lists the order of flats by pitch class index (0..6 = C..B) in a custom key signature.
 * Note that while flats are numbered from 1-7, this table is indexed 0-6.
 *
 * AcciOrderFlats is primarily useful with microtone systems that use standard key
 * signatures. Typically linear modes in 12-EDO will not have this table, but larger EDOs
 * will need to supply an @ref AcciAmountFlats table, and this is here to match it.
 *
 * The cmper is the value returned by #KeySignature::getKeyMode. The built-in major and minor
 * cmpers (0 and 1) ignore this table, and it should not be present for those values. (However, with Finale
 * anything is possible.)
 *
 * This class is identified by the XML node name "acciAmountFlats".
 */
class AcciOrderFlats : public OthersArray<unsigned, 7>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using OthersArray::OthersArray;

    constexpr static std::string_view XmlNodeName = "acciOrderFlats"; ///< The XML node name for this type.
    static const xml::XmlElementArray<AcciOrderFlats>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class AcciOrderSharps
 * @brief Lists the order of sharps by pitch class index (0..6 = C..B) in a custom key signature.
 * Note that while sharps are numbered from 1-7, this table is indexed 0-6.
 *
 * AcciOrderSharps is required for non-linear key signatures. It specifies the order of
 * pitch class indices for the corresponding sharps and flats in @ref AcciAmountSharps, which can be mixed in any order.
 * The first zero value in the @ref AcciAmountSharps table terminates the sequence of sharps or flats, and the rest
 * of the values here are meaningless.
 *
 * AcciOrderSharps is also useful with microtone systems that use standard key
 * signatures. Typically linear modes in 12-EDO will not have this table, but larger EDOs
 * will need to supply an @ref AcciAmountSharps table, and this should have values that correspond.
 *
 * The cmper is the value returned by #KeySignature::getKeyMode. The built-in major and minor
 * cmpers (0 and 1) ignore this table, and it should not be present for those values. (However, with Finale
 * anything is possible.)
 *
 * This class is identified by the XML node name "acciAmountFlats".
 */
class AcciOrderSharps : public OthersArray<unsigned, 7>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using OthersArray::OthersArray;

    constexpr static std::string_view XmlNodeName = "acciOrderSharps"; ///< The XML node name for this type.
    static const xml::XmlElementArray<AcciOrderSharps>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ArticulationDef
 * @brief Stores the properties and behaviors of articulation definitions.
 *
 * The playback "delta" and "percent" values are alternatives. For each type of playback modification, one or the other
 * is used. The other is zero.
 *
 * This class is identified by the XML node name "articDef".
 */
class ArticulationDef : public OthersBase
{
public:
    /**
     * @enum AutoVerticalMode
     * @brief Defines the automatic vertical positioning mode. These values are only meaningful
     * if #autoVert is true. Otherwise #autoVertMode has the default value but it means "Manual"
     */
    enum class AutoVerticalMode
    {
        AlwaysNoteheadSide,         ///< default value (may not appear in xml)
        AutoNoteStem,               ///< "Auto Notehead/Stem Side"
        StemSide,                   ///< "Always Stem Side"
        AlwaysOnStem,               ///< "On Stem"
        AboveEntry,                 ///< "Above Note"
        BelowEntry,                 ///< "Below Note"
    };

    /**
     * @enum CopyMode
     * @brief Defines the character copy mode (vertical or horizontal)
     */
    enum class CopyMode
    {
        None,                       ///< default value (may not appear in xml)
        Vertical,                   ///< Copy symbol vertically (xml value is "both")
        Horizontal                  ///< Copy symbol horizontally
    };

    /**
     * @enum SlurInteractionMode
     * @brief Defines the interaction mode with slurs.
     */
    enum class SlurInteractionMode
    {
        Ignore,             ///< default value (may not appear in xml)
        InsideSlur,
        AvoidSlur
    };

    /// @brief The resolved main or alternate symbol selection for an articulation definition.
    struct SelectedSymbol
    {
        bool usesAlternate{};                    ///< True when the alternate symbol is selected.
        bool isShape{};                          ///< Whether the selected symbol uses a shape definition.
        Cmper shapeId{};                         ///< Shape ID for the selected symbol when #isShape is true.
        Evpu xOffset{};                          ///< Horizontal offset for the selected symbol.
        Evpu yOffset{};                          ///< Vertical offset for the selected symbol.
        char32_t character{};                    ///< The selected symbol character.
        std::shared_ptr<FontInfo> font;          ///< Font info for the selected symbol.
    };

    /**
     * @brief Constructor.
     *
     * Initializes all fields to their default values.
     */
    explicit ArticulationDef(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper), fontMain(new FontInfo(document)), fontAlt(new FontInfo(document)) {}

    char32_t charMain{};                           ///< Main symbol character (utf32).
    std::shared_ptr<FontInfo> fontMain;            ///< The font info for the main symbol. (xml nodes `<fontMain>`, `<sizeMain>`, and `<efxMain>`)
    CopyMode copyMode{};                           ///< "Copy Main Symbol" option.
    bool useTopNote{};                             ///< "Attach to top note"
    bool autoHorz{};                               ///< Whether horizontal auto-positioning is enabled.
    bool autoVert{};                               ///< Whether vertical auto-positioning is enabled.
    AutoVerticalMode autoVertMode{};               ///< Auto vertical positioning mode.
    bool outsideStaff{};                           ///< Whether the articulation is forced outside the staff.
    bool aboveSymbolAlt{};                         ///< Whether the alternate symbol is used above. (Otherwise main symbol is used.)
    bool belowSymbolAlt{};                         ///< Whether the alternate symbol is used below. (Otherwise main symbol is used.)
    bool insideSlur{};                             ///< Whether the articulation is inside a slur. (Used *in addition* to #SlurInteractionMode::InsideSlur)
    bool noPrint{};                                ///< "Display on screen only (do not print)"
    bool autoStack{};                              ///< Whether automatic stacking is enabled.
    bool centerOnStem{};                           ///< Whether centering on the stem is enabled.
    SlurInteractionMode slurInteractionMode{};     ///< Slur interaction mode.
    char32_t charAlt{};                            ///< Alternate symbol character (utf32).
    std::shared_ptr<FontInfo> fontAlt;             ///< The font info for the alternate symbol. (xml nodes `<fontAlt>`, `<sizeAlt>`, and `<efxAlt>`)
    Evpu xOffsetMain{};                            ///< Horizontal offset for the main symbol.
    Evpu yOffsetMain{};                            ///< Vertical offset for the main symbol.
    Evpu defVertPos{};                             ///< Default vertical position.
    bool avoidStaffLines{};                        ///< Whether to avoid staff lines.
    bool isStemSideWhenMultipleLayers{};           ///< "Place stem side when multiple layers are present"
                                                   ///< (Applicable only for #AutoVerticalMode::AboveEntry.)
    bool playArtic{};                              ///< Whether playback articulation is enabled.
    Evpu xOffsetAlt{};                             ///< Horizontal offset for the alternate symbol.
    Evpu yOffsetAlt{};                             ///< Vertical offset for the alternate symbol.
    bool mainIsShape{};                            ///< Whether the main symbol is a shape.
    bool altIsShape{};                             ///< Whether the alternate symbol is a shape.
    Cmper mainShape{};                             ///< Main shape ID (if applicable).
    Cmper altShape{};                              ///< Alternate shape ID (if applicable).
    int startTopNoteDelta{};                       ///< Attack change for the top note.
    int startBotNoteDelta{};                       ///< Attack change for the bottom note.
    int startTopNotePercent{};                     ///< Attack change percent for the top note.
    int startBotNotePercent{};                     ///< Attack change percent for the bottom note.
    int durTopNoteDelta{};                         ///< Duration change for the top note.
    int durBotNoteDelta{};                         ///< Duration change for the bottom note.
    int durTopNotePercent{};                       ///< Duration percent change for the top note.
    int durBotNotePercent{};                       ///< Duration percent change for the bottom note.
    int ampTopNoteDelta{};                         ///< Key velocity change for the top note.
    int ampBotNoteDelta{};                         ///< Key velocity change for the bottom note.
    int ampTopNotePercent{};                       ///< Key velocity percentage for the top note.
    int ampBotNotePercent{};                       ///< Key velocity percentage for the bottom note.
    Evpu distanceFromStemEnd{};                    ///< "On-stem distance from stem end/flag/beam"

    /// @brief Resolves which symbol data applies for the specified placement.
    /// @param placeAbove True when the articulation is placed above the entry; false when below.
    /// @return The selected main or alternate symbol data.
    [[nodiscard]] SelectedSymbol calcSelectedSymbol(bool placeAbove) const;

    constexpr static std::string_view XmlNodeName = "articDef"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ArticulationDef>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class BeatChartElement
 * @brief A single beat chart element from Finale's music spacing system.
 *
 * Beat charts define graphical spacing in measures with custom horizontal layout.
 * They typically consist of a control element followed by several beat positions,
 * each indicating its duration, position, and spacing information.
 *
 * Each instance corresponds to a `<beatChart>` XML node.
 * Only one `<control>` is expected per measure (inci = 0), and it must precede other entries.
 *
 * The cmper is the 1-based measure number for the beat chart.
 * The inci is a 0-based index into the list of elements in that measure’s beat chart.
 * The cmper is shared among all entries for a given measure; inci increases with each new item.
 *
 * Only inci 0 has the `Control` instance, but this control data is needed by all subsequent
 * elements in the same measure. The only data member of interest in inci 0 is the control. Inci 0
 * is not a member of the list but rather a header.
 *
 * After automatic note spacing, the pos / endPos fields contain the “ideal width” for each
 * duration, based on the Music Spacing settings in Document Options. Any additional space is
 * represented in the minPos fields.
 *
 * For any beat chart array of N elements, the display width for element `n` may be calculated as:
 *
 * ```
 * Evpu16 nextMinPos = (n < N - 1) ? element[n + 1].minPos : control->minWidth;
 * Evpu16 width = MAX(element[n].endPos - element[n].pos, nextMinPos - element[n].minPos);
 * ```
 *
 * If the user manually edits a beat chart, Finale transitions the array from automatic to manual
 * as follows: The first pos is set to the first minPos, and then the width value (as calculated
 * above) is used to determine the rest of the pos and endPos fields. The final endPos value
 * is stored in totalWidth. Finale also sets the first minPos to 0 and all others (including
 * control->minWidth) to 1.
 *
 * The control->allotWidth member has a purpose and effect, but it has proved elusive to document
 * exactly how it works.
 *
 * This class is identified by the XML node name "beatChart".
 */
class BeatChartElement : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit BeatChartElement(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci)
    {
    }

    /** @class Control
     *  @brief Control settings for a beat chart, only used when inci == 0.
     *
     *  These values function as a header for the other incis in the chart (starting with inci == 1).
     */
    class Control
    {
    public:
        Edu totalDur{};     ///< Total logical duration of the measure (XML: `<totalDur>`)
        Evpu totalWidth{};  ///< Total width allocated to the measure (XML: `<totalWidth>`)
        Evpu minWidth{};    ///< Minimum required width (XML: `<minWidth>`)
        Evpu allotWidth{};  ///< Allotted width before justification (XML: `<allotWidth>`)

        static const xml::XmlElementArray<Control>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    // Populated only when the XML node includes a <control> element
    std::shared_ptr<Control> control;  ///< Control data for this beat chart element (only present for inci == 0)

    // Populated for standard beat entries (inci > 0)
    Edu dur{};         ///< Duration of this beat span
    Evpu pos{};        ///< Horizontal position within the measure
    Evpu endPos{};     ///< End position of the beat span
    Evpu minPos{};     ///< Minimum position (see remarks in the class-level description of @ref BeatChartElement)

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        OthersBase::integrityCheck(ptrToThis);
        if (control && getInci() != 0) {
            MUSX_INTEGRITY_ERROR("Beat chart for measure " + std::to_string(getCmper()) + " has a control instance in inci " + std::to_string(*getInci()));
        }
        if (getInci() == 0 && !control) {
            control = std::make_shared<Control>();
            MUSX_INTEGRITY_ERROR("Beat chart for measure " + std::to_string(getCmper()) + " is missing its control instance.");
        }
    }

    constexpr static std::string_view XmlNodeName = "beatChart"; ///< The XML node name for this type.
    static const xml::XmlElementArray<BeatChartElement>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ChordSuffixElement
 * @brief Represents a single element in a chord suffix (e.g., a symbol like "1" or "+").
 *
 * Each chord suffix consists of one or more such elements, each with its own font and positioning. The @p inci
 * is a 0-based index for the element.
 *
 * The cmper is a unique identifier used by @ref details::ChordAssign to reference the suffix and its playback information.
 * (See @ref ChordSuffixPlayback.) Suffixes are a library from which chord assignments select their particular suffix.
 *
 * This class is identified by the XML node name "chordSuffix".
 */
class ChordSuffixElement : public OthersBase
{
public:
    /**
     * @enum Prefix
     * @brief Enum for chord symbol prefix options
     * */
    enum class Prefix
    {
        None,   ///< No prefix (default: may not appear in XML)
        Minus,  ///< minus symbol ("-")
        Plus,   ///< plus symbol ("+")
        Sharp,  ///< musical sharp symbol
        Flat    ///< musical flat symbol
    };

    /** @brief Constructor function */
    explicit ChordSuffixElement(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci), font(new FontInfo(document))
    {
    }

    std::shared_ptr<FontInfo> font; ///< Font info for this symbol (xml nodes are `<fontID>`, `<fontSize>`, and `<efx>`)
    char32_t symbol{};              ///< Codepoint of glyph in #font. (xml node is `<suffix>`)
    Evpu xdisp{};                   ///< Horizontal displacement in EVPU
    Evpu ydisp{};                   ///< Vertical displacement in EVPU
    bool isNumber{};                ///< Indicates the #symbol value is numeric rather than a codepoint.
    Prefix prefix{};                ///< Optional prefix for the symbol, e.g., "plus"

    constexpr static std::string_view XmlNodeName = "chordSuffix"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ChordSuffixElement>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ChordSuffixPlayback
 * @brief An array of semitone offsets that define the playback and voicing for the chord suffix. They
 * are offsets from the root of the chord and may be negative or positive. Finale chord playback
 * has no support for microtone scales, so all values in this table are strictly half-steps.
 *
 * Zero-values (usually trailing at the end of the list) should be ignored.
 *
 * The cmper is the cmper of the @ref ChordSuffixElement array with the same cmper.
 *
 * This class is identified by the XML node name "chordSuffixPlay".
 */
class ChordSuffixPlayback : public OthersArray<int16_t>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using OthersArray::OthersArray;

    constexpr static std::string_view XmlNodeName = "chordSuffixPlay"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ChordSuffixPlayback>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ClefList
 * @brief Represents an element in multimeasure clef list with its positioning and percentage values.
 *
 * The cmper is obtained from the @ref details::GFrameHold instance for the staff and measure. Inci 0
 * is the initial clef at the barline.
 *
 * This class is identified by the XML node name "clefEnum".
 */
class ClefList : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit ClefList(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci)
    {
    }

    // Public properties corresponding to the XML structure, in the same order as in the XML.
    ClefIndex clefIndex{};  ///< The 0-based clef index from the `<clef>` element.
    Edu xEduPos{};          ///< The staff edu position from the `<xEduPos>` element.
                            ///< In some versions of Finale this was a global edu value.
                            ///< However, as of F27 it is a staff edu value.
    Evpu yEvpuPos{};        ///< The yEvpuPos value from the `<yEvpuPos>` element.
    int percent{};          ///< The percentage value from the `<percent>` element.
    int xEvpuOffset{};      ///< The xEvpuOffset value from the `<xEvpuOffset>` element.
    ShowClefMode clefMode{}; ///< The clef mode from the `<clefMode>` element.
    bool unlockVert{};      ///< "Allow Vertical Drag"
    bool afterBarline{};    ///< "Place Clef After Barline"

    constexpr static std::string_view XmlNodeName = "clefEnum"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ClefList>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class DrumStaff
 * @brief Identifies the percussion map ("drum library") for a staff with percussion notation.
 *
 * The class is identified by the XML node name "drumStaff".
 */
class DrumStaff : public OthersBase
{
public:
    /** @brief Constructor function. */
    explicit DrumStaff(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    Cmper whichDrumLib{};       ///< Cmper of the associated percussion map. This cmper is used in various classes to pull
                                ///< properties of the percussion map.

    constexpr static std::string_view XmlNodeName = "drumStaff"; ///< The XML node name for this type.
    static const xml::XmlElementArray<DrumStaff>& xmlMappingArray();    ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class DrumStaffStyle
 * @brief Identifies the percussion map ("drum library") for a staff style with percussion notation.
 *
 * The class is identified by the XML node name "drumStaffStyle".
 */
class DrumStaffStyle : public DrumStaff
{
public:
    using DrumStaff::DrumStaff;

    constexpr static std::string_view XmlNodeName = "drumStaffStyle"; ///< The XML node name for this type.
};

/**
 * @class FontDefinition
 * @brief The name and font characteristics of fonts contained.
 *
 * The cmper is the font ID used in classes throughout the document.
 *
 * This class is identified by the XML node name "fontName".
 */
class FontDefinition : public OthersBase
{
public:
    static constexpr uint32_t SYMBOL_CHARSET_MAC = 0xfff; ///< macOS symbol charset (4095).
    static constexpr uint32_t SYMBOL_CHARSET_WIN = 2;     ///< Windows symbol charset.
    /** @brief Constructor function */
    explicit FontDefinition(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    /// @enum CharacterSetBank
    /// @brief The character set for the bank
    enum class CharacterSetBank
    {
        MacOS,          /// xml value is "Mac"
        Windows         /// xml value is "Win"
    };

    // Public properties corresponding to the XML structure
    CharacterSetBank charsetBank{}; ///< The character set bank.
    int charsetVal{};               ///< A value specifying the character set. The meaning is dependent on #charsetBank.
                                    ///< A value of 0 indicates ANSI character set for Windows and MacRoman for macOS.
                                    ///< A value of 2 (`SYMBOL_CHARSET`) indicates a symbol font for Windows.
                                    ///< A value of 4095 (0xfff) indicates a symbol font for macOS.
                                    ///< Other values are possible but less likely.
    int pitch{};                    ///< Represents the `<pitch>` element, e.g., 0. (use unknown)
    int family{};                   ///< Represents the `<family>` element, e.g., 0. (use unknown)
    std::string name;               ///< The font name e.g., "Broadway Copyist Text".

    /**
     * @brief Calculates if this font is a symbol font.
     *
     * The code points in symbol fonts
     * are mapped directly to the glyph index of the font without any attempt at
     * linguistic mapping.
     *
     * Many legacy symbol fonts in Finale were music fonts following more-or-less the layout
     * of Adobe Sonata font, so this may be an appropriate proxy for telling if the font is a legacy
     * music font, at least in some contexts. The same font face (e.g., Engraver) may appear as
     * either a Windows symbol font or a macOS symbol font, depending on the origin of the font
     * and of the document.
    */
    bool calcIsSymbolFont() const
    {
        if (charsetBank == CharacterSetBank::MacOS && charsetVal == SYMBOL_CHARSET_MAC) {
            return true;
        }
        if (charsetBank == CharacterSetBank::Windows && charsetVal == SYMBOL_CHARSET_WIN) {
            return true;
        }
        return false;
    }

    constexpr static std::string_view XmlNodeName = "fontName"; ///< The XML node name for this type.
    static const xml::XmlElementArray<FontDefinition>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class Frame
 * @brief Represents the attributes of a TGF entry frame.
 *
 * The inci is almost always zero. Multiple incis are used to implement legacy pickup measures ("spacers").
 * In this case, the observed order of Frame incis is
 * - `inci 0`: startTime (which implements the spacer)
 * - `inci 1`: startEntry & endEntry
 *
 * The #details::GFrameHoldContext::createEntryFrame function implements spacers by starting the first entry in the frame after the spacer.
 *
 * The class is identified by the XML node name "frameSpec".
 */
class Frame : public OthersBase
{
public:
    /** @brief Constructor function. */
    explicit Frame(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci = 0)
        : OthersBase(document, partId, shareMode, cmper, inci) {}

    EntryNumber startEntry{}; ///< Start entry number for this frame. (Appears to be zero when #startTime is supplied.)
    EntryNumber endEntry{};   ///< End entry number for this frame. (Appears to be zero when #startTime is supplied.)
    Edu startTime{};          ///< The starting position within the measure. (Appears to be zero when #startEntry and #endEntry are supplied.)

    /// @brief Iterates all the raw entries in a frame, passing them to the iterator function. If the iterator function returns false, iteration stops.
    /// @param iterator function.
    /// @return true if all entries iterated, false if the iterator function exited early by returning false.
    bool iterateRawEntries(std::function<bool(const MusxInstance<Entry>& entry)> iterator) const;

    /** @brief Returns a vector of entries contained in the frame.
     *
     * These are raw entries. Use #details::GFrameHoldContext::createEntryFrame for a vector of entries with computed values.
     */
    MusxInstanceList<Entry> getEntries() const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (startTime && (startEntry || endEntry)) {
            MUSX_INTEGRITY_ERROR("Frame " + std::to_string(getCmper()) + " has non-zero startTime and non-zero startEntry or endEntry.");
        }
        if (!startTime && !startEntry) {
            MUSX_INTEGRITY_ERROR("Frame " + std::to_string(getCmper()) + " has no start entry.");
        }
        if (!startTime && !endEntry) {
            MUSX_INTEGRITY_ERROR("Frame " + std::to_string(getCmper()) + " has no end entry.");
        }
    }

    constexpr static std::string_view XmlNodeName = "frameSpec"; ///< The XML node name for this type.
    static const xml::XmlElementArray<Frame>& xmlMappingArray();    ///< Required for musx::factory::FieldPopulator.
};

class FretInstrument;
/**
 * @class FretboardGroup
 * @brief A named group of fretboard diagrams associated with a specific fretboard instrument.
 *
 * The cmper is the same cmper as is used for @ref ChordSuffixElement. (See #details::ChordAssign::suffixId.)
 * There are two special hard-coded cmpers that are used when a chord has no suffix. Value 65533 is used
 * for minor (lowercase) chords and value 65534 is for major (uppercase) chords.
 *
 * The inci comes from #details::ChordAssign::fretboardGroupInci.
 *
 * This class is identified by the XML node name "fretGroup".
 */
class FretboardGroup : public OthersBase
{
public:
    /** @brief Constructor */
    explicit FretboardGroup(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci)
    {
    }

    Cmper fretInstId{};   ///< Fret instrument ID. (xml node `<fretInstID>`)
    std::string name;     ///< Group name.

    /// @brief Get the @ref FretInstrument associated with this fretboard group.
    MusxInstance<FretInstrument> getFretInstrument() const;

    /// @brief Gets the array of @ref details::FretboardDiagram instances associated with this fretboard group.
    MusxInstanceList<details::FretboardDiagram> getFretboardDiagrams() const;

    constexpr static std::string_view XmlNodeName = "fretGroup"; ///< The XML node name for this type.
    static const xml::XmlElementArray<FretboardGroup>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class FretboardStyle
 * @brief Fretboard diagram style: shapes, spacing, fonts, and offsets.
 *
 * The cmper is a unique identifier used in the document. For example, see #details::ChordAssign::fbStyleId.
 *
 * This class is identified by the XML node name "fretStyle".
 */
class FretboardStyle : public OthersBase
{
public:
    /** @brief Constructor */
    explicit FretboardStyle(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper)
    {
    }

    bool showLastFret{};     ///< Show the last fret marker.
    bool rotate{};           ///< Rotate the diagram so the nut is on the left.
    bool fingNumWhite{};     ///< Render fingering numbers in white.
    Cmper fingStrShapeId{};  ///< Fingering string shape ID. (xml node `<fingStrShapeID>`)
    Cmper openStrShapeId{};  ///< Open string shape ID. (xml node `<openStrShapeID>`)
    Cmper muteStrShapeId{};  ///< Muted string shape ID. (xml node `<muteStrShapeID>`)
    Cmper barreShapeId{};    ///< Barre shape ID. (xml node `<barreShapeID>`)
    Cmper customShapeId{};   ///< Custom shape ID. (xml node `<customShapeID>`)
    int defNumFrets{};       ///< Default number of frets.
    Efix stringGap{};        ///< Gap between strings.
    Efix fretGap{};          ///< Gap between frets.
    Efix stringWidth{};      ///< Line width of strings.
    Efix fretWidth{};        ///< Line width of frets.
    Efix nutWidth{};         ///< Width of the nut.
    Efix vertTextOff{};      ///< Vertical text offset.
    Efix horzTextOff{};      ///< Horizontal text offset.
    Efix horzHandleOff{};    ///< Horizontal handle offset.
    Efix vertHandleOff{};    ///< Vertical handle offset.
    Efix whiteout{};         ///< Whiteout thickness/extent. (xml node `<whiteout>`)
    std::shared_ptr<FontInfo> fretNumFont; ///< Font for fret numbers.
    std::shared_ptr<FontInfo> fingNumFont; ///< Font for fingering numbers.
    Efix horzFingNumOff{};   ///< Horizontal fingering number offset.
    Efix vertFingNumOff{};   ///< Vertical fingering number offset.
    std::string name;        ///< Style name. (xml node `<name>`)
    std::string fretNumText; ///< Label preceding fret number (e.g., "fr."). (xml node `<fretNumText>`)

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (!fretNumFont) {
            fretNumFont = std::make_shared<FontInfo>(getDocument());
        }
        if (!fingNumFont) {
            fingNumFont = std::make_shared<FontInfo>(getDocument());
        }
    }

    constexpr static std::string_view XmlNodeName = "fretStyle"; ///< The XML node name for this type.
    static const xml::XmlElementArray<FretboardStyle>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class FretInstrument
 * @brief Describes a fretted instrument (strings, frets, name, clef). It is used for both TAB notation and @ref FretboardGroup instances.
 *
 * The cmper is an arbitrary unique number referenced by @ref Staff, @ref FretboardGroup, and @ref options::ChordOptions.
 *
 * This class is identified by the XML node name "fretInst".
 */
class FretInstrument : public OthersBase
{
public:
    /** @brief Constructor */
    explicit FretInstrument(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper)
    {
    }

    /**
     * @class StringInfo
     * @brief Information for a single string of the fretted instrument.
     */
    class StringInfo
    {
    public:
        int  pitch{};       ///< Open-string MIDI pitch (60 is middle-C.)
        int  nutOffset{};   ///< Optional nut offset in frets (half-steps). The Finale U.I. does not appear to have
                            ///< a way to modify it, so normally it will be zero. A Plugin could have modified it, however.

        static const xml::XmlElementArray<StringInfo>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    int numFrets{};                 ///< Number of frets
    int numStrings{};               ///< Number of strings (max is 24)
    std::string name;               ///< Display name
    std::vector<std::shared_ptr<StringInfo>> strings;  ///< One entry per string: strings.size() should equal #numStrings.
    std::vector<int> fretSteps;     ///< Sequence of fret intervals (in half-steps) measured from the open string.
                                    ///< If empty, all frets are treated as consecutive half-steps (chromatic).
                                    ///< If shorter than #numFrets, any unspecified frets default to one half-step
                                    ///< above the previous value. (xml node is `<diatonic>`)
    ClefIndex speedyClef{};         ///< The clef to use when entering notes for this instrument in Speedy Entry.

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (numStrings != int(strings.size())) {
            MUSX_INTEGRITY_ERROR("Fret instrument " + std::to_string(getCmper()) + " specifies " + std::to_string(numStrings)
                + " strings but has " + std::to_string(strings.size()) + " StringInfo instances.");
        }
        if (!fretSteps.empty() && numFrets != int(fretSteps.size())) {
            util::Logger::log(util::Logger::LogLevel::Info, "Fret instrument " + std::to_string(getCmper()) + " specifies " + std::to_string(numFrets)
                + " frets but has " + std::to_string(fretSteps.size()) + " diatonic fret steps specified.");
        }
    }

    constexpr static std::string_view XmlNodeName = "fretInst"; ///< The XML node name for this type.
    static const xml::XmlElementArray<FretInstrument>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class KeyFormat
 * @brief The key format for a custom key signature.
 *
 * This class is identified by the XML node name "keyFormat".
 */
class KeyFormat : public OthersBase {
public:
    /** @brief Constructor function */
    explicit KeyFormat(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    unsigned semitones{};       ///< Number of semitones in the octave (e.g. 12 for standard keys, 24 for 24-EDO, 31 for 31-EDO, etc.)
    unsigned scaleTones{};      ///< Number of diatonic steps in the scale (almost always 7).

    constexpr static std::string_view XmlNodeName = "keyFormat"; ///< The XML node name for this type.
    static const xml::XmlElementArray<KeyFormat>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class KeyMapArray
 * @brief An array of step elements from which one can create a key map.
 *
 * This class is identified by the XML node name "keyMap".
 */
class KeyMapArray : public OthersBase {
public:
    /** @brief Constructor function */
    explicit KeyMapArray(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    /** @class StepElement
     *  @brief Represents a single `<keych>` element inside the `<keyMap>`.
     */
    class StepElement {
    public:
        bool diatonic{};    ///< Whether the step is diatonic (true if `<diatonic>` is present).
        unsigned hlevel{};  ///< Harmonic level (scale degree) of this step. (xml node is `<hlevel>`)

        static const xml::XmlElementArray<StepElement>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    std::vector<std::shared_ptr<StepElement>> steps; ///< Collection of keych step elements.

    /// @brief Counts the number of diatonic steps in the #steps array
    unsigned countDiatonicSteps() const
    {
        return static_cast<unsigned>(std::count_if(steps.begin(), steps.end(), [](const auto& step) {
            return step->diatonic;
        }));
    }

    constexpr static std::string_view XmlNodeName = "keyMap"; ///< The XML node name for this type.
    static const xml::XmlElementArray<KeyMapArray>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class KeyAttributes
 * @brief Represents the attributes associated with a Finale key signature.
 *
 * This class is identified by the XML node name "keysAttrib".
 * The cmper is the key signature ID.
 */
class KeyAttributes : public OthersBase {
public:
    /** @brief Constructor function */
    explicit KeyAttributes(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // Public properties corresponding to the XML structure
    int harmRefer{};        ///< Changes the meaning of 0 in the note tables for key signatures. The Finale manual recommends
                            ///< never changing it from 0. It also appears not to work as documented if changed. It was part of the original
                            ///< Finale 1.0 release and was likely never tested much since then.
    int middleCKey{};       ///< MIDI key number to use for middle C. This value transposes playback if it is set other than the MIDI middle C value of 60.
    Cmper fontSym{};        ///< If non-zero, the Font symbol ID used for accidentals in this key. Only the font ID can be specified. The size and style are inherited from
                            ///< the settings in #options::FontOptions::FontType::Key.
    int gotoKey{};          ///< According to the Finale manual, this specifies the number of "scale steps" between each key on the MIDI keyboard.
                            ///< Presumably this means EDO division steps, but in any case it is not well implemented and seems not to work as intended.
                            ///< A value other than 1 is unlikely.
    Cmper symbolList{};     ///< Cmper of the symbol list that specifies the accidentals for the key. These are stored in incidents of @ref details::KeySymbolListElement.
    bool hasClefOctv{};     ///< If true, the key signature has clef octave override tables. See @ref details::ClefOctaveFlats and @ref details::ClefOctaveSharps.

    constexpr static std::string_view XmlNodeName = "keysAttrib"; ///< The XML node name for this type.
    static const xml::XmlElementArray<KeyAttributes>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class LayerAttributes
 * @brief Represents the attributes of a Finale "layer".
 *
 * The cmper is the layer ID (0-3). In the Finale UI these are represented as Layer 1 thru Layer 4.
 * The 0-based cmper value is used to identify layers where needed, rather than the 1-based UI value.
 *
 * This class is identified by the XML node name "layerAtts".
 */
class LayerAttributes : public OthersBase {
public:
    /** @brief Constructor function */
    explicit LayerAttributes(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // Public properties corresponding to the XML structure
    int restOffset{};                   ///< "Adjust Floating Rests by " number of staff steps.
    bool freezTiesToStems{};            ///< "Freeze Ties in the Same Direction as Stems" (xml node is `<flipTies>`)
    bool onlyIfOtherLayersHaveNotes{};  ///< "Apply Settings Only if Notes are in Other Layers" (xml node is `<floatLayer>`)
    bool useRestOffset{};               ///< "Adjust Floating Rests by ". Determines if `restOffset` is used. (xml node is `<useRestOff>`)
    bool freezeStemsUp{};               ///< If true, freeze stems up, otherwise freeze stems down. Only used if `freezeLayer` is true. (xml node is `<freezeLayUp>`)
    bool freezeLayer{};                 ///< "Freeze Stems and Ties"
    bool playback{};                    ///< If true, playback this layer. If false, the layer is muted.
    bool affectSpacing{};               ///< "Affect Music Spacing" (xml node is `<spacing>`)
    bool ignoreHiddenNotesOnly{};       ///< "Ignore Layers Containing Only Hidden Notes" (xml node is `<ignoreHidden>`)
    bool ignoreHiddenLayers{};          ///< "Ignore Hidden Layers"
    bool hideLayer{};                   ///< "Hide Layer when Inactive"

    constexpr static std::string_view XmlNodeName = "layerAtts"; ///< The XML node name for this type.
    static const xml::XmlElementArray<LayerAttributes>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @enum RehearsalMarkStyle
 * @brief Specifies the sequencing style for rehearsal marks
 */
enum class RehearsalMarkStyle
{
    None,                       ///< Default value, no rehearsal mark style
    Letters,                    ///< A, B, C...AA, BB, CC
    LetterNumbers,              ///< A, B, C...A1, B1, C1
    LettersLowerCase,           ///< a, b, c...aa, bb, cc
    LettersNumbersLowerCase,    ///< a, b, c...a1, b1, c1
    Numbers,                    ///< 1, 2, 3, 4
    MeasureNumber               ///< Displays the current measure number
};

/**
 * @enum PlaybackType
 * @brief Specifies the playback behavior for the an expression or marking category.
 */
enum class PlaybackType
{
    None,                   ///< Default value, no playback (the default).
    Tempo,                  ///< Playback affects tempo.  (xml value is "time")
    MidiController,         ///< Playback type for MIDI controller commands.
    KeyVelocity,            ///< Playback affects key velocity. (xml value is "amplitude")
    Transpose,              ///< Playback causes transposition.
    Channel,                ///< Playback set the MIDI channel.
    MidiPatchChange,        ///< Playback changes the MIDI patch.
    PercussionMidiMap,      ///< Playback uses percussion MIDI map. (xml value is "percMidiMap")
    MidiPitchWheel,         ///< Playback affects the MIDI pitch wheel. (xml value is "midiPitchwheel")
    ChannelPressure,        ///< Playback affects MIDI channel pressure. (xml value is "midiPressure")
    RestrikeKeys,           ///< Playback retrikes keys. (xml value is "rekey")
    Dump,                   ///< Playback is an arbitrary data dump. (Data is in node `<playDumpText>` with the same @ref Cmper value.)
    PlayTempoToolChanges,   ///< Play changes from Tempo Tool. (xml value is "startTempo")
    IgnoreTempoToolChanges, ///< Ignore changes from Tempo Tool. (xml value is "stopTempo")
    Swing,                  ///< Playback in swing style
    SmartPlaybackOn,        ///< Turn on smart playback. (xml value is "hpOn")
    SmartPlaybackOff,       ///< Turn off smart playback. (xml value is "hpOff")
};

/**
 * @enum HorizontalMeasExprAlign
 * @brief Specifies the horizontal alignment relative to musical elements for an expression or marking category.
 */
enum class HorizontalMeasExprAlign
{
    LeftBarline,            ///< Align with left barline (the default). (xml value is "leftEdge", if encountered)
    Manual,                 ///< "Horizontal Click Position" alignment.
    LeftOfAllNoteheads,     ///< Align left of all noteheads
    LeftOfPrimaryNotehead,  ///< Align to the left of the primary notehead.
    Stem,                   ///< Align to the stem.
    CenterPrimaryNotehead,  ///< Align to the center of the primary notehead.
    CenterAllNoteheads,     ///< Align to the center of all noteheads.
    RightOfAllNoteheads,    ///< Align to the right of all noteheads.
    StartTimeSig,           ///< Align at the start of the time signature.
    AfterClefKeyTime,       ///< Align after clef/key/time/repeat.
    StartOfMusic,           ///< Align at start of music.
    CenterOverBarlines,     ///< Align over the center of the barlines.
    CenterOverMusic,        ///< Align over the center of music.
    RightBarline            ///< Align with left barline. (xml value is "rightEdge")
};

/**
 * @enum VerticalMeasExprAlign
 * @brief Specifies the vertical alignment relative to musical elements for an expression or marking category.
 */
enum class VerticalMeasExprAlign
{
    AboveStaff,          ///< Align above ths staff (the default).
    Manual,              ///< "Vertical Click Position" alignment.
    RefLine,             ///< Align to staff reference line.
    BelowStaff,          ///< Align below the staff.
    TopNote,             ///< Align with top note.
    BottomNote,          ///< Align with bottom note.
    AboveEntry,          ///< Align above entry.
    BelowEntry,          ///< Align below entry.
    AboveStaffOrEntry,   ///< Align above the staff or entry.
    BelowStaffOrEntry    ///< Align below the staff or entry.
};

class ShapeExpressionDef;
class TextExpressionDef;

/**
 * @class MarkingCategory
 * @brief Represents a category of markings used.
 *
 * This class is identified by the XML node name "markingsCategory".
 */
class MarkingCategory : public OthersBase {
public:
    /**
     * @enum CategoryType
     * @brief Enumeration for the type of marking category
     */
    enum class CategoryType : Cmper
    {
        Invalid,            ///< There should always be a category type supplied
        Dynamics,           ///< Dynamics markings, such as forte, piano, etc.
        TempoMarks,         ///< Tempo indications such as Allegro (with or without metronome marking).
        TempoAlterations,   ///< Tempo alteration markings, such as accel. and rit. (xml value is "tempoAlts")
        ExpressiveText,     ///< Expressive text such as "espressivo."
        TechniqueText,      ///< Technique text such as "pizzicato" or "arco."
        RehearsalMarks,     ///< Rehearsal marks, often used for reference points in the score.
        Misc                ///< Represents miscellaneous markings that do not fit into other categories.
    };

    /** @brief Constructor function */
    explicit MarkingCategory(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    CategoryType categoryType{}; ///< Category type of the marking

    // Font information for the marking category
    std::shared_ptr<FontInfo> textFont;      ///< Text font
    std::shared_ptr<FontInfo> musicFont;     ///< Music font
    std::shared_ptr<FontInfo> numberFont;    ///< Number font (not implemented in the Finale U.I.)

    HorizontalMeasExprAlign horzAlign{};     ///< Horizontal alignment for the marking
    VerticalMeasExprAlign vertAlign{};       ///< Vertical alignment for the marking
    AlignJustify justification{};            ///< Justification for the text within the marking

    // Vertical and horizontal offsets for positioning adjustments
    Evpu horzOffset{};         ///< Additional horizontal offset
    Evpu vertOffsetBaseline{}; ///< Additional vertical offset
    Evpu vertOffsetEntry{};    ///< Additional vertical entry offset

    // Usage flags representing certain behaviors and visual elements
    bool usesTextFont{};      ///< whether this category uses the text font
    bool usesMusicFont{};     ///< whether this category uses the music font
    bool usesNumberFont{};    ///< whether this category uses the number font
    bool usesPositioning{};   ///< whether this category uses the positioning elements (Finale UI only allows true)
    bool usesStaffList{};     ///< whether this category uses a staff list
    bool usesBreakMmRests{};  ///< whether this category uses the `breakMmRest` setting
    bool breakMmRest{};       ///< whether expressions in this category break multimeasure rests
    bool userCreated{};       ///< whether the user created this category

    // Staff list represented as an integer
    Cmper staffList{};        ///< The staff list if `useStaffList` is true

    /** @brief A list of shape expressions in this category.
     *
     * (This in not in the xml but is created by the factory.)
     */
    std::map<Cmper, MusxInstanceWeak<ShapeExpressionDef>> shapeExpressions;

    /** @brief A list of text expressions in this category.
     *
     * (This in not in the xml but is created by the factory.)
     */
    std::map<Cmper, MusxInstanceWeak<TextExpressionDef>> textExpressions;

    /** @brief gets the name of the marking category */
    std::string getName() const;

    /// @brief Create a @ref StaffListSet for the given instance. This can be used to interrogate whether a staff appears in the staff set.
    /// @return The created staff list set. If #staffList is zero, it will never find any staves for the staff list.
    CategoryStaffListSet createStaffListSet() const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (!textFont) {
            textFont = std::make_shared<FontInfo>(ptrToThis->getDocument());
            MUSX_INTEGRITY_ERROR("Marking category " + std::to_string(getCmper()) + " is missing text font.");
        }
        if (!musicFont) {
            musicFont = std::make_shared<FontInfo>(ptrToThis->getDocument());
            MUSX_INTEGRITY_ERROR("Marking category " + std::to_string(getCmper()) + " is missing music font.");
        }
        if (!numberFont) {
            numberFont = std::make_shared<FontInfo>(ptrToThis->getDocument());
            util::Logger::log(util::Logger::LogLevel::Info, "Marking category " + std::to_string(getCmper()) + " is missing number font.");
        }
    }

    constexpr static std::string_view XmlNodeName = "markingsCategory"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MarkingCategory>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MarkingCategoryName
 * @brief Represents the name associated with a @ref MarkingCategory.
 *
 * This class has the same #Cmper as its @ref MarkingCategory.
 *
 * This class is identified by the XML node name "markingsCategoryName".
 */
class MarkingCategoryName : public OthersName {
public:
    using OthersName::OthersName;

    constexpr static std::string_view XmlNodeName = "markingsCategoryName"; ///< The XML node name for this type.
};

/// @brief Copies one marking category and its optional name from one document into another.
/// @param target The document to import into.
/// @param source The document whose category and optional name are copied.
/// @param cmper The marking-category cmper to copy.
/// @param onImported Called for every object this operation creates, including font definitions.
/// @return @p cmper, or std::nullopt if the source has no category at @p cmper or the target
/// already has either category component there. A refusal imports nothing.
/// @throws std::invalid_argument if @p target or @p source is null. Debug builds assert instead.
[[nodiscard]]
std::optional<Cmper> importMarkingCategoryInto(const DocumentPtr& target,
    const DocumentPtr& source, Cmper cmper, const ImportObjectCallback& onImported = {});

class MeasureNumberRegion;
/**
 * @class Measure
 * @brief Represents the attributes of a measure.
 *
 * This class is identified by the XML node name "measSpec".
 */
class Measure : public OthersBase {
public:
    /** @brief Constructor function */
    explicit Measure(const DocumentWeakPtr& document, Cmper measureId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, measureId, shareMode, cmper) {}

    /// @enum PositioningType
    /// @brief Positioning (music spacing) type for the measure
    enum class PositioningType
    {
        Manual,                     ///< "Manually (By Dragging)" the default.
        TimeSignature,              ///< Legacy option "Use the Time Signature" (xml node is `<timesig>`)
        BeatChart,                  ///< Legacy option "Use a Beat Positioning Chart" (xml node is `<beatchart>`)
        TimeSigPlusPositioning,     ///< "According to the Time Signature" (xml node is `<timesigPlusPos>`)
        BeatChartPlusPositioning    ///< "Using Beat-Chart Spacing" (xml node is `<beatchartPlusPos>`)
    };

    /// @enum BarlineType
    /// @brief Barline type for left and right barlines.
    enum class BarlineType
    {
        None,                   ///< No barline. (This is the default value for the enum.)
        OptionsDefault,         ///< Left barlines only: use value from @ref options::BarlineOptions (xml values is "default")
        Normal,                 ///< Normal barline
        Double,                 ///< Double barline
        Final,                  ///< Final barline
        Solid,                  ///< Solid (thick) barline
        Dashed,                 ///< Dashed barline (xml value is "dash")
        Tick,                   ///< Right barlines only: tick barline (xml value is "partial")
        Custom                  ///< Custom barline (created with Shape Designer)
    };

    /// @enum ShowKeySigMode
    /// @brief How and whether to show a Key Signature in this measure.
    enum class ShowKeySigMode
    {
        IfNeeded,               ///< Show if needed (the default)
        Always,                 ///< Always show (xml value is "deltaKey")
        Never                   ///< Never show (xml value is "ignoreKey")
    };

    /// @enum ShowTimeSigMode
    /// @brief How and whether to show a Time Signature in this measure.
    enum class ShowTimeSigMode
    {
        IfNeeded,               ///< Show if needed (the default)
        Always,                 ///< Always show (xml value is "deltaTime")
        Never                   ///< Never show (xml value is "ignoreTime")
    };

    Evpu width{};               ///< "Ideal" measure width in Evpu. Page layout determines actual width.
    std::shared_ptr<KeySignature> globalKeySig; ///< the global key signature on this measure. Guaranteed to be non-null. (xml node is `<keySig>`)
    Cmper beats{};              ///< Number of beats in the measure or the Cmper to a `timesigUpper` composite numerator list.
    Cmper divBeat{};            ///< Divisions per beat (Edu) or the Cmper to a `timesigLower` composite denominator list.
    Cmper dispBeats{};          ///< Displayed beats in the measure or the Cmper to a `timesigUpper` composite numerator list.
    Cmper dispDivbeat{};        ///< Displayed divisions per beat (Edu) or the Cmper to a `timesigLower` composite denominator list.
    Cmper customBarShape{};     ///< Cmper of Shape Designer @ref ShapeDef for custom right barline
    Cmper customLeftBarShape{}; ///< Cmper of Shape Designer @ref ShapeDef for custom left barline
    Evpu frontSpaceExtra{};     ///< Extra space at front of bar.
    Evpu backSpaceExtra{};      ///< Extra space at end of bar.
    bool breakWordExt{};        ///< Barline ends word extensions on lyrics.
    bool hideCaution{};         ///< "Hide Cautionary Clefs, Key, and Time Signature"
    bool hasSmartShape{};       ///< Indicates if the measure has a smart shape.
    bool groupBarlineOverride{}; ///< Override the barline specified by a @ref details::StaffGroup (if any)
    bool showFullNames{};       ///< "Show Full Staff & Group Names"
    bool hasMeasNumbIndivPos{}; ///< Has individual measure numbering positioning. (xml node is `<mnSepPlace>`)
    bool allowSplitPoints{};    ///< "Allow Horizontal Split Points" See @ref SplitMeasure. (xml node is `<posSplit>`)
    bool compositeNumerator{};  ///< Indicates a composite numerator for the time signature. (xml node is `<altNumTsig>`)
    bool compositeDenominator{}; ///< Indicates a composite denominator for the time signature. (xml node is `<altDenTsig>`)
    ShowKeySigMode showKey{};   ///< Show mode for key signatures
    ShowTimeSigMode showTime{}; ///< Show mode for time signatures
    bool evenlyAcrossMeasure{}; ///< "Position Evenly Across Measure" (xml node is `<indivPosDef>`)
    PositioningType positioningMode{}; ///< Positioning type for the measure. (xml node is `<posMode>`)
    bool beginNewSystem{};      ///< "Begin a New Staff System" (xml node is `<lineBreak>`)
    bool hasExpression{};       ///< Indicates if the measure has an expression assigned. See @ref MeasureExprAssign. (xml node is `<hasExpr>`)
    bool breakMmRest{};         ///< "Break a Multimeasure Rests" (xml node is `<breakRest>`)
    bool noMeasNum{};           ///< Inverse of "Include in Measure Numbering"
    bool hasOssia{};            ///< Indicates if the measure has an ossia assigned. (xml node is `<arbitMusic>`)
    bool hasTextBlock{};        ///< Indicates if the measure has a measure-assigned text block. See @ref details::MeasureTextAssign.
    BarlineType barlineType{};  ///< Barline type. (xml node is `<barline>`)
    bool forwardRepeatBar{};    ///< Indicates a forward repeat bar on this measure. (xml node is `<forRepBar>`)
    bool backwardsRepeatBar{};  ///< Indicates a backwards repeat bar on this measure. (xml node is `<bacRepBar>`)
    bool hasEnding{};           ///< Indicates the presence of a repeat ending. (xml node is `<barEnding>`)
    bool hasTextRepeat{};       ///< Indicates the presence of one or more text repeat assigments. (xml node is `<txtRepeats>`)
    bool abbrvTime{};           ///< Indicates abbreviated time signature (e.g., Common or Cut time.) Applies to the display time signature only.
                                ///< The actual time signature's abbreviation is controlled by the values in @ref options::TimeSignatureOptions.
    bool useDisplayTimesig{};   ///< Indicates whether to use the display time signature.
    bool hasChord{};            ///< Indicates the presence of one or more chords.
    BarlineType leftBarlineType{}; ///< Left barline type. (xml node is `<leftBarline>`)
    bool compositeDispNumerator{};  ///< Indicates a composite numerator for the display time signature. (xml node is `<displayAltNumTsig>`)
    bool compositeDispDenominator{}; ///< Indicates a composite denominator for the display time signature. (xml node is `<displayAltDenTsig>`)
    bool pageBreak{};           ///< If true, begin new page here. (Behavior is weird if the measure is not the first of its system.)

    /// @brief Calculates if a measure should show full names vs. abbreviated names
    bool calcShouldShowFullNames() const
    { return getCmper() == 1 || showFullNames; }

    /// @brief Finds the measure number region containing this measure.
    /// @return An appropriate MeasureNumberRegion instance that contains the @p measureId, or nullptr if none found.
    /// (Preference is given to regions with `showOnStart` set in the score or part data.)
    MusxInstance<MeasureNumberRegion> findMeasureNumberRegion() const;

    /// @brief Calculates the visible number of the measure, based on the first MeasureNumberRegion that contains it.
    ///
    /// Depending on the display options of the found MeasureNumberRegion, the number may or may not appear in the score.
    /// And if it does, it may not appear as a number.
    /// @return The display number or std::nullopt if the measure is not included in measure numbering or if it is not
    /// covered by a measure number region.
    std::optional<int> calcDisplayNumber() const;

    /// @brief Calculates the visible measure number text, based on the first MeasureNumberRegion that contains it.
    /// @return The display text or std::nullopt if the measure is not included in measure numbering or if it is not
    /// covered by a measure number region.
    std::optional<std::string> calcDisplayNumberText() const;

    /// @brief Creates and returns a shared pointer to an instance of the @ref KeySignature for this measure and staff.
    /// @param forStaff If present, specifies the specific staff for which to create the key signature.
    /// @return A shared pointer to a new instance of KeySignature. The caller may modify it (*e.g.*, for transposition) without affecting the values in the document.
    MusxInstance<KeySignature> createKeySignature(const std::optional<StaffCmper>& forStaff = std::nullopt) const;

    /// @brief Create a shared pointer to an instance of the @ref TimeSignature for this measure and staff.
    /// @param forStaff If present, specifies the specific staff for which to create the time signature.
    MusxInstance<TimeSignature> createTimeSignature(const std::optional<StaffCmper>& forStaff = std::nullopt) const;

    /// @brief Create a shared pointer to an instance of the display @ref TimeSignature for this measure and staff.
    /// @param forStaff If present, specifies the specific staff for which to create the time signature.
    /// @return The display time signature if there is one, otherwise the actual time signature.
    MusxInstance<TimeSignature> createDisplayTimeSignature(const std::optional<StaffCmper>& forStaff = std::nullopt) const;

    /// @brief Calculates the legacy pickup spacer, if any.
    ///
    /// See comments at #details::GFrameHoldContext::calcMinLegacyPickupSpacer.
    /// @param forStaffId The staff to examine.
    /// @return The smallest legacy pickup spacer encountered in a layer for this measure and staff. Zero if none.
    util::Fraction calcMinLegacyPickupSpacer(StaffCmper forStaffId) const;

    /// @brief Returns the default rest value used for empty staves in a pickup, if any.
    ///
    /// For most documents, this returns std::nullopt. However, if the document has a pickup created by the
    /// `Document->Pickup Measure` menu option, this is the value that should be used for default rests.
    ///
    /// @return the rest duration to use if this measure is a pickup measure created with the menu option; otherwise std::nullopt
    std::optional<Duration> calcDefaultPickupRestValue() const;

    /// @brief Calculates the legacy pickup spacer in any staff, in global Edu values.
    ///
    /// See comments at #details::GFrameHoldContext::calcMinLegacyPickupSpacer.
    /// @return The smallest legacy pickup spacer encountered in any staff in global Edu values. Zero if none.
    util::Fraction calcMinLegacyPickupSpacer() const;

    /// @brief Calculates the duration of the measure according to the time signature
    /// @param forStaff  If present, specifies the specific staff for which to create duration.
    /// @return If forStaff is provided, the staff-level duration (taking into account independent time signatures.)
    /// Otherwise, it returns the global duration of the measure.
    util::Fraction calcDuration(const std::optional<StaffCmper>& forStaff = std::nullopt) const;

    /// @brief Calculates the time stretch. This is the value by which independent time edus are multiplied to get global edus.
    /// @param forStaff The staff for which to calculate the time stretch.
    util::Fraction calcTimeStretch(StaffCmper forStaff) const
    {
        return calcDuration() / calcDuration(forStaff);
    }

    /// @brief Checks that score measure cmpers are sequential, starting with 1.
    static void checkMeasureCmperSequence(const DocumentPtr& document);

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (!globalKeySig) {
            globalKeySig = std::make_shared<KeySignature>(getDocument());
        }
    }

    constexpr static std::string_view XmlNodeName = "measSpec"; ///< The XML node name for this type.
    static const xml::XmlElementArray<Measure>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MeasureExprAssign
 * @brief Assigns a @ref TextExpressionDef or @ref ShapeExpressionDef to a measure
 *
 * Only one of #textExprId or #shapeExprId is non-zero.
 *
 * The Cmper for a MeasureExprAssign is the Cmper of the Measure to which it is attached.
 */
class MeasureExprAssign : public OthersBase
{
private:
    /// @brief Calculates the entry alignment type or std::nullopt if it does not align with entries.
    [[nodiscard]] std::optional<HorizontalMeasExprAlign> calcEntryAlignmentType() const;

    /// @brief Calculates the pseudo-tie shape info for this shape expression if it is known.
    [[nodiscard]] std::optional<utils::PseudoTieShapeInfo> calcPseudoTieShape() const;

public:
    /** @brief Constructor function */
    explicit MeasureExprAssign(const DocumentWeakPtr& document, Cmper ID, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, ID, shareMode, cmper, inci) {}

    /// @enum ChannelSwitchTarget
    /// @brief The values for "On Playback Affect". The target channels are specified in the Score Manager dropdown for staves.
    enum class ChannelSwitchTarget
    {
        Current,            ///< Current channel. (Default value may not appear in xml.)
        ToLayer1,           ///< Switch to layer 1 channel. (xml value is "toL1")
        ToLayer2,           ///< Switch to layer 2 channel. (xml value is "toL2")
        ToLayer3,           ///< Switch to layer 3 channel. (xml value is "toL3")
        ToLayer4,           ///< Switch to layer 4 channel. (xml value is "toL4")
        ToChord,            ///< Switch to chord channel. (xml value is "toChord")
        ToExpression,       ///< Switch to expression channel. (xml value is "toDyn")
    };

    /// @enum PlaybackStart
    /// @brief The choice where to start playback.
    enum class PlaybackStart
    {
        BeginningOfMeasure,     ///< Start playback at beginning of measure. (Default value may not appear in xml.)
        AlignmentPoint,         ///< Start playback at alignment point as determined by the associated @ref TextExpressionDef or @ref ShapeExpressionDef.
                                ///< (xml value is "attach")
        PositionInMeasure       ///< Start playback at the Edu position of the assignment. (xml value is "measPos")
    };

    /// @enum ShowStaffList
    /// @brief Where to show the assignment. (Used both for staff list and singleton assignments.)
    enum class ShowStaffList
    {
        ScoreAndPart,           ///< Score and Part(s). (Default value may not appear in xml.)
        ScoreOnly,              ///< Score Only
        PartOnly,               ///< Part(s) Only
    };

    // Public properties corresponding to the XML structure
    Cmper textExprId{};                     ///< The @ref Cmper of a text expression (xml node is `<textExprID>`)
    Cmper shapeExprId{};                    ///< The @ref Cmper of a shape expression (xml node is `<shapeExprID>`)
    Evpu horzEvpuOff{};                     ///< Horizontal Evpu offset from the default position.
    Edu eduPosition{};                      ///< Horizontal Edu position in staff edu (xml node is `<horzEduOff>`)
    Evpu vertEvpuOff{};                     ///< Vertical Evpu offset from the default position (xml node is `<vertOff>`)
    StaffCmper staffAssign{};               ///< The staff to which this expression is assigned, or -1 if it is assigned to top staff and -2 if assigned to bottom staff.
    int layer{};                            ///< The 1-based layer number (1..4) to which this expression is assigned. (0 = all layers.)
    bool voice2{};                          ///< If true, positioning is relative to a voice2 entry. This setting is a legacy setting that Finale sets when it upgrades
                                            ///< a pre-Finale 2009 file. It is not available in the UI. However, an expression does not recognize entry positioning for
                                            ///< a voice2 entry unless this value is true. And if it is true, entry-positioning ignores voice 1 entries. (xml node is `<v2>`)
    ChannelSwitchTarget channelSwitch{};    ///< "On Playback Affect" value.
    bool dontScaleWithEntry{};              ///< Inverse of "Scale Expression with Attached Note".
    PlaybackStart playbackStart{};          ///< Where to start playback.
    ShowStaffList showStaffList{};          ///< "Show On Score and Part(s)|Score Only|Part(s) Only".
    bool createdByHp{};                     ///< This assignment was created by Finale's smart playback engine.
    bool hidden{};                          ///< True if the expression is hidden.
    int staffGroup{};                       ///< Assignment is part of a group of assignments associated with a staff list and should be modified as a group.
    Cmper staffList{};                      ///< The cmper of the marking category staff list that is controlling this assignment (if any).
                                            ///< There will be a separate #MeasureExprAssign instance for every staff in the staff list.
    unsigned graceNoteIndex{};              ///< 1-based index from leftmost grace note. 0 = main note.
    int rehearsalMarkOffset{};              ///< Restarts the rehearsal mark sequence at this 1-based sequence value. If this is zero, the sequence continues normally.

    /// @brief Calculate if this expression and the source expression point to the same definition
    [[nodiscard]] bool calcIsSameDefinition(const MeasureExprAssign& src) const;

    /// @brief Returns true if this shape expression is likely acting as a pseudo tie for the specified mode.
    [[nodiscard]] bool calcIsPseudoTie(utils::PseudoTieMode mode, const EntryInfoPtr& forStartEntry) const;

    /// @brief Gets the assigned text expression.
    /// @return The text expression or nullptr if this assignment is for a shape expression or #textExprId not found.
    MusxInstance<TextExpressionDef> getTextExpression() const;

    /// @brief Gets the assigned shape expression.
    /// @return The shape expression or nullptr if this assignment is for a text expression or #shapeExprId not found.
    MusxInstance<ShapeExpressionDef> getShapeExpression() const;

    /// @brief Gets the marking category for the assigned text or shape expression.
    /// @return The marking category or nullptr if the definition or its category cannot be resolved.
    MusxInstance<MarkingCategory> getMarkingCategory() const;

    /// @brief Create a @ref StaffListSet for the given instance. This can be used to interrogate whether a staff appears in the staff set.
    /// @return The created staff list set. If #staffList is zero, the function falls back to the expression category's
    /// staff list when that category uses staff lists. Otherwise it will never find any staves.
    CategoryStaffListSet createStaffListSet() const;

    /// @brief Calculates if this assignment participates in a staff-list-based assignment group.
    /// @details This checks the assignment-level grouping fields first, then falls back to the assigned expression
    /// category when the category uses staff lists. This works for both text and shape expressions.
    bool calcIsPartOfStaffListAssignment() const;

    /// @brief Calculates the effective staffId for the assignment, returning top or bottom staff if appropriate
    /// @param forPageView Return the top/bottom staff in page view (when appropriate). If the staff does not appear in page view, the function falls back to scroll view.
    /// @return Either the actual staff or the top or bottom staff in page view or scroll view in the current requested part.
    StaffCmper calcAssignedStaffId(bool forPageView = true) const;

    /// @brief Creates the current staff for this assignment
    /// @param forPageView If this value is true, calculates the current staff in page view, otherwise is scroll view.
    MusxInstance<StaffComposite> createCurrentStaff(bool forPageView = true) const;

    /// @brief Calculates the entry associated with the the measure expression. This is the entry that Finale uses for auto-positioning.
    /// The entry must match the elapsed position, #voice2 setting, and the layer (when specified). No fuzzy search is performed.
    /// @return The entry if the assignment's target entry exists; otherwise nullptr.
    EntryInfoPtr calcAssociatedEntry() const;

    /// @brief Calculates if this assignment is assigned in the current part or score, based on the requested part ID of the current instance and #showStaffList.
    bool calcIsAssignedInRequestedPart() const;

    /// @brief Calculates if this assignment applies to the specified layer.
    /// @param layerIndex The 0-based LayerIndex to check (0..3).
    bool calcAppliesToLayer(LayerIndex layerIndex) const;

    /// @brief Calculates if this assignment is hidden by alternate notation.
    bool calcIsHiddenByAlternateNotation() const;

    /// @brief Calculates the baseline position for this expression assignment in EVPU.
    /// @details The staff used for the baseline lookup should be the staff at the beginning of the system.
    /// @param forAbove If true, returns the "above staff" baseline; otherwise returns the "below staff" baseline.
    /// @return The baseline EVPU position, or std::nullopt if the system or staff cannot be resolved.
    [[nodiscard]] std::optional<Evpu> calcBaselinePosition(bool forAbove) const;

    /// @brief Calculates the default vertical position for the assigned expression,
    /// relative to the staff reference line, before any additional vertical offset from either assignment
    /// or expression definition. This includes the expression definition's vertical offsets.
    /// @return The default EVPU position, or std::nullopt if the required staff/entry
    /// context cannot be resolved.
    [[nodiscard]] std::optional<Evpu> calcDefaultVerticalPosition() const;

    /// @brief Calculates a vertical placement classification for this assignment.
    /// @details Uses the assigned expression definition's vertical alignment and collapses the result into a placement
    /// category.
    [[nodiscard]] VerticalPlacement calcVerticalPlacement() const;

    /**
     * @brief Gets the raw text context for parsing this expression assignment, or nullptr if none.
     * @note The main difference between the assignment and the underlying text definition context
     * is that the assignment can resolve rehearsal number text.
     * @param forPartId The linked part to use for ^partname and ^totpages inserts.
    */
    util::EnigmaParsingContext getRawTextCtx(Cmper forPartId) const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (!textExprId && !shapeExprId) {
            MUSX_INTEGRITY_ERROR("Expression assignment at measure " + std::to_string(getCmper()) + " inci " + std::to_string(getInci().value_or(-1))
                + " has no expression definition ID.");
        } else if (textExprId && shapeExprId) {
            MUSX_INTEGRITY_ERROR("Expression assignment at measure " + std::to_string(getCmper()) + " inci " + std::to_string(getInci().value_or(-1))
                + " has both text expr ID " + std::to_string(textExprId) + " and shape expr ID " + std::to_string(shapeExprId));
        }
    }

    constexpr static std::string_view XmlNodeName = "measExprAssign"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MeasureExprAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MeasureNumberRegion
 * @brief Represents the Measure Number Region with detailed font and enclosure settings for score and part data.
 *
 * This class is identified by the XML node name "measNumbRegion".
 */
class MeasureNumberRegion : public OthersBase {
public:
    /** @brief Constructor function */
    explicit MeasureNumberRegion(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    /// @enum TimePrecision
    /// @brief Precision for time display
    enum class TimePrecision
    {
        WholeSeconds,   ///< the default value
        Tenths,
        Hundredths,
        Thousandths,
    };

    /// @brief Measure number data that can differ in score or part.
    class ScorePartData : public ContainedClassBase
    {
    public:
        using ContainedClassBase::ContainedClassBase;

        std::shared_ptr<FontInfo> startFont;          ///< The font used for numbers at start of system.
        std::shared_ptr<FontInfo> multipleFont;       ///< The font used for mid-system numbers.
        std::shared_ptr<FontInfo> mmRestFont;         ///< The font used for multi-measure rest ranges.
        std::shared_ptr<Enclosure> startEnclosure;    ///< Enclosure settings for numbers at start of system.
        std::shared_ptr<Enclosure> multipleEnclosure; ///< Enclosure settings for mid-system numbers.

        Evpu startXdisp{};         ///< Horizontal offset for numbers at start of system.
        Evpu startYdisp{};         ///< Vertical offset for numbers at start of system.
        Evpu multipleXdisp{};      ///< Horizontal offset for mid-system numbers.
        Evpu multipleYdisp{};      ///< Vertical offset for mid-system numbers.
        Evpu mmRestXdisp{};        ///< Horizontal offset for multi-measure rest ranges.
        Evpu mmRestYdisp{};        ///< Vertical offset for multi-measure rest ranges.
        char32_t leftMmBracketChar{};  ///< UTF-32 code for the left bracket of multi-measure rest ranges.
        char32_t rightMmBracketChar{}; ///< UTF-32 code for the right bracket of multi-measure rest ranges.
        int startWith{};           ///< "Beginning with" value. (This value is 0-based. The Finale UI adds 1 for user display.)
        int incidence{};           ///< "Show on Every" value.
        AlignJustify startAlign{};  ///< Alignment of numbers at the start of system
        AlignJustify multipleAlign{}; ///< Alignment for mid-system numbers.
        AlignJustify mmRestAlign{}; ///< Alignment for multi-measure ranges.
        bool showOnStart{};        ///< "Show On Start of Staff System" (xml node is `<startOfLine>`)
        bool showOnEvery{};        ///< "Show on Every" activates mid-system numbers. (xml node is `<multipleOf>`)
        bool hideFirstMeasure{};   ///< "Hide First Measure Number in Region." (xml node is `<exceptFirstMeas>`)
        bool showMmRange{};        ///< "Show Measure Ranges on Multimeasure Rests" (xml node is `<mmRestRange>`)
        bool showOnMmRest{};       ///< "Show on Multimeasure Rests"  (xml node is `<mmRestRangeForce>`)
        bool useStartEncl{};       ///< Use enclosure for start-of-system settings.
        bool useMultipleEncl{};    ///< Use enclosure for mid-system settings.
        bool showOnTop{};          ///< Show measure numbers on the top staff.
        bool showOnBottom{};       ///< Show measure numbers on the bottom staff.
        bool excludeOthers{};      ///< Exclude other staves.
        bool breakMmRest{};        ///< Mid-system numbers break multimeasure rests.
        AlignJustify startJustify{}; ///< Justification for numbers at the start of system.
        AlignJustify multipleJustify{}; ///< Justification for mid-system numbers.
        AlignJustify mmRestJustify{}; ///< Justification for multi-measure rest ranges.

        void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
        {
            this->ContainedClassBase::integrityCheck(ptrToThis);
            if (!startFont) {
                startFont = std::make_shared<FontInfo>(ptrToThis->getDocument());
                //MUSX_INTEGRITY_ERROR("Measure number region score part data is missing start font.");
            }
            if (!multipleFont) {
                multipleFont = std::make_shared<FontInfo>(ptrToThis->getDocument());
                MUSX_INTEGRITY_ERROR("Measure number region score part data is missing multiple numbers font.");
            }
            if (!mmRestFont) {
                mmRestFont = std::make_shared<FontInfo>(ptrToThis->getDocument());
                MUSX_INTEGRITY_ERROR("Measure number region score part data is missing multimeasure rest range font.");
            }
        }

        static const xml::XmlElementArray<ScorePartData>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    // No fixture has a part-specific MeasureNumberRegion. If partial sharing is observed,
    // scoreData and partData must be copied and rebound before their nested fields are overlaid.
    // See PartContextRebinder<details::CenterShape> and the CenterShape field mappings for the pattern.

    // Public properties
    std::shared_ptr<ScorePartData> scoreData; ///< Score-wide measure number data.
    std::shared_ptr<ScorePartData> partData;  ///< Part-specific measure number data.

    MeasCmper startMeas{};      ///< Starting measure number for the region.
    MeasCmper endMeas{};        ///< Ending measure number for the region (non-inclusive).
    char32_t startChar{};       ///< UTF-32 code for the first character in the sequence. (Frequently '0', 'a', or 'A')
    int base{};                 ///< The base used for measure number calculations. (Frequently 10 for numeric or 26 for alpha)
    int numberOffset{};         ///< This value is 1 less than the "Starting Number" field in the Finale UI. (xml node is `<offset>`)
    std::string prefix;         ///< Text prefix for measure numbers (encoded UTF-8).
    std::string suffix;         ///< Text suffix for measure numbers (encoded UTF-8).

    bool countFromOne{};        ///< Start counting from 1 rather than 0, e.g., "1, 2, 3, 4" numbering style (in conjuction with base 10)
    bool noZero{};              ///< Indicates the base has no zero value: true for alpha sequences and false for numeric sequences
    bool doubleUp{};            ///< Indicates "a, b, c...aa, bb, cc" number style: the symbols are repeated when they exceed the base.
    bool time{};                ///< Display real time sequences rather than numbers or letters.
    bool includeHours{};        ///< Display hours (when showing real time measure numbers)
    bool smpteFrames{};         ///< SMPTE frames (when showing real time measure numbers). This option supercedes `timePrecision`.
    bool useScoreInfoForPart{}; ///< Use score-wide settings for parts.
    int region{};               ///< The region ID. This 1-based value is set by Finale and never changes, whereas the @ref Cmper may change when Finale sorts the regions.
    TimePrecision timePrecision{}; ///< Precision for real-time sequences.
    bool hideScroll{};          ///< Indicates if numbers are hidden in Scroll View and Studio View.
    bool hidePage{};            ///< Indicates if numbers are hidden in Page View.

    /// @brief Calculates whether the input measure is covered by this measure number region
    /// @param measureId The measure id to check.
    [[nodiscard]] bool calcIncludesMeasure(MeasCmper measureId) const
    {
        return measureId >= startMeas && measureId < endMeas; // endMeas is non-inclusive!
    }

    /// @brief Returns the starting display measure number for this region. The value is irrespective
    /// of whether the number actually displays on the first measure of the region. (This depends on #Measure::noMeasNum.)
    /// The starting number appears on the first measure in the region that is included in measure numbering.
    [[nodiscard]] int getStartNumber() const { return int(numberOffset + 1); }

    /// @brief Returns the visible number for a measure id with respect to the region.
    /// @return The display number or std::nullopt if the measure is not included in measure numbering
    /// @throw std::logic_error if measureId is not contained in the region
    [[nodiscard]] std::optional<int> calcDisplayNumberFor(MeasCmper measureId) const;

    /// @brief Returns the visible text for a measure id with respect to the region.
    /// @return The display text or std::nullopt if the measure is not included in measure numbering
    /// @throw std::logic_error if measureId is not contained in the region
    [[nodiscard]] std::optional<std::string> calcDisplayNumberTextFor(MeasCmper measureId) const;

    /// @brief Returns the @ref MeasCmper of the first displayed measure number in this region.
    /// @returns The first displayed measure id. Measures with "skip measure numbering" are skipped.
    /// Returns std::nullopt if every measure in the region is set to skip measure numbering. (See #Measure::noMeasNum.)
    [[nodiscard]] std::optional<MeasCmper> calcFirstDisplayedMeasureId() const;

    /// @brief Returns the @ref MeasCmper of the last displayed measure number in this region.
    /// @returns The last displayed measure id. Measures with "skip measure numbering" are skipped.
    /// Returns std::nullopt if every measure in the region is set to skip measure numbering. (See #Measure::noMeasNum.)
    [[nodiscard]] std::optional<MeasCmper> calcLastDisplayedMeasureId() const;

    /// @brief Returns the first visible number in the region. This function takes into account if the first measure
    /// is not included and falls back to the next measure if so. (And repeats if necessary.)
    /// @return The first visible number or std::nullopt if no measure in the region is included in numbering. (See #Measure::noMeasNum.)
    [[nodiscard]] std::optional<int> calcFirstDisplayNumber() const;

    /// @brief Returns the last visible number in the region. This function takes into account if the last measure
    /// is not included and falls back to the previous measure if so. (And repeats if necessary.)
    /// @return The last visible number or std::nullopt if no measure in the region is included in numbering. (See #Measure::noMeasNum.)
    [[nodiscard]] std::optional<int> calcLastDisplayNumber() const;
    
    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (startMeas >= endMeas) {
            MUSX_INTEGRITY_ERROR("Measure number region " + std::to_string(getCmper())
                + " has an invalid measure range: start measure " + std::to_string(startMeas)
                + " is not before end measure " + std::to_string(endMeas) + ".");
        }
        if (!scoreData) {
            scoreData = std::make_shared<ScorePartData>(ptrToThis);
            MUSX_INTEGRITY_ERROR("Measure number region " + std::to_string(getCmper()) + " is missing score data.");
        }
        if (!partData) {
            partData = std::make_shared<ScorePartData>(ptrToThis);
            MUSX_INTEGRITY_ERROR("Measure number region " + std::to_string(getCmper()) + " is missing part data.");
        }
        scoreData->integrityCheck(scoreData);
        partData->integrityCheck(partData);
    }

    constexpr static std::string_view XmlNodeName = "measNumbRegion"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MeasureNumberRegion>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MultimeasureRest
 * @brief Represents the attributes of a multimeasure rest in the page layout.
 *
 * The Cmper is the first measure of the multimeasure rest.
 *
 * This class is identified by the XML node name "mmRest".
 */
class MultimeasureRest : public OthersBase {
public:
    /** @brief Constructor function */
    explicit MultimeasureRest(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper startMeasure)
        : OthersBase(document, partId, shareMode, startMeasure) {}

    Evpu measWidth{};           ///< Width of the multemeasure rest "measure" in Evpu. (xml node is `<meaSpace>`)
    MeasCmper nextMeas{};       ///< Next measure after the multimeasure west.
    Evpu numVertAdj{};          ///< Vertical number adjustment, sign-revered from Finale UI. (xml node is `<numdec>`)
    Cmper shapeDef{};           ///< Cmper of Shape Designer @ref ShapeDef that specifies the H-bar.
    int numStart{};             ///< Number start value. If the number of measures in the multimeasure rest is fewer than this
                                ///< number, no number appears on the multimeasure rest.
    int symbolThreshold{};      ///< If the number of rests is less than this value, symbols are used when #useSymbols is true. (xml node is `<threshold>`)
    Evpu symbolSpacing{};       ///< Spacing between symbols in Evpu. (xml node is `<spacing>`)
    Evpu numHorzAdj{};          ///< Horizontal number adjustment in Evpu. (xml node is `<numAdjX>`)
    Evpu shapeStartAdjust{};    ///< Start adjustment for the H-bar shape in Evpu. (xml node is `<startAdjust>`)
    Evpu shapeEndAdjust{};      ///< End adjustment for the shape in Evpu. (xml node is `<endAdjust>`)
    bool useSymbols{};          ///< Use symbols instead of an H-bar, based on #symbolThreshold. (xml node is `<useCharRestStyle>`)
    bool noHorizontalStretch{}; ///< Inverse of "Stretch Horizontally" option.

    /// @brief Get the start measure of this multimeasure rest
    MeasCmper getStartMeasure() const { return getCmper(); }

    /// @brief Calculates the number of measures spanned by this multimeasure rest
    int calcNumberOfMeasures() const { return (std::max)(nextMeas - getStartMeasure(), 0); }

    /// @brief Calculates whether rest symbols are used instead of the multimeasure rest shape.
    bool calcUsesSymbols() const;

    /// @brief Calculates if the number on this multimeasure rest is visible.
    bool calcIsNumberVisible() const { return calcNumberOfMeasures() >= numStart; }

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (nextMeas <= getStartMeasure()) {
            MUSX_INTEGRITY_ERROR("Multimeasure rest at " + std::to_string(getCmper()) + " in part " + std::to_string(getSourcePartId()) + " spans 0 or fewer measures.");
        }
    }

    constexpr static std::string_view XmlNodeName = "mmRest"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MultimeasureRest>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MultiStaffGroupId
 * @brief Represents a group ID for a multi-staff setup.
 *
 * This class is identified by the XML node name "multiStaffGroupID".
 */
class MultiStaffGroupId : public OthersBase {
public:
    /** @brief Constructor function */
    explicit MultiStaffGroupId(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    Cmper staffGroupId{}; ///< Cmper of @ref details::StaffGroup that has the instrument's full and abbreviated names.

    constexpr static std::string_view XmlNodeName = "multiStaffGroupID"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MultiStaffGroupId>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class MultiStaffInstrumentGroup
 * @brief Represents a group of instruments spanning multiple staves.
 *
 * This class is identified by the XML node name "multiStaffInstGroup".
 */
class MultiStaffInstrumentGroup : public OthersBase {
public:
    /** @brief Constructor function */
    explicit MultiStaffInstrumentGroup(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    std::vector<StaffCmper> staffNums; ///< Vector of Cmper values representing up to 3 staff numbers.

    /// @brief Returns the staff instance (without any staff styles applied) at the index position or null if out of range or not found.
    /// @param x the 0-based index to find
    MusxInstance<Staff> getStaffInstanceAtIndex(size_t x) const;

    /// @brief Returns the first staff instance without any staff styles applied (with integrity check)
    MusxInstance<Staff> getFirstStaffInstance() const;

    /// @brief Returns the index of the input staffId or std::nullopt if not found
    std::optional<size_t> getIndexOf(StaffCmper staffId) const
    {
        for (size_t x = 0; x < staffNums.size(); x++) {
            if (staffNums[x] == staffId) return x;
        }
        return std::nullopt;
    }

    /// @brief Gets the group associated with this multistaff instrument, or nullptr if not found
    /// @param forPartId The part for which to get the group. Pass SCORE_PARTID for the score.
    MusxInstance<details::StaffGroup> getStaffGroup(Cmper forPartId) const;

    /// @brief Used by the factory to calculate all multistaff ids and visual ids for instances of @ref Staff.
    /// @param document
    static void calcAllMultiStaffGroupIds(const DocumentPtr& document);

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        OthersBase::integrityCheck(ptrToThis);
        if (staffNums.empty()) {
            MUSX_INTEGRITY_ERROR("MultiStaffInstrumentGroup " + std::to_string(getCmper()) + " contains no staves.");
        } else if (staffNums.size() > 3) {
            MUSX_INTEGRITY_ERROR("MultiStaffInstrumentGroup " + std::to_string(getCmper()) + " contains more than 3 staves.");
        }
    }

    constexpr static std::string_view XmlNodeName = "multiStaffInstGroup"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MultiStaffInstrumentGroup>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class NamePositionAbbreviated
 * @brief Overrides abbreviated name positioning for @ref Staff.
 *
 * The Cmper is the Staff cmper.
 */
class NamePositionAbbreviated : public NamePositioning
{
public:
    using NamePositioning::NamePositioning;

    constexpr static std::string_view XmlNodeName = "namePosAbbrv"; ///< The XML node name for this type.
};

/**
 * @class NamePositionStyleAbbreviated
 * @brief Overrides abbreviated name positioning for @ref StaffStyle.
 *
 * The Cmper is the StaffStyle cmper.
 */
class NamePositionStyleAbbreviated : public NamePositioning
{
public:
    using NamePositioning::NamePositioning;

    constexpr static std::string_view XmlNodeName = "namePosAbbrvStyle"; ///< The XML node name for this type.
};

/**
 * @class NamePositionFull
 * @brief Overrides full name positioning for @ref Staff.
 *
 * The Cmper is the Staff cmper.
 */
class NamePositionFull : public NamePositioning
{
public:
    using NamePositioning::NamePositioning;

    constexpr static std::string_view XmlNodeName = "namePosFull"; ///< The XML node name for this type.
};

/**
 * @class NamePositionStyleFull
 * @brief Overrides full name positioning for @ref StaffStyle.
 *
 * The Cmper is the StaffStyle cmper.
 */
class NamePositionStyleFull : public NamePositioning
{
public:
    using NamePositioning::NamePositioning;

    constexpr static std::string_view XmlNodeName = "namePosFullStyle"; ///< The XML node name for this type.
};

/**
 * @class Page
 * @brief Represents the attributes of a page in the page layout.
 *
 * This class is identified by the XML node name "pageSpec".
 */
class Page : public OthersBase {
public:
    /** @brief Constructor function */
    explicit Page(const DocumentWeakPtr& document, PageCmper pageId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, pageId, shareMode, cmper) {}

    Evpu height{};              ///< Page height in Evpu.
    Evpu width{};               ///< Page width in Evpu.
    int percent{};              ///< Percent value (scaling factor).
    SystemCmper firstSystemId{}; ///< First system on the page (-1 if page is blank). See @ref StaffSystem. (xml node is `<firstSystem>`)
    bool holdMargins{};         ///< "Hold Margins" (xml node is `<scaleContentOnly>`)
    Evpu margTop{};             ///< Top margin in Evpu. (Sign reversed in Finale UI.)
    Evpu margLeft{};            ///< Left margin in Evpu.
    Evpu margBottom{};          ///< Bottom margin in Evpu.
    Evpu margRight{};           ///< Right margin in Evpu. (Sign reversed in Finale UI.)

    std::optional<SystemCmper> lastSystemId;    ///< Computed by the Resolver function #calcSystemInfo. This value is not in the xml.
    std::optional<MeasCmper> firstMeasureId;    ///< Computed by the Resolver function #calcSystemInfo. This value is not in the xml.
    std::optional<MeasCmper> lastMeasureId;     ///< Computed by the Resolver function #calcSystemInfo. This value is not in the xml.

    /** @brief is this a blank page */
    bool isBlank() const { return firstSystemId == -1; }

    /// @brief Returns whether this page has a calculated layout.
    /// @details Blank pages are valid calculated pages. A nonblank page is calculated when its
    /// system range and corresponding measure range were resolved by #calcSystemInfo.
    bool isLayoutCalculated() const
    {
        return isBlank() || (firstSystemId > 0 && lastSystemId && firstMeasureId && lastMeasureId);
    }

    /// @brief Calculate the page scaling.
    util::Fraction calcPageScaling() const
    { return util::Fraction::fromPercent(percent); }

    /// @brief Resolver function used by factory to compute system and measure information for all pages.
    static void calcSystemInfo(const DocumentPtr& document);

    constexpr static std::string_view XmlNodeName = "pageSpec"; ///< The XML node name for this type.
    static const xml::XmlElementArray<Page>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

class TextBlock;
/**
 * @class PageTextAssign
 * @brief Represents a page text assignment with positioning and page range properties.
 *
 * Instances of #PageTextAssign use page assignment IDs rather than straightforward page numbers.
 * This allows their page assignments to shift based on how many leading blank pages exist in the
 * current score or part view vs. all the others. If all score and parts have the same number of
 * leading blank pages, then page assignment IDs are the same as page numbers.
 *
 * If the cmper is non-0, the #startPage and #endPage values are not used and the cmper
 * specifies a page assignment ID that defines which page this text is assigned to.
 *
 * If the cmper is 0, the #startPage and #endPage values specify the range of page assignment IDs to which
 * this text is assigned. An #endPage of 0 indicates the last page of the document.
 *
 * If cmper is non-zero, #startPage and #endPage should have the same value as the cmper.
 *
 * The inci value specifies a particular page text when more than one exists for the cmper value.
 *
 * This class is identified by the XML node name "pageTextAssign".
 */
class PageTextAssign : public OthersBase
{
public:
    /**
     * @enum PageAssignType
     * @brief Which pages a multipage assignment appears on
     */
    enum class PageAssignType
    {
        AllPages,       ///< default (may not appear in xml)
        Even,
        Odd
    };

    using HorizontalAlignment = options::TextOptions::HorizontalAlignment;  ///< Horizontal alignment options for page text positioning.
    using VerticalAlignment = options::TextOptions::VerticalAlignment;      ///< Vertical alignment options for page text positioning.

    /** @brief Constructor function */
    explicit PageTextAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci)
    {
    }

    Cmper block{};                  ///< The Cmper for the assigned @ref TextBlock. (xml tag is `<block>`)
    Evpu xDisp{};                   ///< The horizontal displacement from the default position. (xml tag is `<xdisp>`)
    Evpu yDisp{};                   ///< The vertical displacement from the default position. (xml tag is `<ydisp>`)
    PageCmper startPage{};          ///< The first page assignment ID on which the text appears.
                                    ///< Note that the page assignment ID may be different than the page number. See #calcStartPageNumber.
    PageCmper endPage{};            ///< The last page assignment ID on which the text appears.
                                    ///< A value of zero indicates the last page in the document, whatever number it may be.
                                    ///< Note that the page assignment ID may be different than the page number. See #calcEndPageNumber.
    PageAssignType oddEven{};       ///< Determines if a multipage assignment appears on all, even (left), or odd (right) pages
    HorizontalAlignment hPosLp{};   ///< Horizontal alignment on left or all pages (depending on #indRpPos). (xml tag is `<hposLp>`)
    HorizontalAlignment hPosRp{};   ///< Horizontal alignment on right pages (if #indRpPos is true). (xml tag is `<hposRp>`)
    bool hidden{};                  ///< Indicates if the page text appears only on screen. (xml tag is `<postIt>`)
    VerticalAlignment vPos{};       ///< Vertical alignment. (xml tag is `<vpos>`)
    bool hPosPageEdge{};            ///< If true, horizontal position is relative to page edge.
                                    ///< Otherwise it is relative to the left / right page margins. (xml tag is `<hposPageEdge>`)
    bool vPosPageEdge{};            ///< If true, vertical position is relative to page edge.
                                    ///< Otherwise it is relative to the top / bottom page margins. (xml tag is `<vposPageEdge>`)
    bool indRpPos{};                ///< Individual right page positioning indicator. (xml tag is `<indRpPos>`)
    Evpu rightPgXDisp{};            ///< Horizontal displacement for right pages (if #indRpPos is true). (xml tag is `<rightPgXdisp>`)
    Evpu rightPgYDisp{};            ///< Vertical displacement for right pages (if #indRpPos is true). (xml tag is `<rightPgYdisp>`)

    /** @brief Gets the TextBlock for this assignment, or nullptr if none. */
    MusxInstance<TextBlock> getTextBlock() const;

    /// @brief Return the starting page number, taking into account leading blank pages in all parts.
    /// This calculation mimics observed behavior in Finale.
    /// @return The first page in @p forPartId on which the page text appears. If it does not appear on the part,
    /// the function returns std::nullopt.
    std::optional<PageCmper> calcStartPageNumber(Cmper forPartId) const;

    /// @brief Return the ending page number, taking into account leading blank pages in all parts
    /// This calculation mimics observed behavior in Finale.
    /// @return The first page in @p forPartId on which the page text appears. If it does not appear on the part,
    /// the function returns std::nullopt.
    std::optional<PageCmper> calcEndPageNumber(Cmper forPartId) const;

    /**
     * @brief Gets the raw text for parsing this assignment, or nullptr if none.
     * @param forPartId The part to use for ^partname and ^totpages inserts.
     * @param forPageId The page number to use for ^page inserts if this is a multipage instance.
     * This value is ignored for single page instances. Note that this is a page number and not a page ID.
     * See #calcStartPageNumber and #calcEndPageNumber.
    */
    util::EnigmaParsingContext getRawTextCtx(Cmper forPartId, std::optional<Cmper> forPageId = std::nullopt) const;

    /// @brief Returns true if this is a multi-page assignment.
    bool isMultiPage() const
    { return getCmper() == 0 && startPage != endPage; }

    /// @brief Returns true if this is a multi-page assignment that is assigned to through last page, no matter how many.
    bool isMultiAssignedThruLastPage() const
    { return isMultiPage() && endPage == 0; }

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (getCmper() != 0) {
            if (startPage != getCmper() || endPage != getCmper()) {
                MUSX_INTEGRITY_ERROR("PageTextAssign " + std::to_string(getCmper()) + " inci " + std::to_string(getInci().value_or(0)) +
                    " has startPage or endPage value that does not match.");
            }
        }
    }

    /// @brief Returns a specific page text assignment for a given page number in a given part.
    /// This allows the caller not to have to know the conversion to page assignment IDs.
    /// @param document The document to search.
    /// @param partId The ID of the linked part to search.
    /// @param pageId The page number to search for, or zero for multipage assignments.
    /// @param inci The inci of the specific page text assignment to retrieve.
    static MusxInstance<others::PageTextAssign> getForPageId(const DocumentPtr& document, Cmper partId, PageCmper pageId, Inci inci);

    /// @brief Returns all the page text assignments for a given page number in a given part.
    /// This allows the caller not to have to know the conversion to page assignment IDs.
    /// @param document The document to search.
    /// @param partId The ID of the linked part to search.
    /// @param pageId The page number to search for, or zero for all multipage assignments.
    static MusxInstanceList<others::PageTextAssign> getArrayForPageId(const DocumentPtr& document, Cmper partId, PageCmper pageId);

    constexpr static std::string_view XmlNodeName = "pageTextAssign"; ///< The XML node name for this type.
    static const xml::XmlElementArray<PageTextAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class PartDefinition
 * @brief Represents the attributes of a Finale "partDef".
 *
 * The cmper is the part definition ID, representing unique part definitions in the Finale document.
 * This class is identified by the XML node name "partDef".
 */
class PartDefinition : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit PartDefinition(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // Public properties corresponding to the XML structure
    Cmper nameId{};                     ///< @ref Cmper of the part name @ref TextBlock. (xml tag is `<nameID>`)
    int partOrder{};                    ///< Value that determines the order of listed parts in Finale's UI.
    int copies{};                       ///< Number of copies to print.
    bool printPart{};                   ///< Indicates the part should be printed.
    bool extractPart{};                 ///< Indicates if the part should be extracted.
    bool applyFormat{};                 ///< Meaning uncertain. May have to do with page format for parts and whether it has been applied.
    bool needsRecalc{};                 ///< Indicates if the part needs update layout.
    bool useAsSmpInst{};                ///< Indicates if the part is used as a SmartMusic instrument.
    bool unlinkInsts{};                 ///< Finale flag with currently unknown semantics. (xml node is `<unlinkInsts>`)
    int smartMusicInst{};               ///< SmartMusic instrument ID (-1 if not used).
    Cmper defaultNameStaff{};           ///< If non-zero, this points to the @ref Staff that has the default name (if unspecified by #nameId.)
    Cmper defaultNameGroup{};           ///< If non-zero, this points to the @ref details::StaffGroup that has the default name (if unspecified by #nameId.)

    int numberOfLeadingBlankPages{};    ///< The number of leading blank pages in the part. This is not in the xml but calculated in #factory::DocumentFactory::create.
    int numberOfPages{};                ///< The total number of pages in the part. This is not in the xml but calculated in #factory::DocumentFactory::create.

    /** @brief Get the raw text context for the part name if any */
    util::EnigmaParsingContext getNameRawTextCtx() const;

    /** @brief Get the part name if any */
    std::string getName(util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii) const;

    /// @brief Returns whether this part has a calculated page layout.
    /// @details Returns false if the part has no pages or any page has unresolved layout information.
    bool isLayoutCalculated() const;

    /** @brief Return true if this part corresponds to the score */
    bool isScore() const { return getCmper() == SCORE_PARTID; }

    /**
     * @brief Returns the @ref StaffUsed cmper for Scroll View for this part. See #Document::calcScrollViewCmper.
    */
    Cmper calcScrollViewCmper() const;

    /// @brief Calculates a page number in this part from a page assignment ID. (See @ref PageTextAssign.)
    /// @param pageAssignmentId The page assignment ID.
    /// @return Page number or potential page number (if greater than the part's number of pages). If the assignment
    /// is for a leading blank page the part does not have, returns std::nullopt.
    std::optional<PageCmper> calcPageNumberFromAssignmentId(PageCmper pageAssignmentId) const;

    /// @brief Calculates a page assignment ID from a page number in the part.
    /// @param pageId The page for which to get the assignment ID.
    /// @return The calculated page assignment ID.
    PageCmper calcAssignmentIdFromPageNumber(PageCmper pageId) const;

    /** @brief Return the instance for the score */
    static MusxInstance<PartDefinition> getScore(const DocumentPtr& document);

    /** @brief Return the linked parts sorted in UI order by #partOrder */
    static MusxInstanceList<PartDefinition> getInUserOrder(const DocumentPtr& document);

    constexpr static std::string_view XmlNodeName = "partDef"; ///< The XML node name for this type.
    static const xml::XmlElementArray<PartDefinition>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class PartGlobals
 * @brief Represents global values that can vary by part
 *
 * The cmper is always 65534.
 *
 * This class is identified by the XML node name "partGlobals".
 */
class PartGlobals : public OthersBase {
public:
    /** @brief Constructor function */
    explicit PartGlobals(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // Public properties corresponding to the XML structure
    bool showTransposed{};                  ///< If true, "Display Concert Pitch" is unchecked for the part.
    Cmper scrollViewIUlist{};               ///< If this value is non-zero, it is the iuList @ref Cmper of the current Staff List in Scroll View.
    Cmper studioViewIUlist{};               ///< The iuList @ref Cmper for Studio View.
    /**
     * @brief If non-zero, Special Part Extraction is enabled and this is the @ref Cmper
     * for its legacy @ref StaffUsed array.
     *
     * In older (pre-optimization) Finale files, this Cmper identified the staff list
     * used for Page View layout of an extracted part. In modern Finale, including all
     * versions that produce `musx` rather than legacy `mus`, every system is fully
     * optimized and maintains its own system-level staff list. As a result, this value
     * is no longer consulted when constructing Page View systems.
     *
     * Its only remaining purpose is as a flag that Special Part Extraction is active,
     * and as a filter for the Finale UI. It limits which staves may be added to a
     * Page View system when the user is working in the extracted-part layout.
     *
     * (xml node is `<pageViewIUlist>`)
    */
    Cmper specialPartExtractionIUList{};

    /**
     * @brief Return the @ref StaffUsed cmper for Scroll View for this part.
     * @returns Either #BASE_SYSTEM_ID or the #specialPartExtractionIUList.
    */
    Cmper calcScrollViewCmper() const;

    constexpr static std::string_view XmlNodeName = "partGlobals"; ///< The XML node name for this type.
    static const xml::XmlElementArray<PartGlobals>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class PartVoicing
 * @brief Defines voicing options for linked parts, either by selection rules or by assigning a layer.
 *
 * In Finale, a linked part can contain multiple staves. When a staff is shared across more than one linked part,
 * each occurrence of that staff has its own voicing definition, represented by a separate PartVoicing record.
 * If no PartVoicing record is present, or if the PartVoicing record is disabled, the staff is included
 * in the linked part in full.
 *
 * This class is identified by the XML node name "voicingDef".
 */
class PartVoicing : public OthersBase {
public:
    /// @brief Constructor function.
    /// @param document The document.
    /// @param partId The part ID (never SCORE_PARTID).
    /// @param shareMode The share mode (always ShareMode::None).
    /// @param staff The staff in the part that this voicing applies to.
    explicit PartVoicing(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper staff)
        : OthersBase(document, partId, shareMode, staff) {}

    /// @brief Choices for how notes are selected from the staff.
    enum class VoicingType
    {
        UseSingleLayer,             ///< Display all notes from the layer specified by #singleLayer. (This is the default value and may never appear in the xml.)
        UseMultipleLayers           ///< Display selected notes from one or more layer(s).
    };

    /// @brief Choices for how notes are selected when the staff contains only one layer.
    enum class SingleLayerVoiceType
    {
        AllNotes,                   ///< Select all notes. (This is the default value and may never appear in the xml.)
        TopNote,                    ///< Select the top note.
        BottomNote,                 ///< Select the bottom note. (xml value is "botNote")
        SelectedNotes               ///< Use flags #select1st, #select2nd, etc., to select notes. (xml value is "selected")
    };

    bool enabled{};                             ///< Indicates that this voicing is enabled.
    VoicingType voicingType{};                  ///< Determines how notes are selected from the staff.
    SingleLayerVoiceType singleLayerVoiceType{}; ///< Determines how notes are selected when the staff contains only one layer. (xml node is `<singleVoiceType>`)
    bool select1st{};                           ///< Select the first note from top or bottom. (See #selectFromBottom.)
    bool select2nd{};                           ///< Select the second note from top or bottom. (See #selectFromBottom.)
    bool select3rd{};                           ///< Select the third note from top or bottom. (See #selectFromBottom.)
    bool select4th{};                           ///< Select the fourth note from top or bottom. (See #selectFromBottom.)
    bool select5th{};                           ///< Select the fifth note from top or bottom. (See #selectFromBottom.)
    bool selectFromBottom{};                    ///< If true, select notes starting at the bottom of a chord going up.
                                                ///< If false, select notes starting at the top of a chord going down.
    bool selectSingleNote{};                    ///< If true, always include any passages containing only a single note.
    LayerIndex singleLayer{};                   ///< The 0-based LayerIndex of the layer to uses when #voicingType is `UseSingleLayer`.
    LayerIndex multiLayer{};                    ///< The 0-based LayerIndex of the layer to include when the staff contains multiple layers and #voicingType is `UseMultipleLayers`.

    /// @brief Returns if this part voicing includes the specified layer in a given frame context.
    /// @param layerIndex The 0-based layer index to check.
    /// @param frameIsMultilayerInScore Whether the frame we are checking is multi-layer in the score.
    /// @return True if the layer shows.
    bool calcShowsLayer(LayerIndex layerIndex, bool frameIsMultilayerInScore) const;

    /// @brief Returns if this part voicing includes the specified note.
    /// @param noteInfoPtr The note to check.
    /// @return True is the note shows in this part.
    bool calcShowsNote(const NoteInfoPtr& noteInfoPtr) const;

    constexpr static std::string_view XmlNodeName = "voicingDef"; ///< The XML node name for this type.
    static const xml::XmlElementArray<PartVoicing>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class PercussionNoteInfo
 * @brief Represents percussion notehead and staff position info for a given percussion note type.
 *
 * The cmper is the percussion map ID. Obtain this from #Staff::percussionMapId.
 * The inci identifies a specific note in the map. They are in no guaranteed order.
 *
 * This class is identified by the XML node name "percussionNoteInfo".
 */
class PercussionNoteInfo : public OthersBase {
public:
    /** @brief Constructor function */
    explicit PercussionNoteInfo(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci) {}

    PercussionNoteTypeId percNoteType{};    ///< The percussion note type ID. Compare this with the value in @ref details::PercussionNoteCode.
    int staffPosition{};                    ///< The fixed vertical staff position of the note. (xml node is `<harmLev>`.)
                                            ///< This value is the staff position relative to the first ledger line below the staff.
                                            ///< The logic behind this choice is that it is the middle-C position on a treble clef, but
                                            ///< middle-C is not relevant to the note's pitch or value. Use #calcStaffReferencePosition to get the
                                            ///< staff position relative to the staff's reference line, which is often a more useful value.
    char32_t closedNotehead{};              ///< Codepoint for closed notehead (from percussion notehead font. See @ref options::FontOptions::FontType::Percussion.)
    char32_t halfNotehead{};                ///< Codepoint for half notehead (from percussion notehead font. See @ref options::FontOptions::FontType::Percussion.)
    char32_t wholeNotehead{};               ///< Codepoint for whole notehead (from percussion notehead font. See @ref options::FontOptions::FontType::Percussion.)
    char32_t dwholeNotehead{};              ///< Codepoint for double whole notehead (from percussion notehead font. See @ref options::FontOptions::FontType::Percussion.)

    /// @brief Calculates the fixed staff position for this percussion note relative to a staff's reference line.
    int calcStaffReferencePosition() const { return staffPosition - 10; }

    /// @brief Gets the base PercussionNoteTypeId.
    /// @return the base percussion note type id (used to look it up in the #percussion::percussionNoteTypeMap.)
    PercussionNoteTypeId getBaseNoteTypeId() const
    { return percNoteType & 0xfff; }

    /// @brief Gets the orderId
    /// @return value used to distinguish different copies of the same note id.
    unsigned getNoteTypeOrderId() const
    { return(percNoteType & 0xf000) >> 12; }

    /// @brief Gets a reference to the PercussionNoteType record for this note id.
    /// @return Record from #percussion::percussionNoteTypeMap.
    const percussion::PercussionNoteType& getNoteType() const;

    constexpr static std::string_view XmlNodeName = "percussionNoteInfo"; ///< The XML node name for this type.
    static const xml::XmlElementArray<PercussionNoteInfo>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @enum RepeatActionType
 * @brief Enum for the possible values of the `<action>` element.
 */
enum class RepeatActionType
{
    JumpAuto,               ///< Automatically Jump. (the default). The targetValue is meaningless for this action.
    JumpAbsolute,           ///< Jump to the measure number specified in the targetValue field.
    JumpRelative,           ///< Jump relative to the current measure. The targetValue field specifies how many measures to jump.
                            ///< The targetValue may be positive or negative for forward or backward relative jumps.
    JumpToMark,             ///< Jump to a specified repeat number (used by text repeats).
    Stop,                   ///< Stops playback after a number of passes (e.g. "Fine")
    NoJump                  ///< Do not jump. The targetValue is meaningless for this action.
};

/**
 * @enum RepeatTriggerType
 * @brief Enum for the possible values of the `<trigger>` element.
 */
enum class RepeatTriggerType
{
    Always,                 ///< Always jump (the default)
    OnPass,                 ///< Jump on a sepecified pass number.
    UntilPass               ///< Jump until a specified pass number is reached.
};

class RepeatIndividualPositioning;

/**
 * @class RepeatBack
 * @brief Represents a repeat-backward marker with positioning and behavior properties.
 *
 * The cmper is the cmper of the @ref Measure that has this item.
 *
 * This class is identified by the XML node name "repeatBack".
 */
class RepeatBack : public OthersBase {
public:
    /** @brief Constructor function */
    explicit RepeatBack(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // Public properties corresponding to the XML structure
    int passNumber{};               ///< Specifies the pass number for the trigger and depends on the value of #trigger. (xml tag is `<actuate>`)
                                    ///< If #trigger is `UntilPass`, this specifies the number of times to play the section.
                                    ///< If #trigger is `OnPass`, this specifies the pass on which to jump.
    int targetValue{};              ///< Absolute or relative measure number, depending on #jumpAction. (xml tag is `<target>`)
    Evpu leftHPos{};                ///< The horizontal position of the left bracket, relative to the default. (xml tag is `<pos1>`)
    Evpu leftVPos{};                ///< The vertical position of the lower left bracket, relative to the default. (xml tag is `<line1>`)
    bool individualPlacement{};     ///< "Allow Individual Edits Per Staff" (xml tag is `<indivPlac>`)
    bool topStaffOnly{};            ///< "Show On: Top Staff Only"
    bool hidden{};                  ///< Inverse of "Show" checkbox.
    bool resetOnAction{};           ///< "Reset on Repeat Action" (xml tag is `<clrOnChange>`)
    RepeatActionType jumpAction{};  ///< The jump action for this repeat ending. The automatic jump is to the next ending. (xml tag is `<action>`)
    RepeatTriggerType trigger{};    ///< The condition that triggers the #jumpAction.
    Cmper staffList{};              ///< If non-zero, specifies a staff list for which staves to show the ending.
    Evpu rightHPos{};               ///< The horizontal position of the upper right bracket, relative to the default. (xml tag is `<pos2>`)
    Evpu rightVPos{};               ///< The vertical position of the upper right bracket, relative to the default. (xml tag is `<line2>`)

    /// @brief Returns the @ref RepeatIndividualPositioning record for a given staff.
    /// Check #individualPlacement to see if this record is used by Finale.
    /// @param staffId The StaffCmper of the staff to search for individual positioning.
    /// @return The individual positioning record or nullptr if not found.
    MusxInstance<RepeatIndividualPositioning> getIndividualPositioning(StaffCmper staffId) const;

    /// @brief Create a @ref StaffListSet for the given instance. This can be used to interrogate whether a staff appears in the staff set.
    /// @return The created staff list set. If #staffList is zero, it will never find any staves for the staff list.
    RepeatStaffListSet createStaffListSet() const;

    /// @brief Calculates the target measure for this repeat-back jump action, if any.
    /// @details For JumpAuto, this resolves to the nearest preceding forward repeat bar.
    /// Returns std::nullopt for actions that do not provide a concrete measure target.
    /// @return The target measure for the jump, or std::nullopt if none applies.
    std::optional<MeasCmper> calcTargetMeasure() const;

    constexpr static std::string_view XmlNodeName = "repeatBack"; ///< The XML node name for this type.
    static const xml::XmlElementArray<RepeatBack>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class RepeatEndingStart
 * @brief Represents a repeat ending start marker in the document.
 *
 * The cmper is the cmper of the @ref Measure that has this item.
 *
 * This class is identified by the XML node name "repeatEndingStart".
 */
class RepeatEndingStart : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit RepeatEndingStart(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper)
    {
    }

    Cmper staffList{};              ///< If non-zero, specifies a staff list for which staves to show the ending.
    int targetValue{};              ///< Absolute or relative measure number, depending on #jumpAction. (xml tag is `<nextEnd>`)
    Evpu textHPos{};                ///< The horizontal position of the text relative to #leftHPos. (xml tag is `<textPos>`)
    Evpu leftHPos{};                ///< The horizontal position of the lower left bracket, relative to the default. (xml tag is `<pos1>`)
    Evpu leftVPos{};                ///< The vertical position of the lower left bracket, relative to the default. (xml tag is `<line1>`)
    bool individualPlacement{};     ///< "Allow Individual Edits Per Staff" (xml tag is `<indivPlac>`)
    bool topStaffOnly{};            ///< "Show On: Top Staff Only"
    bool hidden{};                  ///< Inverse of "Show" checkbox.
    RepeatActionType jumpAction{};  ///< The jump action for this repeat ending. The automatic jump is to the next ending. (xml tag is `<action>`)
    RepeatTriggerType trigger{};    ///< The condition that triggers the #jumpAction.
                                    ///< For `RepeatEndingStart` this value is always `OnPass`, and it seems to mean that
                                    ///< it jumps when the current pass exceeds the final ending number.
    bool jumpIfIgnoring{};          ///< "Skip Ending if Ignoring Repeats" (xml tag is `<jmpIgnore>`)
    Evpu endLineVPos{};             ///< The vertical offset of the final bracket, relative to #rightVPos or 0 if the ending is open. (xml tag is `<endLine>`)
    Evpu textVPos{};                ///< The vertical position of the text relative to #leftVPos. (xml tag is `<textLine>`)
    Evpu rightHPos{};               ///< The horizontal position of the upper right bracket, relative to the default. (xml tag is `<pos2>`)
    Evpu rightVPos{};               ///< The vertical position of the upper right bracket, relative to the default. (xml tag is `<line2>`)

    /// @brief Calculates the number of measures in the ending based on #jumpAction.
    int calcEndingLength() const;

    /// @brief Calculates the target measure for the ending's jump action, if any.
    /// @details Returns the measure immediately following the ending when the action is a normal
    /// jump (automatic, relative, or absolute). Returns std::nullopt for actions that do not
    /// specify a concrete target (e.g., JumpToMark, Stop, NoJump).
    /// @return The target measure for the jump, or std::nullopt if none applies.
    std::optional<MeasCmper> calcTargetMeasure() const;

    /// @brief Calculates if the ending is open or closed, based on a number of factors.
    ///
    /// Openness is a visual feature. If true, it means that the ending bracket has no downward stroke on the right.
    bool calcIsOpen() const;

    /// @brief Returns the @ref RepeatIndividualPositioning record for a given staff.
    /// Check #individualPlacement to see if this record is used by Finale.
    /// @param staffId The StaffCmper of the staff to search for individual positioning.
    /// @return The individual positioning record or nullptr if not found.
    MusxInstance<RepeatIndividualPositioning> getIndividualPositioning(StaffCmper staffId) const;

    /// @brief Returns the @ref RepeatIndividualPositioning record for the text a given staff.
    /// Check #individualPlacement to see if this record is used by Finale.
    /// @param staffId The StaffCmper of the staff to search for individual positioning.
    /// @return The individual positioning record or nullptr if not found.
    MusxInstance<RepeatIndividualPositioning> getTextIndividualPositioning(StaffCmper staffId) const;

    /// @brief Taking into account all options, creates a string containing the text for this ending. The font for this text
    /// is given by #options::FontOptions::FontType::Ending.
    std::string createEndingText() const;

    /// @brief Create a @ref StaffListSet for the given instance. This can be used to interrogate whether a staff appears in the staff set.
    /// @return The created staff list set. If #staffList is zero, it will never find any staves for the staff list.
    RepeatStaffListSet createStaffListSet() const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        OthersBase::integrityCheck(ptrToThis);
        if (trigger != RepeatTriggerType::OnPass) {
            MUSX_INTEGRITY_ERROR("RepeatEndingStart at measure " + std::to_string(getCmper()) + " has an unexpected trigger value " +
                std::to_string(int(trigger)));
        }
    }

    constexpr static std::string_view XmlNodeName = "repeatEndingStart"; ///< The XML node name for this type.
    static const xml::XmlElementArray<RepeatEndingStart>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class RepeatEndingText
 * @brief Represents the text associated with a @ref RepeatEndingStart.
 *
 * This is an optional value. If it is omitted, the ending shows a standard number list.
 *
 * This class has the same #Cmper as its @ref RepeatEndingStart and is the @ref MeasCmper of the ending.
 *
 * This class is identified by the XML node name "repeatEndingText".
 */
class RepeatEndingText : public OthersBase {
public:
    /** @brief Constructor function */
    explicit RepeatEndingText(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    std::string text; ///< The text

    constexpr static std::string_view XmlNodeName = "repeatEndingText"; ///< The XML node name for this type.
    static const xml::XmlElementArray<RepeatEndingText>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class RepeatPassList
 * @brief Represents a list of repeat ending numbers for a @ref RepeatEndingStart instance.
 *
 * The cmper is the cmper of the @ref Measure that has this item.
 *
 * This class is identified by the XML node name "repeatPassList".
 */
class RepeatPassList : public OthersArray<int>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using OthersArray::OthersArray;

    constexpr static std::string_view XmlNodeName = "repeatPassList"; ///< The XML node name for this type.
    static const xml::XmlElementArray<RepeatPassList>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class RepeatIndividualPositioning
 * @brief Per-instance positioning overrides for repeat elements.
 *
 * This class captures individual offsets and visibility for repeat endings and repeat text that can be positioned
 * per instance on a staff/system.
 *
 * This is a base class, and the meaning of the cmper and inci varies according to the subclass.
 */
class RepeatIndividualPositioning : public OthersBase
{
public:
    /**
     * @brief Constructs a RepeatIndividualPositioning object.
     * @param document Shared pointer to the document.
     * @param partId The part ID if this positioning is unlinked.
     * @param shareMode The share mode if this positioning is unlinked.
     * @param cmper Identifier for this record. (See subclasses.)
     * @param inci The specific record with this @p cmper.
     */
    explicit RepeatIndividualPositioning(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci) {}

    StaffCmper staffId{};       ///< Staff for this individual positioning. (xml tag is `<instno>`)
    MeasCmper  measureId{};     ///< Measure for text repeats, otherwise zero. (xml tag is `<id>`)
    bool       hidden{};        ///< Overrides the value in the main record. (Ignored for @ref RepeatEndingTextIndividualPositioning)
    Evpu       x1add{};         ///< Left/start X offset in @ref Evpu.
    Evpu       y1add{};         ///< Left/start Y offset in @ref Evpu.
    Evpu       x2add{};         ///< Right/end X offset in @ref Evpu. (Always zero for @ref RepeatEndingTextIndividualPositioning and @ref TextRepeatIndividualPositioning.)
    Evpu       y2add{};         ///< Right/end Y offset in @ref Evpu. (Always zero for @ref RepeatEndingTextIndividualPositioning and @ref TextRepeatIndividualPositioning.)

    /// Required for musx::factory::FieldPopulator.
    static const xml::XmlElementArray<RepeatIndividualPositioning>& xmlMappingArray();
};

/**
 * @class RepeatBackIndividualPositioning
 * @brief Per-instance positioning overrides for @ref RepeatBack
 *
 * The cmper is the measureId containing the @ref RepeatBack. The inci allows multiple instances per measure.
 * #RepeatIndividualPositioning::measureId is always zero.
 */
class RepeatBackIndividualPositioning : public RepeatIndividualPositioning
{
public:
    using RepeatIndividualPositioning::RepeatIndividualPositioning;

    constexpr static std::string_view XmlNodeName = "separatesRepeatBack"; ///< The XML node name for this type.
};
/**
 * @class RepeatEndingStartIndividualPositioning
 * @brief Per-instance positioning overrides for the line positioning of @ref RepeatEndingStart
 *
 * @note The #hidden property of this class controls the hidden/show state of both the ending line and the ending text.
 *
 * The cmper is the measureId containing the @ref RepeatEndingStart. The inci allows multiple instances per measure.
 * #RepeatIndividualPositioning::measureId is always zero.
 */
class RepeatEndingStartIndividualPositioning : public RepeatIndividualPositioning
{
public:
    using RepeatIndividualPositioning::RepeatIndividualPositioning;

    constexpr static std::string_view XmlNodeName = "separatesRepeatEndingLine"; ///< The XML node name for this type.
};

/**
 * @class RepeatEndingTextIndividualPositioning
 * @brief Per-instance positioning overrides for text positioning of @ref RepeatEndingStart
 *
 * The cmper is the measureId containing the @ref RepeatEndingStart. The inci allows multiple instances per measure.
 * #RepeatIndividualPositioning::measureId is always zero.
 * #RepeatIndividualPositioning::hidden is always false. (The show status of ending text is controlled by @ref RepeatEndingStart or @ref RepeatEndingStartIndividualPositioning.)
 */
class RepeatEndingTextIndividualPositioning : public RepeatIndividualPositioning
{
public:
    using RepeatIndividualPositioning::RepeatIndividualPositioning;

    constexpr static std::string_view XmlNodeName = "separatesRepeatEndingText"; ///< The XML node name for this type.
};

/**
 * @class TextRepeatIndividualPositioning
 * @brief Per-instance positioning overrides for @ref TextRepeatDef
 *
 * The cmper is the cmper for the corresponding @ref TextRepeatDef. The inci allows multiple instances for this definition
 * throughout the document. Use #RepeatIndividualPositioning::measureId to find the instance for a specific measure.
 */
class TextRepeatIndividualPositioning : public RepeatIndividualPositioning
{
public:
    using RepeatIndividualPositioning::RepeatIndividualPositioning;

    constexpr static std::string_view XmlNodeName = "separatesTextRepeat"; ///< The XML node name for this type.
};

/**
 * @class ShapeExpressionDef
 * @brief Stores the properties and behaviors of shape expressions.
 *
 * This class is identified by the XML node name "shapeExprDef".
 */
class ShapeExpressionDef : public OthersBase
{
public:
    /**
     * @brief Constructor.
     *
     * Initializes all fields to their default values.
     */
    explicit ShapeExpressionDef(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    Cmper shapeDef{};                               ///< Identifier for the Shape Designer @ref ShapeDef associated with this expression.
    Cmper categoryId{};                             ///< Identifier for the category of the text expression. (xml node is "categoryID")
    RehearsalMarkStyle rehearsalMarkStyle{};        ///< Auto-sequencing style for rehearsal marks.
    int value{};                                    ///< Value associated with the expression (e.g., velocity).
    Cmper execShape{};                              ///< Executable shape Cmper for playback (@ref ShapeDef)
    int auxData1{};                                 ///< Auxiliary data for the expression. (xml node is "auxdata1")
    int playPass{};                                 ///< "Play Only on Pass" value.
    bool breakMmRest{};                             ///< Whether the text breaks multimeasure rests.
    bool useAuxData{};                              ///< Whether auxiliary data is used.
    bool masterShape{};                             ///< Whether this expression references the master copy of the shape.
    bool noPrint{};                                 ///< Inverse of "Hidden" checkbox.
    bool noHorzStretch{};                           ///< Inverse of "Allow Horizontal Stretching" checkbox.
    PlaybackType playbackType{};                    ///< Playback behavior of the text expression.
    HorizontalMeasExprAlign horzMeasExprAlign{};    ///< Horizontal alignment of the expression.
    VerticalMeasExprAlign vertMeasExprAlign{};      ///< Vertical alignment of the expression.
    AlignJustify horzExprJustification{};           ///< Horizontal justification of the text expression.
    Evpu measXAdjust{};                             ///< Horizontal adjustment for measurement alignment.
    Evpu yAdjustEntry{};                            ///< Vertical adjustment for entry alignment.
    Evpu yAdjustBaseline{};                         ///< Vertical adjustment for baseline alignment.
    bool useCategoryFonts{};                        ///< Whether to use category fonts (may not be meaningful shape expressions).
    bool useCategoryPos{};                          ///< Whether to use category position.
    std::string description;                        ///< Description of the text expression. (xml node is "descStr")

    /// @brief Get the shape for this shape expression
    MusxInstance<ShapeDef> getShape() const;

    constexpr static std::string_view XmlNodeName = "shapeExprDef"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ShapeExpressionDef>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class SplitMeasure
 * @brief Defines the split point(s) where a measure may be divided between two systems. A measure can only
 * split once, but Finale stores them as an array. Perhaps this was intended to support alternative
 * break positions depending on spacing. See the warning below for the actual behavior.
 *
 * The array entries are Evpu offsets in Scroll View. Determining the exact split location would require
 * converting between Scroll View Evpu positions and beat positions, which in turn depends on understanding
 * beat charts and Finale’s layout process. A crude first approximation may be to divide the split position by
 * #Measure::width, which might yield a usable fraction of the graphical measure width to display on the
 * previous system.
 *
 * @note This is a legacy Finale feature. It was never consistently implemented across the program,
 * and over time it became less useful as newer features were developed without support for it.
 * As a result, split points are rare in Finale files. The older the file, the more likely they are to appear,
 * but even then they remain uncommon.
 *
 * @warning Early versions of Finale could create multiple split points, while recent versions allow only one.
 * Older files that contain multiple split points can still be opened and saved in newer Finale releases (even as
 * recent as 27.4) without removing them. In all versions tested, however, multiple split points lead to crashes
 * or unstable measure layouts. They are effectively unusable in any version of Finale.
 *
 * This class is identified by the XML node name "splitMeas".
 */
class SplitMeasure : public OthersArray<Evpu>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using OthersArray::OthersArray;

    constexpr static std::string_view XmlNodeName = "splitMeas"; ///< The XML node name for this type.
    static const xml::XmlElementArray<SplitMeasure>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StaffList
 * @brief Defines a staff list. See subclasses for how it is used.
 *
 * Each value is a specific staff cmper or one of the #StaffList::FloatingValues.
 */
class StaffList : public OthersArray<StaffCmper>
{
public:
    using OthersArray::OthersArray;

    /// @enum FloatingValues
    /// @brief Defines special assignment values used for floating staff assignments
    enum class FloatingValues : StaffCmper
    {
        TopStaff = -1,          ///< This value means the assignment is to the top staff of any system or part
        BottomStaff = -2        ///< This value means the assignment is to the bottom staff of any system or part
    };

    static const xml::XmlElementArray<StaffList>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StaffListCategoryName
 * @brief This class specified the name of a staff list used by @ ref MarkingCategory.
 *
 * The Cmper is a value from 1 to 8, corresponding to one of the eight canned staff lists available to marking categories.
 */
class StaffListCategoryName : public OthersName
{
public:
    using OthersName::OthersName;

    constexpr static std::string_view XmlNodeName = "categoryStaffListName"; ///< The XML node name for this type.
};

/**
 * @class StaffListCategoryParts
 * @brief This class is used by @ref MarkingCategory to define the staves for parts in a staff list.
 *
 * The Cmper is a value from 1 to 8, corresponding to one of the eight canned staff lists available to marking categories.
 */
class StaffListCategoryParts : public StaffList
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using StaffList::StaffList;

    constexpr static std::string_view XmlNodeName = "categoryStaffListParts"; ///< The XML node name for this type.
};

/**
 * @class StaffListCategoryScore
 * @brief This class is used by @ref MarkingCategory to define the staves for parts in a staff list.
 *
 * The Cmper is a value from 1 to 8, corresponding to one of the eight canned staff lists available to marking categories.
 */
class StaffListCategoryScore : public StaffList
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using StaffList::StaffList;

    constexpr static std::string_view XmlNodeName = "categoryStaffListScore"; ///< The XML node name for this type.
};

/// @brief Copies one marking-category staff list from one document into another.
/// @param target The document to import into.
/// @param source The document whose score list, parts list, and optional name are copied.
/// @param cmper The category staff-list cmper to copy.
/// @param onImported Called for every object this operation creates.
/// @return @p cmper, or std::nullopt if the source has no component at @p cmper or the target
/// already has any category staff-list component there. A refusal imports nothing.
/// @throws std::invalid_argument if @p target or @p source is null. Debug builds assert instead.
[[nodiscard]]
std::optional<Cmper> importCategoryStaffListInto(const DocumentPtr& target, const DocumentPtr& source,
    Cmper cmper, const ImportObjectCallback& onImported = {});

/**
 * @class StaffListRepeatName
 * @brief This class specified the name of a staff list used by @ref RepeatEndingStart, @ref RepeatBack, and @ref TextRepeatAssign.
 *
 * The Cmper is the value of the staff list created in the repeat or text repeat dialog. It ties all components of the staff list together.
 */
class StaffListRepeatName : public OthersName
{
public:
    using OthersName::OthersName;

    constexpr static std::string_view XmlNodeName = "repeatStaffListName"; ///< The XML node name for this type.
};

/**
 * @class StaffListRepeatParts
 * @brief This class is used by repeat classes to define the staves for parts in a staff list.
 *
 * The Cmper is the value of the staff list created in the repeat or text repeat dialog. It ties all components of the staff list together.
 */
class StaffListRepeatParts : public StaffList
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using StaffList::StaffList;

    constexpr static std::string_view XmlNodeName = "repeatStaffListParts"; ///< The XML node name for this type.
};

/**
 * @class StaffListRepeatPartsForced
 * @brief This class is used by repeat classes to define the forced staves for parts in a staff list. Repeats on these
 * staves appear even if the staff has turned them off with #Staff::hideRepeats.
 *
 * The Cmper is the value of the staff list created in the repeat or text repeat dialog. It ties all components of the staff list together.
 */
class StaffListRepeatPartsForced : public StaffList
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using StaffList::StaffList;

    constexpr static std::string_view XmlNodeName = "repeatStaffListPartsOverride"; ///< The XML node name for this type.
};

/**
 * @class StaffListRepeatScore
 * @brief This class is used by repeat classes to define the staves for the score in a staff list.
 *
 * The Cmper is the value of the staff list created in the repeat or text repeat dialog. It ties all components of the staff list together.
 */
class StaffListRepeatScore : public StaffList
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using StaffList::StaffList;

    constexpr static std::string_view XmlNodeName = "repeatStaffListScore"; ///< The XML node name for this type.
};

/**
 * @class StaffListRepeatScoreForced
 * @brief This class is used by repeat classes to define the forced staves for the score in a staff list. Repeats on these
 * staves appear even if the staff has turned them off with #Staff::hideRepeats.
 *
 * The Cmper is the value of the staff list created in the repeat or text repeat dialog. It ties all components of the staff list together.
 */
class StaffListRepeatScoreForced : public StaffList
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using StaffList::StaffList;

    constexpr static std::string_view XmlNodeName = "repeatStaffListScoreOverride"; ///< The XML node name for this type.
};

/**
 * @brief Generic resolver for a family of staff lists.
 *
 * @tparam ScoreList         list class used for score
 * @tparam PartsList         list class used for parts
 * @tparam ScoreForcedList   (optional) forced list class for score
 * @tparam PartsForcedList   (optional) forced list class for parts
 */
template<class ScoreList, class PartsList, class ScoreForcedList = void, class PartsForcedList  = void>
class StaffListSet
{
public:
    /// @brief Constructor function
    StaffListSet(const DocumentPtr& document, Cmper partId, Cmper staffListId) noexcept;

    /// @brief Returns true is the staff list includes a particular staff on a particular system.
    /// @param staffId The StaffCmper to check.
    /// @param systemStaves The staves for a particular system (or #Document::calcScrollViewCmper).
    /// @param isHidden If true, only the forced staff list is checked. Normally you will omit this for categories or pass #Staff::hideRepeats for repeats.
    bool contains(StaffCmper staffId, const MusxInstanceList<StaffUsed>& systemStaves, bool isHidden = false) const noexcept;

private:
    MusxInstance<StaffList> m_staffList;
    MusxInstance<StaffList> m_forcedStaffList;
};

/**
 * @class StaffSystem
 * @brief Represents the attributes of a staff system in the page layout.
 *
 * This class is identified by the XML node name "staffSystemSpec".
 */
class StaffSystem : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit StaffSystem(const DocumentWeakPtr& document, Cmper staffSystemId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, staffSystemId, shareMode, cmper)
    {
    }

    MeasCmper startMeas{};          ///< Starting measure of the staff system. See @ref Measure.
    MeasCmper endMeas{};            ///< The ending measure of the staff system, stored as *one greater* than the last measure
                                    ///< on this system. In other words, it is the first measure of the next system, or one past
                                    ///< the last measure in the document. See @ref Measure.
                                    ///< If #endMeas is a split measure, its first part may still appear on this system depending
                                    ///< on spacing. (Finale does not record this in the data.) If the system is locked, the first
                                    ///< part of the split always appears on this system. See @ref SplitMeasure and @ref SystemLock.
    double horzPercent{};           ///< Horizontal scaling percentage (fractional, 100.0 means no scaling).
                                    ///< This value affects "stretchable" items such as word extensions on lyrics.
    int ssysPercent{};              ///< Staff system scaling percentage (100 means no scaling).
    Efix staffHeight{};             ///< Staff height in Efix units. Note that this is different units than #options::PageFormatOptions::PageFormat::rawStaffHeight.
    Evpu top{};                     ///< Top margin in Evpu. (Sign reversed in Finale UI.)
    Evpu left{};                    ///< Left margin in Evpu.
    Evpu right{};                   ///< Right margin in Evpu. (Sign reversed in Finale UI.)
    Evpu bottom{};                  ///< Bottom margin in Evpu. This value is 96 (i.e., 1 5-line staff thickness) less than the U.I. value.
                                    ///< That means if you enter 0 in the Finale UI, this value is -96.
    bool noNames{};                 ///< Suppresses staff & group names along the left system margin. It does not appear to have a UI in Finale 27,
                                    ///< but it works and it occasionally appears in musx files.
    bool hasStaffScaling{};         ///< Indicates if any individual staff in the system has scaling applied.
    bool placeEndSpaceBeforeBarline{}; ///< Indicates that extra space is placed before the barline.
    bool scaleVert{};               ///< "Resize Vertical Space"
    bool holdMargins{};             ///< "Hold Margins" (xml node is `<scaleContentOnly>`)
    Evpu distanceToPrev{};          ///< Distance to the previous staff system in Evpu.
    Evpu extraStartSystemSpace{};   ///< Extra space at the start of the staff system in Evpu.
    Evpu extraEndSystemSpace{};     ///< Extra space at the end of the staff system in Evpu.

    PageCmper pageId{};             ///< The page this system is on. This values is not in the xml but is computed by the factory.

    /// @brief Encapsulates the weird Finale fact that #endMeas is actually one past the end of the system
    /// @return The actual last measure on the system.
    MeasCmper getLastMeasure() const { return endMeas - 1; }

    /// @brief Calculates the number of measures on the system, encapsulating how #endMeas works.
    /// @return The number of measures on the system.
    int calcNumMeasures() const { return endMeas - startMeas; }

    /// @brief Gets the page this system is on, or nullptr if the page layout is unavailable.
    MusxInstance<others::Page> getPage() const;

    /// @brief Calculate the system scaling.
    util::Fraction calcSystemScaling() const
    { return util::Fraction::fromPercent(ssysPercent) * util::Fraction(staffHeight, 4 * int(EFIX_PER_SPACE)); }

    /// @brief Calculate individual staff scaling for a given staff.
    /// @param staffId The staff to get the scaling for.
    /// @return The staff scaling or 1/1 if the staff is not in the system.
    util::Fraction calcStaffScaling(StaffCmper staffId) const;

    /// @brief Calculates the effective scaling on this system, taking into account page scaling.
    util::Fraction calcEffectiveScaling() const;

    /// @brief Calculates the maximum and minimum staff scaling values for this system by searching each staff
    /// for individual staff scaling.
    /// @return A std::pair containing
    ///         - util::Fraction: The scaling of the staff with the minimum (smallest) scaling factor
    ///         - util::Fraction: The scaling of the staff with the maximum (largest) scaling factor
    std::pair<util::Fraction, util::Fraction> calcMinMaxStaffSizes() const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (startMeas == 0 || endMeas == 0) {
            if (getSourcePartId() == SCORE_PARTID) {
                util::Logger::log(util::Logger::LogLevel::Info, "Layout for system " + std::to_string(getCmper())
                    + " of part " + std::to_string(getSourcePartId()) + " has not been calculated.");
            } else {
                // Finale can retain zero-valued system placeholders until a linked-part layout is updated.
                util::Logger::log(util::Logger::LogLevel::Verbose, "Layout for system " + std::to_string(getCmper())
                    + " of part " + std::to_string(getSourcePartId()) + " has not been calculated.");
            }
        }
    }

    constexpr static std::string_view XmlNodeName = "staffSystemSpec"; ///< The XML node name for this type.
    static const xml::XmlElementArray<StaffSystem>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StaffUsed
 * @brief An array of StaffUsed defines a set of staves in a staff system or in Scroll View.
 *
 * The cmper is either #BASE_SYSTEM_ID (for Scroll View) or the SystemCmper of a system. There
 * are some other special cmper values as well for staff sets in Scroll View or Special Part Extraction
 * or Studio View.
 *
 * This class is identified by the XML node name "instUsed".
 */
class StaffUsed : public OthersBase {
public:
    /** @brief Constructor function */
    explicit StaffUsed(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci) {}

    StaffCmper staffId{};                   ///< Staff cmper (xml node is `<inst>`)
    Evpu distFromTop{};                     ///< Distance from the top of the system (negative is down)
    std::shared_ptr<EnigmaMusicRange> range;      ///< The music range. (Late versions of Finale may always include the entire piece here.)

    /// @brief Calculates the effective scaling on this instance, taking into accound system and page scaling.
    util::Fraction calcEffectiveScaling() const;

    /// @brief Returns the @ref Staff instance for this element, without any staff styles applied
    MusxInstance<Staff> getStaffInstance() const;

    /// @brief Returns the @ref Staff instance for this element with staff styles applied at the specified location.
    MusxInstance<StaffComposite> getStaffInstance(MeasCmper measureId, Edu eduPosition = 0) const;

    constexpr static std::string_view XmlNodeName = "instUsed"; ///< The XML node name for this type.
    static const xml::XmlElementArray<StaffUsed>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class SystemLock
 * @brief Locks a span of one or more measures so that they always appear in a @ref StaffSystem together.
 *
 * The cmper indicates the first measure of the locked system.
 * The endMeas element specifies the first measure after the locked system.
 *
 * The class is identified by the XML node name "lockMeas".
 */
class SystemLock : public OthersBase
{
public:
    /** @brief Constructor function. */
    explicit SystemLock(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    MeasCmper endMeas{}; ///< The first measure after the locked system,
                         ///< or one measure past the end of document if this is the last system.
                         ///< If #endMeas has a split, the first part of the measure is on the locked system
                         ///< and the second part is on the next. See @ref SplitMeasure.

    constexpr static std::string_view XmlNodeName = "lockMeas"; ///< The XML node name for this type.
    static const xml::XmlElementArray<SystemLock>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TempoChange
 * @brief A single tempo change value entered with Finale's Tempo tool.
 *
 * Although the Finale UI dropped the Tempo tool around 2010, the notation engine
 * still supported them. It was always possible (even as late as Finale 27.4) to add tempo
 * changes using third-pary plugins.
 *
 * The most common tool for creating TempoChange instances was the plugin JW Tempo.
 * It only ever used absolute ratios, so the focus of this class is on them.
 *
 * The cmper is the measure number, and incis should be stored in order by #eduPosition.
 *
 * This class is identified by the XML node name "tempoDef".
 */
class TempoChange : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit TempoChange(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci)
    {
    }

    int ratio{};                    ///< Absolute or relative ratio.
                                    ///< Relative ratios are are a percent value * 10. 100% would have a value of 1000.
                                    ///< See #getAbsoluteTempo for how to use absolute ratios.
                                    ///< The xml provides one of two nodes for this field: `<relativeRatio>` or `<absoluteRatio>`.
    Edu eduPosition{};              ///< The position within the measure. (xml node is `<eldur>`)
    int unit{};                     ///< Hardware ticks/sec. For Macs this is 1000.
    bool isRelative{};              ///< A computed value that determines if #ratio is relative or absolute.
                                    ///< This value is not represented in the xml but instead captures whether the #ratio
                                    ///< property was populated from `<relativeRatio>` or `<absoluteRatio>`.

    /// @brief Computes the absolute tempo represented by the TempoChange Instance.
    /// @param noteType optional note type for which to get the beats per minute. (The default is NoteType::Quarter.)
    /// @return The per minute value. Because #ratio is an integer, it cannot encode most tempos exactly.
    /// The value returned is the shortest decimal that converts back to #ratio, which states the encoded
    /// tempo without adding precision the encoding does not carry. It therefore usually has a fractional
    /// part, and callers that need a whole number must round it themselves.
    /// @throws std::logic_error if this instance represents a relative tempo.
    double getAbsoluteTempo(NoteType noteType = NoteType::Quarter) const;

    constexpr static std::string_view XmlNodeName = "tempoDef"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TempoChange>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TextBlock
 * @brief Represents the attributes of a Finale "textBlock".
 *
 * The cmper is the textBlock ID, representing unique text blocks in the Finale document.
 * This class is identified by the XML node name "textBlock".
 */
class TextBlock : public OthersBase
{
public:
    using TextJustify = options::TextOptions::TextJustify; ///< justification options

    /**
     * @enum TextType
     * @brief Enum for textTag values
     */
    enum class TextType
    {
        Block,      ///< #textId is a #Cmper for a @ref texts::BlockText
        Expression  ///< #textId is a #Cmper for a @ref texts::ExpressionText
    };

    /** @brief Constructor function */
    explicit TextBlock(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    // Public properties corresponding to the XML structure
    Cmper textId{};                    ///< Cmper of the text block. (xml tag is `<textID>`)
    Cmper shapeId{};                   ///< If non-zero, the Cmper of the custom frame shape. (xml tag is `<shapeID>`)
    Evpu width{};                      ///< Width of standard frame. If zero, the width expands to fit the text.
    Evpu height{};                     ///< Height of standard frame. If zero, the height expands to fit the text.
    /// @brief "Line Spacing: Automatic" percent value.
    ///
    /// Line spacing is either a percent or an absolute distance, and the xml carries whichever applies.
    /// Exactly one of this and #lineSpacingEvpu has a value once #integrityCheck has run.
    std::optional<int> lineSpacingPercentage;
    std::optional<Evpu> lineSpacingEvpu;  ///< "Line Spacing: Set To" absolute value. (See #lineSpacingPercentage.)
    Evpu xAdd{};                       ///< Horizontal offset from handle.
    Evpu yAdd{};                       ///< Vertical offset from handle.
    TextJustify justify{};             ///< Justification (left, center, right, full, force full)
    bool newPos36{};                   ///< "Position from Edge of Frame" compatibility setting.
                                       ///< Best guess is that blocks created before Finale 3.6 do not have this set.
                                       ///< It affects the vertical position of the baseline relative to the block's frame (and handle).
    bool showShape{};                  ///< Show shape
    bool noExpandSingleWord{};         ///< Do not expand single word
    bool wordWrap{};                   ///< Wrap words (in frames)
    Efix inset{};                      ///< Text inset from frame (all sides)
    Efix stdLineThickness{};           ///< Thickness of standard frame line. (xml tag is `<stdLine>`)
    bool roundCorners{};               ///< Use rounded corners on frame
    Efix cornerRadius{};               ///< Corner radius for rounded corners.
    TextType textType{};               ///< Text tag indicating the type of text block. (xml tag is `<textTag>`)

    /// @brief Gets the raw text block context (from the `texts` pool) based on #textType.
    /// @param forPartId The linked part to use for ^partname and ^totpages inserts
    /// @param forPageId The default value to use for ^page inserts. If omitted, the default value is "#", which mimics Finale's behavior.
    /// @param defaultInsertFunc The default text insert replacement function for this context. This function is called if the function supplied
    /// to #util::EnigmaParsingContext::parseEnigmaText returns std::nullopt.
    util::EnigmaParsingContext getRawTextCtx(Cmper forPartId, std::optional<Cmper> forPageId = std::nullopt,
        util::EnigmaString::TextInsertCallback defaultInsertFunc = util::EnigmaString::defaultInsertsCallback) const;

    /** @brief return displayable text with Enigma tags removed */
    static std::string getText(const DocumentPtr& document, const Cmper textId, Cmper forPartId, bool trimTags = false,
        util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii);

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->OthersBase::integrityCheck(ptrToThis);
        if (lineSpacingPercentage && lineSpacingEvpu) {
            lineSpacingEvpu.reset();
            MUSX_INTEGRITY_ERROR("Text options specify both a percent and an absolute line spacing. "
                "The absolute value was discarded.");
        } else if (!lineSpacingPercentage && !lineSpacingEvpu) {
            // this is what Finale reads when neither lineSpacingPercentage nor lineSpacingEvpu is present
            lineSpacingEvpu = 0;
        }
    }

    constexpr static std::string_view XmlNodeName = "textBlock"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TextBlock>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TextExpressionDef
 * @brief Stores the properties and behaviors of text expressions.
 *
 * This class is identified by the XML node name "textExprDef".
 */
class TextExpressionDef : public OthersBase {
public:
    /**
     * @brief Constructor.
     *
     * Initializes all fields to their default values.
     */
    explicit TextExpressionDef(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    Cmper textIdKey{};                              ///< Identifier for the @ref TextBlock associated with this
    Cmper categoryId{};                             ///< Identifier for the category of the text expression.
    RehearsalMarkStyle rehearsalMarkStyle{};        ///< Auto-sequencing style for rehearsal marks.
    int value{};                                    ///< Value associated with the expression (e.g., velocity).
    Cmper execShape{};                              ///< Executable shape Cmper for playback (@ref ShapeDef)
    int auxData1{};                                 ///< Auxiliary data for the expression. (xml node is "auxdata1")
    int playPass{};                                 ///< "Play Only on Pass" value.
    bool hideMeasureNum{};                          ///< "Hide Measure Numbers" (used on Rehearsal Marks)
    bool matchPlayback{};                           ///< "Match Playback to Metronome Marking Text"
                                                    ///< This may be a user interface feature that keeps #value in step with
                                                    ///< the metronome marking in the expression text as the text is edited,
                                                    ///< rather than a setting the playback engine consults. It does not
                                                    ///< appear to affect #auxData1, which continues to supply the beat unit.
    bool useAuxData{};                              ///< Whether auxiliary data is used.
    bool hasEnclosure{};                            ///< Whether the text expression has an enclosure. (xml node is "newEnclosure")
    bool breakMmRest{};                             ///< Whether the text breaks multimeasure rests.
    bool createdByHp{};                             ///< Whether the text block was created by Finale's smart playback system
    PlaybackType playbackType{};                    ///< Playback behavior of the text expression.
    HorizontalMeasExprAlign horzMeasExprAlign{};    ///< Horizontal alignment of the expression.
    VerticalMeasExprAlign vertMeasExprAlign{};      ///< Vertical alignment of the expression.
    AlignJustify horzExprJustification{};           ///< Horizontal justification of the text expression.
    Evpu measXAdjust{};                             ///< Horizontal adjustment for measurement alignment.
    Evpu yAdjustEntry{};                            ///< Vertical adjustment for entry alignment.
    Evpu yAdjustBaseline{};                         ///< Vertical adjustment for baseline alignment.
    bool useCategoryFonts{};                        ///< Whether to use category fonts.
    bool useCategoryPos{};                          ///< Whether to use category position.
    std::string description;                        ///< Description of the text expression. (xml node is "descStr")

    /** @brief Gets the TextBlock for this expression, or nullptr if none. */
    MusxInstance<TextBlock> getTextBlock() const;

    /**
     * @brief Gets the raw text context for parsing this expression, or nullptr if none.
     * @param forPartId The linked part to use for ^partname and ^totpages inserts.
     * @param defaultInsertFunc An optional outer handler layer. If it returns `std::nullopt`, the
     * expression-definition defaults for `^value`, `^control`, and `^pass` are tried next.
    */
    util::EnigmaParsingContext getRawTextCtx(Cmper forPartId,
        util::EnigmaString::TextInsertCallback defaultInsertFunc = util::EnigmaString::defaultInsertsCallback) const;

    /** @brief Gets the enclosure for this expression, or nullptr if none. */
    MusxInstance<Enclosure> getEnclosure() const;

    constexpr static std::string_view XmlNodeName = "textExprDef"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TextExpressionDef>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TextExpressionEnclosure
 * @brief The enclosure for a text expression (if it exists)
 *
 * The cmper is the same as for the associated @ref TextExpressionDef`.
 *
 * This class is identified by the XML node name "textExpressionEnclosure".
 */
class TextExpressionEnclosure : public Enclosure {
public:
    using Enclosure::Enclosure;

    constexpr static std::string_view XmlNodeName = "textExpressionEnclosure"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TextExpressionEnclosure>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TextRepeatAssign
 * @brief Represents a text repeat assignment with positioning and behavior properties.
 *
 * The cmper is the cmper of the @ref Measure that has this item.
 *
 * This class is identified by the XML node name "textRepeatAssign".
 */
class TextRepeatAssign : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit TextRepeatAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci)
    {
    }

    // Public properties corresponding to the XML structure
    Evpu horzPos{};                 ///< The horizontal offset from default of the text repeat marker.
    int passNumber{};               ///< Play Section N Times, Jump on Pass, Stop on Pass value, depending on #jumpAction and #trigger. (xml tag is `<actuate>`)
    int targetValue{};              ///< Measure number, TextRepeatDef ID, or offset, depending on #jumpAction. (xml tag is `<target>`)
    Cmper textRepeatId{};           ///< The Cmper of the assigned @ref TextRepeatDef. (xml tag is `<repnum>`)
    Evpu vertPos{};                 ///< The vertical offset from default of the text repeat marker.
    bool individualPlacement{};     ///< "Allow Individual Edits Per Staff" (xml tag is `<indivPlac>`)
    bool topStaffOnly{};            ///< "Show On: Top Staff Only"
    bool hidden{};                  ///< Inverse of "Show" checkbox
    bool resetOnAction{};           ///< "Reset on Repeat Action" (xml tag is `<clrOnChange>`)
    bool jumpOnMultiplePasses{};    ///< If true, use #TextRepeatDef::passList to get the passes and ignore #passNumber. (xml tag is `<multiActuate>`)
    RepeatActionType jumpAction{};  ///< The jump action for this repeat assignment. (xml tag is `<action>`)
    bool autoUpdate{};              ///< "Auto-Update Target"
    RepeatTriggerType trigger{};    ///< The condition that triggers the #jumpAction.
    bool jumpIfIgnoring{};          ///< "Jump if Ignoring Repeats" (xml tag is `<jmpIgnore>`)
    Cmper staffList{};              ///< If non-zero, specifies a staff list for which staves to show the ending.

    /// @brief Returns the @ref RepeatIndividualPositioning record for a given staff.
    /// Check #individualPlacement to see if this record is used by Finale.
    /// @param staffId The StaffCmper of the staff to search for individual positioning.
    /// @return The individual positioning record or nullptr if not found.
    MusxInstance<RepeatIndividualPositioning> getIndividualPositioning(StaffCmper staffId) const;

    /// @brief Create a @ref StaffListSet for the given instance. This can be used to interrogate whether a staff appears in the staff set.
    /// @return The created staff list set. If #staffList is zero, it will never find any staves for the staff list.
    RepeatStaffListSet createStaffListSet() const;

    /// @brief Calculates the target measure for this text repeat assignment, if any.
    /// @details Returns the explicit or relative target when the action specifies a concrete
    /// measure. For JumpToMark, it resolves the target repeat ID to the referenced assignment.
    /// Returns std::nullopt for actions that do not provide a measure target.
    /// @return The target measure for the jump, or std::nullopt if none applies.
    std::optional<MeasCmper> calcTargetMeasure() const;

    constexpr static std::string_view XmlNodeName = "textRepeatAssign"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TextRepeatAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TextRepeatDef
 * @brief Defines text repeat elements with font styling and justification.
 *
 * Text repeat elements are indications like "D.S. al Fine", "Segno", or "To Coda" that
 * have playback jumps associated with them.
 *
 * The cmper is the unique identifier used in the document.
 *
 * This class is identified by the XML node name "textRepeatDef".
 */
class TextRepeatDef : public OthersBase
{
public:
    /**
     * @enum PoundReplaceOption
     * @brief Enum for poundReplace options
     */
    enum class PoundReplaceOption
    {
        Passes,             ///< "Number of Times Played" (the default: may never appear in xml)
        RepeatID,           ///< "Text Repeat ID in Target" (xml value is "repeatID")
        MeasureNumber       ///< "Measure Number in Target" (xml value is "measNum")
    };

    /** @brief Constructor function */
    explicit TextRepeatDef(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper), font(new FontInfo(document))
    {
    }

    // Public properties corresponding to the XML structure
    std::shared_ptr<FontInfo> font;                 ///< The font for this text repeat. (xml nodes `<fontID>`, `<fontSize>`, and `<efx>`)
    bool hasEnclosure{};                            ///< Whether the text repeat has an enclosure. (xml node is `<newEnclosure>`)
    bool useThisFont{};                             ///< "Use This Font" (for the `#` substitution)
    PoundReplaceOption poundReplace{};              ///< "Replace # With" choice.
    AlignJustify justification{};                   ///< Although called "justification" in Finale's U.I, this value is used
                                                    ///< for both the alignment of the text within the measure as well as its justification.
                                                    ///< (xml node is `<justify >`)
    std::vector<int> passList;                      ///< If this vector contains elements, they define the repeat passes that apply to this instance.

    constexpr static std::string_view XmlNodeName = "textRepeatDef"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TextRepeatDef>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TextRepeatEnclosure
 * @brief The enclosure for a text expression (if it exists)
 *
 * The cmper is the same as for @ref TextRepeatDef.
 *
 * This class is identified by the XML node name "textRepeatEnclosure".
 */
class TextRepeatEnclosure : public Enclosure
{
public:
    using Enclosure::Enclosure;

    constexpr static std::string_view XmlNodeName = "textRepeatEnclosure"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TextRepeatEnclosure>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TextRepeatText
 * @brief Represents the text associated with a @ref TextRepeatDef
 *
 * The cmper is the same as for @ref TextRepeatDef.
 *
 * This class is identified by the XML node name "textRepeatText".
 */
class TextRepeatText : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit TextRepeatText(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper)
    {
    }

    std::string text; ///< The text

    constexpr static std::string_view XmlNodeName = "textRepeatText"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TextRepeatText>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TimeCompositeLower
 * @brief Represents the lower composite time signature array.
 *
 * The cmper taken from the `unit` value when `hasCompositeBottom` is true. (See @ref TimeSignature.)
 *
 * This class is identified by the XML node name "timeLower".
 */
class TimeCompositeLower : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit TimeCompositeLower(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper)
    {
    }

    /** @class CompositeItem
     *  @brief Represents an individual lower composite item in the time signature.
     */
    class CompositeItem
    {
    public:
        Edu unit{};             ///< The size of the unit for this item. (xml node is `<integer>`)
        bool startGroup{};      ///< Indicates the start of a group.

        static const xml::XmlElementArray<CompositeItem>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    std::vector<std::shared_ptr<CompositeItem>> items;  ///< composite items collection

    constexpr static std::string_view XmlNodeName = "timeLower"; ///< XML node name.
    static const xml::XmlElementArray<TimeCompositeLower>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TimeCompositeUpper
 * @brief Represents the upper composite time signature structure.
 *
 * The cmper taken from the `beats` value when `hasCompositeTop` is true. (See @ref TimeSignature.)
 *
 * This class is identified by the XML node name "timeUpper".
 */
class TimeCompositeUpper : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit TimeCompositeUpper(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper)
    {
    }

    /** @class CompositeItem
     *  @brief Represents an individual composite item in the time signature.
     */
    class CompositeItem
    {
    public:
        Edu beats{};                ///< The number of beats in this item. (xml node is `<integer>`)
        util::Fraction fraction{};  ///< Fraction of beats (between 0 <= fraction < 1)
        bool startGroup{};          ///< Indicates the start of a group.

        /// @brief Return the beats as a complete fraction
        util::Fraction fullFraction() const { return fraction + beats; }

        static const xml::XmlElementArray<CompositeItem>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /// Collection of composite items
    std::vector<std::shared_ptr<CompositeItem>> items;  ///< composite items collection

    constexpr static std::string_view XmlNodeName = "timeUpper"; ///< XML node name.
    static const xml::XmlElementArray<TimeCompositeUpper>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TonalCenterFlats
 * @brief Maps the number of flats to a tonal center for a linear custom key. If there are zero flats or sharps,
 * do not use this table. Use @ref TonalCenterSharps for zero. If the key has flats, the number is returned as a negative
 * number (-1..-7) from #KeySignature::getAlteration. Use the absolute value of that number to index this table.
 *
 * The cmper is the value returned by #KeySignature::getKeyMode. The built-in major and minor
 * cmpers (0 and 1) ignore this table, and it should not be present for those values. (However, with Finale
 * anything is possible.)
 *
 * This class is identified by the XML node name "fstKeyFlats".
 */
class TonalCenterFlats : public OthersArray<unsigned, 8>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using OthersArray::OthersArray;

    constexpr static std::string_view XmlNodeName = "fstKeyFlats"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TonalCenterFlats>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TonalCenterSharps
 * @brief Maps number of sharps (0..7) to a tonal center for a linear custom key. Also maps 0 sharps or flats.
 * For a non-linear key, the first element is the tonal center and the rest are meaningless.
 *
 * The cmper is the value returned by #KeySignature::getKeyMode. The built-in major and minor
 * cmpers (0 and 1) ignore this table, and it should not be present for those values. (However, with Finale
 * anything is possible.)
 *
 * This class is identified by the XML node name "fstKeySharps".
 */
class TonalCenterSharps : public OthersArray<unsigned, 8>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using OthersArray::OthersArray;

    constexpr static std::string_view XmlNodeName = "fstKeySharps"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TonalCenterSharps>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

} // namespace others
} // namespace dom
} // namespace musx
