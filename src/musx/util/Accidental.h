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

#include <map>
#include <tuple>

#include "Fraction.h"
#include "musx/dom/Fundamentals.h"
#include "musx/dom/EnumClasses.h"

namespace musx {
namespace dom {
class NoteInfoPtr;
} // namespace dom

namespace util {

/// @brief Accidental-display results grouped by measure context.
/// @details Contains values only; does not retain documents, frames, or notes.
struct AccidentalCache
{
    /// @brief Identifies a part, staff, measure, and pitch context.
    struct Key
    {
        dom::Cmper requestedPartId{};
        dom::StaffCmper staffId{};
        dom::MeasCmper measureId{};
        Fraction timeOffset{};
        dom::PitchMode pitchMode{};

        bool operator<(const Key& other) const
        {
            return std::tie(requestedPartId, staffId, measureId, timeOffset, pitchMode)
                < std::tie(other.requestedPartId, other.staffId, other.measureId, other.timeOffset, other.pitchMode);
        }
    };

    using NoteKey = std::tuple<dom::LayerIndex, dom::EntryNumber, dom::NoteNumber>; ///< Layer, entry, and note identity.
    using MeasureResult = std::map<NoteKey, bool>; ///< Accidental-display decisions for all participating notes.

    std::map<Key, MeasureResult> measures; ///< Completed results indexed by measure context.
};

/// @brief Calculates accidental display in a note's part, staff, measure, and pitch context.
/// @details Results are shared by all note wrappers belonging to the document. The first query
/// evaluates the participating layers of the measure. Call Document::clearAccidentalCache after
/// editing notes, frames, staff settings, key signatures, part voicing, or pitch display settings.
/// Glyph selection, accidental positioning, staff-system visibility, and page clipping are excluded.
class Accidental
{
public:
    /// @brief Returns whether a note displays an accidental, including explicit and courtesy choices.
    /// @param note The note to inspect. Must be valid.
    /// @param pitchMode The pitch representation to use for the entire measure calculation.
    /// @return False for rests, hidden entries, excluded part voices, and notation that replaces accidentals.
    /// @throws std::invalid_argument If the note or pitch mode is invalid.
    /// @throws std::logic_error If the note cannot be found in its measure context.
    [[nodiscard]]
    static bool calcIsDisplayed(const dom::NoteInfoPtr& note,
        dom::PitchMode pitchMode = dom::PitchMode::CurrentView);
};

} // namespace util
} // namespace musx
