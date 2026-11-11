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
#include "musx/util/Layout.h"
#include "musx/util/Fraction.h"

namespace musx {
namespace util {

/// @class EntryGeometry
/// @brief Resolves note and rest positions without interpreting stems or beam direction.
/// @details Staff-relative positions belong to each note's effective staff. calc() returns vertical positions with the
/// entry's source-staff reference line as zero, with upward-positive integer EVPU in the source staff's scale.
/// Results from entries on different source staves require coordinate conversion before comparison.
/// Part-voicing exclusions are honored; hidden and grace entries are not otherwise filtered. These are
/// notation positions, not glyph bounds or manually adjusted notehead drawing coordinates.
class EntryGeometry
{
public:
    /// @brief Availability of the entry's common-coordinate positions.
    enum class Resolution
    {
        Resolved,                 ///< Positions resolved; an empty entry can still have no note positions.
        MissingGeometry,          ///< At least one included position lacks usable metrics, origin, or TAB string data.
        UnsupportedLegacyContext  ///< Legacy mirror-frame transformations are not supported.
    };

    /// @brief Origin of external staff-position information consulted during resolution.
    enum class Source
    {
        NotRequired,    ///< Local staff metrics suffice; no external staff-origin information was consulted.
        Caller,         ///< A caller resolver handled a staff-origin request, including Unavailable.
        SavedSystem,    ///< Saved system staff positions were consulted.
        ScrollViewProxy ///< Scroll-view positions were consulted because no system staff list was available.
    };

    /// @brief An included note's positions, preserving its effective staff identity.
    struct NotePosition
    {
        dom::StaffCmper staffId{}; ///< Effective display staff, honoring the cross-staff display option.
        std::optional<int> staffPosition; ///< Diatonic staff position, or TAB string position, relative to this note's staff reference line.
        std::optional<int64_t> verticalPosition; ///< Upward-positive EVPU in the common coordinate space; empty when geometry is unavailable.
    };

    /// @brief Positions for one entry, with original note indices preserved.
    /// @details When resolution is MissingGeometry, known staff-relative and vertical positions remain available.
    /// Callers requiring complete chord extrema must check resolution before aggregating. A null note element
    /// means excluded by the producer (including part voicing) or replaced by a rest, not missing geometry.
    /// calc() uses the source-staff reference line as zero. Callers may translate all vertical positions to
    /// another common origin; staff-relative positions and source-staff metrics remain unchanged.
    /// Stem-specific adjustments and reversal positions are not included.
    struct Positions
    {
        Resolution resolution{Resolution::Resolved}; ///< Overall availability of the requested entry geometry.
        Source source{Source::NotRequired}; ///< Origin information consulted; Caller takes precedence over saved sources.
        dom::StaffCmper sourceStaffId{}; ///< Entry's source staff; its scale defines the common units and its reference line is the default origin.
        int64_t sourceStaffPositionSize{}; ///< EVPU per staff position on the source staff, using effective even line spacing.
        Fraction sourceStaffScale{1}; ///< Source staff's individual staff scaling ratio; common page/system scaling is excluded.
        std::vector<std::optional<NotePosition>> notes; ///< One element per original note index, including excluded elements.
        std::optional<int> restStaffPosition; ///< Displayed rest position relative to the source staff, if the entry displays as a rest.
        std::optional<int64_t> restVerticalPosition; ///< Displayed rest position in the common coordinate space, in source-staff EVPU.
    };

    /// @brief Resolves positions using effective staff metrics and optional cross-staff origin overrides.
    /// @param entry Valid entry in the requested part and view context.
    /// @param originResolver Optional callback returning downward-positive EVPU offsets before normalization
    /// to source-staff scale. NotHandled uses saved system positions, with Scroll View fallback only when no
    /// system staff list exists. Unavailable prevents fallback for that request. Callback exceptions propagate.
    /// @return Positions and availability; no partial set of positions is presented as a fully resolved chord.
    /// @throws std::invalid_argument If entry is invalid.
    /// @throws dom::integrity_error If required staff data is missing.
    static Positions calc(const dom::EntryInfoPtr& entry, const StaffOriginOffsetResolver& originResolver = {});
};

} // namespace util
} // namespace musx
