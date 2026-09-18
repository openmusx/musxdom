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

#include "musx/util/Logger.h"
#include "musx/util/Fraction.h"

#include "BaseClasses.h"
#include "CommonClasses.h"
 // do not add other dom class dependencies. Use Implementations.cpp for implementations that need total class access.

namespace musx {
namespace utils {
enum class PseudoTieMode;
} // namespace utils

namespace dom {

class EntryInfoPtr;

namespace details {
class SmartShapeEntryAssign;
}

namespace others {
    class SmartShapeMeasureAssign;
}

/// @enum KnownSmartShapeType
/// @brief Enumerates the custom smart shape types we can recognize semantically.
enum class KnownSmartShapeType
{
    Unrecognized,               ///< Fallback when recognition fails.
    VerticalLineRightHooks,     ///< Vertical line with short horizontal hooks extending to the right at both ends.

    // Add more known types here
};

namespace smartshape {

/**
 * @enum DirectionType
 * @brief Indicates the placement context of end- and control-point adjustments of @ref others::SmartShape instances.
 */
enum class DirectionType
{
    None,   ///< No context. (The default value when there is no adjustment or the context is not applicable to the SmartShape.)
    Under,  ///< The shape is positioned below (e.g., slur under)
    Over    ///< The shape is positioned above (e.g., slur over)
};

/**
 * @enum EntryConnectionType
 * @brief Indicates the entry connection type for entry-attached @ref others::SmartShape instances.
 *
 * These values are untested, so the documentation provides only a best guess as to what they do.
 * Some values are also used for beat-attached shapes, but how they apply in this context (if at all) is unknown.
 */
enum class EntryConnectionType
{
    HeadLeftTop,             ///< Attached to left-top of the head rectangle. (Default value.)
    HeadRightTop,            ///< Attached to right-top of the head rectangle.
    HeadRightBottom,         ///< Attached to right-bottom of the head rectangle.
    HeadLeftBottom,          ///< Attached to left-bottom of the head rectangle.

    StemLeftTop,             ///< Attached to left-top of the stem rectangle.
    StemRightTop,            ///< Attached to right-top of the stem rectangle.
    StemRightBottom,         ///< Attached to right-bottom of the stem rectangle.
    StemLeftBottom,          ///< Attached to left-bottom of the stem rectangle.

    NoteLeftTop,             ///< Attached to left-top of the notehead.
    NoteRightTop,            ///< Attached to right-top of the notehead.
    NoteRightBottom,         ///< Attached to right-bottom of the notehead.
    NoteLeftBottom,          ///< Attached to left-bottom of the notehead.
    NoteLeftCenter,          ///< Attached to left-center of the notehead.
    NoteRightCenter,         ///< Attached to right-center of the notehead.

    LyricRightCenter,        ///< Attached to right-center of the lyric.
    LyricLeftCenter,         ///< Attached to left-center of the lyric.
    LyricRightBottom,        ///< Attached to right-bottom of the lyric.
    HeadRightLyricBaseline,  ///< Attached to right of the head rectangle, aligned with the lyric baseline.
    DotRightLyricBaseline,   ///< Attached to the right of the augmentation dot, aligned with the lyric baseline.
    DurationLyricBaseline,   ///< Attached to the duration position, aligned with the lyric baseline.

    SystemLeft,              ///< Attached to the left edge of the system.
    SystemRight              ///< Attached to the right edge of the system.
};

/**
 * @brief Represents the manual adjustments to a smart shape.
 *
 * The meaning of these control point adjustments differs according to the type of SmartShape.
 */
class ControlPointAdjustment : public ContainedClassBase
{
public:
    using ContainedClassBase::ContainedClassBase;

    Evpu startCtlPtX{};             ///< Horizontal offset of the start control point.
    Evpu startCtlPtY{};             ///< Vertical offset of the start control point.
    Evpu endCtlPtX{};               ///< Horizontal offset of the end control point.
    Evpu endCtlPtY{};               ///< Vertical offset of the end control point.
    bool active{};                  ///< If true, this adjustment should be used when it is applicable.
    DirectionType contextDir{};     ///< The direction type for this adjustment.

    static const xml::XmlElementArray<ControlPointAdjustment>& xmlMappingArray();  ///< Required for musx::factory::FieldPopulator.
};

/**
 * @brief Represents an endpoint of the smart shape.
 */
class EndPoint : public ContainedClassBase
{
public:
    using ContainedClassBase::ContainedClassBase;

    StaffCmper staffId{};           ///< Staff ID (xml node is `<inst>`)
    MeasCmper measId{};             ///< Measure ID (xml node is `<meas>`)
    Edu eduPosition{};              ///< Edu position of endpoint (xml node is `<edu>`)
    EntryNumber entryNumber{};      ///< Entry number. Zero if the endpoint is not entry-attached. (xml node is `<entryNum>`)

    /// @brief Finale can leave dead smart shapes around. This function calculates if the endpoint's staff and measure match its
    /// endpoint, if any. It also verifies that the endpoint measure exists in the document.
    [[nodiscard]] bool calcIsValid() const;

    /// @brief Calculates the staff-level position of the endpoint within its measure, based on whether it is measure- or entry-attached
    [[nodiscard]] util::Fraction calcPosition() const;

    /// @brief Calculates the global position of the endpoint within its measure, based on whether it is measure- or entry-attached
    [[nodiscard]] util::Fraction calcGlobalPosition() const;

    /// @brief Compares the metric position of two endpoints.
    /// @param other The endpoint to compare.
    /// @return Integer value where
    ///         - negative (-1) means this is before @p other.
    ///         - 0 means the endpoints have the same metric position.
    ///         - positive (+1) means this is after @p other.
    [[nodiscard]] int compareMetricPosition(const EndPoint& other) const;

    /// @brief Calculates the entry associated with the endpoint.
    /// @note This function does not check for an actual assignment. It simply returns an entry the endpoint would be associated
    /// with if it were assigned. Use #calcIsAssigned to determine if the endpoint is actually assigned.
    /// @param findExact If true, only return an entry whose horizontal position matches the endpoint
    /// within 1 EVPU. No fallback search is performed. If false, return the closest entry in the measure,
    /// provided it lies within a quarter-beat of the endpoint.
    /// @return The entry if the endpoint is entry-attached or measure-attached within 1 Edu of an entry. Null if not.
    [[nodiscard]] EntryInfoPtr calcAssociatedEntry(bool findExact = false) const;

    /// @brief Gets the measure assignment for this endpoint or null if none.
    [[nodiscard]] MusxInstance<others::SmartShapeMeasureAssign> getMeasureAssignment() const;

    /// @brief Gets the entry assignment for this endpoint or null if none. Always null for measure-assigned endpoints.
    [[nodiscard]] MusxInstance<details::SmartShapeEntryAssign> getEntryAssignment() const;

    /// @brief Return true if this endpoint is properly assigned to its measure and to its entry (for entry-attached endpoints).
    [[nodiscard]] bool calcIsAssigned() const;

    /// @brief Return true if this endpoint is properly assigned to its measure and to its entry (for entry-attached endpoints).
    [[nodiscard]] MusxInstance<others::StaffComposite> createCurrentStaff() const;

    static const xml::XmlElementArray<EndPoint>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @brief Represents the endpoint adjustment of a smart shape or center shape.
 */
class EndPointAdjustment : public ContainedClassBase
{
public:
    using ContainedClassBase::ContainedClassBase;

    Evpu horzOffset{};                      ///< Horizontal offset (xml node is `<x>`)
    Evpu vertOffset{};                      ///< Vertical offset (xml node is `<y>`)
    bool active{};                          ///< If true, this adjustment should be used when it is applicable (xml node is `<on>`)
    DirectionType contextDir{};             ///< The direction type for this adjustment.
    EntryConnectionType contextEntCnct{};   ///< The entry conntection type for this adjustment.

    /// @brief Returns the effective horizontal offset, taking into account whether the endpoint is active.
    [[nodiscard]] Evpu calcHorzOffset() const
    { return active ? horzOffset : 0; }

    /// @brief Returns the effective vertical offset, taking into account whether the endpoint is active.
    [[nodiscard]] Evpu calcVertOffset() const
    { return active ? vertOffset : 0; }

    /// @brief Returns true if the two intances of @ref EndPointAdjustment have connection types that allow
    /// their vertical offsets to be compared.
    /// @param other The @ref EndPointAdjustment to compare.
    [[nodiscard]] bool calcHasVerticalEquivalentConnection(const EndPointAdjustment& other) const;

    static const xml::XmlElementArray<EndPointAdjustment>& xmlMappingArray();    ///< Required for musx::factory::FieldPopulator.
};

} // namespace smartshape

namespace others {

/**
 * @class SmartShape
 * @brief Represents a Finale smart shape.
 *
 * This class is identified by the XML node name "smartShape".
 */
class SmartShape : public OthersBase
{
private:
public:
    /** @brief Constructor function */
    explicit SmartShape(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    /**
     * @brief Represents the termination segment of the smart shape.
     */
    class TerminationSeg : public ContainedClassBase
    {
    public:
        using ContainedClassBase::ContainedClassBase;

        std::shared_ptr<smartshape::EndPoint> endPoint;                 ///< Endpoint information.
        std::shared_ptr<smartshape::EndPointAdjustment> endPointAdj;    ///< Endpoint adjustment information.
        std::shared_ptr<smartshape::ControlPointAdjustment> ctlPtAdj;   ///< Manual adjustments for the shape at this endpoint.
        std::shared_ptr<smartshape::EndPointAdjustment> breakAdj;       ///< System break adjustment for first or last system (depending which endpoint it is)
                                                                        ///< Systems other than the first or last are controlled with instances of @ref details::CenterShape.

        void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
        {
            ContainedClassBase::integrityCheck(ptrToThis);
            if (!endPoint) {
                endPoint = std::make_shared<smartshape::EndPoint>(getParent());
            }
            if (!endPointAdj) {
                endPointAdj = std::make_shared<smartshape::EndPointAdjustment>(getParent());
            }
            if (!ctlPtAdj) {
                ctlPtAdj = std::make_shared<smartshape::ControlPointAdjustment>(getParent());
            }
            if (!breakAdj) {
                breakAdj = std::make_shared<smartshape::EndPointAdjustment>(getParent());
            }
            endPoint->integrityCheck(endPoint);
            endPointAdj->integrityCheck(endPointAdj);
            ctlPtAdj->integrityCheck(ctlPtAdj);
            breakAdj->integrityCheck(breakAdj);
        }

        static const xml::XmlElementArray<TerminationSeg>& xmlMappingArray();    ///< Required for musx::factory::FieldPopulator.
    };

    /**
     * @enum ShapeType
     * @brief All the smart shape types supported by Finale
     *
     * The enum values match Finale's internal values. (Note that 24 is skipped.)
     * However, they are saved in the xml as text values. These are camel case and otherwise match
     * the enum names, except as noted.
     */
    enum class ShapeType : int
    {
        SlurDown = 0,       ///< Downward Slur (tips point up). This is the default and may not appear in the xml, but the text is in the Finale binary.
        SlurUp,             ///< Upward Slur (tips point down).
        Decrescendo,        ///< Decrescendo hairpin. (xml value is "decresc")
        Crescendo,          ///< Crescendo hairpin. (xml value is "cresc")
        OctaveDown,         ///< Ottava bassa. (8vb)
        OctaveUp,           ///< Ottava. (8va)
        DashLineUp,         ///< Dashed line with upward hook.
        DashLineDown,       ///< Dashed line with downward hook.                                == 7
        DashSlurDown,       ///< Thin line dashed downward slur (tips point up).
        DashSlurUp,         ///< Thin line dashed upward slur (tips point down).
        DashLine,           ///< Dashed line.
        SolidLine,          ///< Solid line.
        SolidLineDown,      ///< Solid line with downward right hook.
        SolidLineUp,        ///< Solid line with upward right hook.
        Trill,              ///< Trill (includes "tr" symbol at the beginning).                 == 14
        SlurAuto,           ///< Slur with direction automatically determined.
        DashSlurAuto,       ///< Thin line dashed slur with direction automatically determined.
        TrillExtension,     ///< Trill extension: only the wavy line. (xml value is "trillExt")
        SolidLineDownBoth,  ///< Solid line with downward hooks on both ends. (xml value is "solidLineDown2")
        SolidLineUpBoth,    ///< Solid line with upward hooks on both ends. (xml value is "solidLineUp2")
        TwoOctaveDown,      ///< Quindicesima bassa (15ma bassa)
        TwoOctaveUp,        ///< Quindicesima (15ma)                                            == 21
        DashLineDownBoth,   ///< Dashed line with downward hooks on both ends. (xml value is "dashLineDown2")
        DashLineUpBoth,     ///< Dashed line with upward hooks on both ends. (xml value is "dashLineUp2")
        Glissando = 25,     ///< Glissando (shape is determined by #lineStyleId)                == 25 (24 has no value)
        TabSlide,           ///< Tab slide (a solid line that is often used as a note-attached glissando)
        BendHat,            ///< Bend hat (solid up-down line with a corner in the middle)
        BendCurve,          ///< Bend curve
        CustomLine,         ///< User-created custom line with shape determined by #lineStyleId. (xml value is "smartLine")
        SolidLineUpLeft,    ///< Solid line with upward left hook.
        SolidLineDownLeft,  ///< Solid line with downward left hook.
        DashLineUpLeft,     ///< Dashed line with upward left hook.                             == 32
        DashLineDownLeft,   ///< Dashed line with downward left hook.
        SolidLineUpDown,    ///< Solid line with upward left hook and downward right hook.
        SolidLineDownUp,    ///< Solid line with downward left hook and upward right hook.
        DashLineUpDown,     ///< Dashed line with upward left hook and downward right hook.
        DashLineDownUp,     ///< Dashed line with downward left hook and upward right hook.
        Hyphen,             ///< Lyric hyphen, used only with lyric assignments.
        WordExtension,      ///< Lyric word extension, used only with lyric assignments. (xml value is "wordExt")
        DashContourSlurDown, ///< Contoured line dashed downward slur (tips point up).
        DashContourSlurUp,  ///< Contoured line dashed upward slur (tips point down).
        DashContourSlurAuto, ///< Contoured line dashed slur with direction automatically determined.
    };

    /**
     * @enum EngraverSlurState
     * @brief The selection for engraver slurs. (Only applicable for slurs.)
     */
    enum class EngraverSlurState
    {
        Auto,           ///< Take the engraver slur setting from #options::SmartShapeOptions::useEngraverSlurs. (Default value.)
        Off,            ///< Do not use engraver slur behaviour.
        On,             ///< Use engraver slur behaviour.
    };

    /**
     * @enum SlurAvoidAccidentalsState
     * @brief The selection whether this shape avoids accidentals. (Only applicable for slurs.)
     */
    enum class SlurAvoidAccidentalsState
    {
        Auto,           ///< Take the avoid accidentals setting from #options::SmartShapeOptions::slurAvoidAccidentals. (Default value.)
        Off,            ///< Do not use avoid accidentals behaviour.
        On,             ///< Use avoid accidentals behaviour.
        Invalid = -1    ///< May not be used, but exists as a possibility in the Finale app.
    };

    /**
     * @enum SystemBreakType
     * @brief How this shape breaks across systems. Represents the choice between "Make Horizontal Over System Break" and "Maintain Angle
     * Over System Break" in the Finale U.I.
     * @note Although this is presented in the xml as an enum, it appears to have only two values. There is some evidence that at one
     * point the developers had intentions to expand to more options here, but it seems those plans never materialized.
     */
    enum class SystemBreakType
    {
        Same,           ///< System break honors #makeHorz setting over a system break. (Default)
        Opposite,       ///< System break is the opposite of #makeHorz over a system break. So if
                        ///< #makeHorz is true, this allows the system break to be angled. If
                        ///< #makeHorz is false, this forces the system break to be horizontal.
    };

    ShapeType shapeType{};                          ///< Type of smart shape.
    bool entryBased{};                              ///< Whether the shape is entry-based.
    bool rotate{};                                  ///< Purpose unknown: always set for slurs.
    bool noPresetShape{};                           ///< Legacy flag that may no longer be used.
    bool makeHorz{};                                ///< "Make Horizontal"
    bool noPushEndStart{};                          ///< Legacy flag that should always be false in modern files going back to at least Finale 2000.
    bool makeVert{};                                ///< This option has no obvious setting in the Finale U.I. A plugin could perhaps set it, but whether it works is untested.
    EngraverSlurState engraverSlurState{};          ///< The engraver slur setting if this is a slur.
    SlurAvoidAccidentalsState slurAvoidAcciState{}; ///< The avoid accidentals settings if this is a slur.
    SystemBreakType yBreakType{};                   ///< Whether a system break should honor #makeHorz or do its opposite.
    std::shared_ptr<TerminationSeg> startTermSeg;   ///< Start termination segment.
    std::shared_ptr<TerminationSeg> endTermSeg;     ///< End termination segment.
    std::shared_ptr<smartshape::ControlPointAdjustment> fullCtlPtAdj; ///< If the shape is only on one staff system, this is where the manual edits are.
    bool hidden{};                                  ///< Inverse of "Show" option.
    ShapeDirection direction{};                     ///< Direction setting for this shape (xml node is `<dir>`).
    NoteNumber startNoteId{};                       ///< If non-zero, the specific note with the entry that this shape starts from. (xml node is `<startNoteID>`)
    NoteNumber endNoteId{};                         ///< If non-zero, the specific note with the entry that this shape ends on. (xml node is `<endNoteID>`)
    Cmper lineStyleId{};                            ///< If non-zero, the @ref SmartShapeCustomLine for this shape. Several #ShapeType values use it. (xml node is `<lineStyleID>`)
    Cmper startLyricNum{};                          ///< The text block of the lyrics text if this is a word extension or hyphen smart shape.
    Cmper endLyricNum{};                            ///< This value appears to be meaningless. It is often zero for word extensions or the same value as #startLyricNum for hyphens.
                                                    ///< It can also have an apparent nonsense value (.e.g, "-2"). The meaning of this is not known. The Finale U.I. does not appear
                                                    ///< to allow hyphen or word extensions between syllables from two different lyric blocks, so the need for start and end blocks
                                                    ///< is unclear.
    std::optional<LyricTextType> startLyricType;    ///< The type of lyrics block for #startLyricNum. (xml node is `<startLyricTag>`)
    std::optional<LyricTextType> endLyricType;      ///< The type of lyrics block for #endLyricNum. The speculative comments at #endLyricNum also apply here.
                                                    ///< This value has never been seen to be different than #startLyricNum unless endLyricNum is zero. (xml node is `<endLyricTag>`)

    /// @brief Finale sometimes leaves dead smart shapes hanging around that it does not display. This
    /// function tells you if you should process the smart shape.
    [[nodiscard]]
    bool calcIsValid() const;

    /// @brief Returns true if this smart shape is a type of slur.
    [[nodiscard]]
    bool calcIsSlur() const;

    /// @brief Returns true if this smart shape is a lyric hyphen or word extension, which Finale
    /// creates for lyric assignments rather than through the Smart Shape tool.
    [[nodiscard]]
    bool calcIsLyricShape() const;

    /// @brief Calculates if the smart shape applies to the specified entry.
    ///
    /// This function is most useful for shape types like ottavas and hairpins. It does
    /// not check layers or staves in between start and end staves, so it may be less useful for slurs.
    /// @param entryInfo The entry to check
    [[nodiscard]]
    bool calcAppliesTo(const EntryInfoPtr& entryInfo) const;

    /// @brief Creates a music range for the SmartShape in staff EDUs. If the SmartShape spans staves with different
    /// EDUs, the start endpoint uses the start staff's EDUs and the endpoint uses the end staff's EDUs.
    /// @return The created music range.
    [[nodiscard]]
    MusicRange createMusicRange() const;

    /// @brief Creates a music range for the SmartShape in global EDUs.
    /// @return The created music range.
    [[nodiscard]]
    MusicRange createGlobalMusicRange() const;

    /// @brief Returns the note this shape starts from.
    ///
    /// #startNoteId names the note within the start endpoint's entry. The endpoint's own entry is
    /// used as it stands; no nearby entry is substituted.
    /// @note An entry does not imply a note: an endpoint on a zero-note entry resolves to that
    /// entry and still returns null.
    /// @return The note, or null if #entryBased is not set, the start endpoint's entry cannot be
    /// resolved, or that entry contains no note with id #startNoteId.
    [[nodiscard]] NoteInfoPtr calcStartNote() const;

    /// @brief Returns the note this shape ends on.
    ///
    /// #endNoteId names the note within the end endpoint's entry. The endpoint's own entry is used
    /// as it stands; no nearby entry is substituted.
    /// @note An entry does not imply a note: an endpoint on a zero-note entry resolves to that
    /// entry and still returns null.
    /// @return The note, or null if #entryBased is not set, the end endpoint's entry cannot be
    /// resolved, or that entry contains no note with id #endNoteId.
    [[nodiscard]] NoteInfoPtr calcEndNote() const;

    /// @brief Returns the tied-to note if this slur is being used as an arpeggiated tie on the specified entry.
    ///
    /// - The start entry must consist only of a single note.
    /// - There must be no note that the start entry's note could be tied to.
    /// @param forStartEntry The entry to check.
    /// @return The NoteInfoPtr that is the tied-to note for this note, or null if none.
    [[nodiscard]] NoteInfoPtr calcArpeggiatedTieToNote(const EntryInfoPtr& forStartEntry) const;

    /// @brief Determines whether the smart shape's contour runs up or down.
    /// @return The contour direction, or `CurveContourDirection::Unspecified` when the direction is automatic.
    [[nodiscard]]
    CurveContourDirection calcContourDirection() const;

    /// @brief Calculates a vertical placement classification for beat-attached smart shapes.
    /// @returns Above if both endpoints are above the staff, Below if both endpoints are below the staff. Otherwise
    /// it returns Float. If the smart shape is not beat-attached, it returns NotApplicable.
    [[nodiscard]] VerticalPlacement calcVerticalPlacementForBeatAttached() const;
    
    /// @brief Returns true if the smart shape's #ShapeType uses a dashed line style.
    /// @return True when the shape type is dashed; false otherwise.
    [[nodiscard]]
    bool calcIsDashed() const;

    /// @brief Returns true if this slur is being used as a pseudo tie for the specified mode.
    /// It is used by #NoteInfoPtr::calcPseudoTieInfoInternal, which imposes additional rules and checks.
    /// @param mode The pseudo tie mode to evaluate.
    /// @param forStartEntry The entry to check.
    [[nodiscard]]
    bool calcIsPseudoTie(utils::PseudoTieMode mode, const EntryInfoPtr& forStartEntry) const;

    /// @brief Iterates all the entries that start within the staves and music range defined by the SmartShape. It iterates by staff and then measure.
    /// @param iterator The iterator function. Return `false` from this function to stop iterating.
    /// @param staffList Optional staff list to use for iteration. If omitted, the Scroll View staff list of the current part is used.
    /// @return True if all items were iterated. False if the @p iterator returned false and exited early.
    bool iterateEntries(std::function<bool(const EntryInfoPtr&)> iterator, DeferredReference<MusxInstanceList<StaffUsed>> staffList = {}) const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        OthersBase::integrityCheck(ptrToThis);
        if (!startTermSeg) {
            startTermSeg = std::make_shared<TerminationSeg>(ptrToThis);
        }
        if (!endTermSeg) {
            endTermSeg = std::make_shared<TerminationSeg>(ptrToThis);
        }
        if (!fullCtlPtAdj) {
            fullCtlPtAdj = std::make_shared<smartshape::ControlPointAdjustment>(ptrToThis);
        }
        startTermSeg->integrityCheck(ptrToThis);
        endTermSeg->integrityCheck(ptrToThis);
        fullCtlPtAdj->integrityCheck(ptrToThis);
    }

private:
    /// @brief Shared implementation of #calcStartNote and #calcEndNote.
    /// @param termSeg The termination segment to resolve. Must not be null.
    /// @param noteId The note id belonging to @p termSeg.
    [[nodiscard]] NoteInfoPtr calcEndpointNote(
        const std::shared_ptr<TerminationSeg>& termSeg, NoteNumber noteId) const;

public:
    constexpr static std::string_view XmlNodeName = "smartShape"; ///< XML node name
    static const xml::XmlElementArray<SmartShape>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class SmartShapeCustomLine
 * @brief Represents a Finale smart shape custom line style.
 *
 * This class is identified by the XML node name "ssLineStyle".
 */
class SmartShapeCustomLine : public OthersBase
{
    util::EnigmaParsingContext getRawTextCtx(Cmper forPartId, Cmper textBlockId) const;

public:
    /// @brief Constructor function
    explicit SmartShapeCustomLine(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    /// @enum LineStyle
    /// @brief The type of line style.
    enum class LineStyle { Char, Solid, Dashed };

    /// @enum LineCapType
    /// @brief The type of line cap.
    enum class LineCapType { None, Hook, ArrowheadPreset, ArrowheadCustom };

    /// @brief Hold parameters for LineStyle::Char.
    class CharParams : public ContainedClassBase
    {
    public:
    /// @brief Constructor function
        explicit CharParams(const MusxInstance<EnigmaBase>& parent)
            : ContainedClassBase(parent), font(std::make_shared<FontInfo>(parent->getDocument()))
        {
        }

        char32_t lineChar{};            ///< The character to use for the line
        std::shared_ptr<FontInfo> font; ///< `<fontID>`, `<fontSize>`, `<fontEfx>`
        int baselineShiftEms{};         ///< The UI says the units are "EMs", but it may be 1/100 EMs.

        static const xml::XmlElementArray<CharParams>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /// @brief Hold parameters for LineStyle::Solid.
    class SolidParams : public ContainedClassBase
    {
    public:
        using ContainedClassBase::ContainedClassBase;

        Efix lineWidth{};             ///< Solid line width

        static const xml::XmlElementArray<SolidParams>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    /// @brief Hold parameters for LineStyle::Dashed.
    class DashedParams : public ContainedClassBase {
    public:
        using ContainedClassBase::ContainedClassBase;

        Efix lineWidth{};             ///< Dashed line width
        Efix dashOn{};                ///< Dash length
        Efix dashOff{};               ///< Length of gap between dashes

        static const xml::XmlElementArray<DashedParams>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
    };

    LineStyle lineStyle{};                                ///< Line style
    // No fixture has a part-specific SmartShapeCustomLine. If partial sharing is observed, its
    // applicable parameter object must be copied and rebound before nested fields are overlaid.
    // See PartContextRebinder<details::CenterShape> and the CenterShape field mappings for the pattern.
    std::shared_ptr<CharParams> charParams;               ///< Parameters for char line style. Only allocated if lineStyle is LineStyle::Char.
    std::shared_ptr<SolidParams> solidParams;             ///< Parameters for solid line style. Only allocated if lineStyle is LineStyle::Solid.
    std::shared_ptr<DashedParams> dashedParams;           ///< Parameters for dashed line style. Only allocated if lineStyle is LineStyle::Dashed.

    LineCapType lineCapStartType{};                       ///< Line cap start type
    LineCapType lineCapEndType{};                         ///< Line cap end type
    Cmper lineCapStartArrowId{};                          ///< Cmper of start arrowhead (#ArrowheadPreset or cmper of custom @ref ShapeDef). xml node is `<lineCapStartArrowID>`
    Cmper lineCapEndArrowId{};                            ///< Cmper of end arrowhead (#ArrowheadPreset or cmper of custom @ref ShapeDef). xml node is `<lineCapStartArrowID>`

    bool makeHorz{};                                      ///< "Horizontal"
    bool lineAfterLeftStartText{};                        ///< Line adjustments "Start H: After Text"
    bool lineBeforeRightEndText{};                        ///< Line adjustments "Right H: Before Text"
    bool lineAfterLeftContText{};                         ///< Line adjustments "Cont H: After Text"

    Cmper leftStartRawTextId{};                           ///< Cmper of @ref texts::SmartShapeText. xml node is `<leftStartRawTextID>`
    Cmper leftContRawTextId{};                            ///< Cmper of @ref texts::SmartShapeText. xml node is `<leftContRawTextID>`
    Cmper rightEndRawTextId{};                            ///< Cmper of @ref texts::SmartShapeText. xml node is `<rightEndRawTextID>`
    Cmper centerFullRawTextId{};                          ///< Cmper of @ref texts::SmartShapeText. xml node is `<centerFullRawTextID>`
    Cmper centerAbbrRawTextId{};                          ///< Cmper of @ref texts::SmartShapeText. xml node is `<centerAbbrRawTextID>`

    Evpu leftStartX{};                                    ///< Left start position X
    Evpu leftStartY{};                                    ///< Left start position Y
    Evpu leftContX{};                                     ///< Left continuation position X
    Evpu leftContY{};                                     ///< Left continuation position Y
    Evpu rightEndX{};                                     ///< Right end position X
    Evpu rightEndY{};                                     ///< Right end position Y
    Evpu centerFullX{};                                   ///< Center full position X
    Evpu centerFullY{};                                   ///< Center full position Y
    Evpu centerAbbrX{};                                   ///< Center abbreviation position X
    Evpu centerAbbrY{};                                   ///< Center abbreviation position Y

    Evpu lineStartX{};                                    ///< Line adjustments "Start H" value
    Evpu lineStartY{};                                    ///< Line adjustments "V" value (Finale syncs this with #lineEndY.)
    Evpu lineEndX{};                                      ///< Line adjustments "End H" value
    Evpu lineEndY{};                                      ///< Line adjustments "Start H" value
    Evpu lineContX{};                                     ///< Line adjustments "V" value (Finale syncs this with #lineStartY.)

    Efix lineCapStartHookLength{};                        ///< Length of start hook (if #lineStyle is Hook)
    Efix lineCapEndHookLength{};                          ///< Length of end hook (if #lineStyle is Hook)

    /**
     * @brief Gets the raw text context for parsing the left-start, or nullptr if none.
     * @param forPartId The linked part to use for ^partname and ^totpages inserts.
    */
    [[nodiscard]] 
    util::EnigmaParsingContext getLeftStartRawTextCtx(Cmper forPartId) const
    { return getRawTextCtx(forPartId, leftStartRawTextId); }

    /**
     * @brief Gets the raw text context for parsing the left-continuation text, or nullptr if none.
     * @param forPartId The linked part to use for ^partname and ^totpages inserts.
    */
    [[nodiscard]] 
    util::EnigmaParsingContext getLeftContRawTextCtx(Cmper forPartId) const
    { return getRawTextCtx(forPartId, leftContRawTextId); }

    /**
     * @brief Gets the raw text context for parsing the right-end text, or nullptr if none.
     * @param forPartId The linked part to use for ^partname and ^totpages inserts.
    */
    [[nodiscard]] 
    util::EnigmaParsingContext getRightEndRawTextCtx(Cmper forPartId) const
    { return getRawTextCtx(forPartId, rightEndRawTextId); }

    /**
     * @brief Gets the raw text context for parsing the center-full text, or nullptr if none.
     * @param forPartId The linked part to use for ^partname and ^totpages inserts.
    */
    [[nodiscard]] 
    util::EnigmaParsingContext getCenterFullRawTextCtx(Cmper forPartId) const
    { return getRawTextCtx(forPartId, centerFullRawTextId); }

    /**
     * @brief Gets the raw text context for parsing the center-abbreviated text, or nullptr if none.
     * @param forPartId The linked part to use for ^partname and ^totpages inserts.
    */
    [[nodiscard]] 
    util::EnigmaParsingContext getCenterAbbrRawTextCtx(Cmper forPartId) const
    { return getRawTextCtx(forPartId, centerAbbrRawTextId); }

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        OthersBase::integrityCheck(ptrToThis);

        if (lineStyle == LineStyle::Char && !charParams)
            charParams = std::make_shared<CharParams>(ptrToThis);

        if (lineStyle == LineStyle::Solid && !solidParams)
            solidParams = std::make_shared<SolidParams>(ptrToThis);

        if (lineStyle == LineStyle::Dashed && !dashedParams)
            dashedParams = std::make_shared<DashedParams>(ptrToThis);
    }

    constexpr static std::string_view XmlNodeName = "ssLineStyle"; ///< The XML node name for this type.
    static const xml::XmlElementArray<SmartShapeCustomLine>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/// @brief Copies a custom line and its document-local dependencies into another document.
///
/// The copied line receives the next free @ref SmartShapeCustomLine cmper in @p target. Character
/// fonts, custom arrowhead shapes, and raw Smart Shape text strings are copied or resolved through
/// @ref importFontDefinitionInto, @ref importShapeDefInto, and @ref
/// texts::importTextInto. The new line is rewired to their target-document cmpers.
/// Repeated references to one source dependency share one imported target object.
///
/// Nothing is added to the SmartShapeCustomLine pool unless every dependency can be resolved.
/// Successfully imported dependencies may remain when a later dependency cannot be imported;
/// unreferenced fonts, shapes, and raw text strings do not affect rendering.
///
/// @param target The document to import into. May be the same document as @p source's.
/// @param source The custom line to import.
/// @param onImported Called for the custom line and every dependency this operation creates.
/// @return The copied line's cmper within @p target, or std::nullopt when a destination pool has no
/// free cmper or a referenced source object cannot be resolved.
/// @throws std::invalid_argument if @p target or @p source is null. Debug builds assert instead.
[[nodiscard]]
std::optional<Cmper> importSmartShapeCustomLineInto(const DocumentPtr& target,
    const MusxInstance<SmartShapeCustomLine>& source,
    const ImportObjectCallback& onImported = {});

/**
 * @class SmartShapeMeasureAssign
 * @brief Assigns a smart shape or center shape to a measure.
 *
 * Note that *every smart shape* has a measure assignment. This includes entry-assigned smart shapes that
 * *additionally* have an entry assignment.
 *
 * The cmper value is the measure Id.
 *
 * This class is identified by the XML node name "smartShapeMeasMark".
 */
class SmartShapeMeasureAssign : public OthersBase
{
public:
    /** @brief Constructor function */
    explicit SmartShapeMeasureAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci)
    {
    }

    Cmper shapeNum{};       ///< The @ref Cmper of the @ref SmartShape that is being assigned.
    Cmper centerShapeNum{}; ///< If non-zero, the 2nd @ref Cmper of the @ref details::CenterShape.
                            ///< Zero if this measure starts or ends the smart shape.

    constexpr static std::string_view XmlNodeName = "smartShapeMeasMark"; ///< The XML node name for this type.
    static const xml::XmlElementArray<SmartShapeMeasureAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

} // namespace others

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
template <>
struct PartContextRebinder<others::SmartShape>
{
    static std::shared_ptr<others::SmartShape::TerminationSeg> copyTerminationSeg(
        const std::shared_ptr<others::SmartShape::TerminationSeg>& source,
        const std::shared_ptr<others::SmartShape>& parent);
    static void rebind(const std::shared_ptr<others::SmartShape>& shape);
};
#endif

namespace details {

/**
 * @class CenterShape
 * @brief Represents a center shape for a @ref others::SmartShape that spans three or more measures.
 *
 * A @ref others::SmartShape instance controls the endpoint information for the start and end measures. It
 * also controls the first and the final system breaks if the shape spans more than one system.
 *
 * Every measure spanned by the shape in between the first and last measures is assigned its own center shape.
 * These are only applicable if their measure is the first measure in a system that is not the first or last system.
 * That means that for any center shape to be applicable, the shape must span at least three systems. However, the center shape
 * records are created for every measure between the first and last, no matter how many or few systems the shape spans.
 *
 * Cmper1 is the shape number. Cmper2 is the center shape number. (See @ref others::SmartShapeMeasureAssign.)
 * This class is identified by the XML node name "centerShape".
 */
class CenterShape : public DetailsBase
{
public:

    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for.
     * @param shareMode The sharing mode for this #CenterShape.
     * @param shapeNum The shape number from @ref others::SmartShapeMeasureAssign.
     * @param centerShapeNum The center shape number from @ref others::SmartShapeMeasureAssign.
     */
    explicit CenterShape(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper shapeNum, Cmper centerShapeNum)
        : DetailsBase(document, partId, shareMode, shapeNum, centerShapeNum)
    {}

    std::shared_ptr<smartshape::EndPointAdjustment> startBreakAdj; ///< Adjustment at the start break (xml: `<startBreakAdj>`)
    std::shared_ptr<smartshape::EndPointAdjustment> endBreakAdj;   ///< Adjustment at the end break (xml: `<endBreakAdj>`)
    std::shared_ptr<smartshape::ControlPointAdjustment> ctlPtAdj;  ///< Manual adjustments made to this center shape.

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        if (!startBreakAdj) {
            startBreakAdj = std::make_shared<smartshape::EndPointAdjustment>(ptrToThis);
        }
        if (!endBreakAdj) {
            endBreakAdj = std::make_shared<smartshape::EndPointAdjustment>(ptrToThis);
        }
        if (!ctlPtAdj) {
            ctlPtAdj = std::make_shared<smartshape::ControlPointAdjustment>(ptrToThis);
        }
    }

    constexpr static std::string_view XmlNodeName = "centerShape"; ///< The XML node name for this type.
    static const xml::XmlElementArray<CenterShape>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class SmartShapeEntryAssign
 * @brief Assigns a smart shape to an entry.
 *
 * This class is identified by the XML node name "smartShapeEntryMark".
 */
class SmartShapeEntryAssign : public EntryDetailsBase
{
public:
    /** @brief Constructor function */
    explicit SmartShapeEntryAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, EntryNumber entnum, Inci inci)
        : EntryDetailsBase(document, partId, shareMode, entnum, inci)
    {
    }

    Cmper shapeNum{};       ///< The @ref Cmper of the @ref others::SmartShape that is being assigned.

    constexpr static std::string_view XmlNodeName = "smartShapeEntryMark"; ///< The XML node name for this type.
    static const xml::XmlElementArray<SmartShapeEntryAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

} // namespace details

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
template <>
struct PartContextRebinder<details::CenterShape>
{
    static void rebind(const std::shared_ptr<details::CenterShape>& shape);
};
#endif

} // namespace dom
} // namespace musx
