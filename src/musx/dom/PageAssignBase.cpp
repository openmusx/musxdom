/*
 * Copyright (C) 2026, Robert Patterson
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
#include <optional>
#include <string>
#include <vector>

#include "musx/musx.h"

namespace musx {
namespace dom {
namespace others {

// **************************
// ***** PageAssignBase *****
// **************************

// The page assignment shared by PageTextAssign and PageGraphicAssign.

template <typename Derived>
MusxInstance<Derived> PageAssignBase<Derived>::getForPageId(const DocumentPtr& document, Cmper partId, PageCmper pageId, Inci inci)
{
    if (auto part = document->getOthers()->get<PartDefinition>(SCORE_PARTID, partId)) {
        const PageCmper pageAssignmentId = part->calcAssignmentIdFromPageNumber(pageId);
        return document->getOthers()->get<Derived>(partId, pageAssignmentId, inci);
    }
    return nullptr;
}

template <typename Derived>
MusxInstanceList<Derived> PageAssignBase<Derived>::getArrayForPageId(const DocumentPtr& document, Cmper partId, PageCmper pageId)
{
    if (auto part = document->getOthers()->get<PartDefinition>(SCORE_PARTID, partId)) {
        const PageCmper pageAssignmentId = part->calcAssignmentIdFromPageNumber(pageId);
        return document->getOthers()->getArray<Derived>(partId, pageAssignmentId);
    }
    return MusxInstanceList<Derived>(document, partId);
}

template <typename Derived>
std::optional<PageCmper> PageAssignBase<Derived>::calcStartPageNumber(Cmper forPartId) const
{
    if (auto part = getDocument()->getOthers()->template get<PartDefinition>(SCORE_PARTID, forPartId)) {
        if (auto calcValue = part->calcPageNumberFromAssignmentId(getCmper() ? getCmper() : derived().startPage)) {
            if (calcValue.value() <= part->numberOfPages) {
                return calcValue;
            }
        } else if (isMultiPage()) {
            if (auto endPageNum = calcEndPageNumber(forPartId)) {
                if (part->numberOfLeadingBlankPages < endPageNum.value()) {
                    return PageCmper(part->numberOfLeadingBlankPages + 1);
                }
            }
        }
    }
    return std::nullopt;
}

template <typename Derived>
std::optional<PageCmper> PageAssignBase<Derived>::calcEndPageNumber(Cmper forPartId) const
{
    if (auto part = getDocument()->getOthers()->template get<PartDefinition>(SCORE_PARTID, forPartId)) {
        if (isMultiAssignedThruLastPage()) {
            return PageCmper(part->numberOfPages);
        }
        if (auto calcValue = part->calcPageNumberFromAssignmentId(getCmper() ? getCmper() : derived().endPage)) {
            if (calcValue.value() > part->numberOfPages) {
                calcValue = PageCmper(part->numberOfPages);
            }
            return calcValue;
        }
    }
    return std::nullopt;
}

template <typename Derived>
bool PageAssignBase<Derived>::calcMatchesOddEven(PageCmper pageNumber) const
{
    using PageAssignType = typename Derived::PageAssignType;
    switch (derived().getPageAssignType()) {
    case PageAssignType::Even: return pageNumber % 2 == 0;
    case PageAssignType::Odd: return pageNumber % 2 != 0;
    default: return true;
    }
}

template <typename Derived>
bool PageAssignBase<Derived>::calcAppearsOnPage(Cmper forPartId, PageCmper pageNumber) const
{
    const auto startPageNumber = calcStartPageNumber(forPartId);
    if (!startPageNumber) {
        return false;
    }
    // Without an end page in this part, the assignment is on its start page alone.
    const PageCmper endPageNumber = calcEndPageNumber(forPartId).value_or(*startPageNumber);
    return pageNumber >= *startPageNumber && pageNumber <= endPageNumber && calcMatchesOddEven(pageNumber);
}

template <typename Derived>
std::vector<PageCmper> PageAssignBase<Derived>::calcPageNumbers(Cmper forPartId) const
{
    std::vector<PageCmper> result;
    if (const auto startPageNumber = calcStartPageNumber(forPartId)) {
        // Without an end page in this part, the assignment is on its start page alone.
        const PageCmper endPageNumber = calcEndPageNumber(forPartId).value_or(*startPageNumber);
        for (PageCmper pageNumber = *startPageNumber; pageNumber <= endPageNumber; ++pageNumber) {
            if (calcMatchesOddEven(pageNumber)) {
                result.push_back(pageNumber);
            }
        }
    }
    return result;
}

template <typename Derived>
void PageAssignBase<Derived>::integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis)
{
    OthersBase::integrityCheck(ptrToThis);
    if (getCmper() != 0) {
        if (derived().startPage != getCmper() || derived().endPage != getCmper()) {
            MUSX_INTEGRITY_ERROR(std::string(Derived::XmlNodeName) + " " + std::to_string(getCmper()) + " inci "
                + std::to_string(getInci().value_or(0)) + " has startPage or endPage value that does not match.");
        }
    }
}

template class PageAssignBase<PageTextAssign>;
template class PageAssignBase<PageGraphicAssign>;

} // namespace others
} // namespace dom
} // namespace musx
