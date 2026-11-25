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

#include "musx/dom/Fundamentals.h"

namespace musx {
namespace dom {
namespace percussion {

/// @brief Mask that extracts the base note type from a @ref PercussionNoteTypeId, removing its order ID.
constexpr PercussionNoteTypeId BASE_NOTE_TYPE_MASK = 0x0fff;

/**
 * @struct PercussionNoteType
 * @brief Represents a percussion note type mapping entry.
 *
 * Contains fields parsed from Finale's `PercNoteTypes.txt` CSV,
 * including identification, display, and grouping information.
 */
struct PercussionNoteType
{
    int instrumentId;           ///< Unique ID for this percussion note type
    int parentInstrumentId;     ///< ID of the parent percussion instrument
    const char* rawName;        ///< Raw name of the note type. May need to be massaged with group and order id.
    int defaultStaffPos;        ///< Default staff position for this note type, where 0 is the middle-C position of treble clef and up is positive.
    int generalMidi;            ///< General MIDI percussion mapping number or -1 if none
    int groupId;                ///< Group ID for note type grouping in Finale UI.
    int copies;                 ///< Number of copies defined for this note type.
    bool copyGroup;             ///< Whether this note type copies its group's attributes

    /// @brief Creates the human-readable name
    /// @param orderId The order Id that distinguishes which version of the instrument it is (taken from the top four bits of @ref PercussionNoteTypeId)
    /// @return The human-readable name in uf8 encoding.
    std::string createName(unsigned orderId) const;
};

const PercussionNoteType& getPercussionNoteTypeFromId(PercussionNoteTypeId percNoteId);

} // namespace percussion
} // namespace dom
} // namespace musx
