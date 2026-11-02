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

namespace musx {
namespace dom {

/// @enum AlignJustify
/// @brief Alignment and justification options for staff and group names.
enum class AlignJustify
{
    Left,   ///< Left alignment or justification (the default value.)
    Right,  ///< Right alignment.
    Center  ///< Center alignment.
};

/// @enum ArrowheadPreset
/// @brief Finale-defined arrowhead presets.
///
/// These values correspond to a fixed set of arrowhead shapes built into Finale.
/// The numeric values are significant and match Finale’s internal preset IDs.
/// The same preset is used for both the start and end of a line: the arrowhead
/// is automatically mirrored based on whether it appears at the line start or
/// the line end.
///
/// The presets differ only by shape and scale. Their sizes are fixed and not
/// user-adjustable:
/// - Small arrowheads are just under 1 staff space tall.
/// - The medium curved arrowhead is nearly 2.5 staff spaces tall.
/// - The large curved arrowhead is roughly 4 staff spaces tall.
/// In each case, the arrowhead length appears equal to its height.
enum class ArrowheadPreset : int
{
    SmallFilled  = 1,  ///< Small filled arrowhead preset (≈1 staff space tall).
    SmallOutline = 2,  ///< Small outlined arrowhead preset (≈1 staff space tall).
    SmallCurved  = 3,  ///< Small curved-back arrowhead preset (≈1 staff space tall).
    LargeCurved  = 4,  ///< Large curved-back arrowhead preset (≈4 staff spaces tall).
    MediumCurved = 5   ///< Medium curved-back arrowhead preset (≈2.5 staff spaces tall).
};

/// @brief Curve contour direction for ties and slurs.
enum class CurveContourDirection
{
    // NOTE: the order here is important for sorting instances. Do not change it.
    Unspecified,    ///< contour is automatic, indeterminate, or does not apply
    Down,           ///< contour is curved downward (tips face up)
    Up              ///< contour is curved upward (tips face down)
};

/// @enum DateFormat
/// @brief Date format options. This value is coded into the Enigma `date` insert when the page title is created.
enum class DateFormat : int
{
    Short = 0,      ///< Short date format, based on locale. US format is MM/DD/YY (Default value may not appear in the xml)
    Long = 1,       ///< Long date format, based on locale. US format is Month DD, YYYY.
    Abbrev = 2      ///< Abbreviated date format, based on locale. US format is Mon DD, YYYY.
};

/// @enum DefaultClefType
/// @brief Clef types used by default in Finale documents. The values correspond to indices into
/// @ref musx::dom::options::ClefOptions::clefDefs.
/// Note that the user may change the clef list to any clefs the user wishes. These are only defaults, but
/// they are frequently left unchanged.
enum class DefaultClefType : ClefIndex
{
    Treble          = 0,  ///< G clef, standard treble.
    Alto            = 1,  ///< C clef, centered on third line (Alto clef).
    Tenor           = 2,  ///< C clef, centered on fourth line (Tenor clef).
    Bass            = 3,  ///< F clef, standard bass.
    Percussion      = 4,  ///< Percussion clef, open rectangle (no pitch).
    Treble8vb       = 5,  ///< G clef, sounds one octave lower (8vb).
    Bass8vb         = 6,  ///< F clef, sounds one octave lower (8vb).
    Baritone        = 7,  ///< F clef on third line (Baritone clef).
    FrenchViolin    = 8,  ///< G clef placed on first line (French violin clef).
    BaritoneC       = 9,  ///< C clef on fifth line (Baritone clef).
    MezzoSoprano    = 10, ///< C clef on second line (Mezzo-soprano clef).
    Soprano         = 11, ///< C clef on first line (Soprano clef).
    AltPercussion   = 12, ///< Alternate percussion clef, heavy vertical hash marks (no pitch).
    Treble8va       = 13, ///< G clef, sounds one octave higher (8va).
    Bass8va         = 14, ///< F clef, sounds one octave higher (8va).
    Blank           = 15, ///< Blank clef (invisible, no symbol).
    Tab1            = 16, ///< Tablature clef (5 lines).
    Tab2            = 17  ///< Tablature clef (5 lines, alternative style).
};

/// @enum EnharmonicOverride
/// @brief Provides a three-state override option for enharmonic respelling.
enum class EnharmonicOverride
{
    None,       ///< Do not override the existing enharmonic state. (Default)
    Respell,    ///< Apply enharmonic respelling.
    NoRespell   ///< Do not apply enharmonic respelling, even if already respelled.
};

/**
 * @enum LyricTextType
 * @brief The lyric text type if this is a lyrics smart shape.
 */
enum class LyricTextType
{
    Verse,      ///< The assignment is to a Verse lyrics text block.
    Chorus,     ///< The assignment is to a Chorus lyrics text block.
    Section,    ///< The assignment is to a Section lyrics text block.
};

/// @enum MatchVoice
/// @brief Specifies which voice(s) are considered when matching entries.
enum class MatchVoice
{
    Default,    ///< Default matching behavior: accept either voice, but prefer voice 1 when both voices coincide.
    Voice2      ///< Require #Entry::voice2 to be true.
};

/**
 * @enum NoteType
 * @brief Enum class representing note types based on EDU values.
 *
 * The values are expressed in hexadecimal.
 */
enum class NoteType : Edu
{
    Maxima = 0x8000,
    Longa = 0x4000,
    Breve = 0x2000,
    Whole = EDU_PER_WHOLE_NOTE,
    Half = 0x0800,
    Quarter = 0x0400,
    Eighth = 0x0200,
    Note16th = 0x0100,
    Note32nd = 0x0080,
    Note64th = 0x0040,
    Note128th = 0x0020,
    Note256th = 0x0010,
    Note512th = 0x0008,
    Note1024th = 0x0004,
    Note2048th = 0x0002,
    Note4096th = 0x0001
};

/// @enum PitchMode
/// @brief Selects the pitch representation used by contextual pitch calculations.
enum class PitchMode
{
    CurrentView, ///< Use the pitch representation displayed in the requested Finale score or part.
    Written,     ///< Use written pitch regardless of the requested Finale view.
    Concert,     ///< Use concert pitch regardless of the requested Finale view.
    ConcertWithOctavePreference ///< Use concert pitch, retaining written octave transpositions when the document preference enables it.
};

/// @enum ShapeDirection
/// @brief Direction settings shared by smart-shape options and instances.
enum class ShapeDirection
{
    Automatic,  ///< Direction is determined automatically.
    Over,       ///< The shape is placed over its reference.
    Under       ///< The shape is placed under its reference.
};

/**
 * @enum ShowClefMode
 * @brief Enum representing the clef display mode for a frame.
 */
enum class ShowClefMode
{
    WhenNeeded, ///< Clef is displayed only when needed (the default).
    Never,      ///< Clef is never displayed. (xml value is "hidden")
    Always      ///< Clef is always displayed. (xml value is "forced")
};

/// @enum StemDirection
/// @brief Three-state stem direction setting or override.
enum class StemDirection
{
    Default,    ///< No forced direction; use the default calculation.
    AlwaysUp,   ///< Stems are forced up.
    AlwaysDown  ///< Stems are forced down.
};

/**
 * @enum TieConnectStyleType
 * @brief Enumeration for tie connect style types
 */
enum class TieConnectStyleType
{
    OverStartPosInner,
    OverEndPosInner,
    UnderStartPosInner,
    UnderEndPosInner,
    OverHighestNoteStartPosOver,
    OverHighestNoteEndPosOver,
    UnderLowestNoteStartPosUnder,
    UnderLowestNoteEndPosUnder,
    OverHighestNoteStemStartPosOver,
    OverHighestNoteStemEndPosOver,
    UnderLowestNoteStemStartPosUnder,
    UnderLowestNoteStemEndPosUnder
};

/// @enum VerticalPlacement
/// @brief Specifies a vertical placement relationship for notation objects.
enum class VerticalPlacement
{
    NotApplicable, ///< Above/below placement does not meaningfully apply.
    Float,         ///< Above/below placement floats based on context (usually stem direction).
    Below,         ///< Always positioned below the reference point or object.
    Above          ///< Always positioned above the reference point or object.
};

} // namespace dom
} // namespace musx
