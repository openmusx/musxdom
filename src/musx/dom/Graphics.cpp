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

#include "musx/musx.h"

namespace musx {
namespace dom {
namespace others {

MusxInstance<ShapeGraphicAssign> ShapeGraphicAssign::findForGraphic(
    const DocumentPtr& document, Cmper partId, Cmper graphicCmper)
{
    if (!document || !document->getOthers()) return {};
    // ShapeGraphicAssign is an incident class whose observed incidence is zero. Looking it
    // up without that incidence does not match its object key. Older documents may also key
    // assignments independently and carry the instruction comparator in graphicCmper.
    if (auto assignment = document->getOthers()
            ->get<ShapeGraphicAssign>(partId, graphicCmper, Inci(0))) {
        return assignment;
    }
    for (const auto& assignment : document->getOthers()
            ->getArray<ShapeGraphicAssign>(partId)) {
        if (assignment->graphicCmper == graphicCmper) return assignment;
    }
    return {};
}

} // namespace others
} // namespace dom
} // namespace musx
