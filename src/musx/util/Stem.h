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

#include "musx/dom/Entries.h"
#include "musx/util/EntryGeometry.h"

namespace musx {
namespace util {

/// @class Stem
/// @brief Static utilities for logical entry direction, beam direction, and per-note stem assignments.
/// @details Calculations use the entry's requested part, effective staff and layer settings, and part-voicing
/// policy. Cross-staff geometry comes from the document or caller-supplied resolvers. Results describe logical
/// stem choices, not visible stem endpoints or the slope of a manually positioned beam. Calculations do not
/// modify the document or cache caller-dependent results.
class Stem
{
public:
    /// @enum Direction
    /// @brief Logical direction used to select direction-dependent entry properties.
    enum class Direction
    {
        Up,   ///< Logical upstem.
        Down  ///< Logical downstem.
    };

    /// @enum Assignment
    /// @brief Logical stem membership of one note, independent of stem visibility.
    enum class Assignment
    {
        None, ///< No assignment, including notes excluded by part voicing or an unresolved primary direction.
        Up,   ///< Note belongs to the upstem.
        Down, ///< Note belongs to the downstem.
        Both  ///< Note belongs to both stems of a double-stem entry.
    };

    /// @enum Resolution
    /// @brief Outcome of the requested calculation, separate from whether a direction was selected.
    enum class Resolution
    {
        Resolved,                 ///< Calculation completed; a forced-direction query may still have no override.
        MissingGeometry,          ///< Required positions are unavailable, including TAB notes without a usable string assignment.
        UnsupportedLegacyContext, ///< A legacy mirror frame was encountered.
        NoEligiblePositions       ///< No eligible extrema remain after filtering notes, rests, and grace entries.
    };

    /// @brief Provenance of shared entry geometry used by the stem calculation.
    using GeometrySource = EntryGeometry::Source;

    /// @enum Reason
    /// @brief Rule that selected the contextual or geometric direction.
    enum class Reason
    {
        None,         ///< No direction-selection rule has succeeded.
        Rhythmic,     ///< Active rhythmic alternate notation supplies the direction.
        EntryFreeze,  ///< The entry's part-aware frozen direction applies.
        LayerFreeze,  ///< An applicable layer freeze supplies the direction.
        Staff,        ///< The effective staff forces a direction.
        Stored,       ///< Voice, grace, or special-stem behavior uses the stored direction.
        Extremes,     ///< Distances of adjusted extremes from the reversal position select the direction.
        BeamTieBreak  ///< Balanced beam extrema are resolved by counting both extremes of each eligible entry.
    };

    /// @brief Entry positions and stem-specific geometry for one entry.
    /// @details Coordinates are upward-positive integer EVPU. All entries in a beam must use the same origin
    /// and source-staff scale, with staff-origin offsets and relative scaling already applied. Values and their
    /// differences must be representable by int64_t. Supply note positions before reverse-stem adjustments.
    /// A caller may translate the embedded vertical positions and reversal together to another common origin;
    /// staff-relative positions and source-staff metrics retain their original meanings.
    struct Geometry
    {
        EntryGeometry::Positions entryGeometry; ///< Shared note/rest positions, staff metrics, provenance, and availability.
        int64_t reversal{}; ///< This entry's stem-reversal position in the common coordinate space.
        int64_t reverseAdjustment{}; ///< Reverse-stem adjustment in the same units and scale as the positions.
    };

    /// @brief Response from the complete-geometry resolver.
    struct GeometryResult
    {
        StaffOriginOffsetResolverDecision decision{StaffOriginOffsetResolverDecision::NotHandled};
            ///< Offset supplies #geometry; NotHandled requests document geometry; Unavailable prevents fallback for this entry.
        Geometry geometry; ///< Complete entry geometry, read only when #decision is Offset.
    };

    /// @brief Optional geometry providers for a stem calculation.
    /// @details Empty callbacks use document metrics and saved staff positions. Resolver exceptions propagate to the caller.
    struct Options
    {
        /// @brief Optional resolver for cross-staff origin offsets when complete geometry is not supplied.
        /// @details Offsets are downward-positive EVPU, as returned by the layout helpers, before normalization to
        /// source-staff scale. NotHandled uses the saved-system proxy, which falls back to Scroll View only when
        /// no system staff list is available. Unavailable prevents fallback. Document staff scaling still applies.
        StaffOriginOffsetResolver staffOriginOffsetResolver;

        /// @brief Optional resolver invoked for each entry whose geometry is requested.
        /// @details Offset supplies complete @ref musx::util::Stem::Geometry and bypasses the staff-origin resolver and document geometry
        /// for that entry. NotHandled uses document metrics and #staffOriginOffsetResolver. Unavailable produces
        /// #Resolution::MissingGeometry. The note-position array must match the original entry's note count;
        /// part-voicing exclusions still apply. Null note elements affect geometric participation, not stem assignments.
        /// The embedded resolution is honored; an included note with no vertical position is MissingGeometry.
        /// Complete geometry supplied here is reported as Caller provenance regardless of its embedded source.
        std::function<GeometryResult(const dom::EntryInfoPtr&)> geometryResolver;
    };

    /// @brief Direction decision and logical assignments for the queried entry.
    /// @details Inspect #resolution as well as #primaryDirection. An empty direction in a successful forced query
    /// means automatic direction, not missing data. For split/double-stem entries, an effective calculation can
    /// resolve a local primary direction even when the beam's geometry remains unresolved.
    struct DirectionInfo
    {
        std::optional<Direction> primaryDirection; ///< Queried entry's direction, or the selected override in a forced query; empty when none is determined.
        std::optional<Direction> beamDirection; ///< Group direction for a beamed default/effective query; empty for unbeamed or forced-only queries and unresolved groups.
        std::vector<Assignment> assignments; ///< Original-note-index assignments when populated; excluded notes are None. Forced-only queries leave this empty.
        Resolution resolution{Resolution::Resolved}; ///< Completion status; unresolved calculations do not silently substitute a stored direction.
        GeometrySource geometrySource{GeometrySource::NotRequired}; ///< Geometry provenance or attempted source; Caller takes precedence over document sources.
        Reason reason{Reason::None}; ///< Rule selecting the group/default decision or override; a special entry's local primary direction may differ.
        dom::EntryInfoPtr sourceEntry; ///< Entry supplying the override, or beam-start/current entry for a geometric decision; empty without a decision.
    };

    /// @brief Calculates geometric stem direction, including cross-staff placement and beam context.
    /// @details Excludes rhythmic, entry-freeze, layer, staff-direction, and voice forcing. Applies adjusted
    /// extrema, reverse-stem settings, rest-beam compatibility, and part-voicing exclusions. Grace and hidden
    /// entries do not contribute extrema. Balanced unbeamed extrema select down; balanced beams use the
    /// two-extreme vote, with equal votes selecting down. No eligible positions are reported explicitly.
    /// @param entry Valid entry whose direction and assignments are requested.
    /// @param options Optional providers for complete geometry and cross-staff origins.
    /// @return Geometric decision and assignments, or an unresolved status with no primary direction.
    /// @throws std::invalid_argument If entry is invalid or supplied geometry has the wrong note count.
    /// @throws dom::integrity_error If required staff or option records are missing.
    static DirectionInfo calcDefaultDirection(const dom::EntryInfoPtr& entry, const Options& options);

    /// @brief Calculates geometric direction using document geometry and no caller resolvers.
    /// @param entry Valid entry whose direction and assignments are requested.
    /// @return The result of calcDefaultDirection(entry, Options{}).
    /// @throws std::invalid_argument If entry is invalid.
    /// @throws dom::integrity_error If required staff or option records are missing.
    static DirectionInfo calcDefaultDirection(const dom::EntryInfoPtr& entry) { return calcDefaultDirection(entry, Options{}); }

    /// @brief Finds the first applicable contextual direction in beam order.
    /// @details At each entry, checks rhythmic notation, entry freeze, applicable layer freeze, staff forcing,
    /// and stored voice/grace direction. Unbeamed split/double-stem entries also use their stored direction.
    /// Stops at the first applicable entry; does not populate assignments or consult geometry.
    /// @param entry Valid entry whose beam or local override is requested.
    /// @param options Accepted for interface consistency; geometry callbacks are not invoked.
    /// @return Selected override, reason, and source entry; Resolved with no primary direction means automatic.
    /// An encountered legacy mirror frame produces UnsupportedLegacyContext.
    /// @throws std::invalid_argument If entry is invalid.
    /// @throws dom::integrity_error If required staff data is missing.
    static DirectionInfo calcForcedDirection(const dom::EntryInfoPtr& entry, const Options& options);

    /// @brief Finds a contextual direction with default options.
    /// @param entry Valid entry whose beam or local override is requested.
    /// @return The result of calcForcedDirection(entry, Options{}).
    /// @throws std::invalid_argument If entry is invalid.
    /// @throws dom::integrity_error If required staff data is missing.
    static DirectionInfo calcForcedDirection(const dom::EntryInfoPtr& entry) { return calcForcedDirection(entry, Options{}); }

    /// @brief Resolves contextual or geometric direction and per-note logical stem assignments.
    /// @details Uses the first contextual override, otherwise the geometric calculation. Ordinary beam members
    /// inherit the group direction; split/double-stem entries retain their local primary direction. Split flags
    /// assign notes to up or down stems; a split entry without up-split flags uses double-stem assignments.
    /// A resolved override needs no geometry. Visible stem endpoints and manual beam placement are not calculated.
    /// @param entry Valid entry whose effective direction and assignments are requested.
    /// @param options Optional geometry providers, used only if a geometric decision is needed.
    /// @return Effective entry and beam directions, assignments, and resolution status.
    /// @throws std::invalid_argument If entry is invalid or supplied geometry has the wrong note count.
    /// @throws dom::integrity_error If required staff or option records are missing.
    static DirectionInfo calcEffectiveDirection(const dom::EntryInfoPtr& entry, const Options& options);

    /// @brief Resolves effective direction and assignments using document geometry.
    /// @param entry Valid entry whose effective direction and assignments are requested.
    /// @return The result of calcEffectiveDirection(entry, Options{}).
    /// @throws std::invalid_argument If entry is invalid.
    /// @throws dom::integrity_error If required staff or option records are missing.
    static DirectionInfo calcEffectiveDirection(const dom::EntryInfoPtr& entry) { return calcEffectiveDirection(entry, Options{}); }
};

} // namespace util
} // namespace musx
