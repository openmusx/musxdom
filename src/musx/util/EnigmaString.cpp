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
#include <regex>
#include <unordered_map>
#include <cctype>
#include <string>
#include <cmath>
#include <sstream>
#include <iomanip>

#include "musx/musx.h"

using namespace musx::dom;

namespace musx {
namespace util {

namespace {
std::string formatPerfTime(double secondsValue, int format)
{
    if (secondsValue < 0.0) {
        return {};
    }

    const auto secondsRounded = static_cast<long long>(std::llround(secondsValue));
    const auto millisRounded = static_cast<long long>(std::llround(secondsValue * 1000.0));
    auto pad2 = [](long long v) {
        std::ostringstream s;
        s << std::setfill('0') << std::setw(2) << v;
        return s.str();
    };
    auto pad3 = [](long long v) {
        std::ostringstream s;
        s << std::setfill('0') << std::setw(3) << v;
        return s.str();
    };

    switch (format) {
    case 0: { // M:SS
        const auto minutes = secondsRounded / 60;
        const auto seconds = secondsRounded % 60;
        return std::to_string(minutes) + ":" + pad2(seconds);
    }
    case 1: { // HH:MM:SS
        const auto hours = secondsRounded / 3600;
        const auto minutes = (secondsRounded % 3600) / 60;
        const auto seconds = secondsRounded % 60;
        return pad2(hours) + ":" + pad2(minutes) + ":" + pad2(seconds);
    }
    case 2: { // HH:MM:SS.mmm
        const auto hours = millisRounded / 3600000;
        const auto minutes = (millisRounded % 3600000) / 60000;
        const auto seconds = (millisRounded % 60000) / 1000;
        const auto millis = millisRounded % 1000;
        return pad2(hours) + ":" + pad2(minutes) + ":" + pad2(seconds) + "." + pad3(millis);
    }
    case 3: { // MM:SS
        const auto minutes = secondsRounded / 60;
        const auto seconds = secondsRounded % 60;
        return pad2(minutes) + ":" + pad2(seconds);
    }
    case 4: { // M'SS"
        const auto minutes = secondsRounded / 60;
        const auto seconds = secondsRounded % 60;
        return std::to_string(minutes) + "'" + pad2(seconds) + "\"";
    }
    case 5: // M
        return std::to_string(secondsRounded / 60);
    default:
        return {};
    }
}
} // namespace

EnigmaStyles::EnigmaStyles(const std::weak_ptr<dom::Document>& document)
    : font(std::make_shared<dom::FontInfo>(document))
{
}

EnigmaStyles EnigmaStyles::createDeepCopy() const
{
    EnigmaStyles result = *this;
    result.font = std::make_shared<dom::FontInfo>(*this->font);
    return result;
}

EnigmaResolvedStyles EnigmaStyles::resolve() const
{
    EnigmaResolvedStyles result;
    if (font) {
        result.font = font->resolve();
    }
    result.categoryFont = categoryFont;
    result.baseline = baseline;
    result.superscript = superscript;
    result.tracking = tracking;
    return result;
}

EnigmaResolvedTextChunk EnigmaTextChunk::resolve() const
{
    return { text, styles.resolve(), insert };
}

std::string EnigmaString::toU8(char32_t cp)
{
    if (cp == 0) {
        return {};
    }
    std::string result;
    if (cp <= 0x7F) {
        result += static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        result += static_cast<char>(0xC0 | (cp >> 6));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        result += static_cast<char>(0xE0 | (cp >> 12));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0x10FFFF) {
        result += static_cast<char>(0xF0 | (cp >> 18));
        result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    }
    // Invalid code points are ignored (returning empty string).
    return result;
}

std::string EnigmaString::toU8(const std::u32string& value)
{
    std::string result;
    for (char32_t codePoint : value) {
        result += EnigmaString::toU8(codePoint);
    }
    return result;
}

bool EnigmaString::startsWithFontCommand(const std::string& text)
{
    static const std::vector<std::string> kEnigmaFontCommands = { "^font", "^fontid", "^Font", "^fontMus", "^fontTxt", "^fontNum", "^size", "^nfx" };

    for (const auto& textCmd : kEnigmaFontCommands) {
        if (text.rfind(textCmd, 0) == 0) { // Checks if text starts with textCmd
            return true;
        }
    }
    return false;
}

bool EnigmaString::startsWithStyleCommand(const std::string& text)
{
    static const std::vector<std::string> kEnigmaStyleCommands = { "^baseline", "^superscript", "^tracking" };
    if (startsWithFontCommand(text)) {
        return true;
    }
    for (const auto& textCmd : kEnigmaStyleCommands) {
        if (text.rfind(textCmd, 0) == 0) { // Checks if text starts with textCmd
            return true;
        }
    }
    return false;
}

std::vector<std::string> EnigmaString::parseComponents(const std::string& input, size_t* parsedLength)
{
    if (parsedLength) {
        *parsedLength = 0;
    }
    if (input.empty() || input[0] != '^') {
        return {}; // Invalid input
    }

    if (input.size() >= 2 && input[1] == '^') {
        if (parsedLength) {
            *parsedLength = 2;
        }
        return { "^" }; // "^^" returns "^"
    }

    size_t i = 1; // Start after '^'
    while (i < input.size() && std::isalpha(input[i]))
        ++i;

    if (i == 1) {
        return {}; // No valid command found
    }

    std::vector<std::string> components;
    components.push_back(input.substr(1, i - 1)); // Extract command

    if (i < input.size() && input[i] == '(') {
        size_t start = ++i, depth = 1;
        while (i < input.size() && depth > 0) {
            if (input[i] == '(')
                ++depth;
            else if (input[i] == ')')
                --depth;
            ++i;
        }

        if (depth != 0)
            return {}; // Unbalanced parentheses

        std::string params = input.substr(start, i - start - 1);
        size_t j = 0, paramStart = 0, parenDepth = 0;

        // Split parameters by ',' while respecting nested parentheses
        while (j <= params.size()) {
            if (j == params.size() || (params[j] == ',' && parenDepth == 0)) {
                if (j > paramStart)
                    components.push_back(params.substr(paramStart, j - paramStart));
                paramStart = j + 1;
            } else if (params[j] == '(')
                ++parenDepth;
            else if (params[j] == ')')
                --parenDepth;
            ++j;
        }
    }

    // No parenthesis group — command is just ^command
    if (parsedLength) {
        *parsedLength = i;
    }
    return components;
}

bool EnigmaString::parseStyleCommand(std::vector<std::string> components, EnigmaStyles& styles)
{
    static const std::unordered_map<std::string_view, EnigmaStyles::CategoryTracking> trackingMap = {
        { "fontMus", EnigmaStyles::CategoryTracking::MusicFont },
        { "fontTxt", EnigmaStyles::CategoryTracking::TextFont},
        { "fontNum", EnigmaStyles::CategoryTracking::NumberFont },
    };

    if (components.size() < 2) {
        return false;
    }

    const std::string& commandPart = components[0];
    if (commandPart == "fontMus" || commandPart == "fontTxt" || commandPart == "fontNum" || commandPart == "font" || commandPart == "Font" || commandPart == "fontid") {
        const std::string& param1 = components[1];
        if (commandPart == "fontid") {
            styles.font->fontId = Cmper(std::stoi(param1));
        } else if (param1.find("Font") == 0) { // font name starts with "Font"
            const auto fontIdStr = param1.substr(4);
            if (!fontIdStr.empty() && std::all_of(fontIdStr.begin(), fontIdStr.end(), ::isdigit)) {
                styles.font->fontId = Cmper(std::stoi(fontIdStr));
            } else {
                styles.font->setFontIdByName(param1);
            }
        } else {
            styles.font->setFontIdByName(param1);
        }
        auto it = trackingMap.find(commandPart);
        if (it != trackingMap.end()) {
            styles.categoryFont = it->second;
        } else {
            styles.categoryFont = EnigmaStyles::CategoryTracking::None;
        }
        return true;
    } else if (commandPart == "nfx") {
        styles.font->setEnigmaStyles(uint16_t(std::stoi(components[1])));
        return true;
    } else if (commandPart == "size") {
        styles.font->fontSize = std::stoi(components[1]);
        return true;
    } else if (commandPart == "baseline") {
        styles.baseline = std::stoi(components[1]);
        return true;
    } else if (commandPart == "superscript") {
        styles.superscript = std::stoi(components[1]);
        return true;
    } else if (commandPart == "tracking") {
        styles.tracking = std::stoi(components[1]);
        return true;
    }

    return false;
}

std::optional<options::AccidentalInsertSymbolType> EnigmaString::commandIsAccidentalType(std::string_view commandText)
{
    using AcciSymType = options::AccidentalInsertSymbolType;

    static const std::unordered_map<std::string_view, AcciSymType> acciInsertMap{
        { "flat",       AcciSymType::Flat },
        { "natural",    AcciSymType::Natural },
        { "sharp",      AcciSymType::Sharp },
        { "dbflat",     AcciSymType::DblFlat },
        { "dbsharp",    AcciSymType::DblSharp },
    };

    const auto it = acciInsertMap.find(commandText);
    if (it != acciInsertMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool EnigmaString::parseEnigmaTextImpl(const std::shared_ptr<dom::Document>& document, Cmper forPartId, const std::string& rawText,
    const TextChunkWithInsertCallback& onText, const TextInsertCallback& onInsert, const EnigmaParsingOptions& options,
    const EnigmaParsingContext* parsingContext, const EnigmaStyles& startingStyles, const EnigmaTextInsert* enclosingInsert)
{
    auto currentStyles = startingStyles;
    std::string prefix = parsingContext && parsingContext->affixIsPrefix ? parsingContext->affixText : std::string();
    bool parsedResultIsEmpty = true;
    std::string remaining = rawText;
    std::optional<std::string> textBuffer;

    using AcciSymType = options::AccidentalInsertSymbolType;
    using AcciSymValue = std::tuple<char32_t, char32_t, std::string_view>; // smufl, unicode, ascii

    static const std::unordered_map<AcciSymType, AcciSymValue> accidentalSymbols {
        { AcciSymType::Flat,     { U'\uE260', U'\u266D', "b"   } }, // SMuFL flat, Unicode flat ♭, ASCII "b"
        { AcciSymType::Natural,  { U'\uE261', U'\u266E', ""    } }, // SMuFL natural, Unicode natural ♮, no ASCII
        { AcciSymType::Sharp,    { U'\uE262', U'\u266F', "#"   } }, // SMuFL sharp, Unicode sharp ♯, ASCII "#"
        { AcciSymType::DblFlat,  { U'\uE264', U'\U0001D12B', "bb"  } }, // SMuFL double flat, Unicode double flat 𝄫, ASCII "bb"
        { AcciSymType::DblSharp, { U'\uE263', U'\U0001D12A', "x"   } }, // SMuFL double sharp, Unicode double sharp 𝄪, ASCII "x"
    };

    auto addToBuf = [&](const std::string_view text) {
        if (!textBuffer) {
            textBuffer.emplace();
        }
        if (!prefix.empty() && !text.empty()) {
            textBuffer->append(prefix);
            prefix.clear();
        }
        textBuffer->append(text);
        if (parsedResultIsEmpty) {
            // keep trying until we have added something to textBuffer
            parsedResultIsEmpty = textBuffer.value().empty();
        }
    };

    // Emits the buffered text as one chunk. Text inside a recursively parsed insert carries that insert.
    auto processChunk = [&](const EnigmaStyles& styles, const EnigmaTextInsert* insert = nullptr) -> bool {
        if (textBuffer.has_value() && !textBuffer->empty()) {
            bool result = onText(textBuffer.value(), styles, insert ? insert : enclosingInsert);
            textBuffer = std::nullopt;
            if (!result) {
                return false;
            }
        }
        textBuffer.emplace(""); // after parsing a style command, make sure the style change is reported even if no text.
        return true;
    };

    // Emits an insert's substituted text as a chunk of its own, so that a consumer can replace it as a whole.
    auto emitInsert = [&](const EnigmaTextInsert& insert, const std::string_view text) -> bool {
        if (text.empty()) {
            return true; // a stripped insert leaves the surrounding text as one chunk
        }
        if (!processChunk(currentStyles)) {
            return false;
        }
        addToBuf(text);
        const bool result = processChunk(currentStyles, &insert);
        textBuffer = std::nullopt; // only a style command needs an empty chunk reported
        return result;
    };

    while (!remaining.empty()) {
        size_t caretPos = remaining.find('^');

        // Emit text before next command
        if (caretPos != std::string::npos && caretPos > 0) {
            addToBuf(remaining.substr(0, caretPos));
            remaining.erase(0, caretPos);
        } else if (caretPos == std::string::npos) {
            addToBuf(remaining);
            break;
        }

        size_t parsedLen = 0;
        // Try to parse the next command
        auto components = parseComponents(remaining, &parsedLen);

        if (startsWithStyleCommand(remaining)) {
            if (!options.ignoreStyleTags) {
                if (!processChunk(currentStyles)) {
                    return false;
                }
                if (!parseStyleCommand(components, currentStyles)) {
                    throw std::invalid_argument("malformed style command encountered in Enigma text: " + rawText);
                }
            }
            remaining.erase(0, parsedLen);
            continue;
        }

        if (components.empty() || parsedLen == 0) {
            // Not a valid command — treat '^' as literal
            addToBuf("^");
            remaining.erase(0, 1);
            continue;
        }

        std::string fullCommand = remaining.substr(0, parsedLen);
        remaining.erase(0, parsedLen);

        // Handle ^^ (escaped caret)
        if (components.size() == 1 && components[0] == "^") {
            addToBuf("^");
            continue;
        }

        const EnigmaTextInsert insert{ components[0], std::vector<std::string>(components.begin() + 1, components.end()) };

        // Send command to the handler and use that if the handler handles it.
        std::optional<std::string> replacement = onInsert(components);
        if (replacement.has_value()) {
            if (!emitInsert(insert, replacement.value())) {
                return false;
            }
            continue;
        }

        if (auto accidentalType = commandIsAccidentalType(components[0])) {
            switch (options.insertHandling) {
                case AccidentalInsertHandling::Substitute:
                {
                    auto it = accidentalSymbols.find(accidentalType.value());
                    if (it != accidentalSymbols.end()) {
                        const auto [smufl, ucode, asciiStr] = it->second;
                        std::string substitution;
                        switch (options.substitutionStyle) {
                        case AccidentalStyle::Smufl:
                            substitution = toU8(smufl);
                            break;
                        case AccidentalStyle::Unicode:
                            substitution = toU8(ucode);
                            break;
                        case AccidentalStyle::Ascii:
                            substitution = asciiStr;
                            break;
                        }
                        if (!emitInsert(insert, substitution)) {
                            return false;
                        }
                    } else {
                        assert(false);
                        throw std::logic_error("Accidental subsitution type not mapped for " + components[0]);
                    }
                    continue;
                }
                case AccidentalInsertHandling::ParseToGlyphs:
                {
                    if (const auto textOptions = document->getOptions()->get<options::TextOptions>()) {
                        const auto& acciDataIt = textOptions->symbolInserts.find(accidentalType.value());
                        if (acciDataIt != textOptions->symbolInserts.end()) {
                            EnigmaStyles acciStyles(document);
                            const auto& insertInfo = acciDataIt->second;
                            *acciStyles.font.get() = *insertInfo->symFont.get();
                            acciStyles.font->fontSize = int(std::lround(double(insertInfo->symFont->fontSize) * double(currentStyles.font->fontSize) / 100.0));
                            acciStyles.baseline = int(std::lround((double(insertInfo->baselineShiftPerc) * double(currentStyles.font->fontSize)) * (EVPU_PER_POINT / 100.0)));
                            acciStyles.tracking = insertInfo->trackingBefore;
                            if (!processChunk(currentStyles)) {
                                return false;
                            }
                            textBuffer.emplace(toU8(insertInfo->symChar));
                            if (!processChunk(acciStyles, &insert)) {
                                return false;
                            }
                        } else {
                            MUSX_INTEGRITY_ERROR("Document contains no accidental insert options for " + components[0] + ".");
                        }
                    } else {
                        MUSX_INTEGRITY_ERROR("Document contains no text options.");
                    }
                    continue;
                }
                default: break;
            }
        }

        auto processDate = [&](std::time_t time) -> std::string {
            util::DateTime::DateFormat style = util::DateTime::DateFormat::Short;
            if (components.size() > 1) {
                int i = std::stoi(components[1]);
                if (i >= 0 && i <= 2) {
                    style = util::DateTime::DateFormat(i);
                } else {
                    MUSX_INTEGRITY_ERROR("Enigma string encounted tag " + components[0] + " with input value " + components[1]);
                }
            }
            return util::DateTime::formatDate(time, style);
        };

        // Built-in substitutions are collected here and emitted as the insert's own chunk below.
        std::optional<std::string> builtin;
        if (options.ignoreTags.find(components[0]) != options.ignoreTags.end()) {
            continue;
        } else if (components[0] == "arranger") {
            if (auto textInsert = document->getTexts()->get<texts::FileInfoText>(Cmper(texts::FileInfoText::TextType::Arranger))) {
                builtin = trimTags(textInsert->text);
            }
        } else if (components[0] == "composer") {
            if (auto textInsert = document->getTexts()->get<texts::FileInfoText>(Cmper(texts::FileInfoText::TextType::Composer))) {
                builtin = trimTags(textInsert->text);
            }
        } else if (components[0] == "copyright") {
            if (auto textInsert = document->getTexts()->get<texts::FileInfoText>(Cmper(texts::FileInfoText::TextType::Copyright))) {
                builtin = trimTags(textInsert->text);
            }
        } else if (components[0] == "cprsym") {
            builtin = "@";
        } else if (components[0] == "date") {
            builtin = processDate(std::time(nullptr));
        } else if (components[0] == "description") {
            if (auto textInsert = document->getTexts()->get<texts::FileInfoText>(Cmper(texts::FileInfoText::TextType::Description))) {
                builtin = trimTags(textInsert->text);
            }
        } else if (components[0] == "fdate") {
            const auto& modified = document->getHeader()->modified;
            std::time_t fdate = util::DateTime::makeTimeT(modified.year, modified.month, modified.day);
            builtin = processDate(fdate);
        } else if (components[0] == "lyricist") {
            if (auto textInsert = document->getTexts()->get<texts::FileInfoText>(Cmper(texts::FileInfoText::TextType::Lyricist))) {
                builtin = trimTags(textInsert->text);
            }
        } else if (components[0] == "perftime") {
            if (auto scoreDuration = document->getScoreDurationSeconds(); scoreDuration.has_value()) {
                int format = 4;
                if (components.size() > 1) {
                    format = std::stoi(components[1]);
                }
                builtin = formatPerfTime(scoreDuration.value(), format);
            }
        } else if (components[0] == "page") {
            builtin = "#";
        } else if (components[0] == "partname") {
            if (auto linkedPart = document->getOthers()->get<others::PartDefinition>(SCORE_PARTID, forPartId)) {
                if (auto nameRawText = linkedPart->getNameRawTextCtx().getRawText()) {
                    EnigmaParsingOptions partnameOptions = options;
                    partnameOptions.ignoreStyleTags = true;
                    partnameOptions.ignoreTags = { "partname" };
                    if (!processChunk(currentStyles)) {
                        break;
                    }
                    bool parseResult = parseEnigmaTextImpl(document, forPartId, nameRawText->text, onText, onInsert, partnameOptions, nullptr, currentStyles, &insert);
                    if (!parseResult) {
                        return false;
                    }
                } else {
                    AccidentalStyle acciStyle = AccidentalStyle::Ascii;
                    if (options.insertHandling == AccidentalInsertHandling::Substitute) {
                        acciStyle = options.substitutionStyle;
                    }
                    builtin = linkedPart->getName(acciStyle);
                }
            }
        } else if (components[0] == "subtitle") {
            if (auto textInsert = document->getTexts()->get<texts::FileInfoText>(Cmper(texts::FileInfoText::TextType::Subtitle))) {
                builtin = trimTags(textInsert->text);
            }
        } else if (components[0] == "time") {
            bool includeSeconds = false;
            if (components.size() > 1) {
                includeSeconds = bool(std::stoi(components[1]));
            }
            builtin = util::DateTime::formatTime(std::time(nullptr), includeSeconds);
        } else if (components[0] == "title") {
            if (auto textInsert = document->getTexts()->get<texts::FileInfoText>(Cmper(texts::FileInfoText::TextType::Title))) {
                builtin = trimTags(textInsert->text);
            }
        } else if (components[0] == "totpages") {
            auto pages = document->getOthers()->getArray<others::Page>(forPartId);
            builtin = std::to_string(pages.size());
        } else if (components[0] == "url") {
            continue;
        } else {
            // fall-thru causes unhandled commands to be stripped or inserted, based on configuration options
            if (!options.stripUnknownTags) {
                addToBuf(fullCommand);
            }
        }
        if (builtin && !emitInsert(insert, builtin.value())) {
            return false;
        }
    }

    if (!parsedResultIsEmpty && parsingContext && !parsingContext->affixIsPrefix && !parsingContext->affixText.empty()) {
        addToBuf(parsingContext->affixText);
    }

    // Emit any remaining buffered text
    if (textBuffer.has_value()) {
        onText(textBuffer.value(), currentStyles, enclosingInsert);
    }

    return true;
}

std::string EnigmaString::trimTags(const std::string& input)
{
    std::string output;

    // Define the regex for Enigma tags: ^cmd(...)
    std::regex enigmaTagRegex(R"(\^[^(]+\([^)]*\))");
    std::string::size_type pos = 0;

    while (pos < input.size()) {
        if (input[pos] == '^') {
            if (pos + 1 < input.size() && input[pos + 1] == '^') {
                // Handle escaped caret: add a single caret to output
                output += '^';
                pos += 2; // Skip both carets
            } else {
                // Check for an Enigma tag
                std::smatch match;
                if (std::regex_search(input.begin() + pos, input.end(), match, enigmaTagRegex) &&
                    match.position() == 0) {
                    // Skip the matched Enigma tag
                    pos += match.length();
                } else {
                    // It's a lone caret or an invalid tag; add it to output
                    output += '^';
                    ++pos;
                }
            }
        } else {
            // Regular character; add it to output
            output += input[pos++];
        }
    }
    return output;
}

std::string EnigmaString::plainTextFromChunks(const std::vector<EnigmaTextChunk>& chunks)
{
    std::string result;
    for (const auto& chunk : chunks) {
        result += chunk.text;
    }
    return result;
}

bool EnigmaParsingContext::parseEnigmaText(const util::EnigmaString::TextChunkWithInsertCallback& onText, const util::EnigmaString::TextInsertCallback& onInsert,
    const util::EnigmaString::EnigmaParsingOptions& options) const
{
    if (!m_rawText) {
        return false;
    }
    return util::EnigmaString::parseEnigmaText(m_rawText->getDocument(), m_forPartId, m_rawText->text, onText, [&](const std::vector<std::string>& components) -> std::optional<std::string> {
        if (auto result = onInsert(components)) {
            return result;
        }
        if (auto result = m_insertFunc(components)) {
            return result;
        }
        if (m_forPageNumber.has_value()) {
            if (components[0] == "page") {
                int pageOffset = components.size() > 1 ? std::stoi(components[1]) : 0;
                return std::to_string(pageOffset + m_forPageNumber.value());
            }
        }
        return std::nullopt;
    }, options, this);
}

std::vector<EnigmaTextChunk> EnigmaParsingContext::collectEnigmaTextChunks(
    const util::EnigmaString::TextInsertCallback& onInsert,
    const util::EnigmaString::EnigmaParsingOptions& options) const
{
    std::vector<EnigmaTextChunk> result;
    parseEnigmaText([&](const std::string& text, const musx::util::EnigmaStyles& styles, const EnigmaTextInsert* insert) -> bool {
        result.push_back({ text, styles.createDeepCopy(), insert ? std::optional(*insert) : std::nullopt });
        return true;
    }, onInsert, options);
    return result;
}

std::vector<EnigmaTextChunk> EnigmaParsingContext::collectEnigmaTextChunks(
    const util::EnigmaString::EnigmaParsingOptions& options) const
{
    return collectEnigmaTextChunks(util::EnigmaString::defaultInsertsCallback, options);
}

std::string EnigmaParsingContext::getText(bool trimTags, util::EnigmaString::AccidentalStyle accidentalStyle,
    const std::unordered_set<std::string_view>& ignoreTags) const
{
    util::EnigmaString::EnigmaParsingOptions options(accidentalStyle);
    options.stripUnknownTags = trimTags;
    options.ignoreTags = ignoreTags;
    std::string result;
    parseEnigmaText([&](const std::string& text, const musx::util::EnigmaStyles&) -> bool {
            result += text;
            return true;
        }, options);
    return result;
}

MusxInstance<FontInfo> EnigmaParsingContext::parseFirstFontInfo() const
{
    if (!m_rawText || !musx::util::EnigmaString::startsWithFontCommand(m_rawText->text)) {
        return nullptr;
    }
    std::shared_ptr<FontInfo> result;
    util::EnigmaString::parseEnigmaText(m_rawText->getDocument(), m_forPartId, m_rawText->text, [&](const std::string&, const util::EnigmaStyles& styles) {
        result = styles.font;
        return false;
    });
    return result;
}

} // namespace util
} // namespace musx
