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
#include "Accidental.h"

#include <algorithm>
#include <array>
#include <map>
#include <optional>
#include <tuple>
#include <vector>

#include "musx/musx.h"

namespace musx {
namespace util {

namespace {

using namespace dom;
using AccidentalNoteKey = AccidentalCache::NoteKey;
using AccidentalMeasureResult = AccidentalCache::MeasureResult;
using AccidentalPitchKey = std::pair<int, music_theory::NoteName>;

AccidentalNoteKey makeAccidentalNoteKey(const NoteInfoPtr& note)
{
    const auto entry = note.getEntryInfo();
    return { entry.getLayerIndex(), entry->getEntry()->getEntryNumber(), note->getNoteId() };
}

struct AccidentalNoteOccurrence
{
    AccidentalNoteKey identity;
    AccidentalPitchKey pitch;
    Fraction position;
    Evpu horizontal{};
    size_t entryIndex{};
    size_t noteIndex{};
    LayerIndex layer{};
    int alteration{};
    int baseline{};
    bool grace{};
    bool frozen{};
    bool forcedShow{};
    bool parenthesized{};
    bool tied{};
    bool redisplay{};
    bool visible{};
    bool displayed{};

    auto order() const
    {
        return std::tie(pitch, position, horizontal, alteration, layer, entryIndex, noteIndex);
    }

    bool sharesGroup(const AccidentalNoteOccurrence& other) const
    {
        return pitch == other.pitch && position == other.position
            && ((!grace && !other.grace) || horizontal == other.horizontal);
    }
};

bool entryDisplaysPitchedNotesForAccidentals(const EntryInfoPtr& entry, const others::Staff& staff)
{
    if (entry->getEntry()->isHidden || staff.notationStyle != others::Staff::NotationStyle::Standard
        || staff.calcAlternateNotationHidesEntries(entry.getLayerIndex())) {
        return false;
    }
    return staff.altNotation != others::Staff::AlternateNotation::Rhythmic
        || (staff.altLayer != entry.getLayerIndex() && !staff.altHideOtherNotes);
}

std::vector<AccidentalNoteOccurrence> collectAccidentalNotes(const details::GFrameHoldContext& context,
    PitchMode pitchMode, AccidentalMeasureResult& results)
{
    std::vector<AccidentalNoteOccurrence> notes;
    for (LayerIndex layer = 0; layer < MAX_LAYERS; ++layer) {
        const auto frame = context.createEntryFrame(layer);
        if (!frame) {
            continue;
        }
        frame->iterateEntries([&](const EntryInfoPtr& entry) {
            const auto rawEntry = entry->getEntry();
            const auto staff = entry.createCurrentStaff();
            MUSX_ASSERT_IF(!staff) {
                throw std::logic_error("Accidental calculation requires an effective staff.");
            }
            const auto key = entry.getKeySignature();
            MUSX_ASSERT_IF(!key) {
                throw std::logic_error("Accidental calculation requires a key signature.");
            }
            const bool written = staff->calcUsesWrittenPitch(pitchMode);
            const auto keyContext = written ? KeySignature::KeyContext::Written : KeySignature::KeyContext::Concert;
            const bool naturalBaseline = key->keyless || key->hideKeySigShowAccis || staff->noKey
                || (written && staff->hideKeySigsShowAccis);
            const bool visible = entryDisplaysPitchedNotesForAccidentals(entry, *staff);
            for (auto note = NoteInfoPtr(entry, 0); note; note = note.getNext()) {
                const auto identity = makeAccidentalNoteKey(note);
                results.emplace(identity, false);
                if (!rawEntry->isNote || (context.getPolicyPartVoicing() && !note.calcIsIncludedInVoicing())) {
                    continue;
                }
                // Replacement notation does not contribute pitched notes. Individually hidden entries do.
                if (staff->calcAlternateNotationHidesEntries(layer)) {
                    continue;
                }
                NotePropertiesOptions propertiesOptions;
                propertiesOptions.pitchMode = pitchMode;
                propertiesOptions.alwaysUseEntryStaff = true;
                const auto pitch = note.calcNoteProperties(propertiesOptions);
                const int baseline = naturalBaseline ? 0 : key->calcAlterationOnNote(unsigned(pitch.noteName), keyContext);
                notes.push_back({ identity, { pitch.octave, pitch.noteName }, entry->elapsedDuration,
                    entry.calcManuaOffset(), entry.getIndexInFrame(), note.getNoteIndex(), layer,
                    pitch.alteration, baseline, rawEntry->graceNote, note->freezeAcci,
                    note->freezeAcci && note->showAcci, note->parenAcci, note.calcHasTieEnd(),
                    staff->redisplayLayerAccis, visible, note->freezeAcci && note->showAcci });
            }
            return true;
        });
    }
    std::sort(notes.begin(), notes.end(), [](const auto& left, const auto& right) {
        return left.order() < right.order();
    });
    return notes;
}

// Resolve one simultaneous diatonic-pitch group after its provisional decisions are available.
// The return value records a conflict that must be restated at a subsequent occurrence.
bool resolveSimultaneousAccidentals(std::vector<AccidentalNoteOccurrence>& notes, size_t begin, size_t end, bool pending)
{
    const int firstAlteration = notes[begin].alteration;
    bool conflict = false;
    bool allSuppressedTies = true;
    bool anyTie = false;
    for (size_t index = begin; index < end; ++index) {
        const auto& note = notes[index];
        conflict |= note.alteration != firstAlteration;
        allSuppressedTies &= note.tied && !note.forcedShow;
        anyTie |= note.tied;
    }
    if (allSuppressedTies) {
        return false;
    }

    for (size_t alterationBegin = begin; alterationBegin < end;) {
        size_t alterationEnd = alterationBegin;
        bool forcedParentheses = false;
        bool forcedPlain = false;
        while (alterationEnd < end && notes[alterationEnd].alteration == notes[alterationBegin].alteration) {
            const auto& note = notes[alterationEnd];
            if (!note.tied && note.forcedShow) {
                forcedParentheses |= note.parenthesized;
                forcedPlain |= !note.parenthesized;
            }
            ++alterationEnd;
        }

        auto& first = notes[alterationBegin];
        if (forcedParentheses) {
            first.displayed = first.forcedShow && first.parenthesized;
        } else if (!first.tied && conflict) {
            first.displayed = alterationBegin == begin || first.displayed || !anyTie;
        }
        bool displayed = first.displayed;
        for (size_t index = alterationBegin + 1; index < alterationEnd; ++index) {
            auto& note = notes[index];
            if (displayed) {
                note.displayed = note.forcedShow;
            } else if (conflict || note.redisplay) {
                note.displayed = index == begin + 1 && notes[begin].tied && !note.tied && pending;
            } else {
                note.displayed = note.forcedShow && (forcedPlain || (forcedParentheses && note.parenthesized));
            }
            displayed |= note.displayed;
        }
        alterationBegin = alterationEnd;
    }
    return conflict;
}

struct AccidentalLayerHistory
{
    int alteration{};
    size_t order{};
};

void evaluatePitchAccidentals(std::vector<AccidentalNoteOccurrence>& notes, size_t begin, size_t end)
{
    bool pending = false;
    int precedingAlteration = notes[begin].baseline;
    const auto initialPosition = notes[begin].position;
    std::array<std::optional<AccidentalLayerHistory>, MAX_LAYERS> history{};
    size_t historyEnd = begin;
    for (size_t groupBegin = begin; groupBegin < end;) {
        size_t groupEnd = groupBegin;
        while (groupEnd < end && notes[groupBegin].sharesGroup(notes[groupEnd])) {
            ++groupEnd;
        }
        for (size_t index = groupBegin; index < groupEnd; ++index) {
            auto& note = notes[index];
            // Only strictly earlier positions can establish a layer's accidental state.
            while (historyEnd < index
                && std::tie(notes[historyEnd].position, notes[historyEnd].horizontal)
                    < std::tie(note.position, note.horizontal)) {
                const auto& previous = notes[historyEnd];
                history[previous.layer] = AccidentalLayerHistory{ previous.alteration, historyEnd };
                ++historyEnd;
            }
            if (index == begin) {
                const bool needed = note.alteration != note.baseline;
                if (!note.frozen && !note.tied) {
                    note.displayed = needed;
                }
                pending = needed && (note.frozen || note.tied) && !note.forcedShow;
            } else if (note.alteration != precedingAlteration) {
                if (!note.frozen && !note.tied) {
                    note.displayed = true;
                    pending = false;
                }
            } else if (pending && !note.frozen && !note.tied && note.position != initialPosition) {
                note.displayed = true;
                pending = false;
            } else if (note.redisplay) {
                const auto& sameLayer = history[note.layer];
                std::optional<AccidentalLayerHistory> otherLayer;
                for (LayerIndex layer = 0; layer < MAX_LAYERS; ++layer) {
                    if (layer != note.layer && history[layer]
                        && (!otherLayer || history[layer]->order > otherLayer->order)) {
                        otherLayer = history[layer];
                    }
                }
                if (sameLayer) {
                    note.displayed |= note.alteration != sameLayer->alteration;
                } else if (otherLayer) {
                    note.displayed |= note.alteration != note.baseline || note.alteration != otherLayer->alteration;
                }
            }
            if (note.parenthesized && !note.frozen && !note.tied) {
                note.displayed = true;
                pending = false;
            }
            precedingAlteration = note.alteration;
        }
        pending |= resolveSimultaneousAccidentals(notes, groupBegin, groupEnd, pending);
        groupBegin = groupEnd;
    }
}

AccidentalMeasureResult calculateMeasureAccidentals(const details::GFrameHoldContext& context, PitchMode pitchMode)
{
    AccidentalMeasureResult results;
    auto notes = collectAccidentalNotes(context, pitchMode, results);
    for (size_t begin = 0; begin < notes.size();) {
        size_t end = begin;
        while (end < notes.size() && notes[end].pitch == notes[begin].pitch) {
            ++end;
        }
        evaluatePitchAccidentals(notes, begin, end);
        begin = end;
    }
    for (const auto& note : notes) {
        results.at(note.identity) = note.visible && note.displayed;
    }
    return results;
}

} // namespace

bool Accidental::calcIsDisplayed(const dom::NoteInfoPtr& note, dom::PitchMode pitchMode)
{
    MUSX_ASSERT_IF(!note) {
        throw std::invalid_argument("Accidental calculation requires a valid note.");
    }
    switch (pitchMode) {
    case dom::PitchMode::CurrentView:
    case dom::PitchMode::Concert:
    case dom::PitchMode::ConcertWithOctavePreference:
    case dom::PitchMode::Written:
        break;
    default:
        throw std::invalid_argument("Invalid accidental pitch mode.");
    }
    const auto frame = note.getEntryInfo().getFrame();
    const auto document = frame->getDocument();
    const auto& context = frame->getContext();
    const AccidentalCache::Key key{ frame->getRequestedPartId(), frame->getStaff(), frame->getMeasure(),
        context.getTimeOffset(), pitchMode };
    const auto& measures = document->getAccidentalCache().measures;
    auto found = measures.find(key);
    if (found == measures.end()) {
        // Calculate locally: failures must not publish an incomplete measure.
        const dom::details::GFrameHoldContext currentContext(document, frame->getRequestedPartId(),
            frame->getStaff(), frame->getMeasure(), context.getTimeOffset());
        auto result = calculateMeasureAccidentals(currentContext, pitchMode);
        document->addToAccidentalCache(key, std::move(result));
        found = measures.find(key);
    }
    const auto result = found->second.find(makeAccidentalNoteKey(note));
    if (result == found->second.end()) {
        if (!context.calcPolicyVoicingIncludesLayer(frame->getLayerIndex())) {
            return false;
        }
        MUSX_INTEGRITY_ERROR("Accidental note was not found in its measure context.");
        throw std::logic_error("Accidental note was not found in its measure context.");
    }
    return result->second;
}

} // namespace util
} // namespace musx
