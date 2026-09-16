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

namespace musx {
namespace dom {

/**
 * @struct ResolvedFontInfo
 * @brief Document-independent snapshot of a #FontInfo. (See #FontInfo::resolve.)
 *
 * Every member is a value, so an instance remains valid after the document that produced it is released.
 */
struct ResolvedFontInfo
{
    std::string name;       ///< the font name (see #FontInfo::getName)
    int size{};             ///< the font size, or a percent of the preceding size when #sizeIsPercent is true
    bool sizeIsPercent{};   ///< true when #size is a percentage (see #FontInfo::getSizeIsPercent)
    bool bold{};            ///< bold effect
    bool italic{};          ///< italic effect
    bool underline{};       ///< underline effect
    bool strikeout{};       ///< strikeout effect
    bool absolute{};        ///< fixed-size effect
    bool hidden{};          ///< hidden effect
    bool isSymbolFont{};    ///< true when each code point of text in this font is a raw glyph index (see #FontInfo::calcIsSymbolFont)
    bool isSmufl{};         ///< true when this is a SMuFL font (see #FontInfo::calcIsSMuFL)
};

} // namespace dom
} // namespace musx
