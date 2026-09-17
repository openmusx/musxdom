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

#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_set>

#include "musx/dom/Fundamentals.h"
#include "musx/dom/MusxInstance.h"
#include "musx/dom/ResolvedFontInfo.h"

namespace musx {

namespace dom {
class FontInfo;
class Document;
class TextsBase;

namespace options {
enum class AccidentalInsertSymbolType;
}

} // namespace dom

namespace util {

struct EnigmaResolvedStyles;

/// @class EnigmaStyles
/// @brief Text styles for enigma strings
struct EnigmaStyles
{
    /// @enum CategoryTracking
    /// @brief Specifies is the current enigma style tracks an expressions marking category font.
    /// When the marking category font changes, the Finale U.I. searchs all enigma strings for the category
    /// and modifies any tracked fonts to match the new settings in the marking category. See @ref musx::dom::others::MarkingCategory.
    /// @note Finale never implemented number fonts in the U.I., and it is possible they do not occur in real-world Finale files.
    /// However, the enigma data structures support number fonts.
    enum class CategoryTracking
    {
        None,           ///< no category tracking
        MusicFont,      ///< tracks the category's music font
        TextFont,       ///< tracks the category's text font
        NumberFont      ///< tracks the category's number font (not implemented in Finale U.I.)
    };

    /// @brief constructor
    EnigmaStyles(const std::weak_ptr<dom::Document>& document);

    /// @brief Creates a deep copy of the styles, including copying any shared pointer instances
    EnigmaStyles createDeepCopy() const;

    /// @brief Creates a document-independent snapshot of the styles.
    /// @details Call while the document is still alive, because resolving the font name requires it.
    /// @throws std::invalid_argument if #font has no font definition in the document.
    EnigmaResolvedStyles resolve() const;

    std::shared_ptr<dom::FontInfo> font;    ///< the font to use
    CategoryTracking categoryFont{};        ///< how this font is tracked against a marking category
    dom::Evpu baseline{};                   ///< baseline setting (positive means up)
    dom::Evpu superscript{};                ///< superscript setting added to #baseline (positive means up)
    int tracking{};                         ///< inter-character tracking in EMs (1/1000 font size)
};

/// @class EnigmaResolvedStyles
/// @brief Document-independent snapshot of #EnigmaStyles. (See #EnigmaStyles::resolve.)
///
/// Every member is a value, so an instance remains valid after the @ref dom::Document that produced it is released.
struct EnigmaResolvedStyles
{
    dom::ResolvedFontInfo font;                     ///< the resolved font
    EnigmaStyles::CategoryTracking categoryFont{};  ///< how the font is tracked against a marking category
    dom::Evpu baseline{};                           ///< baseline setting (positive means up)
    dom::Evpu superscript{};                        ///< superscript setting added to #baseline (positive means up)
    int tracking{};                                 ///< inter-character tracking in EMs (1/1000 font size)
};

/// @brief The Enigma text insert a chunk was substituted for. (See #EnigmaTextChunk::insert.)
struct EnigmaTextInsert
{
    std::string command;                    ///< the insert name without the leading caret, e.g. "value" or "flat"
    std::vector<std::string> parameters;    ///< the insert's parameters as written, e.g. {"1"} for `^page(1)`
};

/// @brief A text chunk with a document-independent snapshot of its styles. (See #EnigmaTextChunk::resolve.)
struct EnigmaResolvedTextChunk
{
    std::string text;                           ///< the chunk as valid UTF-8 (see #EnigmaTextChunk::text)
    EnigmaResolvedStyles styles;                ///< the resolved styles active for the chunk
    std::optional<EnigmaTextInsert> insert;     ///< the insert the chunk was substituted for (see #EnigmaTextChunk::insert)
};

/// @brief A text chunk with the Enigma styles active for that chunk.
struct EnigmaTextChunk
{
    /// @brief The chunk as valid UTF-8.
    /// @details With a symbol font, each Unicode code point is a raw glyph index; call
    /// #dom::FontInfo::calcIsSymbolFont on the font in #styles to identify those chunks.
    std::string text;
    EnigmaStyles styles;    ///< the styles active for the chunk
    /// @brief The insert the chunk was substituted for, when it was.
    /// @details The substituted text of an insert is always its own chunk, never merged with the text
    /// around it, so a consumer can replace it as a whole. Text that a recursively parsed insert
    /// (such as `^partname`) produces carries that insert. Style commands are not inserts.
    std::optional<EnigmaTextInsert> insert;

    /// @brief Creates a document-independent snapshot of the chunk. (See #EnigmaStyles::resolve.)
    EnigmaResolvedTextChunk resolve() const;
};


class EnigmaParsingContext;

/**
 * @brief Static class that provides utilities to extract information from enigma strings. Enigma strings
 * use text inserts delineated by a preceding caret (^) and parenthesis for parameters. Here is a list of
 * known inserts.
 *
 * <b>Text formatting</b>
 * - `^font(name[, encoding])`: sets the font face. The optional encoding usually specifies mac (4095) or win (2) symbol encoding.
 * Font IDs (especially zero) can be specified as name in the format "FontXX" where XX is the font Id. Typically this occurs for the default
 * music font ("Font0").
 * - `^fontid(fontId[, encoding])`: sets the font face using the font id within the document.
 * - `^Font(name[, encoding])`: variant of `^font`.
 * - `^fontMus(name[, encoding])`: sets the font face but indicates that it tracks the marking category's Music Font setting.
 * - `^fontTxt(name[, encoding])`: sets the font face but indicates that it tracks the marking category's Text Font setting.
 * - `^fontNum(name[, encoding])`: sets the font face but indicates that it tracks the marking category's Number Font setting.
 * (The number font was never implemented in Finale's UI.)
 * - `^size(points)`: sets the font size in points. Only integer values are allowed.
 * - `^nfx(mask)`: a bitmask of style bits
 * - `^baseline(evpu)`: adjusts the baseline up (positive) or down (negative) by the given Evpu value.
 * - `^superscript(evpu)`: adjusts the baseline up (positive) or down (negative) by the given Evpu value. (Added to baseline value.)
 * - `^tracking(em)`: adjusts the inter-letter spacing by the given EM value. (1/1000 of font size.)
 *
 * <b>Accidental inserts</b>
 * - `^flat()`: inserts a flat using settings in @ref musx::dom::options::TextOptions.
 * - `^natural()`: inserts a natural using settings in @ref musx::dom::options::TextOptions.
 * - `^sharp()`: inserts a sharp using settings in @ref musx::dom::options::TextOptions.
 * - `^dbflat()`: inserts a double flat using settings in @ref musx::dom::options::TextOptions.
 * - `^dbsharp()`: inserts a double sharp using settings in @ref musx::dom::options::TextOptions.
 *
 * <b>Expression playback inserts</b>
 * - `^value()`: inserts the playback value from the associated text or shape expression. (See @ref musx::dom::others::TextExpressionDef::value.)
 * - `^control()`: inserts the playback controller value from the associated text or shape expression. (See @ref musx::dom::others::TextExpressionDef::auxData1.)
 * - `^pass()`: inserts the repeat pass value from the associated text or shape expression. (See @ref musx::dom::others::TextExpressionDef::playPass.)
 *
 * <b>Data/time inserts</b>
 * - `^date(format)`: inserts the current date where format 0=short, 1=long, 2=abbreviated. (See @ref musx::dom::DateFormat.)
 * - `^fdate(format)`: inserts the file modified date where the format is the same as for `^date`.
 * - `^time(seconds)`: inserts the current time where seconds 0=omit seconds, nonzero (normally 1)=include seconds.
 * The OS locale settings determine if it is rendered with AM/PM or 24-hour times. (See @ref musx::dom::options::TextOptions::showTimeSeconds.)
 * - `^perftime(format)`: inserts the total performance time. This is populated from NotationMetadata.xml when provided via
 *   @ref musx::factory::DocumentFactory::CreateOptions. For a list of format values, see the following note.
 *
 * @note The Finale U.I. does not seem to have a mechanism to modify the `^perftime` format from its default value of 4. However, plugins
 * can easily create an Enigma string with any of the format values. They are as follows.
 * - 0: M:SS (e.g., `1:02`)
 * - 1: HH:MM:SS (e.g., `00:01:02`)
 * - 2: HH:MM:SS.mmm (e.g., `00:01:02.000`)
 * - 3: MM:SS (e.g., `01:02`)
 * - 4: M&apos;SS&quot; (e.g., `1'02"`)
 * - 5: M (e.g., `1`)
 *
 * <b>Other text subsitution inserts</b>
 * - `^^`: inserts a caret (^).
 * - `^arranger()`: inserts the arranger name from File Info (ScoreManager window).
 * - `^composer()`: inserts the composer name from File Info (ScoreManager window).
 * - `^copyright()`: inserts the copyright text from File Info (ScoreManager window).
 * - `^cprsym()`: inserts a copyright (@) symbol.
 * - `^description()`: inserts the description text from File Info (ScoreManager window).
 * - `^filename()`: inserts the file name (no path).
 * - `^lyricist()`: inserts the lyricist name from File Info (ScoreManager window).
 * - `^page(offset)`: inserts the current page number, offsetting from the offset parameter. (The Finale UI prevents more than one `^page` insert per Enigma string.)
 * - `^partname()`: score or part name. (Score uses the score text from File Info.)
 * - `^rehearsal()`: inserts the text for a rehearsal sequence into a text expression.
 * - `^subtitle()`: inserts the subtitle from File Info (ScoreManager window).
 * - `^title()`: inserts the title from File Info (ScoreManager window).
 * - `^totpages()`: inserts the total number of pages in the document.
 * - `^url(textId)`: probably intended for adding hyperlinks to enigma text but never implemented. The `textId` is the cmper of the hyperlink block text,
 * but Finale does not render anything for this insert.
*/
class EnigmaString
{
public:
    /// @brief Convert u8"" literals to std::string in a way that works both in C++17 and C++20+
    template <typename CharT>
    static std::string fromU8(const CharT* s)
    {
        static_assert(std::is_same<CharT, char>::value || std::is_same<CharT, decltype(u8'a')>::value,
            "fromU8() only accepts char or char8_t pointers");
        return std::string(reinterpret_cast<const char*>(s));
    }

    /// @brief Converts a 32-bit codepoint to a utf8-encoded std::string.
    static std::string toU8(char32_t cp);

    /// @brief Converts a 32-bit codepoint string to a utf8-encoded std::string.
    static std::string toU8(const std::u32string& value);

    /**
     * @brief Enumeration to specify the default handling for accidental insert commands. Like all Enigma commands,
     * accidental insert commands are passed to the TextInsertCallback function if there is one. These options determine
     * how the insert is handled if the callback opts not to handle it.
     */
    enum class AccidentalInsertHandling
    {
        /**
         * @brief Parse accidental insert commands into glyph font changes and character strings.
         *
         * Suitable when the parser should produce fully parsed output ready for rendering.
         */
        ParseToGlyphs,

        /**
         * @brief Substitute accidental insert commands with textual representations.
         *
         * The substitution style is determined by the `substitutionStyle` field in EnigmaParsingOptions.
         * Suitable for environments where accidentals should be represented as text rather than as font changes.
         */
        Substitute
    };

    /**
     * @brief Enumeration to specify the type of accidental substitution representation.
     *
     * Defines how accidentals are represented in text when substitution is selected.
     */
    enum class AccidentalStyle
    {
        /**
         * @brief Use ASCII substitutions for accidentals.
         *
         * - Flat: 'b'
         * - Sharp: '#'
         * - Natural: (empty string)
         * - Double Flat: 'bb'
         * - Double Sharp: 'x'
         *
         * Suitable for environments without Unicode or SMuFL support.
         */
        Ascii,

        /**
         * @brief Use Unicode text accidentals.
         *
         * - Flat: ♭ (U+266D)
         * - Sharp: ♯ (U+266F)
         * - Natural: ♮ (U+266E)
         * - Double Flat: 𝄫 (U+1D12B)
         * - Double Sharp: 𝄪 (U+1D12A)
         *
         * Suitable for inline text representations like "Clarinet in B♭."
         */
        Unicode,

        /**
         * @brief Use SMuFL notation-specific accidentals.
         *
         * - Flat: (U+E260)
         * - Sharp: (U+E262)
         * - Natural: (U+E261)
         * - Double Flat: (U+E264)
         * - Double Sharp: (U+E263)
         *
         * Suitable for musical notation systems or specialized fonts that support SMuFL.
         */
        Smufl
    };

    /**
     * @brief Options for configuring how Enigma strings are parsed.
     *
     * This struct encapsulates all parsing configuration for Enigma string processing,
     * including accidental insert handling behavior and substitution style.
     */
    struct EnigmaParsingOptions
    {
        /// @brief constructor
        EnigmaParsingOptions() :
            insertHandling(AccidentalInsertHandling::ParseToGlyphs),
            substitutionStyle(AccidentalStyle::Unicode) {}

        /// @brief constructor for accidental substitution
        EnigmaParsingOptions(AccidentalStyle accidentalStyle) :
            insertHandling(AccidentalInsertHandling::Substitute),
            substitutionStyle(accidentalStyle) {}

        /**
         * @brief Specifies how accidental insert commands are handled during parsing.
         *
         * Defaults to ParseToGlyphs, which parses insert commands into font changes
         * and glyph strings.
         */
        AccidentalInsertHandling insertHandling;

        /**
         * @brief Specifies the accidental substitution style used when insertHandling is Substitute.
         *
         * Ignored unless insertHandling is set to Substitute.
         * Defaults to Unicode substitution.
         */
        AccidentalStyle substitutionStyle;

        /**
         * @brief Specifies whether to strip unknown tags or dump them into the output string.
         */
        bool stripUnknownTags = true;

        /**
         * @brief If value is true, font & style tags are ignored. Note that you may still get
         * get font and style changes for accidental inserts.
         */
        bool ignoreStyleTags = false;

        /**
         * @brief Skip tags in this set. Primarily used by the "partname" insert to avoid a situation where
         * the partname itself contains a `^partname` tag. (Though the Finale UI prevents this, a
         * plugin might possibly do it in theory.)
         */
        std::unordered_set<std::string_view> ignoreTags;
    };

    /** @brief Returns true if the enigma string starts with a font insert. */
    static bool startsWithFontCommand(const std::string& text);

    /** @brief Returns true if the enigma string starts with a style insert. */
    static bool startsWithStyleCommand(const std::string& text);

    /// @brief Returns the accidental insert symbol type if the input command is an accidental insert
    static std::optional<dom::options::AccidentalInsertSymbolType> commandIsAccidentalType(std::string_view commandText);

    /**
     * @brief Parses an enigma text insert into its constituent components.
     *
     * The function takes an enigma text insert starting with `^` and extracts the insert
     * and its parameters. If the string is invalid or unbalanced, it returns an empty vector.
     *
     * Examples:
     * @code{.cpp}
     * parseComponents("^fontTxt(Times,4096)");   // Returns {"fontTxt", "Times", "4096"}
     * parseComponents("^size(10)");              // Returns {"size", "10"}
     * parseComponents("^nfx(130,(xyz))");        // Returns {"nfx", "130", "(xyz)"}
     * parseComponents("^some");                  // Returns {"some"}
     * parseComponents("^^");                     // Returns {"^"}
     * parseComponents("^unbalanced(abc");        // Returns {}
     * @endcode
     *
     * @param input The enigma text insert to parse.
     * @param parsedLength If supplied, returns the number of characters parsed
     * @return A vector of strings representing the insert and its parameters, or an empty vector if invalid.
     */
    static std::vector<std::string> parseComponents(const std::string& input, size_t* parsedLength = nullptr);

    /// @brief Iteration function type that the parser calls back when the font has changed or when recursively parsing
    /// an insert that is itself an enigma string, such as (in particular) the part name.
    /// - text: the chunk of text, with the same UTF-8 contract as #EnigmaTextChunk::text
    /// - font: the font information.
    using TextChunkCallback = std::function<bool(
        const std::string& text,
        const EnigmaStyles& styles
    )>;
    /// @brief Variant of #TextChunkCallback that also receives the insert a chunk was substituted for,
    /// or nullptr for ordinary text. (See #EnigmaTextChunk::insert.)
    using TextChunkWithInsertCallback = std::function<bool(
        const std::string& text,
        const EnigmaStyles& styles,
        const EnigmaTextInsert* insert
    )>;
    /// @brief Adapts a #TextChunkCallback to a #TextChunkWithInsertCallback that ignores the insert.
    static TextChunkWithInsertCallback ignoringInserts(const TextChunkCallback& onText)
    {
        return [&onText](const std::string& text, const EnigmaStyles& styles, const EnigmaTextInsert*) { return onText(text, styles); };
    }

    /// @brief Iteration function type that the parser calls back when it encounters an Enigma text insert
    /// that requires text subtitution. If the function returns an empty string, the insert is stripped from the
    /// processed text.
    ///
    /// If the function returns std::nullopt, the Enigma parsing function inserts an appropriate
    /// value. Therefore, TextInsertCallback functions should only process known commands and return std::nullopt
    /// for any others.
    ///
    /// - parsedCommand: a vector containing the insert (without the leading '^') and each of its parameters.
    using TextInsertCallback = std::function<std::optional<std::string>(
        const std::vector<std::string>& parsedCommand
    )>;

    /// @brief Inserts callback to take all default insert subsitutions determined by #parseEnigmaText.
    static inline TextInsertCallback defaultInsertsCallback = [](const std::vector<std::string>&) { return std::nullopt; };

    /**
     * @brief Parses an Enigma-formatted string, handling font inserts and escaped carets.
     *
     * This function scans an Enigma-formatted string (typically Finale-style encoded text),
     * extracts text chunks and control inserts, and invokes a callback for each contiguous span
     * of text using the current font state. It automatically interprets font-related inserts
     * like ^fontTxt, ^fontMus, ^fontid, ^size, and ^nfx, updating the font info accordingly.
     *
     * Escaped carets ("^^") are converted to literal '^' characters. All other unrecognized
     * inserts (e.g., ^value(...) or ^page(...)) are passed through to the callback for higher-level
     * processing or markup.
     *
     * The ^date, ^time, and ^fdate inserts default to POSIX functions for formatting date and time on macOS and Linux.
     * On Windows they default to WinAPI. If you need finer control over date or time formatting, you should handle
     * these yourself.
     *
     * @param document The document from which the enigma string is taken.
     * @param forPartId The linked part ID to use for ^partname and ^totpages tags.
     * @param rawText The full input Enigma string to parse.
     * @param onText The handler for when font styling changes.
     * @param onInsert The handler to substitute text for an insert.
     * @param options Parsing options.
     * @param parsingContext Generally, only functions internal to musxdom should provide this.
     * @return true if parsing completed, false if aborted by the @p onText function.
     */
    static bool parseEnigmaText(const std::shared_ptr<dom::Document>& document, dom::Cmper forPartId, const std::string& rawText,
        const TextChunkCallback& onText, const TextInsertCallback& onInsert,
        const EnigmaParsingOptions& options = {}, const EnigmaParsingContext* parsingContext = nullptr)
    {
        return parseEnigmaTextImpl(document, forPartId, rawText, ignoringInserts(onText), onInsert, options, parsingContext, EnigmaStyles(document), nullptr);
    }

    /// @brief Variant of #parseEnigmaText whose text handler also receives the insert each chunk was substituted for.
    static bool parseEnigmaText(const std::shared_ptr<dom::Document>& document, dom::Cmper forPartId, const std::string& rawText,
        const TextChunkWithInsertCallback& onText, const TextInsertCallback& onInsert,
        const EnigmaParsingOptions& options = {}, const EnigmaParsingContext* parsingContext = nullptr)
    {
        return parseEnigmaTextImpl(document, forPartId, rawText, onText, onInsert, options, parsingContext, EnigmaStyles(document), nullptr);
    }

    /// @brief Simplified version of #parseEnigmaText that strips unhandled inserts.
    /// Useful in particular when the caller only cares about the raw text or the font information.
    /// @param document The document from which the enigma string is taken.
    /// @param forPartId The linked part ID to use for ^partname and ^totpages tags.
    /// @param rawText The full input Enigma string to parse.
    /// @param onText The handler for when font styling changes.
    /// @param options Parsing options.
    /// @param parsingContext Generally, only functions internal to musxdom should provide this.
    /// @return true if parsing completed, false if aborted by the @p onText function.
    static bool parseEnigmaText(const std::shared_ptr<dom::Document>& document, dom::Cmper forPartId, const std::string& rawText, const TextChunkCallback& onText,
        const EnigmaParsingOptions& options = {}, const EnigmaParsingContext* parsingContext = nullptr)
    {
        return parseEnigmaTextImpl(document, forPartId, rawText, ignoringInserts(onText), defaultInsertsCallback, options, parsingContext, EnigmaStyles(document), nullptr);
    }

    /**
     * @brief Incorporates an enigma font insert into the supplied @ref dom::FontInfo instance.
     *
     * Enigma font inserts are
     * - one of the font identifying inserts, such as `^font`, `fontid`, etc. (See @ref startsWithFontCommand.)
     * - `^size` specifies the font size in points.
     * - `^nfx` specifies a bit mask of style properties. These are resolved with @ref dom::FontInfo::setEnigmaStyles.
     */
    static bool parseStyleCommand(std::vector<std::string> components, EnigmaStyles& styles);

    /** @brief Trims all enigma tags from an enigma string, leaving just the plain text. */
    static std::string trimTags(const std::string& input);

    /// @brief Concatenate the plain text from collected Enigma text chunks.
    static std::string plainTextFromChunks(const std::vector<EnigmaTextChunk>& chunks);

private:
    /// @param enclosingInsert The insert whose recursive expansion this parse is, or nullptr at the top level.
    static bool parseEnigmaTextImpl(const std::shared_ptr<dom::Document>& document, dom::Cmper forPartId, const std::string& rawText,
        const TextChunkWithInsertCallback& onText, const TextInsertCallback& onInsert,
        const EnigmaParsingOptions& options, const EnigmaParsingContext* parsingContext,
        const EnigmaStyles& startingStyles, const EnigmaTextInsert* enclosingInsert);
};

/// @class EnigmaParsingContext
/// @brief Wrapper class for interpreting and rendering Enigma-style strings with insert handling.
///
/// This class provides implementations of `getText()` and `parseEnigmaText()`
/// that retrieve the raw text from a `TextsBase` object, either directly or by ID,
/// and perform substitution of special insert commands such as `^page`, `^date`, etc.
class EnigmaParsingContext
{
private:
    dom::MusxInstance<dom::TextsBase> m_rawText;
    dom::Cmper m_forPartId;
    std::optional<int> m_forPageNumber;
    EnigmaString::TextInsertCallback m_insertFunc;

public:
    /// @brief Null constructor
    EnigmaParsingContext()
        : m_rawText(nullptr), m_forPartId(dom::SCORE_PARTID), m_insertFunc(EnigmaString::defaultInsertsCallback)
    {}

    /// @brief Constructor
    /// @param rawText The raw text to use
    /// @param forPartId The linked part ID to use for ^partname and ^totpages inserts
    /// @param forPageId The value to use as page number for ^page inserts. ("#" is inserted if not provided, mimicing Finale behavior.)
    /// @param insertFunc A common handler for insert conversions.
    EnigmaParsingContext(dom::MusxInstance<dom::TextsBase> rawText, dom::Cmper forPartId,
            std::optional<dom::Cmper> forPageId = std::nullopt,
            EnigmaString::TextInsertCallback insertFunc = EnigmaString::defaultInsertsCallback)
        : m_rawText(std::move(rawText)), m_forPartId(forPartId), m_forPageNumber(forPageId), m_insertFunc(insertFunc)
    {}

    /// @brief Check whether the context holds a valid raw text pointer.
    explicit operator bool() const noexcept
    { return static_cast<bool>(m_rawText); }

    std::string affixText;          ///< Prefix or suffix to be passed in parsing options.
    bool affixIsPrefix{};           ///< True if #affixText is a prefix; false if it is a suffix.

    /// @brief Return displayable text with Enigma tags converted.
    /// @param trimTags Whether to trim unknown tags or dump them into the output.
    /// @param accidentalStyle The accidental substitution style to use. (ASCII, Unicode, SMuFL)
    /// @param ignoreTags A list of tags to ignore. (Normally only used internally.)
    /// @return The final rendered string.
    std::string getText(bool trimTags = false,
        EnigmaString::AccidentalStyle accidentalStyle = EnigmaString::AccidentalStyle::Ascii,
        const std::unordered_set<std::string_view>& ignoreTags = {}) const;

    /// @brief Parse the Enigma text into structured chunks with insert handling.
    /// @param onText The handler for font and text style changes.
    /// @param onInsert The handler for insert conversions. This function is called first. If it returns a string,
    /// the default insert value is skipped.
    /// @param options The options for the parsing session.
    /// @return True if parsing was successful.
    bool parseEnigmaText(const EnigmaString::TextChunkCallback& onText,
        const EnigmaString::TextInsertCallback& onInsert,
        const EnigmaString::EnigmaParsingOptions& options = {}) const
    {
        return parseEnigmaText(EnigmaString::ignoringInserts(onText), onInsert, options);
    }

    /// @brief Variant of #parseEnigmaText whose text handler also receives the insert each chunk was substituted for.
    /// @param onText The handler for font and text style changes, receiving the chunk's insert or nullptr.
    /// @param onInsert The handler for insert conversions. This function is called first. If it returns a string,
    /// the default insert value is skipped.
    /// @param options The options for the parsing session.
    /// @return True if parsing was successful.
    bool parseEnigmaText(const EnigmaString::TextChunkWithInsertCallback& onText,
        const EnigmaString::TextInsertCallback& onInsert,
        const EnigmaString::EnigmaParsingOptions& options = {}) const;

    /// @brief Parse the Enigma text into structured chunks with insert handling.
    /// @param onText The handler for font and text style changes.
    /// @param options The options for the parsing session.
    /// @return True if parsing was successful.
    bool parseEnigmaText(const EnigmaString::TextChunkCallback& onText,
        const EnigmaString::EnigmaParsingOptions& options = {}) const
    {
        return parseEnigmaText(onText, EnigmaString::defaultInsertsCallback, options);
    }

    /// @brief Collect the Enigma text into styled chunks with insert handling.
    /// @param onInsert The handler for insert conversions.
    /// @param options The options for the parsing session.
    /// @return The collected chunks emitted by #parseEnigmaText.
    std::vector<EnigmaTextChunk> collectEnigmaTextChunks(
        const EnigmaString::TextInsertCallback& onInsert,
        const EnigmaString::EnigmaParsingOptions& options = {}) const;

    /// @brief Collect the Enigma text into styled chunks with default insert handling.
    /// @param options The options for the parsing session.
    /// @return The collected chunks emitted by #parseEnigmaText.
    std::vector<EnigmaTextChunk> collectEnigmaTextChunks(
        const EnigmaString::EnigmaParsingOptions& options = {}) const;

    /**
     * @brief Returns a shared pointer to a FontInfo instance that reflects
     * the first font information in the text.
     */
    dom::MusxInstance<dom::FontInfo> parseFirstFontInfo() const;

    /// @brief Get the raw text pointer
    dom::MusxInstance<dom::TextsBase> getRawText() const { return m_rawText; }

    // If there is ever a need for a non-const version of the pointer, we can always
    // look it up again.

    /// @brief Get the requested part id.
    dom::Cmper getRequestedPartId() const { return m_forPartId; }

    /// @brief Get the page number that was supplied, if any.
    std::optional<int> getPageNumber() const { return m_forPageNumber; }
};

} // namespace util
} // namespace musx
