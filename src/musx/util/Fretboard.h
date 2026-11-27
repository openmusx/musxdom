/*
 * Copyright (C) 2026, Robert Patterson
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

#include <optional>
#include <string>
#include <vector>

#include "musx/dom/Details.h"
#include "musx/dom/Others.h"

namespace musx::util {

/// @brief How strings without frame notes should be displayed.
enum class UnplayedStringDisplay {
    None, ///< Every instrument string has a played frame cell.
    Blank, ///< One or more strings have no frame note, with no explicit muted marker.
    Muted ///< One or more omitted strings are explicitly marked muted.
};

/// @brief Format-neutral display data for one Finale fretboard diagram.
/// @details Fret values are absolute display fret numbers. A cell with fret zero is a string marker
/// whose meaning comes from #Cell::shape, and multiple cells on one string are retained in source order.
/// Exporters may collapse or reinterpret those values according to their target format.
struct FretboardDisplayData
{
    /// @brief One marker or fretted position in the diagram.
    struct Cell
    {
        int string{}; ///< 1-based string number.
        int fret{}; ///< Absolute display fret number; zero is the open-string/marker position.
        dom::details::FretboardDiagram::Shape shape{}; ///< Source marker or dot shape.
        int finger{}; ///< Finger number; zero means no fingering is specified.
    };

    /// @brief A barre spanning a range of strings.
    struct Barre
    {
        int fret{}; ///< Absolute display fret number using the same coordinate as #Cell::fret.
        int startString{}; ///< 1-based first string in the barre.
        int endString{}; ///< 1-based last string in the barre.
    };

    int stringCount{}; ///< Number of strings in the associated fret instrument.
    int fretCount{}; ///< Number of frets displayed by the diagram.
    int fretboardNumber{}; ///< Starting fret number stored by Finale.
    bool showFretboardNumber{}; ///< Whether Finale displays #fretboardNumber.
    /// Text displayed after #fretboardNumber (for example "fr."), from the chord's
    /// @ref dom::others::FretboardStyle. Absent when the chord assignment has no fretboard style.
    std::optional<std::string> fretNumberText;
    UnplayedStringDisplay unplayedStringDisplay{UnplayedStringDisplay::None}; ///< Display state for strings without frame notes.
    std::vector<Cell> cells; ///< Diagram cells in Finale source order.
    std::vector<Barre> barres; ///< Diagram barres in Finale source order.
};

/// @brief Resolve the format-neutral display data for a fretboard diagram.
/// @details The chord assignment supplies the staff and measure identity through its instance coordinates,
/// and supplies the root scale degree and alteration. The effective key signature is an input because callers
/// commonly already have it resolved for the staff and measure. The key signature and context are used to
/// calculate the root's chromatic pitch class and select Finale's corresponding diagram.
///
/// This function owns the entire visibility decision. Finale suppresses a fretboard from the chord assignment,
/// from @ref dom::options::ChordOptions, and from the staff in effect at the chord's position, and all three
/// are checked here. Callers need not consult them, and in particular need not resolve a
/// @ref dom::others::StaffComposite of their own to do so.
/// @param chord Chord assignment whose fretboard is to be resolved.
/// @param keySignature Effective key signature for the chord's staff and measure.
/// @param keyContext Whether the written or concert key-signature interpretation should be used.
/// @return Display data, or std::nullopt when no fretboard is displayed for this chord or when its source
/// data is unavailable.
[[nodiscard]] std::optional<FretboardDisplayData> calcFretboardDisplayData(
    const dom::MusxInstance<dom::details::ChordAssign>& chord,
    const dom::MusxInstance<dom::KeySignature>& keySignature,
    dom::KeySignature::KeyContext keyContext);

} // namespace musx::util
