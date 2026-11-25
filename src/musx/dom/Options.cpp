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
#include <vector>
#include <cstdlib>
#include <exception>
#include <optional>
#include <cwctype>

#include "musx/musx.h"

namespace musx {
namespace dom {
namespace options {

// ***********************
// ***** ClefOptions *****
// ***********************

bool ClefOptions::ClefDef::isBlank() const
{
    if (isShape) {
        if (const auto shape = shapeId ? getDocument()->getOthers()->get<others::ShapeDef>(SCORE_PARTID, shapeId) : nullptr) {
            return shape->isBlank();
        }
        return true;
    }
    // do not let std::iswspace compare outside the ASCII range, because, for example, non-breaking space is used for legacy 8va treble clef
    return !clefChar || (clefChar < 128 && std::iswspace(static_cast<wint_t>(clefChar)));
}

ClefOptions::ClefInfo ClefOptions::ClefDef::calcInfo(const MusxInstance<others::Staff>& currStaff) const
{
    // The glyph identifies a percussion clef wherever it sits, because Finale draws it at any
    // position without changing where pitched notes go. A clef's character is trusted to be a clef
    // glyph: symbol fonts other than Maestro's family use 139 for glyphs no clef would show.
    auto calcPercGlyphType = [&]() -> std::optional<music_theory::ClefType> {
        if (isShape) {
            return std::nullopt;
        }
        if (auto clefFont = calcFont()) {
            if (clefFont->calcIsSMuFL()) {
                switch (clefChar) {
                    case smufl_glyph::unpitchedPercussionClef1:
                    case smufl_glyph::unpitchedPercussionClef1Alt: return music_theory::ClefType::Percussion1;
                    case smufl_glyph::unpitchedPercussionClef2: return music_theory::ClefType::Percussion2;
                    default: break;
                }
            } else if (clefFont->calcIsSymbolFont()) {
                switch (clefFont->calcDrawnCharacter(clefChar)) {
                    case symbol_glyph::unpitchedPercussionClef1:
                    case symbol_glyph::unpitchedPercussionClef1Alt: return music_theory::ClefType::Percussion1;
                    case symbol_glyph::unpitchedPercussionClef2: return music_theory::ClefType::Percussion2;
                    default: break;
                }
            }
        }
        return std::nullopt;
    };

    auto calcTabType = [&]() -> music_theory::ClefType {
        music_theory::ClefType result = music_theory::ClefType::Tab;
        if (isShape) {
            if (auto shape = getDocument()->getOthers()->get<others::ShapeDef>(SCORE_PARTID, shapeId)) {
                shape->iterateInstructions([&](const ShapeDefInstruction::Decoded& inst) -> bool {
                    if (inst.type == ShapeDefInstructionType::SetFont) {
                        auto setFont = std::get<ShapeDefInstruction::SetFont>(inst.data);
                        if (setFont.font.getName().find("Times") != std::string::npos) { // Finale default file uses "Times" or "Times New Roman"
                            result = music_theory::ClefType::TabSerif;
                        }
                        return false;
                    }
                    return true;
                });
            }
            return result;
        }
        if (auto clefFont = calcFont(); clefFont && clefFont->calcIsSMuFL()) {
            if (clefChar == smufl_glyph::fourStringTabClefSerif || clefChar == smufl_glyph::sixStringTabClefSerif) {
                result = music_theory::ClefType::TabSerif;
            }
        }
        return result;
    };

    if (currStaff) {
        if (currStaff->notationStyle == others::Staff::NotationStyle::Tablature) {
            return std::make_pair(calcTabType(), 0);
        }
    } else if (staffPosition == 0 && middleCPos == -10 && isShape) { // identify tab staves based on Finale default file settings
        return std::make_pair(calcTabType(), 0);
    }

    int octave = 0;
    const auto clefPitch = music_theory::NoteName(music_theory::positiveModulus(staffPosition - middleCPos, music_theory::STANDARD_DIATONIC_STEPS, &octave));
    // Pitched notes under a percussion clef sit where a clef would put them, shifted by whole
    // octaves when middle C is moved, so the octave of the glyph's position applies.
    const auto percGlyphType = calcPercGlyphType();
    if (percGlyphType || (currStaff && currStaff->notationStyle == others::Staff::NotationStyle::Percussion)) {
        return std::make_pair(percGlyphType.value_or(music_theory::ClefType::Percussion1), octave);
    }

    music_theory::ClefType clefType = music_theory::ClefType::Unknown;
    switch (clefPitch) {
        case music_theory::NoteName::C: clefType = music_theory::ClefType::C; break;
        case music_theory::NoteName::F: clefType = music_theory::ClefType::F; break;
        case music_theory::NoteName::G: clefType = music_theory::ClefType::G; break;
        case music_theory::NoteName::B: return std::make_pair(music_theory::ClefType::Percussion1, octave); // Finale SMuFL default file settings
        default: break;
    }
    if (clefType == music_theory::ClefType::F) {
        octave++; // The F clef's non-transposing position is below middle C, so compensate.
    }
    return std::make_pair(clefType, octave);
}

MusxInstance<FontInfo> ClefOptions::ClefDef::calcFont() const
{
    MusxInstance<FontInfo> result;
    if (useOwnFont && font) {
        result = font;
    } else if (auto fontOptions = getDocument()->getOptions()->get<FontOptions>()) {
        result = fontOptions->getFontInfo(FontOptions::FontType::Clef);
    }
    if (!result) {
        throw std::invalid_argument("Unable to determine clef font due to missing font definitions.");
    }
    return result;
}

// ***********************
// ***** FontOptions *****
// ***********************

MusxInstance<FontInfo> FontOptions::getFontInfoOrNull(FontOptions::FontType type) const
{
    auto it = fontOptions.find(type);
    return it == fontOptions.end() ? nullptr : it->second;
}

MusxInstance<FontInfo> FontOptions::getFontInfoOrNull(const DocumentPtr& document, FontOptions::FontType type)
{
    auto options = document->getOptions();
    if (!options) {
        return nullptr;
    }
    auto fontOptions = options->get<FontOptions>();
    if (!fontOptions) {
        return nullptr;
    }
    return fontOptions->getFontInfoOrNull(type);
}

MusxInstance<FontInfo> FontOptions::getFontInfo(FontOptions::FontType type) const
{
    if (auto result = getFontInfoOrNull(type)) {
        return result;
    }
    throw std::invalid_argument("Font type " + std::to_string(int(type)) + " not found in document");
}

MusxInstance<FontInfo> FontOptions::getFontInfo(const DocumentPtr& document, FontOptions::FontType type)
{
    if (auto result = getFontInfoOrNull(document, type)) {
        return result;
    }
    throw std::invalid_argument("Font type " + std::to_string(int(type)) + " not found in document");
}

// *****************************
// ***** PageFormatOptions *****
// *****************************

MusxInstance<PageFormatOptions::PageFormat> PageFormatOptions::calcPageFormatForPart(Cmper partId) const
{
    const auto& baseOptions = (partId == SCORE_PARTID) ? pageFormatScore : pageFormatParts;
    auto retval = std::make_shared<PageFormatOptions::PageFormat>(*baseOptions);
    auto pages = getDocument()->getOthers()->getArray<others::Page>(partId);
    auto page1 = pages.size() >= 1 ? pages[0] : nullptr;
    auto page2 = pages.size() >= 2 ? pages[1] : page1; // left page
    auto page3 = pages.size() >= 3 ? pages[2] : page1; // right page that isn't page 1
    if (page2) {
        retval->pageHeight = page2->height;
        retval->pageWidth = page2->width;
        retval->pagePercent = page2->percent;
        retval->leftPageMarginTop = page2->margTop;
        retval->leftPageMarginLeft = page2->margLeft;
        retval->leftPageMarginBottom = page2->margBottom;
        retval->leftPageMarginRight = page2->margRight;
    }
    if (page1) {
        if (retval->differentFirstPageMargin || page1->margTop != page2->margTop) {
            retval->firstPageMarginTop = page1->margTop;
            retval->differentFirstPageMargin = true;
        }
    }
    if (page3) {
        if (retval->facingPages || page3->margTop != page2->margTop || page3->margLeft != page2->margLeft
            || page3->margBottom != page2->margBottom || page3->margRight != page2->margRight) {
            retval->facingPages = true;
            retval->rightPageMarginTop = page3->margTop;
            retval->rightPageMarginLeft = page3->margLeft;
            retval->rightPageMarginBottom = page3->margBottom;
            retval->rightPageMarginRight = page3->margRight;
        }
    }
    auto systems = getDocument()->getOthers()->getArray<others::StaffSystem>(partId);
    auto system1 = systems.size() >= 1 ? systems[0] : nullptr;
    auto system2 = systems.size() >= 2 ? systems[1] : system1;
    if (system2) {
        retval->sysPercent = system2->ssysPercent;
        retval->rawStaffHeight = system2->staffHeight >> 2; // divide by 4 to convert Efix (1/64 Evpu) to Evpu16ths
        retval->sysMarginTop = system2->top;
        retval->sysMarginLeft = system2->left;
        retval->sysMarginBottom = system2->bottom;
        retval->sysMarginRight = system2->right;
        // do not copy system2->distanceToPrev because it varies from the default quite often
    }
    if (system1) {
        if (retval->differentFirstSysMargin || system1->top != system2->top || system1->left != system2->left) {
            retval->differentFirstSysMargin = true;
            retval->firstSysMarginTop = system1->top;
            retval->firstSysMarginLeft = system1->left;
            // do not change retval->firstSysMarginDistance because it varies so much depending on context
        }
    }
    return retval;
}

} // namespace options
} // namespace dom
} // namespace musx