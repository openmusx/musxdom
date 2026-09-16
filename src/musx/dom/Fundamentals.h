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

#include <cstdint>

#if defined(_WIN32)
#define MUSX_RUNNING_ON_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
#define MUSX_RUNNING_ON_MACOS
#elif defined(__EMSCRIPTEN__)
#define MUSX_RUNNING_ON_WASM
#elif defined(__unix__) || defined(__linux__)
#define MUSX_RUNNING_ON_LINUX_UNIX
#else
#error "Unable to detect operating system platform."
#endif

#ifdef MUSX_THROW_ON_INTEGRITY_CHECK_FAIL
#define MUSX_INTEGRITY_ERROR(S) throw ::musx::dom::integrity_error(S)
#else
#define MUSX_INTEGRITY_ERROR(S) ::musx::util::Logger::log(::musx::util::Logger::LogLevel::Warning, (S))
#endif

#ifdef MUSX_THROW_ON_UNKNOWN_XML
#define MUSX_UNKNOWN_XML(S) throw ::musx::factory::unknown_xml_error(S)
#else
#define MUSX_UNKNOWN_XML(S) ::musx::util::Logger::log(::musx::util::Logger::LogLevel::Warning, (S))
#endif

#define MUSX_ASSERT_IF(TEST) \
assert(!(TEST)); \
if (TEST)

namespace musx {
namespace dom {

using Cmper = uint16_t;             ///< Enigma "comperator" key type
using Inci = int16_t;               ///< Enigma "incident" key type
using Evpu = int32_t;               ///< EVPU value (288 per inch)
using EvpuFloat = double;           ///< EVPU fractional value (288.0 per inch)
using Evpu16ths = int32_t;          ///< 1/16 of an EVPU.
using Efix = int32_t;               ///< EFIX value (64 per EVPU, 64*288=18432 per inch)
using Edu = int32_t;                ///< "Enigma Durational Units" value (1024 per quarter note)
using EduFloat = double;            ///< "Enigma Durational Units" floating point value (1024.0 per quarter note)

using MeasCmper = int16_t;          ///< Enigma meas Cmper (may be negative when not applicable)
using StaffCmper = int16_t;         ///< Enigma staff (staffId) Cmper (may be negative when not applicable)
using SystemCmper = int16_t;        ///< Enigma systems Cmper (may be negative when not applicable)
using PageCmper = int16_t;          ///< Enigma page Cmper (may be negative when not applicable)
using ClefIndex = uint16_t;         ///< Index into @ref options::ClefOptions::clefDefs.
using EntryNumber = int32_t;        ///< Entry identifier.
using NoteNumber = uint16_t;        ///< Note identifier.
using LayerIndex = unsigned int;    ///< Layer index (valid values are 0..3)
using BeamNumber = unsigned int;    ///< A number where 1 corresponds to the primary (8th note) beam, 2 the 16th beam, 3 the 32nd beam, etc.
using PercussionNoteTypeId = uint16_t; ///< Percussion note code type. The top 4 bits are an order ID for when the same "pure" note type is used
///< multiple times in one layout. The rest are the "pure" note type. See @ref percussion::PercussionNoteType.

constexpr Cmper MUSX_GLOBALS_CMPER = 65534; ///< The prefs cmper for global variables (used sparingly since Finale 26.2)
constexpr int MAX_LAYERS = 4;       ///< The maximum number of music layers in a Finale document.
constexpr int MAX_ALTERATIONS = 7;  ///< Maximum absolute value for chromatic alterations supported in Finale.
constexpr Cmper SCORE_PARTID = 0;   ///< The part id of the score.

constexpr double EVPU_PER_INCH = 288.0;         ///< Number of Evpu units per inch
constexpr double EVPU_PER_POINT = 4.0;          ///< Number of Evpu units per 72 DPI point
constexpr double EVPU_PER_MM = EVPU_PER_INCH / 25.4; ///< Number of Evpu units per millimeter
constexpr double EVPU_PER_CM = EVPU_PER_MM * 10.0;   ///< Number of Evpu units per centimeter
constexpr double EVPU_PER_PX = EVPU_PER_INCH / 96.0; ///< Number of Evpu units per CSS pixel
constexpr double EVPU_PER_PICA = EVPU_PER_POINT * 12.0; ///< Number of Evpu units per pica
constexpr double EVPU_PER_SPACE = 24.0;         ///< Number of Evpu units per space (spatium)
constexpr double EVPU_PER_STANDARD_STAFF = 4.0 * EVPU_PER_SPACE; ///< Number of Evpu units per standard 5-line staff.
constexpr double EVPU_PER_STAFF_POSITION = EVPU_PER_SPACE / 2.0; ///< Number of Evpu units per half space.
constexpr double EFIX_PER_EVPU = 64.0;          ///< Number of Efix units per Evpu
constexpr double EFIX_PER_SPACE = EVPU_PER_SPACE * EFIX_PER_EVPU; ///< Number of Efix units per space (spatium)

constexpr Edu EDU_PER_WHOLE_NOTE = 0x1000;    ///< Number of Edu units per whole note

constexpr Cmper BASE_SYSTEM_ID = 0; ///< The base system cmper that gives a list of all available staves and their score order (@ref others::StaffUsed cmper)
                                    ///< Generally you should avoid using this directly and instead use #Document::calcScrollViewCmper or
                                    ///< #Document::getScrollViewStaves for the list of scroll view staves, and #Document::isScrollViewCmper to test a cmper.
constexpr Cmper STAFF_SET_1_SYSTEM_ID = 65500;  ///< The system cmoper for the first Scroll View staff set. Add 0..7 to this value to get one of the
                                                ///< 8 staff sets.
constexpr Cmper STUDIO_VIEW_SYSTEM_ID = 65400;  ///< The system cmper for Studio View. Studio View has entries for every click in the click track.
constexpr Cmper STUDIO_VIEW_STAFF_ID = 32767;   ///< The staff cmper Finale reserves for Studio View.
constexpr Cmper SPECIAL_PART_EXTRACTION_SYSTEM_ID = 65528;  ///< The system comper for Special Part Extraction (mostly obsolete in Finale 27.)
                                                            ///< See #others::PartGlobals::specialPartExtractionIUList for more information.
constexpr Cmper TEMP_SYSTEM_ID = 65529;     ///< This is a scratch value. Finale and/or plugins can use this systemId to aggregate an arbitary list
                                            ///< of staves for batch processing. It is possible that a plugin might have saved a file with it.

constexpr int MAX_CUE_PERCENTAGE = 90; ///< Arbitrary value that determines the max scaling value for cue notes. This could be configurable in the future.

/**
 * @namespace smufl_glyph
 * @brief List of SMuFL codepoints used by musxdom
 */
namespace smufl_glyph {
// Use namespace instead of enum class due to comparison ugliness with enum class.
constexpr char32_t unpitchedPercussionClef2 = 0xE06A;   ///< glyph "unpitchedPercussionClef2" (standard)
constexpr char32_t timeSigCommon = 0xE08A;              ///< glyph "timeSigCommon" (standard)
constexpr char32_t timeSigCutCommon = 0xE08B;           ///< glyph "timeSigCutCommon" (standard)
constexpr char32_t noteheadBlack = 0xE0A4;              ///< glyph "noteheadBlack" (standard)
constexpr char32_t accidentalNatural = 0xE261;          ///< glyph "accidentalNatural" (standard)
constexpr char32_t restQuarter = 0xE4E5;                ///< glyph "restQuarter" (standard)
constexpr char32_t sixStringTabClefSerif = 0xF40B;      ///< glyph "6stringTabClefSerif" (from Finale optional set)
constexpr char32_t fourStringTabClefSerif = 0xF40D;     ///< glyph "4stringTabClefSerif" (from Finale optional set)
constexpr char32_t arpeggioVerticalSegment = 0xF700;    ///< glyph "arpeggioVerticalSegment" (from Finale optional set)
} //namespace smufl_glyph

/**
 * @namespace symbol_glyph
 * @brief List of symbol codepoints used by musxdom
 */
namespace symbol_glyph {
// Use namespace instead of enum class due to comparison ugliness with enum class.
constexpr char32_t unpitchedPercussionClef2 = 214;      ///< equivalent symbol glyph for "unpitchedPercussionClef2"
} //namespace symbol_glyph

} // namespace dom
} // namespace musx
