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

#include <numeric>
#include <filesystem>
#include <array>
#include <string>
#include <string_view>
#include <optional>
#include <utility>
#include <vector>

#include "musx/dom/ResolvedFontInfo.h"
#include "musx/util/EnigmaString.h"
#include "musx/util/Fraction.h"
#include "BaseClasses.h"
#include "EnumClasses.h"

namespace music_theory {
class Transposer;
enum class DiatonicMode : int;
struct Pitch;
} // namespace music_theory

namespace musx {
namespace dom {

class EntryInfoPtr;

namespace details { // forward declarations
class IndependentStaffDetails;
class LyricAssign;
} // namespace details

namespace others { // forward declarations
class FontDefinition;
class Measure;
class OssiaHeader;
class Staff;
} // namespace others

/// @brief Expresses a duration as a #NoteType and a number of dots (unsigned)
using Duration = std::pair<NoteType, unsigned>;

// This file contains common classes that are shared among Options, Others, and Details.

/// @brief Normalizes a font name for comparison by removing ASCII whitespace and folding ASCII case.
///
/// Whitespace removal and case folding are applied only to bytes below `0x80`; all non-ASCII
/// bytes pass through unchanged. This prevents locale-sensitive character classification from
/// modifying bytes within a UTF-8 sequence.
///
/// @param name The font name to normalize.
/// @return The normalized font name.
[[nodiscard]]
std::string normalizeFontName(std::string_view name);

/// @brief Copies a font definition into another document, reusing an equivalent one when present.
///
/// A @ref Cmper is meaningful only within the document that issued it. A non-zero definition is
/// therefore matched against @p target by normalized name (see @ref normalizeFontName) and never by
/// cmper. The definition at cmper 0 is excluded from matching. When nothing matches, the
/// definition is copied to the next free cmper, retaining @p source's exact spelling.
///
/// Font id 0 is a sentinel for the document's default music font rather than an index to a
/// typeface. It is returned unchanged, and a definition is created at 0 in @p target when absent,
/// so that the returned id always resolves. The typeface for a created definition comes from
/// @p target's own @ref options::FontOptions music font, falling back to @p source's definition
/// at 0, which is then also given a non-zero cmper in @p target.
///
/// @param target The document to import into. May be the same document as @p source's.
/// @param source The definition to import.
/// @param onImported Called for every definition this operation creates. Existing equivalent
/// definitions that are reused are not reported.
/// @return The cmper naming the typeface within @p target, or std::nullopt when @p target has no
/// free cmper left for a new definition. A returned 0 always means the default music font.
/// @throws std::invalid_argument if @p target or @p source is null. Debug builds assert instead.
[[nodiscard]]
std::optional<Cmper> importFontDefinitionInto(const DocumentPtr& target,
    const MusxInstance<others::FontDefinition>& source,
    const ImportObjectCallback& onImported = {});

/**
 * @struct FontInfo
 * @brief Represents the default font settings for a particular element type.
 *
 * The FontInfo struct holds information about font properties, such as the font ID, size, and styles like
 * bold, italic, underline, strikeout, fixed size, and visibility.
 */
class FontInfo : public CommonClassBase
{
public:
    /**
     * @brief constructor
     * @param document the document containing the font
     * @param sizeIsPercent if true, the size is a percent relative to the preceding font size in an Enigma string.
     */
    FontInfo(const DocumentWeakPtr& document, bool sizeIsPercent = false)
        : CommonClassBase(document), m_sizeIsPercent(sizeIsPercent)
    {
    }

    Cmper fontId{};     ///< Font identifier. This is a Cmper for @ref others::FontDefinition.
    int fontSize{};     ///< Font size or percent (where 100 is 100%) of preceding font size. (See #getSizeIsPercent.)
    bool bold{};        ///< Bold effect.
    bool italic{};      ///< Italic effect.
    bool underline{};   ///< Underline effect.
    bool strikeout{};   ///< Strikeout effect.
    bool absolute{};    ///< Fixed size effect.
    bool hidden{};      ///< Hidden effect.

    /// @brief Return true if the two fonts specify the same typeface, ignoring size and effects.
    ///
    /// A #fontId of 0 is the document's default music font. Finale rewrites that
    /// @ref others::FontDefinition's name whenever the default music font changes, so anything
    /// storing id 0 follows the change while a concrete id keeps whatever typeface it was set to.
    /// The same typeface therefore routinely appears under both id 0 and a concrete id, and those
    /// compare equal here by name.
    ///
    /// Two distinct non-zero ids are never compared by name. The default music font is the only
    /// font duplicated by design, so duplicate names elsewhere indicate a malformed document.
    ///
    /// @param src The input font to compare with.
    /// @throws std::invalid_argument if either font id has no @ref others::FontDefinition.
    bool calcIsSameTypeface(const FontInfo& src) const;

    /// @brief Return true if the two fonts represent the same font.
    ///
    /// Compares typeface (see #calcIsSameTypeface), size, and style effects. To distinguish a
    /// #fontId of 0 from the concrete id naming the same typeface - which matters only when the
    /// source encoding itself is the subject, such as round-tripping, or reporting that a font
    /// option pins a concrete font instead of following the document default - compare #fontId
    /// directly.
    ///
    /// @param src The input font to compare with.
    bool isSame(const FontInfo& src) const;

    /// @brief If true, the size of this font is calculated as a percent of the preceding font size (in an Enigma string)
    bool getSizeIsPercent() const { return m_sizeIsPercent; }

    /// @name Enigma style bitmask constants
    /// These are used to encode and decode style effects in Enigma strings.
    /// @{
    inline static constexpr uint16_t EnigmaStyleBold = 0x01; ///< Bold style bit
    inline static constexpr uint16_t EnigmaStyleItalic = 0x02; ///< Italic style bit
    inline static constexpr uint16_t EnigmaStyleUnderline = 0x04; ///< Underline style bit
    inline static constexpr uint16_t EnigmaStyleStrikeout = 0x20; ///< Strikeout style bit
    inline static constexpr uint16_t EnigmaStyleAbsolute  = 0x40; ///< Fixed-size (absolute) bit
    inline static constexpr uint16_t EnigmaStyleHidden    = 0x80; ///< Hidden text bit
    /// @}

    /**
     * @brief Get the name of the font.
     * @return The name of the font as a string.
     */
    std::string getName() const;

    /**
     * @brief Sets the id of the font from a string name.
     * @param name The font name to find.
     * @throws std::invalid_parameter if the name is not found.
     */
    void setFontIdByName(const std::string& name);

    /**
     * @brief Set style effects based on a bitmask. This is mainly useful for capturing text styles
     * from enigma strings. (See @ref musx::util::EnigmaString::parseStyleCommand.)
     *
     * @param efx A 16-bit integer representing style effects with specific bit masks.
     */
    void setEnigmaStyles(uint16_t efx)
    {
        bold = efx & EnigmaStyleBold;
        italic = efx & EnigmaStyleItalic;
        underline = efx & EnigmaStyleUnderline;
        strikeout = efx & EnigmaStyleStrikeout;
        absolute = efx & EnigmaStyleAbsolute;
        hidden = efx & EnigmaStyleHidden;
    }

    /// @brief Returns the font styles as an nfx bitmask
    uint16_t getEnigmaStyles() const
    {
        uint16_t result = 0;
        if (bold) result |= EnigmaStyleBold;
        if (italic) result |= EnigmaStyleItalic;
        if (underline) result |= EnigmaStyleUnderline;
        if (strikeout) result |= EnigmaStyleStrikeout;
        if (absolute) result |= EnigmaStyleAbsolute;
        if (hidden) result |= EnigmaStyleHidden;
        return result;
    }

    /// @brief Calculates if this is the default music font.
    bool calcIsDefaultMusic() const
    { return fontId == 0; }

    /// @brief Calculates if this is a symbol font. (See #others::FontDefinition::calcIsSymbolFont.)
    bool calcIsSymbolFont() const;

    /// @brief Returns the filepath of the SMuFL font's metadata json file, if any.
    /// @param fontName The font name to find.
    static std::optional<std::filesystem::path> calcSMuFLMetaDataPath(const std::string& fontName);

    /// @brief Returns the filepath of the SMuFL font's metadata json file, if any
    std::optional<std::filesystem::path> calcSMuFLMetaDataPath() const
    { return FontInfo::calcSMuFLMetaDataPath(getName()); }

    /**
     * @brief Calculates whether this is a SMuFL font.
     */
    bool calcIsSMuFL() const;

    /**
     * @brief Creates a document-independent snapshot of the font.
     * @details Call while the document is still alive, because resolving the name requires it.
     * @throws std::invalid_argument if the font has no font definition in the document.
     */
    ResolvedFontInfo resolve() const;

    /**
     * @brief Returns the standard SMuFL font folder.
     *
     * @return a std::vector<std::filesystem::path> where element 0 is the user path and element 1 is the system path
     */
    static std::vector<std::filesystem::path> calcSMuFLPaths();

    static const xml::XmlElementArray<FontInfo>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.

private:
    bool m_sizeIsPercent;
};

/**
 * @class KeySignature
 * @brief Shared key signature class that is contained in other classes. (See @ref others::Measure)
 */
class KeySignature : public CommonClassBase
{
public:
    using CommonClassBase::CommonClassBase;

    /**
     * @enum KeyContext
     * @brief Indicates whether to compute key signature values in concert or written pitch.
     *
     * This enum is used to control whether computations such as alteration,
     * tonal center, or key maps should be performed using the concert pitch
     * (untransposed key) or the written pitch (transposed key).
     *
     * Pass this as an optional argument to key signature methods to specify the desired context.
     */
    enum class KeyContext {
        Concert,   ///< Use concert pitch (untransposed)
        Written    ///< Use written pitch (with transposition)
    };

    /** @brief 16-bit value intepreted as follows:
     * - <b> Linear Keys </b>: top bit is 0, the next 7 bits define the key mode (see #getKeyMode),
     * bottom 8 bits is a signed byte (+/-7) giving the number of sharps or flats.
     * - <b> Nonlinear Keys </b>: Always have values from 0x4000-0x4fff. These are Cmper values to the records
     * that define the nonlinear key.
     */
    uint16_t key{};
    bool keyless{};                 ///< Indicates the absence of a key signature
    bool hideKeySigShowAccis{};     ///< Instead of a key signature, show accidentals for the key on the notes where they occur.

    /** @brief Returns the key mode.
     *
     * For linear keys, this is a value from 0..127 where 0 is reserved for built-in major keys and 1 is reserved for
     * built-in minor keys. Values from 2..127 are user-defined linear modes and are a @ref Cmper value for finding
     * the custom key signature information related to the key. User-defined linear keys might, for example, represent diatonic
     * jazz modes or church modes. They can also be used to define microtonal scales with key signatures.
     *
     * For non-linear keys, this is a value from 0x4000..0x4fff. This is a Cmper that is used to find all the custom key
     * signature information that defines the key. These are sometimes used to define modes like Klezmer Freyish mode, or
     * other modes whose accidentals do not follow the circle of fifths. They can also be used to define
     * microtonal scales without key signatures.
     *
     * Values from 0x8000 and higher are invalid.
     */
    Cmper getKeyMode() const { return isLinear() ? key >> 8 : key;  }

    /// @brief For linear keys, returns the number of sharps or flats from -7..7 (if any).
    /// @param ctx (optional) Specifies whether to return the concert or written alteration value. Default is written.
    /// @return Number of sharps/flats for linear keys or 0 for non-linear or invalid keys
    int getAlteration(KeyContext ctx) const
    { return isLinear() ? int(int8_t(key & 0xff)) + getAlterationOffset(ctx) : 0; }

    bool isLinear() const { return (key & 0xC000) == 0; }                   ///< whether this is a linear key
    bool isNonLinear() const { return (key & 0xC000) != 0; }                ///< whether this is a non-linear key
    bool isBuiltIn() const { return isLinear() && getKeyMode() <= 1; }      ///< whether this is a built-in key
    bool isMajor() const { return getKeyMode() == 0; }                      ///< whether this is a built-in major key
    bool isMinor() const { return getKeyMode() == 1; }                      ///< whether this is a built-in minor key

    /// @brief If this key specifies a diatonic mode, returns the mode. This value is independent of EDO divisions.
    /// Any correctly configured linear key signature should return a value.
    /// @return The diatonic mode or std::nullopt if it is not one.
    std::optional<music_theory::DiatonicMode> calcDiatonicMode() const;

    /// @brief returns whether the two key signatures represent the same key signature, taking into account transposition.
    bool isSame(const KeySignature& src) const
    {
        return isSameConcert(src) && m_alterationOffset == src.m_alterationOffset && m_octaveDisplacement == src.m_octaveDisplacement;
    }

    /// @brief returns whether the two key signatures represent the same concert key signature, ignoring transposition.
    bool isSameConcert(const KeySignature& src) const
    {
        return key == src.key && keyless == src.keyless && hideKeySigShowAccis == src.hideKeySigShowAccis;
    }

    /// @brief Calculates the scale degree for the given displacement, where 0 is the tonic
    /// @param displacement The diatonic displacement value. Zero is the tonic in the middle-C octave.
    /// @return The scale degree, where 0 is the tonic.
    int calcScaleDegree(int displacement) const;

    /**
     * @brief Transposes the key by the specified amounts. Set them to zero to remove transposition.
     *
     * This is used to adjust the key signature for staves that use Key Signature transposition.
     * Staves that use Chromatic transposition do not transpose the key. They transpose the pitches directly.
     *
     * Finale works pretty well with key signature transposition in microtone scales. However, it does not
     * simplify key signatures usefully. This function simplifies them correctly, provided that the step
     * values for accidentals are set to cycle through sharps and flats by chromatic half-steps rather than
     * the default of 1.
     * See @ref others::AcciAmountSharps and @ref others::AcciAmountFlats.
     *
     * @param interval The interval by which to transpose.
     * @param keyAdjustment The key adjustment (positive for sharps, negative for flats)
     * @param simplify If true, enharmonically adjust the key to have 6 or fewer accidentals
     */
    void setTransposition(int interval, int keyAdjustment, bool simplify);

    /// @brief Sets the key's transposition based on the input staff's transposition settings.
    void setTransposition(const MusxInstance<others::Staff>& staff);

    /// @brief Calculates the tonal center index for the key, where C=0, D=1, E=2, ...
    ///
    /// This is the modal tonal center, so a minor key with no sharps or flats returns 5 (=A).
    int calcTonalCenterIndex(KeyContext ctx) const;

    /// @brief Calculates the amount of alteration on a note int the key.
    /// @param noteIndex note index, where C=0, D=1, E=3, F=3, G=4, A=5, B=6
    /// @param ctx Whether to calculate the alteration for concert or written pitch.
    int calcAlterationOnNote(unsigned noteIndex, KeyContext ctx ) const;

    /**
     * @brief Converts a key-relative pitch representation into a spelled pitch.
     *
     * @param displacement Diatonic displacement relative to the key tonic in the C4 octave.
     * @param alteration Alteration relative to the key signature, in EDO divisions.
     * @param ctx Whether to use concert or written key-signature values.
     * @return The resulting note name, octave, and alteration relative to the natural note name.
     */
    [[nodiscard]]
    music_theory::Pitch calcPitch(int displacement, int alteration, KeyContext ctx) const;

    /**
     * @brief Converts a staff position into a spelled pitch using the specified clef and key context.
     *
     * @param staffPosition Staff position relative to the staff reference line.
     * @param clefIndex Index of the clef used to interpret the staff position.
     * @param ctx Whether to use concert or written key-signature values.
     * @param actualAlteration Alteration relative to the natural note name, in EDO divisions. If omitted, the
     * effective key signature supplies the alteration.
     * @return The resulting note name, octave, and alteration relative to the natural note name.
     * @throws std::invalid_argument if the document contains no clef options.
     * @throws std::out_of_range if @p clefIndex is invalid.
     */
    [[nodiscard]]
    music_theory::Pitch calcPitchFromStaffPosition(int staffPosition, ClefIndex clefIndex, KeyContext ctx,
        std::optional<int> actualAlteration = std::nullopt) const;

    /// @brief The octave displacement if this key is a transposed key.
    /// @return 0 for non-transposing keys or the octave displacement for transposed keys.
    int getOctaveDisplacement(KeyContext ctx) const
    { return ctx == KeyContext::Written ? m_octaveDisplacement : 0; }

    /// @brief Calculates the number of EDO division for the key. (The standard value is 12.)
    int calcEDODivisions() const;

    /// @brief Calculates the key's diatonic key map
    std::optional<std::vector<int>> calcKeyMap() const;

    /// @brief Creates a transposer for this KeySignature instance.
    /// @param displacement Displacement value (e.g., from @ref Note)
    /// @param alteration Alteration value (e.g., from @ref Note)
    /// @return A unique pointer to a transposer for this key.
    std::unique_ptr<music_theory::Transposer> createTransposer(int displacement, int alteration) const;

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        this->CommonClassBase::integrityCheck(ptrToThis);
        if (key >= 0x8000) {
            MUSX_INTEGRITY_ERROR("Key signature has invalid key value: " + std::to_string(key));
        }
    }

    static const xml::XmlElementArray<KeySignature>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.

private:
    std::vector<unsigned> calcTonalCenterArrayForSharps() const;
    std::vector<unsigned> calcTonalCenterArrayForFlats() const;
    std::vector<unsigned> calcTonalCenterArray(KeyContext ctx) const;
    std::vector<int> calcAcciAmountsArray(KeyContext ctx) const;
    std::vector<unsigned> calcAcciOrderArray(KeyContext ctx) const;

    int m_octaveDisplacement{};         ///< Displace notes by this many octaves (for transposed keys)
    int m_alterationOffset{};           ///< Offset of alteration (for transposed keys)

    int getAlterationOffset(KeyContext ctx) const
    { return ctx == KeyContext::Written ? m_alterationOffset : 0; }

};

namespace texts {
class LyricsTextBase; // forward delcaration
} // namespace texts

/**
 * @class LyricsLineInfo
 * @brief Contains information about a line of lyrics on a system.
 */
class LyricsLineInfo : CommonClassBase
{
public:
    /// @brief Constructor function
    /// @param document The document for this lyric line
    /// @param requestedPartId The requested part ID for this lyric line.
    /// @param type The type of lyric (from XmlNodeName string)
    /// @param lyricNo The lyric number for this line info.
    /// @param baseline Use #others::Staff::calcBaselinePosition for this value.
    LyricsLineInfo(const DocumentWeakPtr& document, Cmper requestedPartId, std::string_view type, Cmper lyricNo, Evpu baseline) :
        CommonClassBase(document), baselinePosition(baseline), lyricsType(type), lyricNumber(lyricNo), assignments(document, requestedPartId)
    {
    }

    Evpu baselinePosition;              ///< baseline position of this line on this system, relative to the staff's reference line
    std::string_view lyricsType;        ///< the type of lyric ("chorus", "verse", or "section", corresponding to the xml tags for lyrics text)
    Cmper lyricNumber;                  ///< the text number for all lyric assignments on this line.
    MusxInstanceList<details::LyricAssign> assignments; ///< The lyric assignments on this line. They all share the same `lyricNumber` value.
};

/**
 * @class LyricsSyllableInfo
 * @brief Contains the syllable information for a single syllable. (See @ref texts::LyricsTextBase)
 */
class LyricsSyllableInfo : DocumentElementNoPart
{
public:
    /**
     * @class ElisionRun
     * @brief One text run produced by splitting a syllable's text at an embedded Finale elision
     * marker. See @ref LyricsSyllableInfo::calcElisionRuns.
     *
     * #hasHyphenBefore and #hasHyphenAfter are redistributed from the whole syllable: only the
     * first run can carry hasHyphenBefore, and only the last can carry hasHyphenAfter. A syllable
     * preceded and followed by a hyphen therefore splits into an #hasHyphenAfter-only run
     * followed by an #hasHyphenBefore-only run; hasHyphenBefore and hasHyphenAfter are never both
     * true on the same run of a split.
     *
     * A run's style chunks are resolved when the run is created, so it holds no reference back to
     * its syllable and is safe to copy or store independently of it.
     */
    class ElisionRun
    {
    public:
        std::optional<char32_t> joinMarker;     ///< The codepoint joining this run to the previous one; absent for the first run.
        bool hasHyphenBefore{};                 ///< See LyricsSyllableInfo::hasHyphenBefore.
        bool hasHyphenAfter{};                  ///< See LyricsSyllableInfo::hasHyphenAfter.

        /// @brief This run's text (all style chunks concatenated).
        std::string text() const;

        /// @brief Iterates this run's already-resolved style chunks. In the common case (one
        /// style throughout the run) the callback is called exactly once.
        /// @param callback The callback for each chunk of this run with a different style.
        /// @return True if the run was fully parsed. False if parsing was aborted by the callback function.
        bool iterateStyles(util::EnigmaString::TextChunkCallback callback) const;

    private:
        std::vector<std::pair<std::string, util::EnigmaStyles>> m_chunks;

        friend class LyricsSyllableInfo;
    };

    std::string syllable;       ///< the syllable text with no hyphenation or font information.
    bool hasHyphenBefore;       ///< indicates the syllable is preceded by a hyphen.
    bool hasHyphenAfter;        ///< indicates the syllable if followed by a hyphen.
    int strippedUnderscores;    ///< indicates the number of trailing underscores stripped (because smart word extensions convert them to word extensions).

    /// @brief Parse this syllable into chunks with EnigmaStyles for that chunk. In the most common case, the callback is called exactly once.
    /// However, this design allows for detecting uncommon cases of style changes within a syllable.
    /// @param callback The callback for each chunk of the syllable with a different style.
    /// @return True if the syllable was fully parsed. False if parsing was aborted by the callback function.
    bool iterateStyles(util::EnigmaString::TextChunkCallback callback) const;

    /// @brief Splits #syllable at embedded Finale elision markers, redistributing
    /// #hasHyphenBefore and #hasHyphenAfter onto the first and last run respectively. A syllable
    /// with no markers returns a single run spanning the whole syllable.
    /// @return At least one run.
    std::vector<ElisionRun> calcElisionRuns() const;

private:
    /**
     * @class LyricsSyllableStyleSpan
     * @brief Contains font style information for a lyrics syllable. (See @ref texts::LyricsSyllableInfo)
     */
    class StyleSpan
    {
    public:
        size_t start;        // start byte
        size_t end;          // end byte (exclusive)
        size_t styleIndex;   // index into LyricsTextBase's style table
    };

    /// @brief Constructor function
    /// @param document Shared pointer to the document.
    /// @param owner MusxInstance ptr to the owning LyricsTextBase, to reach its style table.
    /// @param text The syllable text.
    /// @param before Whether there is a hyphen before the syllable.
    /// @param after Whether there is a hyphen after the syllable.
    /// @param underscores The number of trailing underscores stripped.
    /// @param enigmaStylesIndex The enigma style (in LyricsTextBase) for this syllable.
    LyricsSyllableInfo(const DocumentWeakPtr& document, const MusxInstance<texts::LyricsTextBase>& owner, const std::string text, bool before, bool after, int underscores, std::vector<StyleSpan>&& enigmaStyleMap)
        : DocumentElementNoPart(document), syllable(text), hasHyphenBefore(before), hasHyphenAfter(after), strippedUnderscores(underscores), m_owner(owner), m_enigmaStyleMap(std::move(enigmaStyleMap))
    {
    }

    /// @brief Shared implementation for #iterateStyles and #ElisionRun::iterateStyles: iterates
    /// style chunks clipped to [@p rangeStart, @p rangeEnd).
    bool iterateStylesInRange(size_t rangeStart, size_t rangeEnd, const util::EnigmaString::TextChunkCallback& callback) const;

    MusxInstanceWeak<texts::LyricsTextBase> m_owner; ///< the LyricsTextBase that owns this syllable's style table.
    std::vector<StyleSpan> m_enigmaStyleMap; ///< the enigma style (in LyricsTextBase) for this syllable.

    friend class texts::LyricsTextBase;
};

/**
 * @class MusicPoint
 * @brief Utility class that represents a single location in musical time.
 *
 * This class is used to specify a point in musical time using a measure and a
 * @ref util::Fraction position within that measure. The position is expressed
 * as a fraction of a whole note, so `1/4` is a quarter-note position.
 *
 * The class is agnostic as to whether the position is a global- or staff-position
 * value. The consuming code makes this determination.
 */
class MusicPoint
{
public:
    /// @brief Constructs a MusicPoint at measure 1, position 0.
    constexpr MusicPoint() = default;

    /// @brief Constructs a MusicPoint object.
    /// @param measId The measure ID.
    /// @param pos The position within the measure.
    constexpr MusicPoint(MeasCmper measId, util::Fraction pos)
        : measureId(measId), position(pos)
    {
    }

    MeasCmper measureId{1};        ///< Measure ID of the point.
    util::Fraction position{};     ///< Position within the measure, where 1/4 is a quarter note value.

    /// @brief Equality comparison operator.
    /// @param other The other point to compare.
    /// @return True if both the measure and position match.
    [[nodiscard]]
    constexpr bool operator==(const MusicPoint& other) const
    { return measureId == other.measureId && position == other.position; }

    /// @brief Inequality comparison operator.
    /// @param other The other point to compare.
    /// @return True if either the measure or position differs.
    [[nodiscard]]
    constexpr bool operator!=(const MusicPoint& other) const
    { return !(*this == other); }

    /// @brief Less-than comparison operator.
    /// @param other The other point to compare.
    /// @return True if this point occurs before @p other.
    [[nodiscard]]
    constexpr bool operator<(const MusicPoint& other) const
    { return measureId < other.measureId || (measureId == other.measureId && position < other.position); }

    /// @brief Less-than-or-equal-to comparison operator.
    /// @param other The other point to compare.
    /// @return True if this point occurs at or before @p other.
    [[nodiscard]]
    constexpr bool operator<=(const MusicPoint& other) const
    { return *this < other || *this == other; }

    /// @brief Greater-than comparison operator.
    /// @param other The other point to compare.
    /// @return True if this point occurs after @p other.
    [[nodiscard]]
    constexpr bool operator>(const MusicPoint& other) const
    { return other < *this; }

    /// @brief Greater-than-or-equal-to comparison operator.
    /// @param other The other point to compare.
    /// @return True if this point occurs at or after @p other.
    [[nodiscard]]
    constexpr bool operator>=(const MusicPoint& other) const
    { return other <= *this; }
};

/**
 * @class MusicRange
 * @brief Utility class that represents of a range of musical time.
 *
 * This class is used to specify start and end points in a musical range using measures and util::Fraction values.
 *
 * The class is agnostic as to whether the positions are global- or staff-position values. The consuming code
 * makes this determination.
 */
class MusicRange : public DocumentElementNoPart
{
public:
    /// @brief Constructs a MusicRange object.
    /// @param document Shared pointer to the document.
    /// @param startMeasId The start measure ID.
    /// @param startPos The start position. (Caller determines whether position is global- or staff-level.)
    /// @param endMeasId The end measure ID.
    /// @param endPos The end position. (Caller determines whether position is global- or staff-level.)
    explicit MusicRange(const DocumentWeakPtr& document, MeasCmper startMeasId, util::Fraction startPos, MeasCmper endMeasId, util::Fraction endPos)
        : MusicRange(document, MusicPoint(startMeasId, startPos), MusicPoint(endMeasId, endPos))
    {
    }

    /// @brief Constructs a MusicRange object.
    /// @param document Shared pointer to the document.
    /// @param startPoint The starting point of the range.
    /// @param endPoint The ending point of the range.
    explicit MusicRange(const DocumentWeakPtr& document, MusicPoint startPoint, MusicPoint endPoint)
        : DocumentElementNoPart(document), start(startPoint), end(endPoint)
    {
    }

    MusicPoint start;   ///< Starting point in the range.
    MusicPoint end;     ///< Ending point in the range.

    /// @brief Returns true of the given metric location is contained in this MusicRange instance.
    /// @param measId The measure ID to search for.
    /// @param position The fractional position within the measure to search for.
    bool contains(MeasCmper measId, util::Fraction position) const
    { return contains(MusicPoint(measId, position)); }

    /// @brief Returns true if the given metric location is contained in this MusicRange instance.
    /// @param point The metric location to search for.
    bool contains(const MusicPoint& point) const
    { return start <= point && point <= end; }

    /// @brief Returns true if the metric location of the specified entry is contained in this MusicRange instance.
    // The MusicRange must be expressed in the Staff EDUs of the entry.
    /// @param entryInfo The entry to check.
    bool contains(const EntryInfoPtr& entryInfo) const;

    /// @brief Returns the next metric location following the music range.
    /// @param forStaff If provided, calculates the next metric location using staff-level Edus.
    /// @return The next location, or std::nullopt if it is past the end of the document or cannot be calculated.
    [[nodiscard]]
    std::optional<MusicPoint> nextLocation(const std::optional<StaffCmper>& forStaff = std::nullopt) const;
};

/**
 * @class TimeSignature
 * @brief Shared time signature class that is derived from other classes. (See @ref others::Measure)
 */
class TimeSignature : public DocumentElementNoPart
{
public:
    /// @enum Abbreviation
    /// @brief Specifies whether a time signature is displayed in numeric or abbreviated form.
    enum class Abbreviation
    {
        NotApplicable, ///< The time signature is not for display, so abbreviation is inapplicable. (Default)
        Numeric,       ///< Display the numeric time signature (e.g., 4/4 or 2/2).
        Abbreviated    ///< Display the abbreviated time signature when available (i.e., common time or cut time).
    };

    /// @brief A single time signature component
    struct TimeSigComponent
    {
        std::vector<util::Fraction> counts; ///< The array of counts in this component.
                                            ///< For simple time signatures this is a single value, and it is an integer like 4/1.
        std::vector<Edu> units;             ///< The array of beat values in this component.
                                            ///< For simple time signatures this is a single value, and it is a note value like 1024 (quarter note)
                                            ///< For straightforward compound time it may be a single dotted value like 1536 (dotted quarter)

        /// @brief Test if two TimeSigComponent values are the same.
        bool operator==(const TimeSigComponent& src) const
        { return counts == src.counts && units == src.units; }

        /// @brief Compute the sum of all counts.
        util::Fraction sumCounts() const
        { return std::accumulate(counts.begin(), counts.end(), util::Fraction{}); }

        /// @brief Compute the sum of all units.
        Edu sumUnits() const
        { return std::accumulate(units.begin(), units.end(), Edu{}); }

        /// @brief Normalizes a single time signature count and unit to a power-of-two unit.
        /// @return The adjusted count and unit.
        static std::pair<util::Fraction, Edu> normalizeCompoundUnit(util::Fraction count, Edu unit);

        /// @brief Returns a copy of this component normalized for compound meter. For example, 2 dotted quarters becomes 6/8.
        /// @return If this component is unambiguously compound meter, returns a normalized version. Otherwise, returns an exact copy
        /// of this component.
        TimeSigComponent normalizeCompound() const;
    };

    std::vector<TimeSigComponent> components;     ///< the components in the time signature

    /// @brief Calculates the simplest form of of this time signature, expressed as a fractional count of @ref NoteType units.
    ///
    /// In typical cases, the returned @ref util::Fraction has a denominator of 1, but Finale supports other kinds of fractions.
    /// Use #util::Fraction::quotient to get the integer value and #util::Fraction::remainder to get the residual fractional component.
    std::pair<util::Fraction, NoteType> calcSimplified() const;

    /// @brief Calculates the total duration of the time signature as a fraction of a whole note.
    util::Fraction calcTotalDuration() const
    {
        util::Fraction result = std::accumulate(components.begin(), components.end(), util::Fraction{},
            [](const util::Fraction& acc, const TimeSigComponent& comp)
            { return acc + (comp.sumCounts() * comp.sumUnits()); }
        );
        return result / Edu(NoteType::Whole);
    }
    
    /// @brief Returns the beat value (duration) at the given EDU position.
    ///
    /// Each TimeSigComponent is treated as a run of identical beats:
    /// - The beat value is the sum of comp.units (in EDU).
    /// - The number of beats is the sum of comp.counts (as util::Fraction).
    ///
    /// @param eduPosition Position from the start of the measure, in EDU.
    /// @return Beat value at @p eduPosition as a fraction of a whole note.
    ///         Returns 0 if no beat information is available.
    util::Fraction calcBeatValueAt(Edu eduPosition) const;

    /// @brief returns whether the two time signatures represent the same time signature
    bool isSame(const TimeSignature& src) const;

    /// @brief Creates a time signature corresponding to the component at @p index
    /// @param index The 0-based component index
    /// @throw std::invalid_argument if @p index is out of range
    MusxInstance<TimeSignature> createComponent(size_t index) const
    {
        checkIndex(index);
        return MusxInstance<TimeSignature>(new TimeSignature(getDocument(), components[index], m_abbreviation));
    }

    /// @brief Returns the abbreviated symbol (code point) for this time signature, or std::nullopt if none.
    ///
    /// If the musx document lacks music symbol options but abbreviation was requested, the SMuFL values
    /// are returned as default substitute values.
    std::optional<char32_t> getAbbreviatedSymbol() const;

    /// @brief Returns if this time signature is common time
    bool isCommonTime() const;
    /// @brief Returns if this time signature is cut time
    bool isCutTime() const;

private:
    void checkIndex(size_t index) const
    {
        if (index > components.size()) {
            throw std::invalid_argument("Index out of range. The time signature has " + std::to_string(components.size())
                + " elements. The index requested was " + std::to_string(index) + ".");
        }
    }

    /// @brief Constructor for measures and independent time signature records.
    explicit TimeSignature(const DocumentWeakPtr& document, int beats, Edu unit, bool hasCompositeTop, bool hasCompositeBottom,
        Abbreviation abbreviate = {});

    /// @brief Constructor for components.
    explicit TimeSignature(const DocumentWeakPtr& document, const TimeSigComponent& timeSigUnit, Abbreviation abbreviate = {})
        : DocumentElementNoPart(document), m_abbreviation(abbreviate)
    {
        components.push_back(timeSigUnit);
    }

    Abbreviation m_abbreviation{};

    friend class others::Measure;
    friend class others::OssiaHeader;
    friend class details::IndependentStaffDetails;
};

namespace others {

// The following classes are defined here because they are shared by multiple subclasses and container classes.

/**
 * @class Enclosure
 * @brief Represents the enclosure settings for text expressions.
 */
class Enclosure : public OthersBase
{
public:
    /**
     * @enum Shape
     * @brief Enumeration for enclosure shapes.
     */
    enum class Shape : uint8_t
    {
        NoEnclosure = 0,    ///< No enclosure
        Rectangle = 1,      ///< Rectangle
        Ellipse = 2,        ///< Ellipse
        Triangle = 3,       ///< Triangle
        Diamond = 4,        ///< Diamond
        Pentagon = 5,       ///< Pentagon
        Hexagon = 6,        ///< Hexagon
        Heptagon = 7,       ///< Heptagon
        Octogon = 8         ///< Octogon
    };

    /**
     * @brief Constructs an Enclosure object.
     * @param document Shared pointer to the document.
     * @param partId Usually 0. This parameter is needed for the generic factory routine.
     * @param shareMode Usually `ShareMode::All`. This parameter is needed for the generic factory routine.
     * @param cmper Comperator parameter. This value is zero for enclosures taken from @ref others::MeasureNumberRegion.
     */
    explicit Enclosure(const DocumentWeakPtr& document, Cmper partId = SCORE_PARTID, ShareMode shareMode = ShareMode::All, Cmper cmper = 0)
        : OthersBase(document, partId, shareMode, cmper) {}

    Evpu xAdd{};              ///< Center X offset - offsets text from center (in EVPU).
    Evpu yAdd{};              ///< Center Y offset - offsets text from center (in EVPU).
    Evpu xMargin{};           ///< Half width - extra space on left/right sides (in EVPU).
    Evpu yMargin{};           ///< Half height - extra space on top/bottom sides (in EVPU).
    Efix lineWidth{};         ///< Line thickness in 64ths of an EVPU (EFIX).
    Shape shape{};            ///< Enclosure shape (default: NoEnclosure).
    Efix cornerRadius{};      ///< Corner radius (in EFIX).
    bool fixedSize{};         ///< Whether the enclosure is fixed size (ignore text bounding box)
    bool equalAspect{};       ///< "Match Height and Width"
    bool notTall{};           ///< "Enforce Minimum Width": don't let shape get taller than it is wide
    bool opaque{};            ///< Whether the enclosure is opaque.
    bool roundCorners{};      ///< Whether the enclosure has rounded corners.

    static const xml::XmlElementArray<Enclosure>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 *
 * @class EnigmaMusicRange
 * @brief The representation of a range of music used by Enigma files.
 *
 * This class is used to specify start and end points in a musical range using measures and EDUs.
 * This is a data class and not a utility class. Internal and external callers should prefer
 * @ref MusicRange, which uses Fraction values to capture the range rather than EDUs.
 */
class EnigmaMusicRange : public OthersBase
{
public:
    /**
     * @brief Constructs a EnigmaMusicRange object.
     * @param document Shared pointer to the document.
     * @param partId The part ID if this range is unlinked, otherwise 0.
     * @param shareMode The share mode if this range is unlinked.
     * @param cmper Comperator parameter. This value is zero for ranges taken from @ref others::StaffUsed.
     * @param inci incident value, for subclasses that have them.
     */
    explicit EnigmaMusicRange(const DocumentWeakPtr& document, Cmper partId = SCORE_PARTID, ShareMode shareMode = ShareMode::All,
            Cmper cmper = 0, std::optional<Inci> inci = std::nullopt)
        : OthersBase(document, partId, shareMode, cmper, inci)
    {
    }

    MeasCmper startMeas{};      ///< Starting measure in the range.
    Edu startEdu{};             ///< Starting EDU (Elapsed Durational Unit) in the range.
    MeasCmper endMeas{};        ///< Ending measure in the range.
    Edu endEdu{};               ///< Ending EDU (Elapsed Durational Unit) in the range.

    /// @brief Returns true of the given metric location is contained in this EnigmaMusicRange instance.
    /// @param measId The measure ID to search for.
    /// @param eduPosition The Edu position within the measure to search for.
    bool contains(MeasCmper measId, Edu eduPosition) const
    {
        return (startMeas < measId || (startMeas == measId && startEdu <= eduPosition)) &&
               (endMeas > measId || (endMeas == measId && endEdu >= eduPosition));
    }

    /// @brief Creates a @ref MusicRange instance corresponding to this instance. The @ref MusicRange
    /// uses @ref util::Fraction for position rather than EDUs. It also has more utility functions.
    MusicRange createMusicRange() const
    {
        return MusicRange(getDocument(), startMeas, util::Fraction::fromEdu(startEdu), endMeas, util::Fraction::fromEdu(endEdu));
    }

    /// @brief Returns the next metric location following the music range.
    /// @param forStaff If provided, calculates the next metric location using staff-level Edus.
    /// @return The next location, or std::nullopt if it is past the end of the document or cannot be calculated.
    std::optional<MusicPoint> nextLocation(const std::optional<StaffCmper>& forStaff = std::nullopt) const
    { return createMusicRange().nextLocation(forStaff); }

    static const xml::XmlElementArray<EnigmaMusicRange>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class NamePositioning
 * @brief Contains horizontal and vertical offsets, alignment, and expansion settings for name positioning.
 *
 * This class is used both for default names as well as name positioning @ref Staff, @ref StaffStyle,
 * and @ref details::StaffGroup.
 */
class NamePositioning : public OthersBase
{
public:

    /**
     * @brief Constructs an NamePositioning object.
     * @param document Shared pointer to the document.
     * @param partId The part ID if this name positioning is unlinked.
     * @param shareMode The share mode if this name positioning is unlinked.
     * @param cmper Comperator parameter. This value is zero for name positioning taken from @ref options::StaffOptions.
     */
    explicit NamePositioning(const DocumentWeakPtr& document, Cmper partId = SCORE_PARTID, ShareMode shareMode = ShareMode::All, Cmper cmper = 0)
        : OthersBase(document, partId, shareMode, cmper) {}

    Evpu horzOff{};             ///< Horizontal distance from staff in @ref Evpu.
    Evpu vertOff{};             ///< Vertical offset from staff in @ref Evpu.
    AlignJustify justify{};     ///< Justification for the name text.
    bool indivPos{};            ///< Indicates that this positioning overrides the default positioning. (Not used by @ref options::StaffOptions.)
    AlignJustify hAlign{};      ///< Horizontal alignment for the name text. (xml node is `<halign>`)
    bool expand{};              ///< "Expand Single Word"
    bool hidden{};              ///< Inverse of "Show": it hides the name.

    static const xml::XmlElementArray<NamePositioning>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

} // namespace others
} // namespace dom
} // namespace mux
