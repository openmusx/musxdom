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
#include "musx/dom/SmartShape.h"
#include "musx/util/Stem.h"
#include "musx/util/EntryGeometry.h"

namespace musx {
namespace util {

/// @brief Logical contour for solid, dashed, and dashed-contour slurs, without
/// curve layout.
class Slur
{
public:
    /// @brief Provenance of entry geometry used to resolve slur contour.
    using GeometrySource = EntryGeometry::Source;

    enum class Resolution
    {
        Resolved,
        AutomaticBeatAttached,
        MissingEndpoint,
        InvalidSpan,
        MissingGeometry,
        UnsupportedLegacyContext,
        NoEligibleEntries
    };
    enum class Reason
    {
        None,
        Definition,
        Tablature,
        GraceException,
        ContextualOverride,
        MixedStems,
        UniformStems
    };
    struct Options
    {
        /// @brief Optional cross-staff origin provider used when resolving automatic contour.
        /// @details Offsets are downward-positive EVPU before source-staff normalization.
        /// NotHandled uses document geometry; Unavailable prevents fallback.
        StaffOriginOffsetResolver staffOriginOffsetResolver;

        /// @brief Optional complete stem geometry provider for entries consulted by the slur calculation.
        /// @details Uses Stem::GeometryResult's decision and geometry contract. Supplied geometry
        /// bypasses the staff-origin resolver; NotHandled uses document geometry and
        /// staffOriginOffsetResolver. Unavailable produces MissingGeometry when direction requires it.
        /// Empty callbacks use document geometry. Resolver exceptions propagate to the caller.
        std::function<Stem::GeometryResult(const dom::EntryInfoPtr&)> geometryResolver;
    };
    struct DirectionInfo
    {
        dom::CurveContourDirection direction{
            dom::CurveContourDirection::Unspecified};
        Resolution resolution{Resolution::Resolved};
        Reason reason{Reason::None};
        dom::EntryInfoPtr sourceEntry; ///< Entry selecting the rule, if applicable.
        GeometrySource geometrySource{GeometrySource::NotRequired};
    };
    /// @brief Calculates automatic contour, retaining musical overrides but
    /// ignoring shape contour settings.
    /// @details Beat-attached slurs remain automatic. Uses the shape's requested
    /// part; does not modify the document.
    /// @throws std::invalid_argument If the shape is not a slur or caller
    /// geometry is invalid.
    /// @throws dom::integrity_error If required document records are missing.
    static DirectionInfo
    calcDefaultDirection(const dom::others::SmartShape& shape,
                         const Options& options);
    static DirectionInfo
    calcDefaultDirection(const dom::others::SmartShape& shape)
    {
        return calcDefaultDirection(shape, Options{});
    }
    /// @brief Honors definition contour before calculating automatic contour.
    /// @details Explicit contour does not require endpoints or geometry.
    /// Unresolved automatic contour carries a resolution status.
    /// @throws std::invalid_argument If the shape is not a slur or caller
    /// geometry is invalid.
    /// @throws dom::integrity_error If required document records are missing.
    static DirectionInfo
    calcEffectiveDirection(const dom::others::SmartShape& shape,
                           const Options& options);
    static DirectionInfo
    calcEffectiveDirection(const dom::others::SmartShape& shape)
    {
        return calcEffectiveDirection(shape, Options{});
    }
};

} // namespace util
} // namespace musx
