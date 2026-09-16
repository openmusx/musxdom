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
#include <string>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <sstream>
#include <functional>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

 // This header includes method implementations that need to see all the classes in the dom

#include "musx/musx.h"

#if ! defined(MUSX_RUNNING_ON_WINDOWS) && ! defined(MUSX_RUNNING_ON_WASM)
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace musx {
namespace dom {

namespace {

struct SmuflMetadataPathCacheEntry
{
    bool found{};
    std::filesystem::path path;
};

static bool looksLikeSmuflDefaultMusicFont(const FontInfo& fontInfo)
{
    const auto document = fontInfo.getDocument();
    const auto fontOptions = document->getOptions()->get<options::FontOptions>();
    const auto musicSymbols = document->getOptions()->get<options::MusicSymbolOptions>();
    if (!fontOptions || !musicSymbols) {
        return false;
    }

    const auto defaultMusicFontIt = fontOptions->fontOptions.find(options::FontOptions::FontType::Music);
    if (defaultMusicFontIt == fontOptions->fontOptions.end() || !defaultMusicFontIt->second
        || !fontInfo.calcIsSameTypeface(*defaultMusicFontIt->second)) {
        return false;
    }

    const auto categoryUsesCandidateFont = [&](options::FontOptions::FontType type) {
        const auto it = fontOptions->fontOptions.find(type);
        return it != fontOptions->fontOptions.end() && it->second && fontInfo.calcIsSameTypeface(*it->second);
    };

    size_t matches{};
    if (categoryUsesCandidateFont(options::FontOptions::FontType::Noteheads)
        && musicSymbols->noteheadQuarter == smufl_glyph::noteheadBlack) {
        ++matches;
    }
    if (categoryUsesCandidateFont(options::FontOptions::FontType::Accis)
        && musicSymbols->natural == smufl_glyph::accidentalNatural) {
        ++matches;
    }
    if (categoryUsesCandidateFont(options::FontOptions::FontType::Rests)
        && musicSymbols->restQuarter == smufl_glyph::restQuarter) {
        ++matches;
    }
    return matches >= 2;
}

} // namespace

std::string normalizeFontName(std::string_view name)
{
    std::string result;
    result.reserve(name.size());
    for (unsigned char ch : name) {
        if (ch < 0x80) {
            if (std::isspace(ch)) {
                continue;
            }
            ch = static_cast<unsigned char>(std::tolower(ch));
        }
        result.push_back(static_cast<char>(ch));
    }
    return result;
}

// ********************
// ***** FontInfo *****
// ********************

namespace {

/// @brief Copies every stored field of a font definition onto a fresh instance in @p target.
MusxInstance<others::FontDefinition> cloneFontDefinition(const DocumentPtr& target,
    const MusxInstance<others::FontDefinition>& source, Cmper cmper,
    const ImportObjectCallback& onImported)
{
    // SCORE_PARTID and ShareMode::All are correct for anything newly created: no linked part can
    // reference an object that did not exist a moment ago, so there is nothing to unshare from.
    // An importer for a ShareMode::None class would know to say so.
    auto clone = std::make_shared<others::FontDefinition>(
        target, SCORE_PARTID, EnigmaBase::ShareMode::All, cmper);
    *clone = *source;
    target->getOthers()->add(others::FontDefinition::XmlNodeName, clone);
    if (onImported) {
        onImported(*clone);
    }
    return clone;
}

/// @brief The non-zero cmper in @p target naming @p source's typeface, cloning when absent.
std::optional<Cmper> importNonZeroFontDefinition(const DocumentPtr& target,
    const MusxInstance<others::FontDefinition>& source,
    const ImportObjectCallback& onImported)
{
    // Match on typeface, never on cmper. Cmper 0 is excluded: its name follows whatever the
    // document's default music font currently is, so matching a concrete typeface onto it would
    // make that element track later changes to the music font.
    //
    // At most one non-zero definition should name a given typeface; more than one means the
    // document is malformed, and which of them is picked then does not matter.
    const auto wanted = normalizeFontName(source->name);
    for (const auto& font : target->getOthers()->getArray<others::FontDefinition>(SCORE_PARTID)) {
        const auto cmper = font->getCmper();
        if (cmper != 0 && normalizeFontName(font->name) == wanted) {
            return cmper;
        }
    }
    const auto available = target->getOthers()->nextFreeCmper<others::FontDefinition>(SCORE_PARTID);
    if (!available) {
        return std::nullopt;
    }
    return cloneFontDefinition(target, source, *available, onImported)->getCmper();
}

/// @brief The definition naming @p target's own default music font, when it can be determined.
MusxInstance<others::FontDefinition> findDefaultMusicFont(const DocumentPtr& target)
{
    const auto music = options::FontOptions::getFontInfoOrNull(
        target, options::FontOptions::FontType::Music);
    // A music font stored as 0 says nothing here: it is the very reference being resolved.
    if (!music || music->fontId == 0) {
        return nullptr;
    }
    auto result = target->getOthers()->get<others::FontDefinition>(SCORE_PARTID, music->fontId);
    if (!result) {
        return nullptr;
    }
    const auto placeholderName = "Missing Font (" + std::to_string(music->fontId) + ")";
    return result->name == placeholderName ? nullptr : result;
}

} // namespace

std::optional<Cmper> importFontDefinitionInto(const DocumentPtr& target,
    const MusxInstance<others::FontDefinition>& source,
    const ImportObjectCallback& onImported)
{
    MUSX_ASSERT_IF(!target) {
        throw std::invalid_argument("importFontDefinitionInto received a null target document");
    }
    MUSX_ASSERT_IF(!source) {
        throw std::invalid_argument("importFontDefinitionInto received a null font definition");
    }

    if (source->getCmper() != 0) {
        return importNonZeroFontDefinition(target, source, onImported);
    }

    // Id 0 names the destination's own default music font rather than a typeface, so it is passed
    // through. The definition is still created when missing, because the returned id has to
    // resolve: a reference naming nothing is the defect this function exists to avoid.
    const auto existing = target->getOthers()->getArray<others::FontDefinition>(SCORE_PARTID);
    const auto hasZero = std::any_of(existing.begin(), existing.end(),
        [](const MusxInstance<others::FontDefinition>& font) { return font->getCmper() == 0; });
    if (!hasZero) {
        if (const auto music = findDefaultMusicFont(target)) {
            // The target's own FontOptions already say which typeface id 0 stands for, and that
            // answer outranks the source's.
            cloneFontDefinition(target, music, 0, onImported);
        } else {
            cloneFontDefinition(target, source, 0, onImported);
            // The same typeface must also be reachable by a concrete cmper. Id 0 tracks whatever
            // the music font later becomes, and matching never selects it, so without this the
            // typeface would be unaddressable as itself. Failing that leaves 0 resolvable, which
            // is what was asked for, so the result still stands.
            static_cast<void>(importNonZeroFontDefinition(target, source, onImported));
        }
    }
    // Cmper rather than a bare 0: constructing the optional from an int narrows, which MSVC
    // reports under /W4 and the build treats as an error.
    return Cmper(0);
}

std::string FontInfo::getName() const
{
    if (auto fontDef = getDocument()->getOthers()->get<others::FontDefinition>(SCORE_PARTID, fontId)) {
        return fontDef->name;
    }
    throw std::invalid_argument("font definition not found for font id " + std::to_string(fontId));
}

void FontInfo::setFontIdByName(const std::string& name)
{
    auto fontDefs = getDocument()->getOthers()->getArray<others::FontDefinition>(SCORE_PARTID);
    for (auto fontDef : fontDefs) {
        if (fontDef->name == name) {
            fontId = fontDef->getCmper();
            return;
        }
    }
    throw std::invalid_argument("font definition not found for font \"" + name + "\"");
}

bool FontInfo::calcIsSameTypeface(const FontInfo& src) const
{
    if (fontId == src.fontId) {
        return true;
    }
    // The default music font is the only font duplicated by design, so it is the only case where
    // differing ids can still name the same typeface.
    if (!calcIsDefaultMusic() && !src.calcIsDefaultMusic()) {
        return false;
    }
    return getName() == src.getName();
}

bool FontInfo::isSame(const FontInfo& src) const
{
    return calcIsSameTypeface(src) && fontSize == src.fontSize && m_sizeIsPercent == src.m_sizeIsPercent
        && getEnigmaStyles() == src.getEnigmaStyles();
}

std::optional<std::filesystem::path> FontInfo::calcSMuFLMetaDataPath(const std::string& fontName)
{
    static std::unordered_map<std::string, SmuflMetadataPathCacheEntry> cache;
    if (const auto it = cache.find(fontName); it != cache.end()) {
        return it->second.found ? std::make_optional(it->second.path) : std::nullopt;
    }

    auto standardFontPaths = calcSMuFLPaths();
    for (const auto& path : standardFontPaths) {
        if (!path.empty()) {
            std::filesystem::path metaFilePath(path / fontName / fontName);
            metaFilePath.replace_extension(".json");
            if (std::filesystem::is_regular_file(metaFilePath)) {
                cache.emplace(fontName, SmuflMetadataPathCacheEntry{ true, metaFilePath });
                return metaFilePath;
            }
        }
    }
    cache.emplace(fontName, SmuflMetadataPathCacheEntry{});
    return std::nullopt;
}

ResolvedFontInfo FontInfo::resolve() const
{
    ResolvedFontInfo result;
    result.name = getName();
    result.size = fontSize;
    result.sizeIsPercent = getSizeIsPercent();
    result.bold = bold;
    result.italic = italic;
    result.underline = underline;
    result.strikeout = strikeout;
    result.absolute = absolute;
    result.hidden = hidden;
    result.isSymbolFont = calcIsSymbolFont();
    result.isSmufl = calcIsSMuFL();
    return result;
}

bool FontInfo::calcIsSMuFL() const
{
    const auto document = getDocument();
    const auto cached = document->getCachedFontIsSMuFL(fontId);
    if (cached.has_value()) {
        return cached.value();
    }

    // Mirrors the SMuFL fonts configured in source_json/smufl_fonts.json of the smufl-mapping
    // project (https://github.com/openmusx/smufl-mapping). That project is deliberately not a
    // dependency of musxdom, so this list is maintained by hand and may need updating when
    // smufl-mapping adds fonts. Entries are stored pre-normalized (see normalizeFontName), i.e.
    // the family name lowercased with whitespace removed: add new fonts in that form. The comment
    // on each line is the canonical family spelling, to make re-syncing with smufl_fonts.json easier.
    static const std::unordered_set<std::string_view> knownSmuflFontNames =
    {
        "bravura",                   // Bravura
        "bravuratext",               // Bravura Text
        "chaconneex",                // Chaconne Ex
        "emmentaler",                // Emmentaler
        "emmentalertext",            // Emmentaler Text
        "finaleash",                 // Finale Ash
        "finaleashtext",             // Finale Ash Text
        "finalebroadway",            // Finale Broadway
        "finalebroadwaylegacytext",  // Finale Broadway Legacy Text
        "finalebroadwaytext",        // Finale Broadway Text
        "finaleengraver",            // Finale Engraver
        "finalejazz",                // Finale Jazz
        "finalejazztext",            // Finale Jazz Text
        "finalejazztextlowercase",   // Finale Jazz Text Lowercase
        "finalelegacy",              // Finale Legacy
        "finalemaestro",             // Finale Maestro
        "finalemaestrotext",         // Finale Maestro Text
        "gonville",                  // Gonville
        "gonvilletext",              // Gonville Text
        "leland",                    // Leland
        "lelandtext",                // Leland Text
        "musejazz",                  // MuseJazz
        "musejazztext",              // MuseJazz Text
        "petaluma",                  // Petaluma
        "petalumascript",            // Petaluma Script
        "petalumatext",              // Petaluma Text
        "sebastian",                 // Sebastian
        "sebastiantext",             // Sebastian Text
    };

    bool result;
    if (calcSMuFLMetaDataPath().has_value()) {
        result = true;
    } else if (knownSmuflFontNames.count(normalizeFontName(getName())) > 0) {
        result = true;
    } else {
        result = looksLikeSmuflDefaultMusicFont(*this);
    }

    document->setCachedFontIsSMuFL(fontId, result);
    return result;
}

bool FontInfo::calcIsSymbolFont() const
{
    if (auto fontDef = getDocument()->getOthers()->get<others::FontDefinition>(SCORE_PARTID, fontId)) {
        return fontDef->calcIsSymbolFont();
    }
    throw std::invalid_argument("font definition not found for font id " + std::to_string(fontId));
}

std::vector<std::filesystem::path> FontInfo::calcSMuFLPaths()
{
    if (const auto& testPath = util::TestConfiguration::getTestDataPath()) {
        return { std::filesystem::path(testPath.value()) / "font_metadata" };
    }
#if defined(MUSX_RUNNING_ON_WASM)
    // No filesystem or user-profile access in a wasm sandbox: skip probing entirely and
    // rely on FontInfo::calcIsSMuFL()'s hardcoded knownSmuflFontNames fallback.
    return {};
#else
#if defined(MUSX_RUNNING_ON_WINDOWS)
    auto systemEnv = "COMMONPROGRAMFILES";
    auto userEnv = "LOCALAPPDATA";
#elif defined(MUSX_RUNNING_ON_MACOS)
    auto systemEnv = "";
    auto userEnv = "HOME";
#elif defined(MUSX_RUNNING_ON_LINUX_UNIX)
    auto systemEnv = "XDG_DATA_DIRS";
    auto userEnv = "XDG_DATA_HOME";
#else
    static_assert(false, "Unsupported OS for FontInfo::calcSMuFLPaths");
#endif

#if ! defined(MUSX_RUNNING_ON_WINDOWS)
    auto getHomePath = []() -> std::string {
        auto homeEnv = getenv("HOME");
        if (homeEnv) {
            return homeEnv;
        }
        uid_t uid = getuid(); // Get the current user's UID
        struct passwd* pw = getpwuid(uid); // Fetch the password entry for the UID
        if (pw) {
            return pw->pw_dir;
        }
        return "";
        };
#else
    auto getHomePath = []() -> void {};
#endif

    auto getBasePaths = [getHomePath](const std::string& envVariable) -> std::vector<std::string> {
        std::vector<std::string> paths;
#if defined(MUSX_RUNNING_ON_WINDOWS)
        char* buffer = nullptr;
        size_t bufferSize = 0;
        if (_dupenv_s(&buffer, &bufferSize, envVariable.c_str()) == 0 && buffer != nullptr) {
            paths.emplace_back(buffer);
            free(buffer);
        }
        else {
            return {};
        }
#else
        if (envVariable == "HOME") {
            paths.emplace_back(getHomePath());
        } else if (!envVariable.empty()) {
            if (auto envValue = getenv(envVariable.c_str())) {
                std::stringstream ss(envValue);
                std::string path;
                while (std::getline(ss, path, ':')) {
                    paths.push_back(path);
                }
#if defined(MUSX_RUNNING_ON_LINUX_UNIX)
            }
            else if (envVariable == "XDG_DATA_HOME") {
                paths.emplace_back(getHomePath() + "/.local/share");
            }
            else if (envVariable == "XDG_DATA_DIRS") {
                paths.emplace_back("/usr/local/share");
                paths.emplace_back("/usr/share");
#endif
            } else {
                return {};
            }
        } else {
            paths.emplace_back("/");
        }
#endif
        return paths;
        };
    auto paths = getBasePaths(userEnv);
    auto temp = getBasePaths(systemEnv);
    paths.insert(paths.end(),
        std::make_move_iterator(temp.begin()),
        std::make_move_iterator(temp.end()));
    std::vector<std::filesystem::path> retval;
    for (const auto& next : paths) {
        std::filesystem::path path = next;
#if defined(MUSX_RUNNING_ON_MACOS)
        path = path / "Library" / "Application Support";
#endif
        path = path / "SMuFL" / "Fonts";
        retval.emplace_back(std::move(path));
    }
    return retval;
#endif // MUSX_RUNNING_ON_WASM
}

// ************************
// ***** KeySignature *****
// ************************

std::vector<unsigned> KeySignature::calcTonalCenterArrayForSharps() const
{
    if (isMinor()) {
        return { 5, 2, 6, 3, 0, 4, 1, 5 };
    }
    if (!isBuiltIn()) {
        if (auto centers = getDocument()->getOthers()->get<others::TonalCenterSharps>(SCORE_PARTID, getKeyMode())) {
            return centers->values;
        }
    }
    // Major or default
    return { 0, 4, 1, 5, 2, 6, 3, 0 };
}

std::vector<unsigned> KeySignature::calcTonalCenterArrayForFlats() const
{
    if (isMinor()) {
        return { 5, 1, 4, 0, 3, 6, 2, 5 };
    }
    if (!isBuiltIn()) {
        if (auto centers = getDocument()->getOthers()->get<others::TonalCenterFlats>(SCORE_PARTID, getKeyMode())) {
            return centers->values;
        }
    }
    // Major or default
    return { 0, 3, 6, 2, 5, 1, 4, 0 };
}

std::vector<unsigned> KeySignature::calcTonalCenterArray(KeyContext ctx) const
{
    int alter = getAlteration(ctx);
    if (alter >= 0) {
        return calcTonalCenterArrayForSharps();
    } else {
        return calcTonalCenterArrayForFlats();
    }
}

std::vector<int> KeySignature::calcAcciAmountsArray(KeyContext ctx) const
{
    int alter = getAlteration(ctx);

    if (!isBuiltIn()) {
        if (alter >= 0) {
            if (auto amounts = getDocument()->getOthers()->get<others::AcciAmountSharps>(SCORE_PARTID, getKeyMode())) {
                return amounts->values;
            }
        } else {
            if (auto amounts = getDocument()->getOthers()->get<others::AcciAmountFlats>(SCORE_PARTID, getKeyMode())) {
                return amounts->values;
            }
        }
    }

    // Major/minor or default
    if (alter >= 0) {
        return std::vector<int>(music_theory::STANDARD_DIATONIC_STEPS, 1);
    } else {
        return std::vector<int>(music_theory::STANDARD_DIATONIC_STEPS, -1);
    }
}

std::vector<unsigned> KeySignature::calcAcciOrderArray(KeyContext ctx) const
{
    int alter = getAlteration(ctx);

    if (!isBuiltIn()) {
        if (alter >= 0) {
            if (auto order = getDocument()->getOthers()->get<others::AcciOrderSharps>(SCORE_PARTID, getKeyMode())) {
                return order->values;
            }
        } else {
            if (auto order = getDocument()->getOthers()->get<others::AcciOrderFlats>(SCORE_PARTID, getKeyMode())) {
                return order->values;
            }
        }
    }

    // Major/minor or default
    if (alter >= 0) {
        return { 3, 0, 4, 1, 5, 2, 6 };
    } else {
        return { 6, 2, 5, 1, 4, 0, 3 };
    }
}

int KeySignature::calcTonalCenterIndex(KeyContext ctx) const
{
    if (!isNonLinear() && !isLinear()) {
        MUSX_INTEGRITY_ERROR("Key signature mode " + std::to_string(getKeyMode()) + " is neither linear nor non-linear. It is invalid.");
    }

    int alter = getAlteration(ctx);
    auto centers = calcTonalCenterArray(ctx);
    return int(centers[std::abs(alter) % centers.size()]);
}

int KeySignature::calcAlterationOnNote(unsigned noteIndex, KeyContext ctx) const
{
    if (!isNonLinear() && !isLinear()) {
        MUSX_INTEGRITY_ERROR("Key signature mode " + std::to_string(getKeyMode()) + " is neither linear nor non-linear. It is invalid.");
    }

    auto amounts = calcAcciAmountsArray(ctx);
    auto order = calcAcciOrderArray(ctx);

    int keySigAlteration = 0;

    if (isNonLinear()) {
        for (size_t i = 0; i < amounts.size() && i < order.size(); i++) {
            if (amounts[i] == 0) {
                break;
            }
            if (noteIndex == order[i]) {
                keySigAlteration += amounts[i];
            }
        }
    } else {
        unsigned keyFifths = std::abs(getAlteration(ctx));
        for (size_t i = 0; i < keyFifths && i < amounts.size() && i < order.size(); ++i) {
            if (noteIndex == order[i % order.size()]) {
                keySigAlteration += amounts[i];
            }
        }
    }

    return keySigAlteration;
}

music_theory::Pitch KeySignature::calcPitch(int displacement, int alteration, KeyContext ctx) const
{
    const int keyAdjustedLev = calcTonalCenterIndex(ctx) + displacement
                             + (getOctaveDisplacement(ctx) * music_theory::STANDARD_DIATONIC_STEPS);
    int octave = (keyAdjustedLev / music_theory::STANDARD_DIATONIC_STEPS) + 4;
    int step = keyAdjustedLev % music_theory::STANDARD_DIATONIC_STEPS;
    if (step < 0) {
        step += music_theory::STANDARD_DIATONIC_STEPS;
        --octave;
    }

    return { music_theory::noteNames[step], octave, alteration + calcAlterationOnNote(step, ctx) };
}

music_theory::Pitch KeySignature::calcPitchFromStaffPosition(int staffPosition, ClefIndex clefIndex, KeyContext ctx,
    std::optional<int> actualAlteration) const
{
    const auto& clefOptions = getDocument()->getOptions()->get<options::ClefOptions>();
    if (!clefOptions) {
        throw std::invalid_argument("Document contains no clef options!");
    }
    const int middleCPosition = clefOptions->getClefDef(clefIndex)->middleCPos;
    const int displacement = staffPosition - middleCPosition - calcTonalCenterIndex(ctx)
                           - (getOctaveDisplacement(ctx) * music_theory::STANDARD_DIATONIC_STEPS);
    auto result = calcPitch(displacement, 0, ctx);
    if (actualAlteration) {
        result.alteration = *actualAlteration;
    }
    return result;
}

int KeySignature::calcScaleDegree(int displacement) const
{
    const int diatonicSteps = [&]() {
        if (const auto keyMap = calcKeyMap()) {
            return int(keyMap.value().size());
        }
        return music_theory::STANDARD_DIATONIC_STEPS;
    }();
    return music_theory::positiveModulus(displacement, diatonicSteps);
}

void KeySignature::setTransposition(int interval, int keyAdjustment, bool simplify)
{
    if (!isLinear()) {
        return;
    }
    m_octaveDisplacement = interval / music_theory::STANDARD_DIATONIC_STEPS;
    m_alterationOffset = 0; // suppresses transposed tone center and alteration calc
    int concertAlteration = getAlteration(KeyContext::Concert);
    int concertTonalCenterIndex = calcTonalCenterIndex(KeyContext::Concert);
    int tonalCenterOffset = interval % music_theory::STANDARD_DIATONIC_STEPS;

    int alteration = concertAlteration + keyAdjustment;
    if (simplify && keyAdjustment) {
        // Finale does not simplify microtonal key sigs correctly.
        // This simplification *does* work correctly with microtonal key sigs, provided the custom key sig
        // is set up to increment each key signature accidental by the number of EDO divisions
        // in a chromatic half-step.
        int direction = music_theory::sign(alteration);
        while (std::abs(alteration) >= music_theory::STANDARD_DIATONIC_STEPS) {
            alteration -= direction * music_theory::STANDARD_12EDO_STEPS;
            tonalCenterOffset += direction;
        }
    }
    m_alterationOffset = alteration - concertAlteration;
    m_octaveDisplacement += (concertTonalCenterIndex + tonalCenterOffset) / music_theory::STANDARD_DIATONIC_STEPS;
}

void KeySignature::setTransposition(const MusxInstance<others::Staff>& staff)
{
    if (staff && staff->transposition && staff->transposition->keysig) {
        const auto& keysig = *staff->transposition->keysig;
        setTransposition(keysig.interval, keysig.adjust, !staff->transposition->noSimplifyKey);
    }
}

std::optional<std::vector<int>> KeySignature::calcKeyMap() const
{
    size_t tonalCenter = static_cast<size_t>(calcTonalCenterArrayForSharps()[0]);
    auto keyMap = getDocument()->getOthers()->get<others::KeyMapArray>(SCORE_PARTID, getKeyMode());
    if (!keyMap || keyMap->steps.empty()) {
        return std::nullopt;
    }
    unsigned numDiatonicSteps = keyMap->countDiatonicSteps();

    // Find the index of the first step whose diatonic step matches the tonal center
    auto centerIt = std::find_if(keyMap->steps.begin(), keyMap->steps.end(), [&](const auto& s) {
        return s->diatonic && s->hlevel == tonalCenter;
    });
    if (centerIt == keyMap->steps.end()) {
        return std::nullopt;
    }
    size_t indexOfTonalCenter = std::distance(keyMap->steps.begin(), centerIt);

    // Initialize result
    std::vector<int> result(numDiatonicSteps, 0);
    int currDiatonicStep = -1;
    const size_t stepCount = keyMap->steps.size();
    for (size_t i = 0; i < stepCount; ++i) {
        size_t wrappedIndex = (indexOfTonalCenter + i) % stepCount;
        const auto& step = keyMap->steps[wrappedIndex];
        if (step->diatonic) {
            ++currDiatonicStep;
        }
        for (size_t diatonicStep = 1; diatonicStep < numDiatonicSteps; ++diatonicStep) {
            if (int(diatonicStep) > currDiatonicStep) {
                ++result[diatonicStep];
            }
        }
    }

    return result;
}

int KeySignature::calcEDODivisions() const
{
    if (auto keyFormat = getDocument()->getOthers()->get<others::KeyFormat>(SCORE_PARTID, getKeyMode())) {
        return static_cast<int>(keyFormat->semitones);
    }
    return music_theory::STANDARD_12EDO_STEPS;
}

std::unique_ptr<music_theory::Transposer> KeySignature::createTransposer(int displacement, int alteration) const
{
    return std::make_unique<music_theory::Transposer>(displacement, alteration, isMinor(), calcEDODivisions(), calcKeyMap());
}

std::optional<music_theory::DiatonicMode> KeySignature::calcDiatonicMode() const
{
    if (isLinear()) {
        const auto centers = calcTonalCenterArrayForSharps();
        const unsigned index = centers[0];
        if (index < music_theory::STANDARD_DIATONIC_STEPS) {
            return static_cast<music_theory::DiatonicMode>(index);
        } else {
            MUSX_INTEGRITY_ERROR("KeyMode " + std::to_string(getKeyMode()) + " returned invalid tonal center at index 0: " + std::to_string(index));
        }
    }
    return std::nullopt;
}

// **********************
// ***** MusicRange *****
// **********************

std::optional<MusicPoint> MusicRange::nextLocation(const std::optional<StaffCmper>& forStaff) const
{
    std::optional<MusicPoint> result;
    const Edu endEdu = end.position.calcEduDuration();
    if (auto currMeasure = getDocument()->getOthers()->get<others::Measure>(SCORE_PARTID, end.measureId)) {
        MeasCmper nextMeas = end.measureId;
        const Edu maxEdu = currMeasure->calcDuration(forStaff).calcEduDuration() - 1;
        Edu nextEdu = 0;
        if (endEdu < maxEdu) {
            nextEdu = endEdu + 1;
        } else {
            nextMeas++;
            if (!getDocument()->getOthers()->get<others::Measure>(SCORE_PARTID, nextMeas)) {
                return std::nullopt;
            }
        }
        result = MusicPoint(nextMeas, util::Fraction::fromEdu(nextEdu));
    } else {
        auto measures = getDocument()->getOthers()->getArray<others::Measure>(SCORE_PARTID);
        // don't even report a range whose end measure is 1 past the end. This is common non-inclusive range logic.
        if (end.measureId > MeasCmper(measures.size() + 1)) {
            // music ranges are explicitly allowed to be beyond the end of a document, so treat this as a verbose message.
            util::Logger::log(util::Logger::LogLevel::Verbose, "MusicRange has invalid end measure " + std::to_string(end.measureId));
        }
    }
    return result;
}

bool MusicRange::contains(const EntryInfoPtr& entryInfo) const
{
    return contains(entryInfo.getMeasure(), entryInfo->elapsedDuration);
}

// *************************
// ***** TimeSignature *****
// *************************

TimeSignature::TimeSignature(const DocumentWeakPtr& document, int beats, Edu unit, bool hasCompositeTop, bool hasCompositeBottom, Abbreviation abbreviate)
    : DocumentElementNoPart(document), m_abbreviation(abbreviate)
{
    auto tops = [&]() -> std::vector<std::vector<util::Fraction>> {
        if (hasCompositeTop) {
            if (auto comps = getDocument()->getOthers()->get<others::TimeCompositeUpper>(SCORE_PARTID, Cmper(beats))) {
                std::vector<std::vector<util::Fraction>> result;
                for (const auto& nextItem : comps->items) {
                    if (nextItem->startGroup || result.empty()) {
                        result.push_back({});
                    }
                    result[result.size() - 1].push_back(nextItem->fullFraction());
                }
                return result;
            } else {
                return {};
            }
        }
        return { {beats} };
    }();
    auto bots = [&]() -> std::vector<std::vector<Edu>> {
        if (hasCompositeBottom) {
            if (auto comps = getDocument()->getOthers()->get<others::TimeCompositeLower>(SCORE_PARTID, Cmper(unit))) {
                std::vector<std::vector<Edu>>result;
                for (const auto& nextItem : comps->items) {
                    if (nextItem->startGroup || result.empty()) {
                        result.push_back({});
                    }
                    result[result.size() - 1].push_back(nextItem->unit);
                }
                return result;
            } else {
                return {};
            }
        }
        return { {unit} };
    }();

    if (tops.empty() || bots.empty()) {
        throw std::invalid_argument("Time signature is missing composite top array or composite bottom array.");
    }
    if (tops.size() != bots.size()) {
        MUSX_INTEGRITY_ERROR("Composite top group for time signature does not match composite bottom group.");
    }
    for (size_t x = 0; x < (std::min)(tops.size(), bots.size()); x++) {
        components.push_back({ tops[x], bots[x] });
    }
}

std::optional<char32_t> TimeSignature::getAbbreviatedSymbol() const
{
    auto musicChars = getDocument()->getOptions()->get<options::MusicSymbolOptions>();
    const char32_t commonTimeSymbol = musicChars ? musicChars->timeSigAbrvCommon : smufl_glyph::timeSigCommon;
    const char32_t cutTimeSymbol = musicChars ? musicChars->timeSigAbrvCut : smufl_glyph::timeSigCutCommon;
    switch (m_abbreviation) {
        case Abbreviation::Abbreviated:
            if (isCutTime()) {
                return cutTimeSymbol;
            } else if (isCommonTime()) {
                return commonTimeSymbol;
            }
            return std::nullopt;
        case Abbreviation::Numeric:
            return std::nullopt;
        case Abbreviation::NotApplicable:
            break;
    }
    if (auto options = getDocument()->getOptions()->get<options::TimeSignatureOptions>()) {
        if (options->timeSigDoAbrvCut && isCutTime()) {
            return cutTimeSymbol;
        } else if (options->timeSigDoAbrvCommon && isCommonTime()) {
            return commonTimeSymbol;
        }
    }
    return std::nullopt;
}

bool TimeSignature::isSame(const TimeSignature& src) const
{
    if (components != src.components) {
        return false;
    }
    if (isCutTime() || isCommonTime()) {
        return getAbbreviatedSymbol() == src.getAbbreviatedSymbol();
    }
    return true;
}

bool TimeSignature::isCommonTime() const
{
    if (components.size() != 1 || components[0].counts.size() != 1 || components[0].units.size() != 1) {
        return false;
    }
    return components[0].counts[0] == 4 && components[0].units[0] == Edu(NoteType::Quarter);
}

bool TimeSignature::isCutTime() const
{
    if (components.size() != 1 || components[0].counts.size() != 1 || components[0].units.size() != 1) {
        return false;
    }
    return components[0].counts[0] == 2 && components[0].units[0] == Edu(NoteType::Half);
}

std::pair<util::Fraction, Edu> TimeSignature::TimeSigComponent::normalizeCompoundUnit(util::Fraction count, Edu unit)
{
    if (!unit) {
        throw std::logic_error("The beat size is zero.");
    }

    int power2 = 0;
    Edu otherPrimes = unit;
    while ((otherPrimes & 0x01) == 0) {
        otherPrimes >>= 1;
        power2++;
    }

    return { count * otherPrimes, Edu(1 << power2) };
}

TimeSignature::TimeSigComponent TimeSignature::TimeSigComponent::normalizeCompound() const
{
    if (units.size() != 1) {
        return *this;
    }

    TimeSigComponent result = *this;
    for (auto& count : result.counts) {
        auto [normalizedCount, normalizedUnit] = normalizeCompoundUnit(count, units[0]);
        count = normalizedCount;
        result.units[0] = normalizedUnit;
    }
    return result;
}

std::pair<util::Fraction, NoteType> TimeSignature::calcSimplified() const
{
    // Lambda to compute GCD of a vector
    auto computeGCD = [](const std::vector<Edu>& values) {
        return values.empty() ? 1 : std::reduce(values.begin() + 1, values.end(), values[0], std::gcd<Edu, Edu>);
    };

    std::vector<Edu> allUnits;
    std::vector<std::pair<util::Fraction, Edu>> summedUnits;

    for (const auto& ts : components) {
        Edu totalUnit = ts.sumUnits();
        summedUnits.emplace_back(ts.sumCounts(), totalUnit);
        allUnits.push_back(totalUnit);
    }

    Edu finalUnit = computeGCD(allUnits); // The final unit size
    if (!finalUnit) {
        throw std::logic_error("The beat size is zero.");
    }

    // Compute final beats relative to finalUnit
    auto totalBeats = std::accumulate(summedUnits.begin(), summedUnits.end(), util::Fraction{}, [finalUnit](auto acc, const auto& p) {
        return acc + p.first * (p.second / finalUnit);
    });

    auto [count, unit] = TimeSigComponent::normalizeCompoundUnit(totalBeats, finalUnit);
    return { count, NoteType(unit) };
}

util::Fraction TimeSignature::calcBeatValueAt(Edu eduPosition) const
{
    if (components.empty()) {
        return {};
    }

    const util::Fraction pos = util::Fraction::fromEdu(eduPosition); // fraction of whole note
    util::Fraction cursor{};
    util::Fraction lastBeatValue{};

    for (const TimeSigComponent& comp : components) {
        MUSX_ASSERT_IF(comp.units.empty() || comp.counts.empty()) {
            // detect this in debug mode, but handle without throw in production.
            continue;
        }

        const Edu beatEdu = comp.sumUnits();
        const util::Fraction beatCount = comp.sumCounts();

        MUSX_ASSERT_IF(beatEdu <= 0 || beatCount <= util::Fraction{}) {
            // detect this in debug mode, but handle without throw in production.
            continue;
        }

        lastBeatValue = util::Fraction::fromEdu(beatEdu);
        const util::Fraction compDur = beatCount * lastBeatValue;

        if (pos >= cursor && pos < (cursor + compDur)) {
            return lastBeatValue;
        }

        cursor += compDur;
    }

    return lastBeatValue;
}

// ******************************
// ***** LyricsSyllableInfo *****
// ******************************

namespace {

// The only two characters Finale's own MusicXML exporter treats as elision markers when
// embedded in an otherwise ordinary lyric syllable: U+00A0 (no-break space) and U+203F
// (undertie). Any other character, including a non-breaking hyphen, en dash, or em dash, is
// left embedded as ordinary text.
const std::array<char32_t, 2> kElisionMarkerCodepoints = { char32_t(0x00A0), char32_t(0x203F) };

const std::array<std::string, 2> kElisionMarkerUtf8 = {
    util::EnigmaString::toU8(kElisionMarkerCodepoints[0]),
    util::EnigmaString::toU8(kElisionMarkerCodepoints[1]),
};

struct ElisionMarkerMatch
{
    size_t position;
    size_t markerIndex;
};

/// @brief Finds the first elision marker in @p text at or after @p searchFrom.
/// @return The byte offset and which marker (index into kElisionMarkerCodepoints/kElisionMarkerUtf8)
/// was found, or std::nullopt if none.
std::optional<ElisionMarkerMatch> findNextElisionMarker(const std::string& text, size_t searchFrom)
{
    std::optional<ElisionMarkerMatch> result;
    for (size_t i = 0; i < kElisionMarkerUtf8.size(); ++i) {
        if (const auto pos = text.find(kElisionMarkerUtf8[i], searchFrom); pos != std::string::npos) {
            if (!result || pos < result->position) {
                result = ElisionMarkerMatch{ pos, i };
            }
        }
    }
    return result;
}

} // namespace

bool LyricsSyllableInfo::iterateStylesInRange(size_t rangeStart, size_t rangeEnd, const util::EnigmaString::TextChunkCallback& callback) const
{
    auto owner = m_owner.lock();
    MUSX_ASSERT_IF(!owner) {
        throw std::logic_error("Attempt to iterate styles for a lyric syllable, but its owning LyricsTextBase is no longer allocated.");
    }
    for (const auto& span : m_enigmaStyleMap) {
        MUSX_ASSERT_IF(span.start > syllable.size() || span.end > syllable.size()) {
            throw std::logic_error("syllable's enigmaStyles map contained out-of-range start or end values.");
        }
        MUSX_ASSERT_IF(span.start > span.end) {
            throw std::logic_error("syllable's enigmaStyles map contained start value greater than end value.");
        }
        const size_t clipStart = (std::max)(span.start, rangeStart);
        const size_t clipEnd = (std::min)(span.end, rangeEnd);
        if (clipStart >= clipEnd) {
            continue;
        }
        MUSX_ASSERT_IF(span.styleIndex >= owner->m_syllableStyles.size()) {
            throw std::logic_error("syllable's enigmaStyles map contained out-of-range styleIndex.");
        }
        std::string_view segment(syllable.data() + clipStart, clipEnd - clipStart);
        const auto& style = owner->m_syllableStyles.at(span.styleIndex);
        if (!callback(std::string(segment), style)) {
            return false;
        }
    }
    return true;
}

bool LyricsSyllableInfo::iterateStyles(util::EnigmaString::TextChunkCallback callback) const
{
    return iterateStylesInRange(0, syllable.size(), callback);
}

std::string LyricsSyllableInfo::ElisionRun::text() const
{
    std::string result;
    for (const auto& [chunkText, style] : m_chunks) {
        result += chunkText;
    }
    return result;
}

bool LyricsSyllableInfo::ElisionRun::iterateStyles(util::EnigmaString::TextChunkCallback callback) const
{
    for (const auto& [chunkText, style] : m_chunks) {
        if (!callback(chunkText, style)) {
            return false;
        }
    }
    return true;
}

std::vector<LyricsSyllableInfo::ElisionRun> LyricsSyllableInfo::calcElisionRuns() const
{
    std::vector<ElisionRun> result;

    std::vector<ElisionMarkerMatch> matches;
    size_t searchFrom = 0;
    while (const auto found = findNextElisionMarker(syllable, searchFrom)) {
        matches.push_back(*found);
        searchFrom = found->position + kElisionMarkerUtf8[found->markerIndex].size();
    }

    const auto addRun = [&](size_t rangeStart, size_t rangeEnd, std::optional<char32_t> joinMarker, bool runHasHyphenBefore, bool runHasHyphenAfter) {
        ElisionRun run;
        run.joinMarker = joinMarker;
        run.hasHyphenBefore = runHasHyphenBefore;
        run.hasHyphenAfter = runHasHyphenAfter;
        iterateStylesInRange(rangeStart, rangeEnd, [&](const std::string& chunkText, const util::EnigmaStyles& style) -> bool {
            run.m_chunks.emplace_back(chunkText, style);
            return true;
        });
        result.emplace_back(std::move(run));
    };

    if (matches.empty()) {
        addRun(0, syllable.size(), std::nullopt, hasHyphenBefore, hasHyphenAfter);
        return result;
    }

    size_t rangeStart = 0;
    for (size_t i = 0; i <= matches.size(); ++i) {
        const size_t rangeEnd = (i < matches.size()) ? matches[i].position : syllable.size();
        const std::optional<char32_t> joinMarker = (i > 0) ? std::optional<char32_t>(kElisionMarkerCodepoints[matches[i - 1].markerIndex]) : std::nullopt;
        addRun(rangeStart, rangeEnd, joinMarker, i == 0 && hasHyphenBefore, i == matches.size() && hasHyphenAfter);
        if (i < matches.size()) {
            rangeStart = matches[i].position + kElisionMarkerUtf8[matches[i].markerIndex].size();
        }
    }

    return result;
}

} // namespace dom
} // namespace musx
