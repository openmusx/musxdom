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

#include "musx/util/Fretboard.h"

#include <algorithm>

#include "music_theory/music_theory.hpp"

namespace musx::util {

namespace {

std::optional<dom::MusxInstance<dom::details::FretboardDiagram>> getDiagramForPitchClass(
    const dom::MusxInstance<dom::others::FretboardGroup>& group,
    int pitchClass)
{
    if (!group) {
        return std::nullopt;
    }

    const auto diagrams = group->getFretboardDiagrams();
    if (diagrams.empty()) {
        return std::nullopt;
    }

    // Finale orders the twelve diagrams from A, so pitch class 0 (C) is the fourth of them.
    const size_t diagramIndex = static_cast<size_t>(music_theory::positiveModulus(
        pitchClass + 3, music_theory::STANDARD_12EDO_STEPS));
    if (diagramIndex >= diagrams.size()) {
        return std::nullopt;
    }
    return diagrams[diagramIndex];
}

int displayFret(int sourceFret, int fretboardNumber)
{
    if (sourceFret == 0 || fretboardNumber <= 1) {
        return sourceFret;
    }
    return sourceFret + fretboardNumber - 1;
}

} // namespace

std::optional<FretboardDisplayData> calcFretboardDisplayData(
    const dom::MusxInstance<dom::details::ChordAssign>& chord,
    const dom::MusxInstance<dom::KeySignature>& keySignature,
    dom::KeySignature::KeyContext keyContext)
{
    if (!chord || !keySignature || !chord->showFretboard || chord->useFretboardFont) {
        return std::nullopt;
    }
    // Finale suppresses a fretboard from three independent places: the chord assignment itself
    // (checked above), the document-wide chord options, and the staff in effect where the chord sits.
    const auto chordOptions = chord->getDocument()->getOptions()->get<dom::options::ChordOptions>();
    if (!chordOptions || !chordOptions->showFretboards) {
        return std::nullopt;
    }
    const auto staff = dom::others::StaffComposite::createCurrent(chord->getDocument(), chord->getRequestedPartId(),
        static_cast<dom::StaffCmper>(chord->getCmper1()), static_cast<dom::MeasCmper>(chord->getCmper2()), chord->horzEdu);
    if (!staff || staff->hideFretboards) {
        return std::nullopt;
    }

    const auto group = chord->getFretboardGroup();
    if (!group) {
        return std::nullopt;
    }
    const auto instrument = group->getFretInstrument();
    if (!instrument) {
        return std::nullopt;
    }
    const auto root = keySignature->calcPitch(chord->rootScaleNum, chord->rootAlter, keyContext);
    // A fretboard group always stores exactly twelve diagrams, one per 12-EDO pitch class, so the root
    // selects one of them in 12-EDO regardless of the key signature's own EDO.
    const auto diagram = getDiagramForPitchClass(group, music_theory::calcPitchClass(root));
    if (!diagram) {
        return std::nullopt;
    }

    FretboardDisplayData result;
    result.stringCount = instrument->numStrings;
    result.fretCount = (*diagram)->numFrets;
    result.fretboardNumber = (*diagram)->fretboardNum;
    result.showFretboardNumber = (*diagram)->showNum;
    if (const auto style = chord->getFretboardStyle()) {
        result.fretNumberText = style->fretNumText;
    }
    result.cells.reserve((*diagram)->cells.size());
    for (const auto& cell : (*diagram)->cells) {
        result.cells.push_back({ cell->string, displayFret(cell->fret, (*diagram)->fretboardNum), cell->shape, cell->fingerNum });
    }
    for (int stringNumber = 1; stringNumber <= result.stringCount; ++stringNumber) {
        const auto cell = std::find_if(result.cells.begin(), result.cells.end(),
            [stringNumber](const auto& candidate) {
                return candidate.string == stringNumber;
            });
        if (cell == result.cells.end()) {
            result.unplayedStringDisplay = UnplayedStringDisplay::Blank;
            break;
        }
    }
    const auto mutedCell = std::find_if(result.cells.begin(), result.cells.end(),
        [](const auto& cell) {
            return cell.shape == dom::details::FretboardDiagram::Shape::Muted;
        });
    if (mutedCell != result.cells.end()) {
        result.unplayedStringDisplay = UnplayedStringDisplay::Muted;
    }
    result.barres.reserve((*diagram)->barres.size());
    for (const auto& barre : (*diagram)->barres) {
        // Barre frets count from zero at the diagram's first fret, one less than the cell coordinate
        // in which zero is the open string. Shift into the cell coordinate before adding the display offset.
        result.barres.push_back({ displayFret(barre->fret + 1, (*diagram)->fretboardNum), barre->startString, barre->endString });
    }
    return result;
}

} // namespace musx::util
