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

#include "musx/factory/DocumentFactory.h"

#include <functional>
#include <map>
#include <stdexcept>
#include <string>

#include "musx/dom/Details.h"
#include "musx/dom/Entries.h"
#include "musx/dom/Options.h"
#include "musx/dom/Others.h"
#include "musx/dom/ShapeDesigner.h"
#include "musx/dom/Staff.h"
#include "musx/dom/Texts.h"
#include "musx/factory/HeaderFactory.h"
#include "musx/factory/PoolFactory.h"
#include "musx/util/Logger.h"

namespace musx {
namespace factory {

void resolveFontDefinitions(const dom::DocumentPtr& document, const ConstructionContext& context)
{
    for (const auto fontId : context.referencedFontIds()) {
        if (fontId == dom::DEFAULT_MUSIC_FONT_ID) continue;
        if (document->getOthers()->get<dom::others::FontDefinition>(dom::SCORE_PARTID, fontId)) {
            continue;
        }
        auto font = std::make_shared<dom::others::FontDefinition>(
            document,
            dom::SCORE_PARTID,
            dom::EnigmaBase::ShareMode::All,
            fontId);
        if (document->getHeader()->created.platform == dom::header::Platform::Windows) {
            font->charsetBank = dom::others::FontDefinition::CharacterSetBank::Windows;
        }
        font->name = "Missing Font (" + std::to_string(fontId) + ")";
        document->getOthers()->add(dom::others::FontDefinition::XmlNodeName, font);
        util::Logger::log(util::Logger::LogLevel::Info, "Font " + std::to_string(fontId)
            + " is referenced but not defined; it will show as \"" + font->name + "\".");
    }
}

namespace {

using Resolver = std::function<void(const dom::DocumentPtr&, ConstructionContext&)>;

template <typename BeamType>
void resolveBeamAlterations(const dom::DocumentPtr& document)
{
    if (document->getDetails()->getArray<BeamType>(dom::SCORE_PARTID).empty()) return;
    dom::details::BeamAlterations::calcAllActiveFlags<BeamType>(document);
}

void resolveClefOptions(const dom::DocumentPtr& document)
{
    auto options = document->getOptions()->get<dom::options::ClefOptions>();
    if (!options) return;
    for (size_t i = 0; i < options->clefDefs.size(); ++i) {
        const auto& def = options->clefDefs[i];
        if (def->useOwnFont && !def->font) {
            MUSX_INTEGRITY_ERROR("Use own font was specified for clef " + std::to_string(i)
                + ", but no font was found in the xml.");
        }
    }
}

void resolveEntries(const dom::DocumentPtr& document)
{
    if (document->getEntries()->empty()) return;
    dom::Entry::calcLocations(document);
}

template <typename ExpressionType>
void resolveExpressions(const dom::DocumentPtr& document)
{
    for (const auto& expression : document->getOthers()->getArray<ExpressionType>(dom::SCORE_PARTID)) {
        if (!expression->categoryId) continue;
        auto category = document->getOthers()->get<dom::others::MarkingCategory>(
            expression->getSourcePartId(), expression->categoryId);
        if (!category) {
            util::Logger::log(util::Logger::LogLevel::Info, "Marking category for expression "
                + std::to_string(expression->getCmper()) + " does not exist.");
            continue;
        }
        auto* mutableCategory = const_cast<dom::others::MarkingCategory*>(category.get());
        if constexpr (std::is_same_v<ExpressionType, dom::others::ShapeExpressionDef>) {
            mutableCategory->shapeExpressions.emplace(expression->getCmper(), expression);
        } else {
            mutableCategory->textExpressions.emplace(expression->getCmper(), expression);
        }
    }
}

void resolveKeyMapArrays(const dom::DocumentPtr& document)
{
    auto arrays = document->getOthers()->getArray<dom::others::KeyMapArray>(dom::SCORE_PARTID);
    for (const auto& array : arrays) {
        auto* mutableArray = const_cast<dom::others::KeyMapArray*>(array.get());
        auto trimSteps = [&](size_t newSize) {
            while (array->steps.size() > newSize) {
                const auto& element = array->steps.back();
                if (element->diatonic || element->hlevel != 0) break;
                mutableArray->steps.pop_back();
            }
        };
        if (auto format = document->getOthers()->get<dom::others::KeyFormat>(
                dom::SCORE_PARTID, array->getCmper())) {
            trimSteps(format->semitones);
            if (format->scaleTones != array->countDiatonicSteps()
                || format->semitones != array->steps.size()) {
                MUSX_INTEGRITY_ERROR("KeyMapArray " + std::to_string(array->getCmper())
                    + " does not match KeyFormat.");
            }
        } else {
            trimSteps(12);
            if (array->countDiatonicSteps() != 7 || array->steps.size() != 12) {
                MUSX_INTEGRITY_ERROR("KeyMapArray " + std::to_string(array->getCmper())
                    + " has no KeyFormat but does not match default values.");
            }
        }
    }
}

void resolveLayerAttributes(const dom::DocumentPtr& document)
{
    const auto layers = document->getOthers()->getArray<dom::others::LayerAttributes>(dom::SCORE_PARTID);
    if (layers.empty()) return;
    if (layers.size() < dom::MAX_LAYERS) {
        MUSX_INTEGRITY_ERROR("Expected at least " + std::to_string(dom::MAX_LAYERS)
            + " <layerAtts> elements. Got " + std::to_string(layers.size()) + ".");
    } else if (layers.size() > dom::MAX_LAYERS) {
        util::Logger::log(util::Logger::LogLevel::Verbose,
            "Ignoring " + std::to_string(layers.size() - dom::MAX_LAYERS) + " extra <layerAtts> elements.");
    }
    for (size_t i = 0; i < dom::MAX_LAYERS; ++i) {
        if (layers[i]->getCmper() != i) {
            MUSX_INTEGRITY_ERROR("Expected <layerAtts> elements to have cmper values 0, 1, 2, 3 in order.");
        }
    }
}

template <typename LyricsType>
void resolveLyrics(const dom::DocumentPtr& document)
{
    for (const auto& text : document->getTexts()->getArray<LyricsType>()) {
        const_cast<LyricsType*>(text.get())->createSyllableInfo(text);
    }
}

void resolveMarkingCategories(const dom::DocumentPtr& document)
{
    for (const auto& category : document->getOthers()->getArray<dom::others::MarkingCategory>(dom::SCORE_PARTID)) {
        if (category->categoryType == dom::others::MarkingCategory::CategoryType::Invalid) {
            MUSX_INTEGRITY_ERROR("Encountered <markingsCategory> node (cmper "
                + std::to_string(category->getCmper()) + ") with no categoryType");
        }
    }
}

void resolveMultiStaffGroupIds(const dom::DocumentPtr& document)
{
    for (const auto& part : document->getOthers()->getArray<dom::others::PartDefinition>(dom::SCORE_PARTID)) {
        for (const auto& instance : document->getOthers()->getArray<dom::others::MultiStaffGroupId>(part->getCmper())) {
            if (auto group = document->getDetails()->get<dom::details::StaffGroup>(part->getCmper(),
                    document->calcScrollViewCmper(part->getCmper()), instance->staffGroupId)) {
                const_cast<dom::details::StaffGroup*>(group.get())->multiStaffGroupId = instance->getCmper();
            } else if (instance->staffGroupId != 0) {
                MUSX_INTEGRITY_ERROR("Group " + std::to_string(instance->staffGroupId)
                    + " appears in MultiStaffGroupId " + std::to_string(instance->getCmper())
                    + " but does not exist.");
            }
        }
    }
}

void resolveMultiStaffInstrumentGroups(const dom::DocumentPtr& document)
{
    if (document->getOthers()->getArray<dom::others::MultiStaffInstrumentGroup>(dom::SCORE_PARTID).empty()) return;
    dom::others::MultiStaffInstrumentGroup::calcAllMultiStaffGroupIds(document);
}

void resolvePartDefinitions(const dom::DocumentPtr& document)
{
    for (const auto& part : document->getOthers()->getArray<dom::others::PartDefinition>(dom::SCORE_PARTID)) {
        if (!document->getOthers()->get<dom::others::PartGlobals>(
                part->getCmper(), dom::MUSX_GLOBALS_CMPER)) {
            auto partGlobals = std::make_shared<dom::others::PartGlobals>(document, part->getCmper(),
                dom::EnigmaBase::ShareMode::None, dom::MUSX_GLOBALS_CMPER);
            partGlobals->showTransposed = true;
            partGlobals->scrollViewIUlist = dom::BASE_SYSTEM_ID;
            partGlobals->studioViewIUlist = dom::STUDIO_VIEW_SYSTEM_ID;
            document->getOthers()->add(dom::others::PartGlobals::XmlNodeName, partGlobals);
            util::Logger::log(util::Logger::LogLevel::Verbose, "Part " + std::to_string(part->getCmper())
                + " has no PartGlobals. A default PartGlobals was created.");
        }
    }
}

void resolveShapeDefs(const dom::DocumentPtr& document, ConstructionContext& context)
{
    for (const auto& shape : document->getOthers()->getArray<dom::others::ShapeDef>(dom::SCORE_PARTID)) {
        const auto instructions = document->getOthers()->get<dom::others::ShapeInstructionList>(
            shape->getRequestedPartId(), shape->instructionList);
        const auto data = document->getOthers()->get<dom::others::ShapeData>(
            shape->getRequestedPartId(), shape->dataList);
        if (!instructions || !data) continue;

        std::size_t dataIndex = 0;
        for (const auto& instruction : instructions->instructions) {
            const auto dataCount = static_cast<std::size_t>(instruction->numData);
            if (dataIndex + dataCount > data->values.size()) break;
            if (instruction->type == dom::ShapeDefInstructionType::SetFont && dataCount >= 3) {
                context.registerFontId(dom::Cmper(data->values[dataIndex]));
            }
            dataIndex += dataCount;
        }
    }
}

void resolveStaff(const dom::DocumentPtr& document)
{
    if (document->getOthers()->getArray<dom::others::Staff>(dom::SCORE_PARTID).empty()) return;
    dom::others::Staff::calcAllRuntimeValues<dom::others::Staff>(document);
    if (document->getOthers()->getArray<dom::others::MultiStaffInstrumentGroup>(dom::SCORE_PARTID).empty()) {
        dom::others::Staff::calcAllAutoNumberValues(document);
    }
}

void resolveStaffGroups(const dom::DocumentPtr& document)
{
    for (const auto& part : document->getOthers()->getArray<dom::others::PartDefinition>(dom::SCORE_PARTID)) {
        auto groups = document->getDetails()->getArray<dom::details::StaffGroup>(
            part->getCmper(), document->calcScrollViewCmper(part->getCmper()));
        const auto staves = document->getScrollViewStaves(part->getCmper());
        for (const auto& group : groups) {
            auto start = staves.getIndexForStaff(group->startInst);
            auto end = staves.getIndexForStaff(group->endInst);
            if (!start || !end) {
                util::Logger::log(util::Logger::LogLevel::Verbose, "Group "
                    + std::to_string(group->getCmper2()) + " in part " + part->getName()
                    + " [" + std::to_string(part->getCmper())
                    + "] has non-existent start or end staff cmpers");
                continue;
            }
            auto* mutableGroup = const_cast<dom::details::StaffGroup*>(group.get());
            for (size_t i = *start; i <= *end && i < staves.size(); ++i) {
                mutableGroup->staves.insert(staves[i]->staffId);
            }
        }
    }
}

void resolveStaffStyles(const dom::DocumentPtr& document)
{
    if (document->getOthers()->getArray<dom::others::StaffStyle>(dom::SCORE_PARTID).empty()) return;
    dom::others::Staff::calcAllRuntimeValues<dom::others::StaffStyle>(document);
}

void resolveStaffStyleAssigns(const dom::DocumentPtr& document)
{
    for (const auto& assign : document->getOthers()->getAllSources<dom::others::StaffStyleAssign>()) {
        // A missing style ID is already an integrity error.
        if (!assign->styleId) continue;
        if (!document->getOthers()->get<dom::others::StaffStyle>(assign->getSourcePartId(), assign->styleId)) {
            util::Logger::log(util::Logger::LogLevel::Verbose, "Staff style assignment on staff "
                + std::to_string(assign->getCmper()) + " in part " + std::to_string(assign->getSourcePartId())
                + " refers to staff style " + std::to_string(assign->styleId)
                + ", which does not exist. The assignment is ignored.");
        }
    }
}

template <typename Function>
Resolver makeResolver(Function function)
{
    return [function](const dom::DocumentPtr& document, ConstructionContext&) { function(document); };
}

const std::vector<std::pair<std::string_view, Resolver>>& resolvers()
{
    static const std::vector<std::pair<std::string_view, Resolver>> result = {
        {dom::details::BeamAlterationsDownStem::XmlNodeName,
            makeResolver(resolveBeamAlterations<dom::details::BeamAlterationsDownStem>)},
        {dom::details::BeamAlterationsUpStem::XmlNodeName,
            makeResolver(resolveBeamAlterations<dom::details::BeamAlterationsUpStem>)},
        {dom::options::ClefOptions::XmlNodeName, makeResolver(resolveClefOptions)},
        {dom::Entry::XmlNodeName, makeResolver(resolveEntries)},
        {dom::others::KeyMapArray::XmlNodeName, makeResolver(resolveKeyMapArrays)},
        {dom::others::LayerAttributes::XmlNodeName, makeResolver(resolveLayerAttributes)},
        {dom::texts::LyricsChorus::XmlNodeName, makeResolver(resolveLyrics<dom::texts::LyricsChorus>)},
        {dom::texts::LyricsSection::XmlNodeName, makeResolver(resolveLyrics<dom::texts::LyricsSection>)},
        {dom::texts::LyricsVerse::XmlNodeName, makeResolver(resolveLyrics<dom::texts::LyricsVerse>)},
        {dom::others::MarkingCategory::XmlNodeName, makeResolver(resolveMarkingCategories)},
        {dom::others::Measure::XmlNodeName, makeResolver(dom::others::Measure::checkMeasureCmperSequence)},
        {dom::others::MultiStaffGroupId::XmlNodeName, makeResolver(resolveMultiStaffGroupIds)},
        {dom::others::MultiStaffInstrumentGroup::XmlNodeName,
            makeResolver(resolveMultiStaffInstrumentGroups)},
        {dom::others::Page::XmlNodeName, makeResolver(dom::others::Page::calcSystemInfo)},
        {dom::others::PartDefinition::XmlNodeName, makeResolver(resolvePartDefinitions)},
        {dom::details::SecondaryBeamAlterationsDownStem::XmlNodeName,
            makeResolver(resolveBeamAlterations<dom::details::SecondaryBeamAlterationsDownStem>)},
        {dom::details::SecondaryBeamAlterationsUpStem::XmlNodeName,
            makeResolver(resolveBeamAlterations<dom::details::SecondaryBeamAlterationsUpStem>)},
        {dom::others::ShapeDef::XmlNodeName, resolveShapeDefs},
        {dom::others::ShapeExpressionDef::XmlNodeName, makeResolver(resolveExpressions<dom::others::ShapeExpressionDef>)},
        {dom::others::Staff::XmlNodeName, makeResolver(resolveStaff)},
        {dom::details::StaffGroup::XmlNodeName, makeResolver(resolveStaffGroups)},
        {dom::others::StaffStyle::XmlNodeName, makeResolver(resolveStaffStyles)},
        {dom::others::StaffStyleAssign::XmlNodeName, makeResolver(resolveStaffStyleAssigns)},
        {dom::others::TextExpressionDef::XmlNodeName, makeResolver(resolveExpressions<dom::others::TextExpressionDef>)},
        // Keep resolveFontDefinitions last so every resolver has registered its font references first.
        {dom::others::FontDefinition::XmlNodeName, resolveFontDefinitions}
    };
    return result;
}

} // namespace

DocumentFactory::ConstructionSession DocumentFactory::begin(ConstructionOptions options)
{
    DocumentPtr document(new dom::Document);
    document->m_self = document;
    document->m_partVoicingPolicy = options.partVoicingPolicy;
    document->m_scoreDurationSeconds = options.scoreDurationSeconds;
    document->m_embeddedGraphics = std::move(options.embeddedGraphics);
    document->m_sourcePath = std::move(options.sourcePath);
    document->getHeader() = std::make_shared<dom::Header>();
    document->getOptions() = std::make_shared<dom::OptionsPool>(document);
    document->getOthers() = std::make_shared<dom::OthersPool>(document);
    document->getDetails() = std::make_shared<dom::DetailsPool>(document);
    document->getEntries() = std::make_shared<dom::EntryPool>(document);
    document->getTexts() = std::make_shared<dom::TextsPool>(document);
    return ConstructionSession(std::move(document));
}

DocumentFactory::ConstructionSession DocumentFactory::begin()
{
    return begin(ConstructionOptions{});
}

const DocumentFactory::DocumentPtr& DocumentFactory::ConstructionSession::getDocument() const
{
    if (m_state != State::Active || !m_document) {
        throw std::logic_error("Document construction session has already been consumed.");
    }
    return m_document;
}

DocumentFactory::DocumentPtr DocumentFactory::ConstructionSession::finish() &&
{
    if (m_state != State::Active || !m_document) {
        throw std::logic_error("Document construction session has already been consumed.");
    }
    m_state = State::Consumed;
    auto document = std::move(m_document);
    DocumentFactory::finalize(document, m_context);
    return document;
}

DocumentFactory::DocumentPtr DocumentFactory::createFromXmlRoot(
    const xml::XmlElementPtr& root, ConstructionOptions&& options)
{
    auto session = begin(std::move(options));
    const auto& document = session.getDocument();
    for (auto element = root->getFirstChildElement(); element; element = element->getNextSibling()) {
        if (element->getTagName() == "header") {
            document->getHeader() = HeaderFactory::create(element);
        } else if (element->getTagName() == "options") {
            document->getOptions() = OptionsFactory::create(session.getConstructionContext(), element, document);
        } else if (element->getTagName() == "others") {
            document->getOthers() = OthersFactory::create(session.getConstructionContext(), element, document);
        } else if (element->getTagName() == "details") {
            document->getDetails() = DetailsFactory::create(session.getConstructionContext(), element, document);
        } else if (element->getTagName() == "entries") {
            document->getEntries() = EntryFactory::create(session.getConstructionContext(), element, document);
        } else if (element->getTagName() == "texts") {
            document->getTexts() = TextsFactory::create(session.getConstructionContext(), element, document);
        }
    }
    return std::move(session).finish();
}

void DocumentFactory::finalize(const DocumentPtr& document, ConstructionContext& context)
{
    document->getOptions()->integrityCheckAll();
    document->getOthers()->integrityCheckAll();
    document->getDetails()->integrityCheckAll();
    document->getEntries()->integrityCheckAll();
    document->getTexts()->integrityCheckAll();

#ifdef MUSX_DISPLAY_NODE_NAMES
    util::Logger::log(util::Logger::LogLevel::Verbose, "============");
#endif
    for (const auto& [key, resolver] : resolvers()) {
        (void)key;
        resolver(document, context);
    }
    document->m_instruments = document->createInstrumentMap(dom::SCORE_PARTID);
    document->createRehearsalMarkMap();
    document->m_maxBlankPages = 0;
    for (const auto& part : document->getOthers()->getArray<dom::others::PartDefinition>(dom::SCORE_PARTID)) {
        auto* mutablePart = const_cast<dom::others::PartDefinition*>(part.get());
        mutablePart->numberOfLeadingBlankPages = 0;
        const auto pages = document->getOthers()->getArray<dom::others::Page>(part->getCmper());
        mutablePart->numberOfPages = static_cast<int>(pages.size());
        for (const auto& page : pages) {
            if (!page->isBlank()) break;
            ++mutablePart->numberOfLeadingBlankPages;
        }
        document->m_maxBlankPages = (std::max)(document->m_maxBlankPages,
            mutablePart->numberOfLeadingBlankPages);
    }
}

} // namespace factory
} // namespace musx
