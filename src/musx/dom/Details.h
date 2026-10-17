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
#include <vector>
#include <unordered_set>
#include <cmath>
#include <map>

#include "music_theory/music_theory.hpp"

#include "musx/util/EnigmaString.h"
#include "musx/util/Logger.h"
#include "musx/util/PseudoTieUtils.h"
#include "MusxInstance.h"
#include "BaseClasses.h"
#include "CommonClasses.h"
#include "Options.h"
#include "Others.h"
#include "Staff.h"
 // do not add other dom class dependencies. Use Implementations.h for implementations that need total class access.

namespace musx {
namespace dom {

class EntryInfoPtr;
class EntryFrame;
class EntryInfo;

namespace others {
class ChordSuffixElement;
class FretboardGroup;
class FretInstrument;
class FretboardStyle;
class StaffUsed;
class Measure;
class MultiStaffInstrumentGroup;
class TextBlock;
} // namespace others

namespace texts {
class ExpressionText;
class LyricsChorus;
class LyricsSection;
class LyricsVerse;
} // namespace others

/**
 * @namespace musx::dom::details
 * @brief Classes in the @ref DetailsPool.
 */
namespace details {

/**
 * @class AccidentalAlterations
 * @brief Represents display alterations to an accidental for a specific note.
 *
 * #Entry::noteDetail is set if any note in the entry has visual alterations such as adjusted accidental appearance or position.
 *
 * This class is identified by the XML node name "acciAlter".
 */
class AccidentalAlterations : public NoteDetailsBase
{
public:
    /** @brief Constructor */
    explicit AccidentalAlterations(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : NoteDetailsBase(document, partId, shareMode, entnum, inci), customFont(std::make_shared<FontInfo>(document))
    {
    }

    NoteNumber noteId{};                            ///< The ID of the note being altered.
    int percent{};                                  ///< The percentage size for the accidental, where 100 is 100%.
    Evpu vOffset{};                                 ///< Vertical offset: positive is up. (XML node: `<ayDisp>`)
    Evpu hOffset{};                                 ///< Horizontal offset: positive is right. (XML node: `<axDisp>`)
    char32_t altChar{};                             ///< If non-zero, the character to use for the accidental. (Utf-32 if the font is a Unicode font.)
    std::shared_ptr<FontInfo> customFont;           ///< Font settings for the accidental (populated from `<fontID>`, `<fontSize>`, and `<efx>`)
    bool useOwnFont{};                              ///< Whether to use #customFont.
    bool allowVertPos{};                            ///< Whether to use #vOffset.

    NoteNumber getNoteId() const override { return noteId; }

    constexpr static std::string_view XmlNodeName = "acciAlter";   ///< The XML node name for this type.
    static const xml::XmlElementArray<AccidentalAlterations>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ArticulationAssign
 * @brief Assigns an articulation to an entry.
 *
 * #Entry::articDetail is set if the entry has any articulations.
 */
class ArticulationAssign : public EntryDetailsBase
{
public:
    /// @class SelectedSymbolContext
    /// @brief Provides information about the best analysis as to which articulation symbol in being used on this assignment.
    /// @details The symbol may be the main or alternate symbol, and it may be either a codepoint or a shape id. For automatically
    /// positioned articulations, see the caveats at #calcPlacementAbove.
    struct SelectedSymbolContext
    {
        MusxInstance<others::ArticulationDef> definition;   ///< The associated articulation definition.
        others::ArticulationDef::SelectedSymbol symbol;     ///< Information about the selected symbol.
        /// @brief The resolved placement for the selected symbol.
        VerticalPlacement placement = VerticalPlacement::NotApplicable;
    };

private:
    struct PseudoTieShapeContext
    {
        utils::PseudoTieShapeInfo info;
        MusxInstance<others::ArticulationDef> definition;
        others::ArticulationDef::SelectedSymbol selectedSymbol;
    };

public:
    /**
     * @brief Constructor function
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (probably always 0).
     * @param shareMode The sharing mode for this @ref ArticulationAssign (probably always #ShareMode::All)
     * @param entnum The entry number of this assignment.
     * @param inci The 0-based inci. Each articulation assignment has a separate instance.
     */
    explicit ArticulationAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : EntryDetailsBase(document, partId, shareMode, entnum, inci) {}

    Cmper articDef{};               ///< The articulation definition ID.
    Evpu horzOffset{};              ///< Horizontal offset from the default position.
    Evpu horzAdd{};                 ///< Horizontal displacement for character copying. (E.g., trill lines)
    Evpu vertOffset{};              ///< Vertical offset from the default position.
    Evpu vertAdd{};                 ///< Vertical displacement for character copying. (E.g., arpeggiated chord lines)
    bool overridePlacement{};       ///< Whether the default placement is overridden.
    bool aboveEntry{};              ///< When #overridePlacement is true: whether the articulation is forced above or below the entry.
    bool hide{};                    ///< Inverse of "Show" option.
    bool neverStack{};              ///< Whether the articulation should never stack relative to others.
    bool avoidSlur{};               ///< Whether the articulation should avoid slurs.
    int numSlursAvoided{};          ///< Number of slurs avoided. Used internally by Finale's stacking algorithm.

    /// @brief Heuristically finds the note in the entry that this articulation assignment appears to be associated with.
    /// @details Finale cannot model an articulation attached to a specific note of a chord, but users approximate it by
    /// giving the articulation definition manual vertical positioning and setting #vertOffset so that the symbol aligns
    /// with the staff position of the target note, where zero is the staff position of the note furthest from the stem.
    /// This function reverses that convention: it rounds #vertOffset to the nearest staff position and returns the note
    /// (if any) at exactly that position. Definition-level offsets are ignored, since they orient the symbol relative
    /// to the selected note rather than select it.
    /// @note This is a heuristic classification aid. Combine it with other signals (such as the glyph) before concluding
    /// that an articulation is note-associated.
    /// @param entryInfo The entry to which this articulation is assigned. Its entry number must match this
    /// assignment's entry number.
    /// @return The associated note, or an empty NoteInfoPtr if the heuristic does not identify one.
    /// @throws std::logic_error if @p entryInfo does not match this assignment's entry number.
    [[nodiscard]] NoteInfoPtr calcAssociatedNote(const EntryInfoPtr& entryInfo) const;

    /// @brief Calculates the pseudo-tie shape info if this articulation is acting as a pseudo tie for the specified mode.
    /// @return The pseudo-tie shape info when a qualifying shape exists; otherwise std::nullopt.
    [[nodiscard]] std::optional<utils::PseudoTieShapeInfo> calcIsPseudoTie(utils::PseudoTieMode mode,
        const EntryInfoPtr& forStartEntry) const;

    /// @brief Resolves the symbol information used by this articulation assignment on the specified entry.
    /// @details The returned symbol context reflects the assignment-level placement semantics used by this library. Manual placement
    /// cases are evaluated exactly from the assignment data. Automatic placement cases use the best available policy-based
    /// interpretation of Finale's settings and may differ from Finale's final rendered side after layout-dependent positioning
    /// or manual dragging.
    /// @return The resolved symbol context, or std::nullopt if the entry context or articulation definition cannot be resolved.
    [[nodiscard]] std::optional<SelectedSymbolContext> calcSelectedSymbolContext(const EntryInfoPtr& entryInfo) const;

    static const xml::XmlElementArray<ArticulationAssign>& xmlMappingArray();   ///< Required for musx::factory::FieldPopulator.
    constexpr static std::string_view XmlNodeName = "articAssign"; ///< The XML node name for this type.

private:
    /// @brief Calculates whether the articulation should be treated as placed above the entry.
    /// @details This returns the semantic placement used for articulation behavior, not a guaranteed reconstruction of
    /// Finale's final rendered position in every automatic-layout scenario. Manual placement cases are evaluated exactly
    /// from the assignment data. Automatic placement cases use the best available policy-based interpretation of Finale's
    /// settings and may differ from Finale's final rendered side after layout-dependent positioning or manual dragging.
    [[nodiscard]] bool calcPlacementAbove(const MusxInstance<others::ArticulationDef>& definition,
        const EntryInfoPtr& entryInfo) const;
    /// @brief Resolves the placement semantics for this articulation assignment on the specified entry.
    /// @details This distinguishes fixed above/below placement from context-dependent floating placement and respects
    /// assignment-level overrides and Finale's multi-layer stem-side modifier for otherwise fixed above/below modes.
    [[nodiscard]] VerticalPlacement calcVerticalPlacement(const MusxInstance<others::ArticulationDef>& definition,
        const EntryInfoPtr& entryInfo) const;
    [[nodiscard]] std::optional<PseudoTieShapeContext> calcPseudoTieShapeContext(const EntryInfoPtr& forStartEntry) const;
};

/**
 * @class Baseline
 * @brief Contains the baseline information for all baseline types
 *
 * `cmper1` and `cmper2` are used as follows.
 *
 * | cmper1   | cmper2  | Meaning                                                       | Related classes   |
 * |:--------:|:-------:|:--------------------------------------------------------------|:------------------|
 * | `0`      | `0`     | Global baseline                                               | `Baseline*`       |
 * | `0`      | `staff` | Staff baseline                                                | `Baseline*`       |
 * | `system` | `staff` | System & staff baseline (a specific staff within a system)    | `BaselineSystem*` |
 *
 */
class Baseline : public DetailsBase
{
public:
    /**
     * @brief Constructor function
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (probably always 0).
     * @param shareMode The sharing mode for this @ref Baseline.
     * @param system For system baselines, the system number. For global baselines, 0.
     * @param staff For staff-level baselines, the staff number. For global baselines, 0.
     * @param inci The 0-based inci. (For lyrics baselines only. Others use std::nullopt.)
     */
    explicit Baseline(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper system, Cmper staff, std::optional<Inci> inci)
        : DetailsBase(document, partId, shareMode, system, staff, inci) {}

    Evpu baselineDisplacement{};        ///< The displacement of the baseline from its default position. (xml node is `<basedisp>`)
                                        ///< @note The cumulative sum of all displacements is measured relative to value returned by #others::Staff::calcBaselineZeroPosition.
                                        ///< The full calculation is encapsulated in the #others::Staff::calcBaselinePosition method.
    std::optional<Cmper> lyricNumber{}; ///< The 1-based text number of the lyric, if this is a lyrics baseline. Otherwise unused and should be std::nullopt.

    static const xml::XmlElementArray<Baseline>& xmlMappingArray();   ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class BaselineNoInci
 * @brief Encapsulates inci handling for non-inci baselines.
 */
class BaselineNoInci : public Baseline
{
public:
    /**
     * @brief Constructor function
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (probably always 0).
     * @param shareMode The sharing mode for this @ref Baseline.
     * @param system For system baselines, the system number. For global baselines, 0.
     * @param staff For staff-level baselines, the staff number. For global baselines, 0.
     */
    explicit BaselineNoInci(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper system, Cmper staff)
        : Baseline(document, partId, shareMode, system, staff, std::nullopt) {}
};

/**
 * @class BaselineSystemChords
 * @brief Contains the baseline offsets for chords.
 */
class BaselineSystemChords : public BaselineNoInci
{
public:
    using BaselineNoInci::BaselineNoInci;

    constexpr static std::string_view XmlNodeName = "baselinesSysChords"; ///< The XML node name for this type.
};

/**
 * @class BaselineSystemExpressionsAbove
 * @brief Contains the baseline offsets for expressions above the staff.
 */
class BaselineSystemExpressionsAbove : public BaselineNoInci
{
public:
    using BaselineNoInci::BaselineNoInci;

    constexpr static std::string_view XmlNodeName = "baselinesSysExprAboveStaff"; ///< The XML node name for this type.
};

/**
 * @class BaselineSystemExpressionsBelow
 * @brief Contains the baseline offsets for expressions below the staff.
 */
class BaselineSystemExpressionsBelow : public BaselineNoInci
{
public:
    using BaselineNoInci::BaselineNoInci;

    constexpr static std::string_view XmlNodeName = "baselinesSysExprBelowStaff"; ///< The XML node name for this type.
};

/**
 * @class BaselineSystemFretboards
 * @brief Contains the baseline offsets for fretboards.
 */
class BaselineSystemFretboards : public BaselineNoInci
{
public:
    using BaselineNoInci::BaselineNoInci;

    constexpr static std::string_view XmlNodeName = "baselinesSysFingerboards"; ///< The XML node name for this type.
};

/**
 * @class BaselineSystemLyricsChorus
 * @brief Contains the baseline offsets for lyrics chorus records.
 */
class BaselineSystemLyricsChorus : public Baseline
{
public:
    using Baseline::Baseline;

    constexpr static std::string_view XmlNodeName = "baselinesSysLyricsChorus"; ///< The XML node name for this type.
};

/**
 * @class BaselineSystemLyricsSection
 * @brief Contains the baseline offsets for lyrics section records.
 */
class BaselineSystemLyricsSection : public Baseline
{
public:
    using Baseline::Baseline;

    constexpr static std::string_view XmlNodeName = "baselinesSysLyricsSection"; ///< The XML node name for this type.
};

/**
 * @class BaselineSystemLyricsVerse
 * @brief Contains the baseline offsets for lyrics verse records.
 */
class BaselineSystemLyricsVerse : public Baseline
{
public:
    using Baseline::Baseline;

    constexpr static std::string_view XmlNodeName = "baselinesSysLyricsVerse"; ///< The XML node name for this type.
};

/**
 * @class BaselineChords
 * @brief Contains the baseline offsets for chords.
 *
 * System (cmper1) is always zero.
 */
class BaselineChords : public BaselineNoInci
{
public:
    using BaselineNoInci::BaselineNoInci;

    using PerSystemType = BaselineSystemChords; ///< The associated type for staff records on each system
    constexpr static std::string_view XmlNodeName = "baselinesChords"; ///< The XML node name for this type.
};

/**
 * @class BaselineExpressionsAbove
 * @brief Contains the baseline offsets for expressions above the staff.
 *
 * System (cmper1) is always zero.
 */
class BaselineExpressionsAbove : public BaselineNoInci
{
public:
    using BaselineNoInci::BaselineNoInci;

    using PerSystemType = BaselineSystemExpressionsAbove; ///< The associated type for staff records on each system
    constexpr static std::string_view XmlNodeName = "baselinesExprAboveStaff"; ///< The XML node name for this type.
};

/**
 * @class BaselineExpressionsBelow
 * @brief Contains the baseline offsets for expressions below the staff.
 *
 * System (cmper1) is always zero.
 */
class BaselineExpressionsBelow : public BaselineNoInci
{
public:
    using BaselineNoInci::BaselineNoInci;

    using PerSystemType = BaselineSystemExpressionsBelow; ///< The associated type for staff records on each system
    constexpr static std::string_view XmlNodeName = "baselinesExprBelowStaff"; ///< The XML node name for this type.
};

/**
 * @class BaselineFretboards
 * @brief Contains the baseline offsets for fretboards.
 *
 * System (cmper1) is always zero.
 */
class BaselineFretboards : public BaselineNoInci
{
public:
    using BaselineNoInci::BaselineNoInci;

    using PerSystemType = BaselineSystemFretboards; ///< The associated type for staff records on each system
    constexpr static std::string_view XmlNodeName = "baselinesFingerboards"; ///< The XML node name for this type.
};

/**
 * @class BaselineLyricsChorus
 * @brief Contains the baseline offsets for lyrics chorus records.
 *
 * System (cmper1) is always zero.
 */
class BaselineLyricsChorus : public Baseline
{
public:
    using Baseline::Baseline;

    using PerSystemType = BaselineSystemLyricsChorus; ///< The associated type for staff records on each system
    using TextType = texts::LyricsChorus; ///< The text type for this item.
    constexpr static std::string_view XmlNodeName = "baselinesLyricsChorus"; ///< The XML node name for this type.
};

/**
 * @class BaselineLyricsSection
 * @brief Contains the baseline offsets for lyrics section records.
 *
 * System (cmper1) is always zero.
 */
class BaselineLyricsSection : public Baseline
{
public:
    using Baseline::Baseline;

    using PerSystemType = BaselineSystemLyricsSection; ///< The associated type for staff records on each system
    using TextType = texts::LyricsSection; ///< The text type for this item.
    constexpr static std::string_view XmlNodeName = "baselinesLyricsSection"; ///< The XML node name for this type.
};

/**
 * @class BaselineLyricsVerse
 * @brief Contains the baseline offsets for lyrics verse records.
 *
 * System (cmper1) is always zero.
 */
class BaselineLyricsVerse : public Baseline
{
public:
    using Baseline::Baseline;

    using PerSystemType = BaselineSystemLyricsVerse; ///< The associated type for staff records on each system
    using TextType = texts::LyricsVerse; ///< The text type for this item.
    constexpr static std::string_view XmlNodeName = "baselinesLyricsVerse"; ///< The XML node name for this type.
};

/**
 * @class BeamAlterations
 * @brief Represents beam alterations applied to a specific entry. This is used to apply additional shaping or offset
 * values both primary and secondary beams. Which stem direction this instance controls is determined by the subclass that inherits
 * this class as a base.
 */
class BeamAlterations : public EntryDetailsBase
{
private:
    bool m_active = true; // this value is set by the factory.

protected:
    /// @brief Implementation of #SecondaryBeamAlterationsDownStem::calcIsFeatheredBeam and #SecondaryBeamAlterationsUpStem::calcIsFeatheredBeam.
    template <typename SecondaryBeamType>
    static bool calcIsFeatheredBeamImpl(const EntryInfoPtr& entryInfo, Evpu& outLeftY, Evpu& outRightY);

    /// @brief Generalized getter for secondary beam type arrays for either stem direction.
    template <typename SecondaryBeamType>
    static MusxInstanceList<SecondaryBeamType> getSecondaryBeamArray(const EntryInfoPtr& entryInfo);

public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part this is for.
     * @param shareMode The sharing mode.
     * @param entnum The entry number this applies to.
     * @param inci The inci (if supplied)
     */
    explicit BeamAlterations(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, std::optional<Inci> inci = std::nullopt)
        : EntryDetailsBase(document, partId, shareMode, entnum, inci) {}

    /// @brief see @ref options::BeamOptions::FlattenStyle
    using FlattenStyle = options::BeamOptions::FlattenStyle;

    Evpu leftOffsetH{};         ///< Horizontal adjustment of the beam start. (xml node is `<xAdd>`)
    Evpu leftOffsetY{};         ///< Vertical adjustment of the beam start. (xml node is `<yAdd>`)
    Evpu rightOffsetH{};        ///< Horizontal adjustment of the beam end. (xml node is `<sxAdd>`)
    Evpu rightOffsetY{};        ///< Vertical adjustment of the beam end. (xml node is `<syAdd>`)
    Edu dura{};                 ///< For secondary beams, specifies the duration corresponding to the secondary beam (16th beam is 256, 32nd beam is 128, etc.)
    FlattenStyle flattenStyle{}; ///< Beam shaping style (xml node is `<context>`).
    Efix beamWidth{};            ///< Beam width. A value of -1 indicates to use the default beam width from @ref options::BeamOptions.
                                ///< The #calcEffectiveBeamWidth method handles this for you.

    /// @brief Calculates the effective beam width by returning either the default width or the width override value specified by #beamWidth.
    /// @note The #beamWidth of primary beams also controls the width of secondary beams.
    /// @return The effective beam width of this beam, taking into account all conditions that prefer the default width to the value in #beamWidth.
    Efix calcEffectiveBeamWidth() const;

    /// @brief Returns whether this beam alteration record is active. Its #flattenStyle must match the value in @ref options::BeamOptions.
    /// @return True if active, otherwise false.
    bool isActive() const { return m_active; }

    /// @brief Used by the factory to set active indicators
    template <typename T,
              std::enable_if_t<std::is_base_of_v<BeamAlterations, T>, int> = 0>
    static void calcAllActiveFlags(const DocumentPtr& document);

    /// @brief Gets the beam extension record for the specified @p stemSelection
    /// @param entryInfo The entry to search.
    /// @param stemSelection The choice of which stem version to get.
    static MusxInstance<BeamAlterations> getPrimaryForStem(const EntryInfoPtr& entryInfo, StemSelection stemSelection = StemSelection::MatchEntry);  // must be defined in Details.cpp

    /// @brief Gets the beam extension record for the specified @p stemSelection
    /// @param entryInfo The entry to search.
    /// @param dura The duration corresponding to the secondary beam to retrieve. (16th beam is 256, 32nd beam is 128, etc.)
    /// @param stemSelection The choice of which stem version to get.
    static MusxInstance<BeamAlterations> getSecondaryForStem(const EntryInfoPtr& entryInfo, Edu dura, StemSelection stemSelection = StemSelection::MatchEntry);  // must be defined in Details.cpp

    static const xml::XmlElementArray<BeamAlterations>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class BeamAlterationsDownStem
 * @brief Beam alteration for downstem primary beams.
 *
 * #Entry::stemDetail is set if the entry has any beam extensions.
 */
class BeamAlterationsDownStem : public BeamAlterations
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part this is for.
     * @param shareMode The sharing mode.
     * @param entnum The entry number this applies to.
     */
    explicit BeamAlterationsDownStem(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum)
        : BeamAlterations(document, partId, shareMode, entnum) {}

    constexpr static std::string_view XmlNodeName = "beamAltPrimDownStem"; ///< The XML node name for this type.
};

/**
 * @class BeamAlterationsUpStem
 * @brief Beam alteration for upstem primary beams.
 *
 * #Entry::stemDetail is set if the entry has any beam extensions.
 */
class BeamAlterationsUpStem : public BeamAlterations
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part this is for.
     * @param shareMode The sharing mode.
     * @param entnum The entry number this applies to.
     */
    explicit BeamAlterationsUpStem(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum)
        : BeamAlterations(document, partId, shareMode, entnum) {}

    constexpr static std::string_view XmlNodeName = "beamAltPrimUpStem"; ///< The XML node name for this type.
};

/**
 * @class BeamExtension
 * @brief Represents both sides of a beam extension. It is attached to the first entry in the beam. Which stem direction this instance controls
 * is determined by the subclass that inherits this class as a base.
 *
 * #Entry::beamExt is set if the entry has any beam extensions.
 */
class BeamExtension : public EntryDetailsBase
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part this is for.
     * @param shareMode The sharing mode.
     * @param entnum The entry number of the first entry in the beam this extension applies to.
     */
    explicit BeamExtension(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum)
        : EntryDetailsBase(document, partId, shareMode, entnum) {}

    Evpu leftOffset{};      ///< Left extension offset. (xml node is `<x3Disp>`)
    Evpu rightOffset{};     ///< Right extension offset. (xml node is `<x4Disp>`)
    unsigned mask{};        ///< Composite mask of beams to extend, derived from `<do8th>` through `<do4096th>` tags.
                            ///< A value of 512 is 8th, 256 is 16th, 128 is 32nd, etc. These correspond to the values
                            ///< in @ref NoteType.
    bool extBeyond8th{};    ///< Legacy flag. Original versions of Finale could either extend only the 8th beam or extend
                            ///< a selection of beams. In musx files (Finale 2014+) this boolean has never been observed to be false.
                            ///< Instead, the mask appears always to be used to determine the beams to be extended. However, if a false
                            ///< value were encountered, it would mean only the 8th beam is extended.

    /// @brief Calculates the maximum beam extension
    /// @return 0, 1, 2, 3, ... where 0 means no beam is extendeed, 1 is the 8th beam, 2 is 16th, etc.
    unsigned calcMaxExtension() const;

    /// @brief Gets the beam extension record for the specified @p stemSelection
    /// @param entryInfo The entry to search.
    /// @param stemSelection The choice of which stem version to get.
    static MusxInstance<BeamExtension> getForStem(const EntryInfoPtr& entryInfo, StemSelection stemSelection = StemSelection::MatchEntry);  // must be defined in Details.cpp

    static const xml::XmlElementArray<BeamExtension>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class BeamExtensionDownStem
 * @brief Beam extension for downstem beams.
 */
class BeamExtensionDownStem : public BeamExtension
{
public:
    using BeamExtension::BeamExtension;

    constexpr static std::string_view XmlNodeName = "beamExtendDownStem"; ///< The XML node name for this type.
};

/**
 * @class BeamExtensionUpStem
 * @brief Beam extension for upstem beams.
 */
class BeamExtensionUpStem : public BeamExtension
{
public:
    using BeamExtension::BeamExtension;

    constexpr static std::string_view XmlNodeName = "beamExtendUpStem"; ///< The XML node name for this type.
};

/**
 * @class BeamStubDirection
 * @brief Specifies the direction for beam stubs (if they are manually overridden.)
 *
 * #Entry::stemDetail is set if the entry has a beam stub direction change.
 *
 * This class is identified by the XML node name "beamStub".
 */
class BeamStubDirection : public EntryDetailsBase
{
public:
    /** @brief Constructor function */
    explicit BeamStubDirection(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum)
        : EntryDetailsBase(document, partId, shareMode, entnum)
    {
    }

    unsigned mask{};        ///< Composite mask of beam stubs, derived from `<do8th>` through `<do4096th>` tags.
                            ///< A value of `true` means left and a value of `false` means right.
                            ///< Because the Finale U.I. is a single handle, these masks are generally all on or all off.

    /// @brief True if the beam stub(s) point left.
    bool isLeft() const { return mask != 0; }

    constexpr static std::string_view XmlNodeName = "beamStub"; ///< The XML node name for this type.
    static const xml::XmlElementArray<BeamStubDirection>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class Bracket
 * @brief Represents a bracket, used in two different contexts:
 *
 * - **Ossia bracket groups**: Bracket is a top-level object that helps define an ossia.
 *   - cmper1: The bracket group number for the ossia.
 *   - cmper2: Always appears to be zero.
 *   - inci: The index within the ossia bracket group.
 *
 * - **Staff groups**: Bracket is embedded inside @ref StaffGroup.
 *   - In this case cmper1, cmper2, and inci have no meaning and should be zero.
 */
class Bracket : public DetailsBase
{
public:
    /// @brief Constructor for top-level instances.
    explicit Bracket(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper1, Cmper cmper2, Inci inci)
        : DetailsBase(document, partId, shareMode, cmper1, cmper2, inci)
    {
    }

    /// @brief Constructor for embedded instances.
    explicit Bracket(const DocumentWeakPtr& document)
        : DetailsBase(document, SCORE_PARTID, EnigmaBase::ShareMode::All, Cmper{}, Cmper{}, Inci{})
    {
    }

    /**
     * @enum BracketStyle
     * @brief Bracket types
     */
    enum class BracketStyle : int
    {
        None = 0,                   ///< No bracket (the default)
        ThickLine = 1,              ///< Thick line, no hooks
        BracketStraightHooks = 2,   ///< Thick bracket with straight hooks
        PianoBrace = 3,             ///< Piano brace
        Unknown4,                   ///< Possibly never used
        Unknown5,                   ///< Possibly never used
        BracketCurvedHooks = 6,     ///< Thick bracket with curved hooks
        Unknown7,                   ///< Possibly never used
        DeskBracket = 8             ///< Thin bracket with horizontal hook lines
    };

    BracketStyle style{};       ///< Bracket style (xml node is `<id>`)
    Evpu horzAdjLeft{};         ///< "Distance from Left Edge of Staff" (xml node is `<bracPos>`)
    Evpu vertAdjTop{};          ///< "Vertical Adjust (Top of Bracket)" (xml node is `<bracTop>`)
    Evpu vertAdjBot{};          ///< "Vertical Adjust (Bottom of Bracket)" (xml node is `<bracBot>`)
    bool showOnSingleStaff{};   ///< "Show Bracket If Group Contains Only One Staff" (only applies to staff groups: xml node is `<onSingle>`)

    constexpr static std::string_view XmlNodeName = "brackSpec"; ///< The XML node name for this type.
    static const xml::XmlElementArray<Bracket>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ChordAssign
 * @brief Represents chord symbol assignment for a staff and measure.
 *
 * Cmper1 is the staff (staffId) @ref Cmper and Cmper2 is the measure @ref Cmper.
 * This class is identified by the XML node name "chordAssign".
 */
class ChordAssign : public DetailsBase
{
    Cmper calcFretboardGroupCmper() const;

public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (probably always 0).
     * @param shareMode The sharing mode for this #ChordAssign (probably always #ShareMode::All).
     * @param staffId The staff ID for this #ChordAssign.
     * @param meas The measure ID for this #ChordAssign.
     * @param inci The 0-based incident.
     */
    explicit ChordAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper staffId, Cmper meas, Inci inci)
        : DetailsBase(document, partId, shareMode, staffId, meas, inci)
    {
    }

    /**
     * @enum BassPosition
     * @brief Bass position options
     */
    enum class BassPosition
    {
        AfterRoot,                  // default value may not appear in xml (but text appears in Finale binary)
        UnderRoot,
        Subtext
    };

    Cmper suffixId{};               ///< The Cmper of the @ref others::ChordSuffixElement instances. Zero means there is no suffix.
                                    ///< When #useFretboardFont is false, this same Cmper is also used to look up the
                                    ///< @ref others::FretboardGroup and related fretboard data.
                                    ///< When #useFretboardFont is true, only the chord-suffix lookup applies (no fretboard group lookup).
    Cmper fbStyleId{};              ///< The @ref others::FretboardStyle ID. (Ignored when #useFretboardFont is true.)
    int rootScaleNum{};             ///< Root scale degree: 0–6, where zero is the tonic
    int rootAlter{};                ///< Root alteration
    bool rootLowerCase{};           ///< Display root in lowercase (minor triad)
    bool playSuffix{};              ///< Playback suffix
    bool showRoot{};                ///< Show root
    bool playRoot{};                ///< Playback root
    bool showFretboard{};           ///< Show fretboard
    bool showSuffix{};              ///< Show suffix
    bool playFretboard{};           ///< Playback fretboard
    int bassScaleNum{};             ///< Bass scale degree: 0–6, where zero is the tonic
    int bassAlter{};                ///< Bass alteration
    bool bassLowerCase{};           ///< Display bass in lowercase
    BassPosition bassPosition{};    ///< Visual position of bass relative to root
    bool showAltBass{};             ///< Show alternate bass
    bool playAltBass{};             ///< Playback alternate bass
    int capoValue{};                ///< Capo value (if #useLocalCapo is true)
    bool useLocalCapo{};            ///< Use local capo
    Inci fretboardGroupInci{};      ///< The inci for retrieving the @ref others::FretboardGroup. (Meaningless if #useFretboardFont is true.)
                                    ///< The cmper is the #suffixId above (when it is non-zero) or one of the hard-coded values 65533
                                    ///< for minor (lowercase) chords and 65534 for major (uppercase) chords. (xml node is `<fretInci>`)
    bool useFretboardFont{};        ///< When true, this overrides any fretboard group or style and uses the fretboard font instead. (xml node is `<useFretFont >`)
                                    ///< See #options::FontOptions::FontType::Fretboard.
    Evpu horzOff{};                 ///< Horizontal offset of chord (in EVPU)
    Evpu vertOff{};                 ///< Vertical offset of chord (in EVPU)
    Evpu fbHorzOff{};               ///< Horizontal offset of fretboard (in EVPU)
    Evpu fbVertOff{};               ///< Vertical offset of fretboard (in EVPU)
    Edu horzEdu{};                  ///< Edu position in measure
    int chPercent{};                ///< Chord scaling (100 means 100%)
    int fbPercent{};                ///< FretboardDiagram scaling (100 means 100%)

    /// @brief Returns the chord suffix as an array of @ref others::ChordSuffixElement.
    MusxInstanceList<others::ChordSuffixElement> getChordSuffix() const;

    /// @brief Get the @ref others::FretboardGroup instance for this chord assignment. You can use this to get the fret instrument.
    /// @return The fretboard group, if it exists, or @c nullptr if #useFretboardFont is true.
    MusxInstance<others::FretboardGroup> getFretboardGroup() const;

    /// @brief Returns the @ref others::FretboardStyle instance for this chord assignment.
    /// @return The fretboard style, if it exists, or @c nullptr #useFretboardFont is true or #fbStyleId is zero.
    MusxInstance<others::FretboardStyle> getFretboardStyle() const;

    constexpr static std::string_view XmlNodeName = "chordAssign"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ChordAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ClefOctaveFlats
 * @brief Defines the octaves in which each clef should display flats in key signatures. Only linear key signatures
 * use this class.
 *
 * Note that while flats are numbered from 1-7 (B-F), this table is indexed 0-6.
 *
 * Cmper1 is the value returned by #KeySignature::getKeyMode. Cmper2 is the clef index to which
 * this array applies.
 *
 * Each value in the array specifies an octave to display the accidental, where 0 is the middle-C octave.
 *
 * This class is identified by the XML node name "clefOctvFlats".
 */
class ClefOctaveFlats : public DetailsArray<int, 7>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using DetailsArray::DetailsArray;

    constexpr static std::string_view XmlNodeName = "clefOctvFlats"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ClefOctaveFlats>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ClefOctaveSharps
 * @brief Defines the octaves in which each clef should display sharps in key signatures.
 *
 * For non-linear keys, this class defines the octave in which each clef should display
 * the accidentals in the key signature, regardless whether they are sharps or flats.
 * (Non-linear keys do not use @ref ClefOctaveFlats.) Therefore, this class will almost certainly exist
 * for non-linear keys.
 *
 * Note that while sharps are numbered from 1-7 (F-B), this table is indexed 0-6.
 *
 * Cmper1 is the value returned by #KeySignature::getKeyMode. Cmper2 is the clef index to which
 * this array applies.
 *
 * Each value in the array specifies an octave to display the accidental, where 0 is the middle-C octave.
 *
 * This class is identified by the XML node name "clefOctvSharps".
 */
class ClefOctaveSharps : public DetailsArray<int, 7>
{
    std::string_view xmlTag() const override { return XmlNodeName; }

public:
    using DetailsArray::DetailsArray;

    constexpr static std::string_view XmlNodeName = "clefOctvSharps"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ClefOctaveSharps>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class CrossStaff
 * @brief Represents a cross-staff assignment for the note, if any.
 *
 * #Entry::crossStaff is set if any note in the entry is crossed to another staff.
 *
 * This class is identified by the XML node name "crossStaff".
 */
class CrossStaff : public NoteDetailsBase
{
public:
    /** @brief Constructor function */
    explicit CrossStaff(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : NoteDetailsBase(document, partId, shareMode, entnum, inci)
    {
    }

    NoteNumber noteId{};    ///< The ID of the note being assigned to a different staff (XML node: `<noteID>`)
    StaffCmper staff{};     ///< The target staff (XML node: `<instrument>`)

    NoteNumber getNoteId() const override { return noteId; }

    constexpr static std::string_view XmlNodeName = "crossStaff";    ///< The XML node name for this type.
    static const xml::XmlElementArray<CrossStaff>& xmlMappingArray();   ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class CustomStem
 * @brief Represents a custom stem definition (up or down) for an entry. Which stem direction this instance controls
 * is determined by the subclass that inherits this class as a base.
 *
 * The entry number refers to the entry to which the stem is attached.
 * The shape is defined by a shape definition number, and optional x/y displacements can adjust placement.
 */
class CustomStem : public EntryDetailsBase
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part this is for.
     * @param shareMode The sharing mode.
     * @param entnum The entry number this stem applies to.
     */
    explicit CustomStem(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum)
        : EntryDetailsBase(document, partId, shareMode, entnum) {}

    Cmper shapeDef{}; ///< The Cmper of the associated @ref others::ShapeDef. Setting this value to zero hides the stem.
    Evpu xOffset{};   ///< Optional horizontal displacement. (xml node is `<xdisp>`)
    Evpu yOffset{};   ///< Optional vertical displacement. (xml node is `<ydisp>`)

    /// @brief Calculates if this custom stem record hides the stem. The stem is determined to be hidden if one of the following is true.
    ///     - #shapeDef is 0. (This is only possible with effort using the Finale UI, but several popular plugins do it commonly.)
    ///     - The shape indicated by #shapeDef does not exist.
    ///     - The shape indicated by #shapeDef has no instructions.
    ///
    /// @return If true, the associated entry has no stem in the direction controlled by the subclass.
    bool calcIsHiddenStem() const;

    /// @brief Gets the beam extension record for the specified @p stemSelection
    /// @param entryInfo The entry to search.
    /// @param stemSelection The choice of which stem version to get.
    static MusxInstance<CustomStem> getForStem(const EntryInfoPtr& entryInfo, StemSelection stemSelection = StemSelection::MatchEntry);  // must be defined in Details.cpp

    static const xml::XmlElementArray<CustomStem>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class CustomDownStem
 * @brief Custom stem for downstem context.
 *
 * #Entry::stemDetail is set if the entry has a custom down stem.
 */
class CustomDownStem : public CustomStem
{
public:
    using CustomStem::CustomStem;

    constexpr static std::string_view XmlNodeName = "stemDefDown"; ///< The XML node name for this type.
};

/**
 * @class CustomUpStem
 * @brief Custom stem for upstem context.
 *
 * #Entry::stemDetail is set if the entry has a custom up stem.
 */
class CustomUpStem : public CustomStem
{
public:
    using CustomStem::CustomStem;

    constexpr static std::string_view XmlNodeName = "stemDefUp"; ///< The XML node name for this type.
};

/**
 * @class DotAlterations
 * @brief Represents display offsets and spacing adjustments for augmentation dots on a specific note.
 *
 * #Entry::dotTieAlt is set if any note in the entry has visual dot modifications such as position or spacing.
 *
 * This class is identified by the XML node name "dotOffset".
 */
class DotAlterations : public NoteDetailsBase
{
public:
    /** @brief Constructor */
    explicit DotAlterations(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : NoteDetailsBase(document, partId, shareMode, entnum, inci)
    {
    }

    NoteNumber noteId{};         ///< The ID of the note whose dots are adjusted.
    Evpu hOffset{};              ///< Horizontal offset for the dot (XML: `<xadd>`)
    Evpu vOffset{};              ///< Vertical offset for the dot (XML: `<yadd>`)
    Evpu interdotSpacing{};      ///< Additional spacing between dots (XML: `<posIncr>`)

    NoteNumber getNoteId() const override { return noteId; }

    constexpr static std::string_view XmlNodeName = "dotOffset";   ///< The XML node name for this type.
    static const xml::XmlElementArray<DotAlterations>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class EntryPartFieldDetail
 * @brief Per-part overrides for entry properties such as manual position and stem direction.
 *
 * @note: Finale creates this as a partially shared item, but it should be treated as non shared
 * item. If a part does not have it (or if it is the score), this entity should be ignored.
 *
 * These details apply to a specific part and entry and override the default
 * entry settings defined at the score level.
 */
class EntryPartFieldDetail : public EntryDetailsBase
{
public:
    /**
     * @brief Constructor function.
     * @param document A weak pointer to the associated document.
     * @param partId The part that this detail is for.
     * @param shareMode The sharing mode for this @ref EntryPartFieldDetail.
     * @param entnum The entry number for which these details apply.
     */
    explicit EntryPartFieldDetail(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum)
        : EntryDetailsBase(document, partId, shareMode, entnum) {}

    Evpu hOffset{};         ///< Manual offset created with the Note Position Tool. (xml node is `<posi>`.)
    bool freezeStem{};      ///< Whether the stem direction is frozen for this entry in this part.
    bool upStem{};          ///< When #freezeStem is true: whether the stem is forced up (otherwise down).

    static const xml::XmlElementArray<EntryPartFieldDetail>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    constexpr static std::string_view XmlNodeName = "entryPartFieldDetail";     ///< The XML node name for this type.
};

/**
 * @class EntrySize
 * @brief Specifies a custom size for an entry. It scales the entire entry, including the stem and all noteheads.
 * For beamed entries, it only takes effect if it is applied to the first entry in a beamed group, and then it affects
 * every entry in the beamed group.
 *
 * #Entry::noteDetail is set if the entry has a custom size record.
 */
class EntrySize : public EntryDetailsBase
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (probably always 0).
     * @param shareMode The sharing mode for this @ref EntrySize (probably always #ShareMode::All).
     * @param entnum The entry number this size applies to.
     */
    explicit EntrySize(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum)
        : EntryDetailsBase(document, partId, shareMode, entnum) {}

    int percent{}; ///< The note/chord size as a percent (e.g., 65 for 65%).

    static const xml::XmlElementArray<EntrySize>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    constexpr static std::string_view XmlNodeName = "entrySize"; ///< The XML node name for this type.
};

/**
 * @class FretboardDiagram
 * @brief FretboardDiagram diagram for chord symbols.
 *
 * cmper1: Corresponds to the cmper for @ref others::FretboardGroup.
 *
 * cmper2: Calculated sequence number. The inci from @ref others::FretboardGroup multiplied
 * by sixteen and then a value from (0..11) added. These correspond to the twelve pitch classes A, A#, B, ... G#.
 *
 * Contains fret count, display options, and a collection of
 * individual string/fret cells and optional barres.
 */
class FretboardDiagram : public DetailsBase
{
public:
    /**
     * @brief Constructor function.
     */
    explicit FretboardDiagram(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper1, Cmper cmper2)
        : DetailsBase(document, partId, shareMode, cmper1, cmper2)
    {}

    /**
     * @enum Shape
     * @brief Shape type for a fretboard cell.
     * @note The actual symbol shapes are obtained from the associated @ref others::FretboardStyle used by
     * the @ref ChordAssign that references this instance. They do not have to correspond to the descriptions in this
     * enum, but they nearly always do.
     */
    enum class Shape
    {
        None,       ///< No explicit shape. This is the default value and may not occur in the xml.
        Closed,     ///< Closed dot (filled).
        Open,       ///< Open circle.
        Muted,      ///< Muted string (X).
        Custom      ///< Custom symbol. By default this is an open diamond signifying a harmonic.
    };

    /**
     * @class Cell
     * @brief Represents a single fretboard cell (string/fret position).
     */
    class Cell
    {
    public:
        int string{};       ///< 1-based string number.
        int fret{};         ///< 0-based fret number, where 0 signifies the open string.
        Shape shape{};      ///< Cell shape.
        int fingerNum{};    ///< Finger number 0..5, where 0 means there is no finger number and 5 means T (thumb).

        static const xml::XmlElementArray<Cell>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /**
     * @class Barre
     * @brief Represents a barre (spanning multiple strings).
     */
    class Barre
    {
    public:
        /// @brief 0-based fret number counting from the diagram's first fret, so 0 is the fret that
        /// @ref Cell identifies as fret 1. This differs from @ref Cell::fret, which reserves 0 for the
        /// open string; a barre's equivalent cell fret is always `fret + 1`.
        int fret{};
        int startString{};  ///< Starting 1-based string number. This is the lowest-numbered string in the barre.
        int endString{};    ///< Ending 1-based string number. This is the highest-numbered string in the barre.

        static const xml::XmlElementArray<Barre>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    int numFrets{};         ///< Number of frets.
    int fretboardNum{};     ///< FretboardDiagram number: the starting fret of the fretboard diagram (xml node `<fretNum>`).
    bool lock{};            ///< Lock flag (xml node `<lock>`).
    bool showNum{};         ///< Show fretboard number.
    int numFretCells{};     ///< Number of fret cells (xml node `<numFretCells>`).
    int numFretBarres{};    ///< Number of fret barres (xml node `<numFretBarres>`).

    std::vector<std::shared_ptr<Cell>> cells;   ///< Array of fretboard cells.
    std::vector<std::shared_ptr<Barre>> barres; ///< Array of fretboard barres.

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->DetailsBase::integrityCheck(ptrToThis);
        if (numFretCells != int(cells.size())) {
            const int oldVal = numFretCells;
            numFretCells = int(cells.size());
            MUSX_INTEGRITY_ERROR("FretboardDiagram " + std::to_string(getCmper1()) + ", " + std::to_string(getCmper2())
                + " specifies the wrong number of cells (" + std::to_string(oldVal) + ").");
        }
        if (numFretBarres != int(barres.size())) {
            const int oldVal = numFretBarres;
            numFretBarres = int(barres.size());
            MUSX_INTEGRITY_ERROR("FretboardDiagram " + std::to_string(getCmper1()) + ", " + std::to_string(getCmper2())
                + " specifies the wrong number of barres (" + std::to_string(oldVal) + ").");
        }
    }

    constexpr static std::string_view XmlNodeName = "fretboard"; ///< XML node name for this type.
    static const xml::XmlElementArray<FretboardDiagram>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class GFrameHold
 * @brief Represents the attributes of a Finale frame holder.
 *
 * Cmper1 is the staff (staffId) @ref Cmper and Cmper2 is the measur @ref Cmper
 * This class is identified by the XML node name "gfhold".
 */
class GFrameHold : public DetailsBase
{
public:
    /**
     * @brief Constructor function
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (probably always 0).
     * @param shareMode The sharing mode for this #GFrameHold (probably always #ShareMode::All)
     * @param staffId The staff ID for this #GFrameHold.
     * @param meas The measure ID for this #GFrameHold.
     */
    explicit GFrameHold(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper staffId, Cmper meas)
        : DetailsBase(document, partId, shareMode, staffId, meas), frames(MAX_LAYERS) {}

    // Public properties corresponding to the XML structure
    std::optional<ClefIndex> clefId;        ///< clef index when there are no mid-measure clef changes. (xml tag is `<clefID>`).
    Cmper clefListId{};                     ///< The clef list ID when there are mid-measure clef changes, if non-zero. (xml tag is `<clefListID>`).
    ShowClefMode showClefMode{};            ///< "Show Clef" mode. (xml tag is `<clefMode>`)
    bool mirrorFrame{};                     ///< Indicates this is a mirror frame. (Not used after Finale 14.5.)
    bool clefAfterBarline{};                ///< "Place Clef After Barline"
    int clefPercent{};                      ///< Clef percent where 100 means 100%.
    std::vector<Cmper> frames;              ///< @ref others::Frame values for layers 1..4 (layer indices 0..3) if non-zero

    /// @brief returns the staffId (staff) number for this #GFrameHold
    StaffCmper getStaff() const { return StaffCmper(getCmper1()); }

    /// @brief returns the measure number for this #GFrameHold
    MeasCmper getMeasure() const { return MeasCmper(getCmper2()); }

    /// @brief Returns true if the #GFrameHold instance has more than one layer
    bool calcIsMultiLayer() const
    {
        return std::count_if(frames.begin(), frames.end(), [](Cmper frameId) { return frameId != 0; }) > 1;
    }

    /// @brief Returns true if the #GFrameHold contains any layer frames.
    bool calcContainsMusic() const
    {
        return std::find_if(frames.begin(), frames.end(), [](Cmper frameId) { return frameId != 0; }) != frames.end();
    }

    /// @brief Find the layer frame and Edu start position for the given layer. A non-zero start position indicates a spacer for a legacy pickup.
    /// @param layerIndex The layer index to find (0..3)
    /// @return std::pair containing the frame and the start position.
    std::pair<MusxInstance<others::Frame>, Edu> findLayerFrame(LayerIndex layerIndex) const;

    /// @brief Iterates all the raw entries in a the GFrameHold, passing them to the iterator function. If the iterator function returns false, iteration stops.
    /// @param iterator function.
    /// @return true if all entries iterated, false if the iterator function exited early by returning false.
    bool iterateRawEntries(std::function<bool(const MusxInstance<Entry>&, LayerIndex)> iterator) const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->DetailsBase::integrityCheck(ptrToThis);
        if (clefListId && clefId.has_value()) {
            MUSX_INTEGRITY_ERROR("GFrameHold for staff " + std::to_string(getCmper1()) + " and measure " + std::to_string(getCmper2()) + " has both clef and clef list.");
        }
        if (!clefListId && !clefId.has_value()) {
            clefId = ClefIndex(0);
            util::Logger::log(
                util::Logger::LogLevel::Verbose,
                "GFrameHold for staff " + std::to_string(getCmper1()) + " and measure " + std::to_string(getCmper2()) +
                    " has neither clef nor clef list.");
        }
    }

    constexpr static std::string_view XmlNodeName = "gfhold"; ///< The XML node name for this type.
    static const xml::XmlElementArray<GFrameHold>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class IndependentStaffDetails
 * @brief Represents independent time and key signature overrides for a staff.
 *
 * Cmper1 is the staff (staffId) @ref Cmper and Cmper2 is the measure @ref Cmper.
 * This class is identified by the XML node name "floats".
 */
class IndependentStaffDetails : public DetailsBase
{
public:
    /**
     * @brief Constructor function
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (probably always 0).
     * @param shareMode The sharing mode for this #IndependentStaffDetails (probably always #ShareMode::All).
     * @param staffId The staff ID for this #IndependentStaffDetails.
     * @param meas The measure ID for this #IndependentStaffDetails.
     */
    explicit IndependentStaffDetails(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper staffId, Cmper meas)
        : DetailsBase(document, partId, shareMode, staffId, meas)
    {
    }

    // Key signature info
    std::shared_ptr<KeySignature> keySig; ///< The key signature value if present. (xml tag is `<keySig>/<key>`)
    bool hasKey{};                        ///< If true, this item contains an independent key signature.

    // Time signature info
    Cmper beats{};                      ///< Time signature numerator or a Cmper to a composite record.
    Cmper divBeat{};                    ///< Time signature denominator in Edu or a Cmper to a composite record.
    Cmper dispBeats{};                  ///< Display time signature numerator or a Cmper to a composite record.
    Cmper dispDivBeat{};                ///< Display time signature denominator in Edu or a Cmper to a composite record.

    bool displayAltNumTsig{};           ///< If true, #dispBeats is the Cmper of a @ref others::TimeCompositeUpper record
    bool displayAltDenTsig{};           ///< If true, #dispDivBeat is the Cmper of a @ref others::TimeCompositeLower record
    bool altNumTsig{};                  ///< If true, #beats is the Cmper of a @ref others::TimeCompositeUpper record
    bool altDenTsig{};                  ///< If true, #divBeat is the Cmper of a @ref others::TimeCompositeLower record
    bool displayAbbrvTime{};            ///< If true, the display time signature is abbreviated
    bool hasDispTime{};                 ///< If true, the display time signature is present.
    bool hasTime{};                     ///< If true, this item contains an independent time signature.

    /// @brief Create a shared pointer to an instance of the @ref TimeSignature for this instance.
    MusxInstance<TimeSignature> createTimeSignature() const;

    /// @brief Create a shared pointer to an instance of the display @ref TimeSignature for this measure and staff.
    /// @return The display time signature if there is one, otherwise the actual time signature.
    MusxInstance<TimeSignature> createDisplayTimeSignature() const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        DetailsBase::integrityCheck(ptrToThis);
        if (hasKey && !keySig) {
            keySig = std::make_shared<KeySignature>(getDocument());
        }
    }

    constexpr static std::string_view XmlNodeName = "floats"; ///< The XML node name for this type.
    static const xml::XmlElementArray<IndependentStaffDetails>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class KeySymbolListElement
 * @brief Represents a single element in a Finale accidental symbol list.
 *
 * This class is identified by the XML node name "keySymList".
 * cmper1 is the symbol list ID from #others::KeyAttributes::symbolList. cmper2 is the accidental slot, which should be interpreted as a signed int.
 */
class KeySymbolListElement : public DetailsBase {
public:
    /** @brief Constructor function */
    explicit KeySymbolListElement(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper1, Cmper cmper2)
        : DetailsBase(document, partId, shareMode, cmper1, cmper2) {}

    std::string accidentalString;   ///< The symbol string used to represent the accidental. The string uses either the default font for keys
                                    ///< or the font specified by #others::KeyAttributes::fontSym if its value is non-zero. (xml node is `<string>`)

    /** @brief Returns the alteration value that this symbol corresponds with. */
    int getAlterationValue() const {
        return static_cast<int16_t>(getCmper2());
    }

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        DetailsBase::integrityCheck(ptrToThis);
        if (std::abs(getAlterationValue()) > MAX_ALTERATIONS) {
            util::Logger::log(util::Logger::LogLevel::Verbose,
                "KeySymbolListElement for list " + std::to_string(getCmper1()) + " has invalid value " + std::to_string(getAlterationValue()));
        }
    }

    constexpr static std::string_view XmlNodeName = "keySymList"; ///< The XML node name for this type.
    static const xml::XmlElementArray<KeySymbolListElement>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class LyricAssign
 * @brief Contains assignment data for a lyric assignment (a single syllable)
 */
class LyricAssign : public EntryDetailsBase
{
public:
    /**
     * @brief Constructor function
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for.
     * @param shareMode The sharing mode for this @ref LyricAssign
     * @param entnum The entry number of this assignment.
     * @param inci The 0-based inci. Each lyric text block has a separate instance, if assigned.
     */
    explicit LyricAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : EntryDetailsBase(document, partId, shareMode, entnum, inci) {}

    Cmper lyricNumber{};            ///< the text number of the lyric.
    unsigned syllable{};            ///< the 1-based syllable number. Subtract 1 for an index into #texts::LyricsTextBase::syllables. (xml node is `<syll>`)
    Evpu horzOffset{};              ///< horizontal offset from default position. (xml node is `<horzOff>`)
    Evpu vertOffset{};              ///< horizontal offset from default position. (xml node is `<vertOff>`)
    Evpu floatingHorzOff{};         ///< This appears to have something to do with note spacing. It may simply be a cache that Finale changes as needed.
    Evpu wext{};                    ///< If smart word extensions are being used, a non-zero value indicates the existence of a word extension SmartShape.
                                    ///< If smart word extensions are not being used, this is the actual length of the word extension in Evpu.
                                    ///< See #options::LyricOptions::useSmartWordExtensions.
    bool displayVerseNum{};         ///< If set, the text block number displays to the left of the syllable. (E.g., when numbering verses in a hymn.)

    /// @brief Get the raw lyric text for this assignment.
    virtual MusxInstance<texts::LyricsTextBase> getLyricText() const = 0;

    /// @brief Get the lyric text type for this assignment.
    virtual LyricTextType getLyricTextType() const = 0;

    /// @brief Return an Enigma parsing context for the associated lyrics text block.
    /// @note The Finale UI prevents the use of text inserts in lyric text blocsk, so there
    /// is no need for a part ID to be passed in, as for other Enigma parsing contexts.
    util::EnigmaParsingContext getRawTextCtx() const;

    /// @brief Calculate the entry to which an automatic word extension extends.
    /// @return The terminating entry of the word extension, or null if no word extension or if the document
    /// uses legacy word extensions.
    EntryInfoPtr calcWordExtensionEndpoint() const;

    /// @brief Calculate the verse/chorus/section number to display to the left of this
    /// assignment's syllable, if any.
    /// @return The formatted number (e.g. "1."), or std::nullopt if no number should be displayed.
    std::optional<std::string> calcDisplayNumberText() const;

    static const xml::XmlElementArray<LyricAssign>& xmlMappingArray();   ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class LyricAssignChorus
 * @brief Contains the syllable assignments for lyrics chorus blocks.
 *
 * #Entry::lyricDetail is set if the entry has chorus lyric assignment.
 */
class LyricAssignChorus : public LyricAssign
{
public:
    using LyricAssign::LyricAssign;

    MusxInstance<texts::LyricsTextBase> getLyricText() const override;
    LyricTextType getLyricTextType() const override;
        
    using BaselineType = BaselineLyricsChorus; ///< The baseline type for this item.
    using TextType = texts::LyricsChorus; ///< The text type for this item.
    constexpr static std::string_view XmlNodeName = "lyrDataChorus"; ///< The XML node name for this type.
};

/**
 * @class LyricAssignSection
 * @brief Contains the syllable assignments for lyrics section blocks.
 *
 * #Entry::lyricDetail is set if the entry has section lyric assignment.
 */
class LyricAssignSection : public LyricAssign
{
public:
    using LyricAssign::LyricAssign;

    MusxInstance<texts::LyricsTextBase> getLyricText() const override;
    LyricTextType getLyricTextType() const override;

    using BaselineType = BaselineLyricsSection; ///< The baseline type for this item.
    using TextType = texts::LyricsSection; ///< The text type for this item.
    constexpr static std::string_view XmlNodeName = "lyrDataSection"; ///< The XML node name for this type.
};

/**
 * @class LyricAssignVerse
 * @brief Contains the syllable assignments for lyrics verse blocks.
 *
 * #Entry::lyricDetail is set if the entry has verse lyric assignment.
 */
class LyricAssignVerse : public LyricAssign
{
public:
    using LyricAssign::LyricAssign;

    MusxInstance<texts::LyricsTextBase> getLyricText() const override;
    LyricTextType getLyricTextType() const override;

    using BaselineType = BaselineLyricsVerse; ///< The baseline type for this item.
    using TextType = texts::LyricsVerse; ///< The text type for this item.
    constexpr static std::string_view XmlNodeName = "lyrDataVerse"; ///< The XML node name for this type.
};

/**
 * @class LyricEntryInfo
 * @brief Specifies lyric alignment and justification for a single entry. This affects all lyric assignments
 * on the entry.
 *
 * Entry::lyricDetail is set if there are any instances of this class.
 */
class LyricEntryInfo : public EntryDetailsBase
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part this is for.
     * @param shareMode The sharing mode.
     * @param entnum The entry number this applies to.
     * @param inci The inci (if supplied)
     */
    explicit LyricEntryInfo(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, std::optional<Inci> inci = std::nullopt)
        : EntryDetailsBase(document, partId, shareMode, entnum, inci) {}

    std::optional<AlignJustify> justify{}; ///< Override default justification if present. (xml node is `<justify>`)
    std::optional<AlignJustify> align{};   ///< Override default alignment if present. (xml node is `<align>`)

    static const xml::XmlElementArray<LyricEntryInfo>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    constexpr static std::string_view XmlNodeName = "lyricEntryInfo"; ///< The XML node name for this type.
};

/**
 * @class MeasureNumberIndividualPositioning
 * @brief Per-staff/per-measure overrides for the position and appearance of a measure number.
 *
 * Cmper1 is the staff (staffId) @ref Cmper and Cmper2 is the measure @ref Cmper.
 * This class is identified by the XML node name "measNumbIndivPos".
 */
class MeasureNumberIndividualPositioning : public DetailsBase
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The source part for this instance.
     * @param shareMode The sharing mode for this #MeasureNumberIndividualPositioning.
     * @param staffId The staff ID for this instance.
     * @param meas The measure ID for this instance.
     * @param inci The 0-based incident.
     */
    explicit MeasureNumberIndividualPositioning(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper staffId, Cmper meas, Inci inci)
        : DetailsBase(document, partId, shareMode, staffId, meas, inci)
    {
    }

    /**
     * @enum ForceVisibility
     * @brief Force-visibility options for the measure number at this location.
     */
    enum class ForceVisibility
    {
        None,       ///< No override (default; node may be omitted in XML)
        Show,       ///< Force the number to display (xml text value: "force")
        Hide        ///< Force the number to be hidden (xml text value: "hide")
    };

    Cmper measNumRegion{};                          ///< The Cmper of the @ref others::MeasureNumberRegion that this override applies to. (xml node is `<region>`)
    Evpu xOffset{};                                 ///< Horizontal offset for the measure number. (xml node is `<x1add>`)
    Evpu yOffset{};                                 ///< Vertical offset for the measure number. (xml node is `<y1add>`)
    Evpu xOffset2{};                                ///< Horizontal offset for the measure number range under a multimeasure rest (Page View only). (xml node is `<x2add>`)
    ForceVisibility forceVisibility{};              ///< Force-visibility behavior. (xml node is `<forceHide>`)
    bool isAlternateNumber{};                       ///< Whether this is an alternate number, such as a boxed number. (xml node is `<isAltNum>`)
    bool useEnclosure{};                            ///< Whether to use an enclosure for this number. (xml node is `<useEncl>`)
    std::shared_ptr<others::Enclosure> enclosure;   ///< Optional enclosure settings. (xml node is `<encl>`)

    constexpr static std::string_view XmlNodeName = "measNumbIndivPos"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MeasureNumberIndividualPositioning>& xmlMappingArray(); ///< Required for musx::
};

/**
 * @class MeasureTextAssign
 * @brief Represents a text block assignment for a staff and measure.
 *
 * Cmper1 is the staff (staffId) @ref Cmper and Cmper2 is the measure @ref Cmper.
 *
 * Note that each instance either has a positive horizontal displacement in Edu,
 * meaning it maintains metrical alignment as spacing changes. Or it has a negative horizontal displacement
 * in Evpu, meaning a negative offset maintains its displacement in any spacing context.
 *
 * The UI interpolates these values and shows different numbers to the user, but the actual origin point
 * where both #xDispEdu and #xDispEvpu are zero coincides with Edu position 0.
 *
 * This class is identified by the XML node name "measTextAssign".
 */
class MeasureTextAssign : public DetailsBase
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (probably always 0).
     * @param shareMode The sharing mode for this #MeasureTextAssign (probably always #ShareMode::All).
     * @param staffId The staff ID for this #MeasureTextAssign.
     * @param meas The measure ID for this #MeasureTextAssign.
     * @param inci The 0-based incident.
     */
    explicit MeasureTextAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper staffId, Cmper meas, Inci inci)
        : DetailsBase(document, partId, shareMode, staffId, meas, inci)
    {
    }

    Cmper block{};      ///< Text block ID
    Edu xDispEdu{};     ///< Positive horizontal displacement in Edu (xml node is `<xdispEdu>`)
                        ///< Note that the UI apparently interpolates between Evpu and Edu for this field.
    Evpu xDispEvpu{};   ///< Negative horizontal displacement in Evpu, when #xDispEdu is zero. (xml node is `<xdispEvpu>`)
    Evpu yDisp{};       ///< Vertical displacement in Evpu (xml node is `<ydisp>`)
    bool hidden{};      ///< Indicates the text appears only on screen (xml node is `<postIt/>`)

    /** @brief Gets the TextBlock for this assignment, or nullptr if none. */
    MusxInstance<others::TextBlock> getTextBlock() const;

    /** @brief Gets the raw text for this assignment, or nullptr if none. */
    util::EnigmaParsingContext getRawTextCtx(Cmper forPartId) const;

    constexpr static std::string_view XmlNodeName = "measTextAssign"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MeasureTextAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class NoteAlterations
 * @brief Represents graphical and notational alterations applied to a note.
 *
 * #Entry::noteDetail is set if any note in the entry has note alterations.
 *
 * This class is identified by the XML node name "noteAlter".
 */
class NoteAlterations : public NoteDetailsBase
{
public:
    /** @brief Constructor function */
    explicit NoteAlterations(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : NoteDetailsBase(document, partId, shareMode, entnum, inci), customFont(new FontInfo(document))
    {
    }

    NoteNumber noteId{};                    ///< The ID of the note being altered. (xml node is `<noteID>`)
    int percent{};                          ///< Size percentage for the notehead. (A value of 100 means 100%.)
    Evpu nxdisp{};                          ///< Horizontal notehead offset.
    char32_t altNhead{};                    ///< Alternate notehead character. Zero (the default) means no override is specified.
    bool useOwnFont{};                      ///< Whether to use the custom font.
    std::shared_ptr<FontInfo> customFont{}; ///< Custom font info (consolidates: `<fontID>`, `<fontSize>`, `<efx>`)
    bool allowVertPos{};                    ///< Whether vertical positioning is allowed .
    Evpu nydisp{};                          ///< Vertical notehead offset.
    bool enharmonic{};                      ///< Whether this note is enharmonically respelled.
                                            ///< It is normally only true in linked parts as an unlinked value.

    NoteNumber getNoteId() const override { return noteId; }

    constexpr static std::string_view XmlNodeName = "noteAlter"; ///< The XML node name for this type.
    static const xml::XmlElementArray<NoteAlterations>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class PercussionNoteCode
 * @brief Represents a percussion note code override for a single note.
 *
 * Entry::noteDetail is set if any note in the entry has associated percussion note codes.
 *
 * This class is identified by the XML node name "percussionNoteCode".
 */
class PercussionNoteCode : public NoteDetailsBase
{
public:
    /** @brief Constructor function */
    explicit PercussionNoteCode(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : NoteDetailsBase(document, partId, shareMode, entnum, inci)
    {
    }

    NoteNumber noteId{};                ///< The ID of the note being assigned a code.
    PercussionNoteTypeId noteCode{};    ///< The percussion note code. Use this to search the incis of the percussion map for the specific note.

    NoteNumber getNoteId() const override { return noteId; }

    constexpr static std::string_view XmlNodeName = "percussionNoteCode"; ///< The XML node name for this type.
    static const xml::XmlElementArray<PercussionNoteCode>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class SecondaryBeamAlterationsDownStem
 * @brief Beam alteration for downstem secondary beams.
 *
 * #Entry::stemDetail is set if the entry has any beam extensions.
 */
class SecondaryBeamAlterationsDownStem : public BeamAlterations
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part this is for.
     * @param shareMode The sharing mode.
     * @param entnum The entry number this applies to.
     * @param inci The incident, since each secondary beam has a record. They are in no guaranteed order. Use the #BeamAlterations::dura value to determine
     * which beam this pertains to.
     */
    explicit SecondaryBeamAlterationsDownStem(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : BeamAlterations(document, partId, shareMode, entnum, inci) {}

    /// @brief Calculates if the input entry starts a feathered beam and returns information about it.
    /// @note Instead of calling this function directly, consider using #EntryInfoPtr::calcIsFeatheredBeamStart instead.
    /// @param [in] entryInfo The entry info to search.
    /// @param [out] outLeftY The height of the left side of the feathered beam
    /// @param [out] outRightY The height of the right side of the feathered beam
    /// @return true if this is a feathered beam. If the return value is false, outLeftY and outRightY are unchanged.
    static bool calcIsFeatheredBeam(const EntryInfoPtr& entryInfo, Evpu& outLeftY, Evpu& outRightY)
    { return calcIsFeatheredBeamImpl<SecondaryBeamAlterationsDownStem>(entryInfo, outLeftY, outRightY); }

    constexpr static std::string_view XmlNodeName = "beamAltSecDownStem"; ///< The XML node name for this type.
};

/**
 * @class SecondaryBeamAlterationsUpStem
 * @brief Beam alteration for downstem secondary beams.
 *
 * #Entry::stemDetail is set if the entry has any beam extensions.
 */
class SecondaryBeamAlterationsUpStem : public BeamAlterations
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part this is for.
     * @param shareMode The sharing mode.
     * @param entnum The entry number this applies to.
     * @param inci The incident, since each secondary beam has a record. They are in no guaranteed order. Use the #BeamAlterations::dura value to determine
     * which beam this pertains to.
     */
    explicit SecondaryBeamAlterationsUpStem(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : BeamAlterations(document, partId, shareMode, entnum, inci) {}

    /// @brief Calculates if the input entry starts a feathered beam and returns information about it
    /// @note Instead of calling this function directly, consider using #EntryInfoPtr::calcIsFeatheredBeamStart instead.
    /// @param [in] entryInfo The entry info to search.
    /// @param [out] outLeftY The height of the left side of the feathered beam
    /// @param [out] outRightY The height of the right side of the feathered beam
    /// @return true if this is a feathered beam. If the return value is false, outLeftY and outRightY are unchanged.
    static bool calcIsFeatheredBeam(const EntryInfoPtr& entryInfo, Evpu& outLeftY, Evpu& outRightY)
    { return calcIsFeatheredBeamImpl<SecondaryBeamAlterationsUpStem>(entryInfo, outLeftY, outRightY); }

    constexpr static std::string_view XmlNodeName = "beamAltSecUpStem"; ///< The XML node name for this type.
};

/**
 * @class SecondaryBeamBreak
 * @brief Specifies which secondary beams break and restart on the associated entry.
 *
 * #Entry::secBeam is set if the entry a secondary beam break.
 *
 * This class is identified by the XML node name "secBeamBreak".
 */
class SecondaryBeamBreak : public EntryDetailsBase
{
public:
    /** @brief Constructor function */
    explicit SecondaryBeamBreak(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum)
        : EntryDetailsBase(document, partId, shareMode, entnum)
    {
    }

    unsigned mask{};        ///< Composite mask of beam breaks, derived from `<do16th>` through `<do4096th>` tags.
    bool breakThrough{};    ///< True if the beam should be broken through to the largest specified beam value.

    /// @brief Calculates the lowest (largest note-value) beam specified for a secondary beam break.
    ///
    /// This function ignores #breakThrough, since it is nearly meaningless even in Finale.
    ///
    /// @return A @ref BeamNumber value (minimum 2) for the lowest beam number that should be broken.
    /// Returns 0 if the #mask is invalid.
    BeamNumber calcLowestBreak() const
    {
        MUSX_ASSERT_IF(!mask || mask >= unsigned(NoteType::Eighth)) {
            return 0; // invalid mask values have already been flagged and supposedly corrected by #integrityCheck.
        }
        for (unsigned shift = 0; true; shift++) {
            if (mask & (unsigned(NoteType::Note16th) >> shift)) {
                return shift + 2; // the 2nd beam is the 16th beam and the first one we checked.
            }
        }
        assert(false); // should not be able to get here
        return 0;
    }

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        EntryDetailsBase::integrityCheck(ptrToThis);
        if (!mask) {
            mask = unsigned(NoteType::Note4096th);
            util::Logger::log(util::Logger::LogLevel::Verbose,
                "Secondary beam break for entry " + std::to_string(getEntryNumber())
                + " has no breaks; the 4096th-note beam was assumed.");
        }
        if (mask >= unsigned(NoteType::Eighth)) {
            mask = unsigned(NoteType::Eighth) - 1;
            MUSX_INTEGRITY_ERROR("Secondary beam break for entry " + std::to_string(getEntryNumber()) + " specifies a value that cannot be a secondary beam.");
        }
    }

    constexpr static std::string_view XmlNodeName = "secBeamBreak"; ///< The XML node name for this type.
    static const xml::XmlElementArray<SecondaryBeamBreak>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ShapeNoteBase
 * @brief Shape Note settings for staves and staffs style.
 *
 * See @ref ShapeNote and @ref ShapeNoteStyle for cmper1 and cmper2 values.
 *
 * The glyph codes are from font #options::FontOptions::FontType::Noteheads unless the notehead font
 * is overridden by the applicable @ref others::Staff or @ref others::StaffStyle.
 */
class ShapeNoteBase : public DetailsBase
{
public:
    /**
     * @brief Constructor function. */
    explicit ShapeNoteBase(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper1, Cmper cmper2)
        : DetailsBase(document, partId, shareMode, cmper1, cmper2)
    {}

    /**
     * @brief Embedded shape record for a single index in #noteShapes.
     */
    class NoteShapes
    {
    public:
        char32_t doubleWhole{};     ///< Double-whole notehead symbol (xml nodename is `<d>`).
        char32_t whole{};           ///< Whole notehead symbol (xml nodename is `<w>`).
        char32_t half{};            ///< Half notehead symbol (xml nodename is `<h>`).
        char32_t quarter{};         ///< Filled (quarter) notehead symbol (xml nodename is `<q>`).

        static const xml::XmlElementArray<NoteShapes>& xmlMappingArray();    ///< Required for musx::factory::FieldPopulator.
    };

    std::vector<std::shared_ptr<NoteShapes>> noteShapes; ///< Notehead shapes (only the first seven elements are used).
    bool arrangedByPitch{};     ///< If true, the shapes correspond to pitches (C, D, E, F, G, A, B).
                                ///< If false, the shapes correspond to scale degrees 0..6 in the key signauture.

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->DetailsBase::integrityCheck(ptrToThis);
        const size_t currentSize = noteShapes.size();
        if (currentSize < music_theory::STANDARD_DIATONIC_STEPS) {
            for (size_t x = currentSize; x < music_theory::STANDARD_DIATONIC_STEPS; x++) {
                noteShapes.emplace_back(std::make_shared<NoteShapes>());
            }
            MUSX_INTEGRITY_ERROR("For ShapeNoteBase cmper " + std::to_string(getCmper1()) + ", only " + std::to_string(currentSize)
                + " note shapes were provided but at least " + std::to_string(music_theory::STANDARD_DIATONIC_STEPS) + " were expected.");
        }
    }

    static const xml::XmlElementArray<ShapeNoteBase>& xmlMappingArray();    ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ShapeNote
 * @brief Shape Note settings for staves.
 *
 * cmper1: If zero, the note shapes for @ref options::NoteRestOptions. If non-zero, the StaffCmper of the staff using this instance.
 * cmper2: Seems always to be zero.
 */
class ShapeNote: public ShapeNoteBase
{
public:
    using ShapeNoteBase::ShapeNoteBase;

    constexpr static std::string_view XmlNodeName = "shapeNote"; ///< XML node name for this type.
};

/**
 * @class ShapeNoteStyle
 * @brief Shape Note settings for staff styles.
 *
 * cmper1: The Cmper of the staff style using this instance.
 * cmper2: Seems always to be zero.
 */
class ShapeNoteStyle: public ShapeNoteBase
{
public:
    using ShapeNoteBase::ShapeNoteBase;

    constexpr static std::string_view XmlNodeName = "shapeNoteStyle"; ///< XML node name for this type.
};

/**
 * @class StaffGroup
 * @brief Represents the attributes of a Finale staff group that brackets staves
 *
 * This class is identified by the XML node name "staffGroup".
 */
class StaffGroup : public DetailsBase {
public:
    /** @brief Constructor
     *
     * @param document The document owner of this instance
     * @param partId The part that owns this staff group
     * @param shareMode The sharing mode deduced from xml attributes
     * @param cmper1 In modern Finale files, all groups have 0 for cmper1. In legacy files this was a cmper for a @ref others::StaffUsed list.
     * @param cmper2 The identifier for the StaffGroup.
     */
    StaffGroup(const DocumentWeakPtr& document, Cmper partId, EnigmaBase::ShareMode shareMode, Cmper cmper1, Cmper cmper2)
        : DetailsBase(document, partId, shareMode, cmper1, cmper2) {}

    /** @brief Enum for barline justification */
    using BarlineType = others::Measure::BarlineType;

    /**
     * @enum DrawBarlineStyle
     * @brief Enum for how to draw group barlines
     */
    enum class DrawBarlineStyle
    {
        OnlyOnStaves,       ///< Default value (may not appear in xml)
        ThroughStaves,      ///< Draw barlines through staves (xml value is "group")
        Mensurstriche       ///< Draw barlines between staves (xml value is "Mensurstriche" with capitalization)
    };

    /**
     * @enum HideStaves
     * @brief Enum for optimization options
     */
    enum class HideStaves
    {
        Normally,       ///< Hide staves as if there were no StaffGroup (this is the default and may not appear in the xml)
        AsGroup,        ///< Hide staves only if all staves are empty
        None            ///< Never hide the staves in this StaffGroup
    };

    // Public properties corresponding to the XML structure, ordered as they appear in the XML
    StaffCmper startInst{};                   ///< Starting staff ID
    StaffCmper endInst{};                     ///< Ending staff ID
    MeasCmper startMeas{};                    ///< Starting measure number
    MeasCmper endMeas{};                      ///< Ending measure number
    Cmper fullNameId{};                       ///< Full name TextBlock cmper (xml node is `<fullID>`)
    int fullNameXadj{};                       ///< Horizontal adjustment for full name (xml node is `<fullXadj>`)
    int fullNameYadj{};                       ///< Vertical adjustment for full name (xml node is `<fullYadj>`)
    std::shared_ptr<Bracket> bracket{};       ///< Bracket Options. The factory guarantees this value to exist.
    BarlineType barlineType{};                ///< Group barline type (xml node is `<barline>`)
    AlignJustify fullNameJustify{};           ///< Full name justification (xml node is `<fullJustify>`)
    AlignJustify abbrvNameJustify{};          ///< Abbreviated name justification (xml node is `<abbrvJustify>`)
    DrawBarlineStyle drawBarlines{};          ///< "Draw Barlines" option (xml node is `<groupBarlineStyle>`)
    bool ownBarline{};                        ///< "Use Alternate Group Barline"
    bool fullNameIndivPos{};                  ///< Indicates if full name has individual position (xml node is `<fullIndivPos>`)
    bool abbrvNameIndivPos{};                 ///< Indicates if abbreviated name has individual position (xml node is `<abbrvIndivPos>`)
    bool hideName{};                          ///< Inverse of "Show Group Name"
    Cmper abbrvNameId{};                      ///< Abbreviated name TextBlock cmper (xml node is `<abbrvID>`)
    int abbrvNameXadj{};                      ///< Horizontal adjustment for abbreviated name (xml node is `<abbrvXadj>`)
    int abbrvNameYadj{};                      ///< Vertical adjustment for abbreviated name (xml node is `<abbrvYadj>`)
    AlignJustify fullNameAlign{};             ///< Full name horizontal alignment (xml node is `<fullHAlign>`)
    AlignJustify abbrvNameAlign{};            ///< Abbreviated name horizontal alignment (xml node is `<abbrvHAlign>`)
    bool fullNameExpand{};                    ///< "Expand Single Word" for full name (xml node is `<fullExpand>`)
    bool abbrvNameExpand{};                   ///< "Expand Single Word" for abbreviated name (xml node is `<abbrvExpand>`)
    HideStaves hideStaves{};                  ///< "When Hiding Empty Staves" option (xml node is `<optimize>`)
    Cmper customBarShape{};                   ///< Cmper of Shape Designer @ref others::ShapeDef for the custom group barline. (Used when #barlineType is `BarlineType::Custom`.)

    Cmper multiStaffGroupId{};      ///< Calculated cmper for @ref others::MultiStaffGroupId, if any. This value is not in the xml.
                                    ///< It is set by the factory with the Resolver function for @ref others::MultiStaffGroupId.
                                    ///< @note This is the value (if any) for a defined multistaff instrument. Normally you should use
                                    ///< #Document::getInstruments to determine the instrument groupings. These include any that
                                    ///< the factory detected from legacy staves or other special circumstances.
    std::unordered_set<StaffCmper> staves; ///< Calculated list of staves in the group

    /// @brief Get the raw text context for the full staff group name.
    util::EnigmaParsingContext getFullNameCtx() const;

    /// @brief Get the full staff group name without Enigma tags
    /// @param accidentalStyle The style for accidental subsitution in names like "Clarinet in Bb".
    std::string getFullName(util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii) const;

    /// @brief Get the raw text context for the abbreviated staff group name.
    util::EnigmaParsingContext getAbbreviatedNameCtx() const;

    /// @brief Get the abbreviated staff group name without Enigma tags
    /// @param accidentalStyle The style for accidental subsitution in names like "Clarinet in Bb".
    std::string getAbbreviatedName(util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii) const;

    /// @brief Returns the @ref others::MultiStaffInstrumentGroup for this group if it is part of one. Otherwise nullptr.
    MusxInstance<others::MultiStaffInstrumentGroup> getMultiStaffInstGroup() const;

    /// @brief Returns the raw text context for the full instrument name for this group, with autonumbering context if any.
    /// @return Full instrument name context if this group is associated with a @ref others::MultiStaffInstrumentGroup. Otherwise #fullNameId context.
    util::EnigmaParsingContext getFullInstrumentNameCtx() const;

    /// @brief Returns the full instrument name for this group without Enigma tags and with autonumbering (if any).
    /// @note Ordinal prefix numbering is currently supported only for English.
    /// @param accidentalStyle The style for accidental subsitution in names like "Clarinet in Bb".
    /// @return Full instrument name if this group is associated with a @ref others::MultiStaffInstrumentGroup. Otherwise #getFullName.
    std::string getFullInstrumentName(util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii) const;

    /// @brief Returns the raw text context for the abbreviated instrument name for this group, with autonumbering context if any.
    /// @return Abbreviated instrument name context if this group is associated with a @ref others::MultiStaffInstrumentGroup. Otherwise #abbrvNameId context.
    util::EnigmaParsingContext getAbbreviatedInstrumentNameCtx() const;

    /// @brief Returns the abbreviated instrument name for this group without Enigma tags and with autonumbering (if any)
    /// @note Ordinal prefix numbering is currently supported only for English.
    /// @param accidentalStyle The style for accidental subsitution in names like "Clarinet in Bb".
    /// @return Abbreviated instrument name if this group is associated with a @ref others::MultiStaffInstrumentGroup. Otherwise #getAbbreviatedName.
    std::string getAbbreviatedInstrumentName(util::EnigmaString::AccidentalStyle accidentalStyle = util::EnigmaString::AccidentalStyle::Ascii) const;

    /// @brief Returns true if this group spans all measures.
    bool isAllMeasures() const
    { return (startMeas == 1 && endMeas == (std::numeric_limits<MeasCmper>::max)()); }

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->DetailsBase::integrityCheck(ptrToThis);
        if (endMeas < startMeas || startMeas <= 0) {
            // Especially when a file was upgraded from pre-modern group settings,
            // the staff groups can get a bit squirrelly. Groups like this should just
            // be ignored.                
            util::Logger::log(util::Logger::LogLevel::Info,
                "Staff group " + std::to_string(getCmper2()) + " for part " + std::to_string(getSourcePartId())
                + " starts at measure " + std::to_string(startMeas) + " and ends at measure " + std::to_string(endMeas));
        }
        if (!bracket) {
            // this is not an error. Finale omits the bracket node for groups with entirely default bracket info.
            bracket = std::make_shared<Bracket>(getDocument());
        }
    }

    constexpr static std::string_view XmlNodeName = "staffGroup"; ///< XML node name for this type.
    static const xml::XmlElementArray<StaffGroup>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StaffGroupInfo
 * @brief Provides run-time information about Staff Groups within a system or list of @ref others::StaffUsed.
 */
class StaffGroupInfo
{
public:
    std::optional<size_t> startSlot;                    ///< the 0-based start slot (index) of the group in the system staves.
    std::optional<size_t> endSlot;                      ///< the 0-based end slot (index) of the group in the system staves.
    MusxInstance<StaffGroup> group;                     ///< the StaffGroup record for the group.
    MusxInstanceList<others::StaffUsed> systemStaves;    ///< the system staves referred to by startSlot and endSlot

    /// @brief Constructs information about a specific StaffGroup as it relates the the @p systemStaves
    /// @param staffGroup The staff group
    /// @param inpSysStaves The @ref others::StaffUsed list for a system or Scroll view.
    StaffGroupInfo(const MusxInstance<StaffGroup>& staffGroup,
        const MusxInstanceList<others::StaffUsed>& inpSysStaves);

    /// @brief The number of staves in the group for the #systemStaves.
    std::optional<size_t> numStaves() const
    {
        if (startSlot && endSlot) {
            return endSlot.value() - startSlot.value() + 1;
        }
        return std::nullopt;
    }

    /// @brief Iterates the staves in the group in order according to #systemStaves.
    /// @param measId The measure for which to construct each @ref others::StaffComposite instance.
    /// @param eduPosition The Edu position for which to construct each @ref others::StaffComposite instance.
    /// @param iterator The iterator function. Returning false from this function terminates iteration.
    void iterateStaves(MeasCmper measId, Edu eduPosition, std::function<bool(const MusxInstance<others::StaffComposite>&)> iterator) const;

    /// @brief Returns true if this staff group spans multiple staves with the same exact instrument UUID,
    ///        excluding groups that represent one logical instrument in the document's instrument map.
    /// @param measureId The measure for which to evaluate staff styles and group membership.
    bool calcIsSingleInstrumentSection(MeasCmper measureId) const;

    /// @brief Creates a vector of #StaffGroupInfo instances for the measure, part, and system staves
    /// @param measureId The measure to find.
    /// @param linkedPartId The ID of the linked part in which to find the groups.
    /// @param systemStaves The @ref others::StaffUsed list for a system or Scroll view.
    static std::vector<StaffGroupInfo> getGroupsAtMeasure(MeasCmper measureId, Cmper linkedPartId,
        const MusxInstanceList<others::StaffUsed>& systemStaves);
};

/**
 * @class StaffSize
 * @brief Represents a per-staff-size override for a specific staff in a system.
 *
 * Cmper1 is the system number and Cmper2 is the staff number (staffId) @ref Cmper.
 *
 * This class is identified by the XML node name "staffSize".
 */
class StaffSize : public DetailsBase
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for.
     * @param shareMode The sharing mode for this #StaffSize.
     * @param system The staff system number (Cmper1).
     * @param staffId The staff number (Cmper2).
     */
    explicit StaffSize(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper system, Cmper staffId)
        : DetailsBase(document, partId, shareMode, system, staffId)
    {
    }

    int staffPercent{}; ///< The staff size percentage override. (A value of 100 means 100%, i.e, no staff scaling.)

    /// @brief Calculate the staff scaling.
    util::Fraction calcStaffScaling() const
    { return util::Fraction::fromPercent(staffPercent); }

    constexpr static std::string_view XmlNodeName = "staffSize"; ///< The XML node name for this type.
    static const xml::XmlElementArray<StaffSize>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StemAlterations
 * @brief Specifies horizontal and vertical stem adjustments for upstem and downstem contexts.
 *
 * Entry::stemDetail is set if there are any instances of this class.
 *
 * This class is identified by the XML node name "stemAdjust".
 */
class StemAlterations : public EntryDetailsBase
{
public:
    /** @brief Constructor function */
    explicit StemAlterations(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum)
        : EntryDetailsBase(document, partId, shareMode, entnum)
    {
    }

    Evpu upVertAdjust{};    ///< Vertical adjustment for upstem context (positive is up).
    Evpu downVertAdjust{};  ///< Vertical adjustment for downstem context (positive is up).
    Evpu upHorzAdjust{};    ///< Horizontal adjustment for upstem context (positive is right).
    Evpu downHorzAdjust{};  ///< Horizontal adjustment for downstem context (positive is right).

    constexpr static std::string_view XmlNodeName = "stemAdjust"; ///< The XML node name for this type.
    static const xml::XmlElementArray<StemAlterations>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class StemAlterationsUnderBeam
 * @brief Specifies horizontal and vertical adjustments for stems under beam
 *
 * Entry::stemDetail is set if there are any instances of this class.
 *
 * This class is identified by the XML node name "stemAdjust".
 */
class StemAlterationsUnderBeam : public StemAlterations
{
public:
    using StemAlterations::StemAlterations;

    constexpr static std::string_view XmlNodeName = "beamStemAdjust"; ///< The XML node name for this type.
};

/**
 * @class TablatureNoteMods
 * @brief Specifies the TAB string a note appears on. Finale automatically figures out the fret number from
 * the open string note and fret intervals specified in the staff's associated @ref others::FretInstrument.
 *
 * #Entry::noteDetail is set if any note in the entry has tablature note mods.
 *
 * This class is identified by the XML node name "tabAlter".
 */
class TablatureNoteMods : public NoteDetailsBase
{
public:
    /** @brief Constructor function */
    explicit TablatureNoteMods(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : NoteDetailsBase(document, partId, shareMode, entnum, inci)
    {
    }

    NoteNumber noteId{};   ///< The ID of the tablature note being modified. (xml node is `<noteID>`)
    int stringNumber{};    ///< The 1-based string number on which the note is played. String numbers go from top to bottom on the tablature staff.
                           ///< Subtract 1 for an index into #others::FretInstrument::strings.

    NoteNumber getNoteId() const override { return noteId; }

    constexpr static std::string_view XmlNodeName = "tabAlter"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TablatureNoteMods>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TieAlterBase
 * @brief Base class for tie alteration properties. (Used for both ties and tie ends.)
 */
class TieAlterBase : public NoteDetailsBase
{
public:
    /** @brief Constructor function */
    explicit TieAlterBase(const DocumentWeakPtr& document, Cmper partId, EnigmaBase::ShareMode shareMode, EntryNumber entnum, Inci inci)
        : NoteDetailsBase(document, partId, shareMode, entnum, inci)
    {
    }

    /// @brief Gets the the @ref TieAlterStart or @ref TieAlterEnd for this note, based on the input boolean.
    /// @param noteInfoPtr The note for which to get the specified tie alteration record.
    /// @param forTieEnd If @p forTieEnd is true, get the @ref TieAlterStart. Otherwise get the @ref TieAlterEnd.
    static MusxInstance<TieAlterBase> fromNoteInfo(const NoteInfoPtr& noteInfoPtr, bool forTieEnd = false);

    /**
     * @enum ConnectionType
     * @brief Defines tie connection points for start and end attachment locations.
     *
     * These values correspond to Finale’s internal tie connection codes,
     * represented in XML by tags such as `<stPtEntCnct>noteCenterNoteTop</stPtEntCnct>`.
     */
    enum class ConnectionType
    {
        None,                   ///< No connection
        EntryLeftNoteCenter,    ///< Left of entry, center of note
        EntryRightNoteCenter,   ///< Right of entry, center of note
        NoteLeftNoteCenter,     ///< Left of note, center of note
        NoteRightNoteCenter,    ///< Right of note, center of note
        NoteCenterNoteBottom,   ///< Horizontal center of note, bottom of note
        NoteCenterNoteTop,      ///< Horizontal center of note, top of note
        DotRightNoteCenter,     ///< Right of dot(s), center of note
        AccidentalLeftNoteCenter, ///< Left of accidental, center of note
        EntryCenterNoteBottom,  ///< Horizontal center of entry, bottom of note
        EntryCenterNoteTop,     ///< Horizontal center of entry, top of note
        NoteLeftNoteBottom,     ///< Left of note, bottom of note
        NoteRightNoteBottom,    ///< Right of note, bottom of note
        NoteLeftNoteTop,        ///< Left of note, top of note
        NoteRightNoteTop,       ///< Right of note, top of note
        SystemEnd,              ///< System end (for the right side across systems)
        SystemStart             ///< System start (for the left side across systems)
    };

    /**
     * @enum DirectionContext
     * @brief Indicates the placement context of end- and control-point adjustments.
     */
    enum class DirectionContext
    {
        None,       ///< No context specified.
        Under,      ///< The tie is positioned below (e.g., tie under)
        Over        ///< The tie is positioned above (e.g., tie over)
    };

    NoteNumber noteId{};                   ///< Note ID associated with the tie alteration. (xml node is `<noteID>`)
    Evpu xStart{};                         ///< Horizontal start position of the tie.
    Evpu xEnd{};                           ///< Horizontal end position of the tie.
    Evpu yStart{};                         ///< Vertical start position of the tie.
    Evpu yEnd{};                           ///< Vertical end position of the tie.
    bool outerLocal{};                     ///< Local setting for "Outer Tie".
    bool outerOn{};                        ///< Enable "Outer" placement.
    bool afterSingleDotLocal{};            ///< Local setting for "After Single Dot".
    bool afterSingleDotOn{};               ///< Enable "After Single Dot".
    bool afterMultiDotsLocal{};            ///< Local setting for "After Multiple Dots".
    bool afterMultiDotsOn{};               ///< Enable "After Multiple Dots".
    bool shiftForSecondsLocal{};           ///< Local setting for "Shift for Seconds".
    bool shiftForSecondsOn{};              ///< Enable "Shift for Seconds".
    bool beforeSingleAcciLocal{};          ///< Local setting for "Before Single Accidental".
    bool beforeSingleAcciOn{};             ///< Enable "Before Single Accidental".
    bool down{};                           ///< Downward tie. Only applicable if #freezeDirection is true. (False freezes the tie up.)
    bool breakTimeLocal{};                 ///< Local setting for "Break at Time Signature".
    bool breakTimeOn{};                    ///< Enable "Break at Time Signature".
    bool breakKeyLocal{};                  ///< Local setting for "Break at Key Signature".
    bool breakKeyOn{};                     ///< Enable "Break at Key Signature".
    bool freezeDirection{};                ///< Freeze tie direction. (xml node is `<freeze>`)
    bool noSpecialArc{};                   ///< Inverse of "Avoid Staff Lines". (See @ref options::TieOptions for more information about the naming.)
    bool stPtAdjOn{};                      ///< Enable start point adjustment.
    ConnectionType stPtEntCnct{};          ///< Start point must match this connection type for #xStart & #yStart to apply.
    DirectionContext stPtContext{};        ///< Start point must match this direction context for #xStart & #yStart to apply.
    bool enPtAdjOn{};                      ///< Enable end point adjustment.
    ConnectionType enPtEntCnct{};          ///< End point must match this connection type for #xEnd & #yEnd to apply.
    DirectionContext enPtContext{};        ///< End point must match this direction context for #xEnd & #yEnd to apply.
    Evpu insetRatio1{};                    ///< Inset ratio for the first control point.
    Evpu height1{};                        ///< Height of the first control point.
    Evpu insetRatio2{};                    ///< Inset ratio for the second control point.
    Evpu height2{};                        ///< Height of the second control point.
    bool ctlPtAdjOn{};                     ///< Enable control point adjustment.
    bool ctlPtFixed{};                     ///< Indicates that the inset values are fixed Evpu rather than percentages.
                                           ///< This is a legacy compatibility setting and cannot be set in the tie alteration UI.

    NoteNumber getNoteId() const override { return noteId; }

    static const xml::XmlElementArray<TieAlterBase>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TieAlterEnd
 * @brief Alterations for tie ends.
 *
 * #Entry::dotTieAlt is set if any note in the entry has tie alterations.
 *
 * This class is identified by the XML node name "tieAlterEnd".
 */
class TieAlterEnd : public TieAlterBase
{
public:
    using TieAlterBase::TieAlterBase;

    constexpr static std::string_view XmlNodeName = "tieAlterEnd"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TieAlterEnd>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TieAlterStart
 * @brief Alterations for tie starts. (Tie starts are normal ties.)
 *
 * #Entry::dotTieAlt is set if any note in the entry has tie alterations.
 *
 * This class is identified by the XML node name "tieAlterStart".
 */
class TieAlterStart : public TieAlterBase
{
public:
    using TieAlterBase::TieAlterBase;

    constexpr static std::string_view XmlNodeName = "tieAlterStart"; ///< The XML node name for this type.
    static const xml::XmlElementArray<TieAlterStart>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class TupletDef
 * @brief Options controlling the appearance of tuplets.
 *
 * #Entry::tupletStart is set if the entry starts a tuplet.
 *
 * This class is identified by the XML node name "tupletDef".
 */
class TupletDef : public EntryDetailsBase
{
public:
    /** @brief Constructor function */
    explicit TupletDef(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : EntryDetailsBase(document, partId, shareMode, entnum, inci)
    {
    }

    /// @brief see @ref options::TupletOptions::AutoBracketStyle
    using AutoBracketStyle = options::TupletOptions::AutoBracketStyle;
    /// @brief see @ref options::TupletOptions::NumberStyle
    using NumberStyle = options::TupletOptions::NumberStyle;
    /// @brief see @ref options::TupletOptions::PositioningStyle
    using PositioningStyle = options::TupletOptions::PositioningStyle;
    /// @brief see @ref options::TupletOptions::BracketStyle
    using BracketStyle = options::TupletOptions::BracketStyle;

    int displayNumber{};                    ///< The number of notes to display (xml node is `<symbolicNum>`)
    Edu displayDuration{};                  ///< The duration of each note to display (xml node is `<symbolicDur>`)
    int referenceNumber{};                  ///< The number of notes "in the time of" (xml node is `<refNum>`)
    Edu referenceDuration{};                ///< The duration of each note "in the time of" (xml node is `<refDur>`)
    bool alwaysFlat{};                      ///< "Always Flat" (xml node is `<flat>`)
    bool fullDura{};                        ///< "Bracket Full Duration"
    bool metricCenter{};                    ///< "Center Number Using Duration"
    bool avoidStaff{};                      ///< "Avoid Staff"
    AutoBracketStyle autoBracketStyle{};    ///< Autobracket style
    bool hidden{};                          ///< Inverse of "Show"
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

    /** @brief return the reference duration as a @ref util::Fraction of a whole note */
    util::Fraction calcReferenceDuration() const { return util::Fraction(referenceNumber * referenceDuration, Edu(NoteType::Whole)); }

    /** @brief return the display duration as a @ref util::Fraction of a whole note */
    util::Fraction calcDisplayDuration() const { return util::Fraction(displayNumber * displayDuration, Edu(NoteType::Whole)); }

    /** @brief return the tuplet ratio (reference / display) */
    util::Fraction calcRatio() const { return util::Fraction(referenceNumber * referenceDuration, displayNumber * displayDuration); }

    constexpr static std::string_view XmlNodeName = "tupletDef";    ///< The XML node name for this type.
    static const xml::XmlElementArray<TupletDef>& xmlMappingArray();   ///< Required for musx::factory::FieldPopulator.
};

} // namespace details

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
template <>
struct PartContextRebinder<details::MeasureNumberIndividualPositioning>
{
    static void rebind(const std::shared_ptr<details::MeasureNumberIndividualPositioning>& positioning);
};
#endif

} // namespace dom
} // namespace entries
