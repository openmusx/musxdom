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

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "musx/dom/Document.h"
#include "musx/factory/ConstructionContext.h"
#include "musx/factory/FactoryExceptions.h"
#include "musx/xml/XmlInterface.h"

namespace musx {
namespace factory {

/** @brief Creates and finalizes musxdom documents from XML or client-provided data. */
class DocumentFactory
{
    using DocumentPtr = dom::DocumentPtr;

public:
    template <typename Container>
    using IsCharContainer = std::enable_if_t<
        (sizeof(std::remove_cv_t<typename Container::value_type>) == 1)
        && !std::is_same_v<std::remove_cv_t<typename Container::value_type>, bool>>;

    /** @brief Normalized inputs for a non-XML document construction session. */
    struct ConstructionOptions
    {
        dom::PartVoicingPolicy partVoicingPolicy = dom::PartVoicingPolicy::Ignore;
        std::optional<double> scoreDurationSeconds;
        dom::EmbeddedGraphicsMap embeddedGraphics;
        std::optional<std::filesystem::path> sourcePath;
        dom::PlaybackResourcesPtr playbackResources; ///< Playback resources retained by the document. May be null.
    };

    /** @brief Optional inputs accepted by the existing XML creation interface. */
    struct CreateOptions
    {
        struct EmbeddedGraphicFile
        {
            std::string filename;
            dom::EmbeddedGraphicBlob bytes;
        };
        using EmbeddedGraphicFiles = std::vector<EmbeddedGraphicFile>;

        CreateOptions() = default;
        CreateOptions(dom::PartVoicingPolicy policy) : partVoicingPolicy(policy) {}
        CreateOptions(std::filesystem::path inputFilepath,
                      std::vector<char> notationMetadata,
                      EmbeddedGraphicFiles embeddedGraphicFiles,
                      dom::PartVoicingPolicy policy = dom::PartVoicingPolicy::Ignore)
            : partVoicingPolicy(policy),
              m_notationMetadata(std::move(notationMetadata)),
              m_embeddedGraphics(parseEmbeddedGraphicFiles(std::move(embeddedGraphicFiles))),
              m_sourcePath(std::move(inputFilepath))
        {}

        dom::PartVoicingPolicy partVoicingPolicy = dom::PartVoicingPolicy::Ignore;
        /// @brief Playback resources retained by the document. May be null. See @ref dom::Document::getPlaybackResources.
        dom::PlaybackResourcesPtr playbackResources;

        [[nodiscard]] const std::vector<char>& getNotationMetadata() const { return m_notationMetadata; }
        [[nodiscard]] const dom::EmbeddedGraphicsMap& getEmbeddedGraphics() const { return m_embeddedGraphics; }
        [[nodiscard]] const std::optional<std::filesystem::path>& getSourcePath() const { return m_sourcePath; }
        [[nodiscard]] dom::EmbeddedGraphicsMap takeEmbeddedGraphics() { return std::move(m_embeddedGraphics); }

    private:
        static dom::EmbeddedGraphicsMap parseEmbeddedGraphicFiles(EmbeddedGraphicFiles&& files)
        {
            dom::EmbeddedGraphicsMap result;
            for (auto& file : files) {
                const auto dot = file.filename.find('.');
                if (dot == std::string::npos || dot == 0 || dot + 1 >= file.filename.size()) continue;
                const std::string_view idText(file.filename.data(), dot);
                if (!std::all_of(idText.begin(), idText.end(),
                    [](unsigned char ch) { return std::isdigit(ch) != 0; })) continue;
                unsigned long long id = 0;
                try {
                    id = std::stoull(std::string(idText));
                } catch (...) {
                    continue;
                }
                if (id > static_cast<unsigned long long>((std::numeric_limits<dom::Cmper>::max)())) continue;
                auto& entry = result[static_cast<dom::Cmper>(id)];
                entry.extension = file.filename.substr(dot + 1);
                entry.bytes = std::move(file.bytes);
            }
            return result;
        }

        std::vector<char> m_notationMetadata;
        dom::EmbeddedGraphicsMap m_embeddedGraphics;
        std::optional<std::filesystem::path> m_sourcePath;
    };

    /** @brief A move-only draft document that can be populated and finalized exactly once. */
    class ConstructionSession
    {
    public:
        ConstructionSession(const ConstructionSession&) = delete;
        ConstructionSession& operator=(const ConstructionSession&) = delete;
        ConstructionSession(ConstructionSession&&) noexcept = default;
        ConstructionSession& operator=(ConstructionSession&&) noexcept = default;

        [[nodiscard]] const DocumentPtr& getDocument() const;
        [[nodiscard]] ConstructionContext& getConstructionContext() { return m_context; }
        [[nodiscard]] const ConstructionContext& getConstructionContext() const { return m_context; }
        [[nodiscard]] DocumentPtr finish() &&;

    private:
        enum class State { Active, Consumed };
        explicit ConstructionSession(DocumentPtr document) : m_document(std::move(document)) {}

        DocumentPtr m_document;
        ConstructionContext m_context;
        State m_state = State::Active;
        friend class DocumentFactory;
    };

    /** @brief Begins construction of a document from client-provided data. */
    [[nodiscard]] static ConstructionSession begin();
    [[nodiscard]] static ConstructionSession begin(ConstructionOptions options);

    template <typename XmlDocumentType>
    [[nodiscard]] static DocumentPtr create(const char* data, size_t size)
    {
        return create<XmlDocumentType>(data, size, CreateOptions{});
    }

    template <typename XmlDocumentType, typename Container, typename = IsCharContainer<Container>>
    [[nodiscard]] static DocumentPtr create(const Container& xmlBuffer)
    {
        return create<XmlDocumentType>(asCharData(xmlBuffer), xmlBuffer.size(), CreateOptions{});
    }

    template <typename XmlDocumentType>
    [[nodiscard]] static DocumentPtr create(const char* data, size_t size, CreateOptions&& createOptions)
    {
        static_assert(std::is_base_of_v<xml::IXmlDocument, XmlDocumentType>,
            "XmlDocumentType must derive from IXmlDocument.");
        auto xmlDocument = std::make_unique<XmlDocumentType>();
        xmlDocument->loadFromBuffer(data, size);
        auto root = xmlDocument->getRootElement();
        if (!root || root->getTagName() != "finale") {
            throw std::invalid_argument("Missing <finale> element.");
        }

        ConstructionOptions options;
        options.partVoicingPolicy = createOptions.partVoicingPolicy;
        options.scoreDurationSeconds = parseScoreDurationSeconds<XmlDocumentType>(
            createOptions.getNotationMetadata());
        options.embeddedGraphics = createOptions.takeEmbeddedGraphics();
        options.sourcePath = createOptions.getSourcePath();
        options.playbackResources = std::move(createOptions.playbackResources);
        return createFromXmlRoot(root, std::move(options));
    }

    template <typename XmlDocumentType, typename Container, typename = IsCharContainer<Container>>
    [[nodiscard]] static DocumentPtr create(const Container& xmlBuffer, CreateOptions&& createOptions)
    {
        return create<XmlDocumentType>(asCharData(xmlBuffer), xmlBuffer.size(), std::move(createOptions));
    }

private:
    static DocumentPtr createFromXmlRoot(
        const xml::XmlElementPtr& root, ConstructionOptions&& options);
    static void finalize(const DocumentPtr& document, ConstructionContext& context);

    template <typename Container>
    static const char* asCharData(const Container& buffer)
    {
        return reinterpret_cast<const char*>(buffer.data());
    }

    template <typename XmlDocumentType>
    static std::optional<double> parseScoreDurationSeconds(const std::vector<char>& metadata)
    {
        if (metadata.empty()) return std::nullopt;
        auto document = std::make_unique<XmlDocumentType>();
        try {
            document->loadFromBuffer(metadata.data(), metadata.size());
        } catch (...) {
            return std::nullopt;
        }
        auto root = document->getRootElement();
        if (!root || root->getTagName() != "metadata") return std::nullopt;
        auto fileInfo = root->getFirstChildElement("fileInfo");
        if (!fileInfo) return std::nullopt;
        auto creator = fileInfo->getFirstChildElement("creatorString");
        if (!creator || creator->getTextTrimmed().rfind("Finale", 0) != 0) return std::nullopt;
        auto duration = fileInfo->getFirstChildElement("scoreDuration");
        if (!duration) return std::nullopt;
        try {
            return duration->template getTextAs<double>();
        } catch (...) {
            return std::nullopt;
        }
    }
};

} // namespace factory
} // namespace musx
