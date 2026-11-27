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

#include "BaseClasses.h"
#include "CommonClasses.h"
#include "Others.h"
 // do not add other dom class dependencies. Use .cpp files for implementations that need total class access.

namespace musx {
namespace dom {

namespace others {

/**
 * @class FileAlias
 * @brief Opaque storage for a platform-specific alias handle associated with a graphic.
 *
 * The DOM does not interpret aliasHandle bytes; callers may decode/resolve them if desired.
 */
class FileAlias : public OthersBase
{
public:
    /// @brief Constructor
    explicit FileAlias(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    size_t length{};                            ///< Declared byte length.
    std::vector<std::uint8_t> aliasHandle{};    ///< Opaque blob (decoded from hex text)

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        OthersBase::integrityCheck(ptrToThis);
        if (length != aliasHandle.size()) {
            MUSX_INTEGRITY_ERROR("Stated length does not match actual size of FileAlias " + std::to_string(getCmper()) + ".");
        }
    }

    constexpr static std::string_view XmlNodeName = "fileAlias"; ///< The XML node name for this type.
    static const xml::XmlElementArray<FileAlias>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class FileDescription
 * @brief Describes the locator flavor Finale used for the associated file/graphic.
 *
 * The DOM does not enforce/interpret pathType semantics. The pathType is preserved as-is.
 */
class FileDescription : public OthersBase
{
public:
    /// @brief Constructor
    explicit FileDescription(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    /// @enum PathType
    /// @brief Defines the possible path types.
    enum class PathType
    {
        DosPath,                ///< Path is a DOS filepath.
        MacFsSpec,              ///< Path is a Mac FS Spec.
        MacAlias,               ///< Path is defined by mac alias. (See @ref FileAlias.)
        MacPosixPath,           ///< Path is a POSIX filepath.
        MacUrlBookmark          ///< Path is a Mac URL bookmark. (See @ref FileUrlBookmark.)
    };

    uint32_t version{};         ///< Always 0x100, meaning perhaps "v1.0". (This was intended for tracking changes to the data format, but it was never used.)
    int volRefNum{};            ///< Classic Mac OS volume reference number, used with legacy MacFsSpec path.
    int dirId{};                ///< Classic Mac OS directory ID, used with legacy MacFsSpec path. (xml node is `<dirID>`)
    PathType pathType{};        ///< Path type. (Determines which of @ref FileAlias and @ref FileUrlBookmark to use as well as how to interpret #FilePath::path.)
    Cmper pathId{};             ///< Cmper of @ref FilePath? It is unknown if this value ever differs from the cmper of #FileDescription. (xml node is `<pathID>`)

    constexpr static std::string_view XmlNodeName = "fileDesc"; ///< The XML node name for this type.
    static const xml::XmlElementArray<FileDescription>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class FilePath
 * @brief Stores a human-readable path/leaf name recorded by Finale.
 *
 * @note This value is not authoritative when graphics are embedded.
 * Embedded graphics files are stored in the `/graphics/` directory of the musx zip file.
 * For how to interpret the path value, see @ref FileDescription.
 */
class FilePath : public OthersBase
{
public:
    /// @brief Constructor
    explicit FilePath(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    std::string path;   ///< Display/path string exactly as in XML. For MacAlias paths, this is only the source file name.

    constexpr static std::string_view XmlNodeName = "filePath"; ///< The XML node name for this type.
    static const xml::XmlElementArray<FilePath>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class FileUrlBookmark
 * @brief Opaque storage for a platform-specific URL/bookmark blob (e.g., macOS CFURL bookmark).
 *
 * The DOM does not interpret urlBookmarkData bytes; callers may decode/resolve them if desired.
 */
class FileUrlBookmark : public OthersBase
{
public:
    /// @brief Constructor
    explicit FileUrlBookmark(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper)
        : OthersBase(document, partId, shareMode, cmper) {}

    size_t length{};                                ///< Declared byte length.
    std::vector<std::uint8_t> urlBookmarkData{};    ///< Opaque blob decoded from hex text.

    void integrityCheck(const std::shared_ptr<EnigmaBase>& ptrToThis) override
    {
        OthersBase::integrityCheck(ptrToThis);
        if (length != urlBookmarkData.size()) {
            MUSX_INTEGRITY_ERROR("Stated length does not match actual size of FileUrlBookmark " + std::to_string(getCmper()) + ".");
        }
    }

    constexpr static std::string_view XmlNodeName = "fileURLBookmark"; ///< The XML node name for this type.
    static const xml::XmlElementArray<FileUrlBookmark>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class PageGraphicAssign
 * @brief Represents a page graphic assignment with positioning and page-range properties.
 *
 * The pages it is on are a page assignment: see @ref PageAssignBase. #displayType governs whether a
 * multi-page assignment repeats on odd, even or all pages.
 *
 * The inci value specifies a particular page graphic when more than one exists for a cmper value.
 *
 * This class is identified by the XML node name "pageGraphicAssign".
 */
class PageGraphicAssign : public PageAssignBase<PageGraphicAssign>
{
public:

    /**
     * @enum PageAssignType
     * @brief Which pages a multipage assignment appears on
     */
    enum class PageAssignType
    {
        AllPages,
        Even,
        Odd,
        One
    };

    /**
     * @enum PositionFrom
     * @brief Reference frame for positioning.
     */
    enum class PositionFrom
    {
        Margins,    ///< Position is relative to page margins.
        PageEdge    ///< Position is relative to page edge.
    };

    using HorizontalAlignment = options::TextOptions::HorizontalAlignment;  ///< Horizontal alignment options.
    using VerticalAlignment = options::TextOptions::VerticalAlignment;      ///< Vertical alignment options.

    /** @brief Constructor */
    explicit PageGraphicAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : PageAssignBase(document, partId, shareMode, cmper, inci)
    {
    }

    uint32_t version{};                 ///< Always 0x100, meaning perhaps "v1.0". (This was intended for tracking changes to the data format, but it was never used.)
    Evpu left{};                        ///< Horizontal position from reference frame.
    Evpu bottom{};                      ///< Vertical position from reference frame.
    Evpu width{};                       ///< Display width of the placed graphic.
    Evpu height{};                      ///< Display height of the placed graphic.
    Cmper fDescId{};                    ///< The Cmper of the assigned @ref FileDescription. (xml tag is `<fDescID>`)
    PageAssignType displayType{};       ///< Whether the assignment appears on all/even/odd pages. It applies to single-page
                                        ///< assignments as well, which otherwise store PageAssignType::One. See #calcMatchesOddEven.
    bool hidden{};                      ///< If true, the graphic does not display: inverse of "Show" context menu option. (xml tag is `<displayHidden>`)
    HorizontalAlignment hAlign{};       ///< Horizontal alignment for left/all pages. (xml tag is `<halign>`)
    VerticalAlignment vAlign{};         ///< Vertical alignment for left/all pages. (xml tag is `<valign>`)
    PositionFrom posFrom{};             ///< Position reference for left/all pages.
    bool fixedPerc{};                   ///< If true, preserve aspect ratio. This is a UI setting. Use width/origWidth and height/origHeight for actual scaling.
    PageCmper startPage{};              ///< First page assignment ID where the graphic appears when cmper==0.
    PageCmper endPage{};                ///< Last page assignment ID where the graphic appears when cmper==0.
    bool savedRecord{};                 ///< Indicates a stored/saved record. (Used internally by Finale when a graphic is created.)
    Evpu origWidth{};                   ///< Intrinsic/original width of the graphic.
    Evpu origHeight{};                  ///< Intrinsic/original height of the graphic.
    HorizontalAlignment rightPgHAlign{};///< Horizontal alignment on right pages.
    VerticalAlignment rightPgVAlign{};  ///< Vertical alignment on right pages.
    PositionFrom rightPgPosFrom{};      ///< Position reference for right pages. This is a UI setting. Use width/origWidth and height/origHeight for actual scaling.
                                        ///< The Finale UI appears to sync this with #posFrom.
    bool rightPgFixedPerc{};            ///< If true, preserve aspect ratio on right-pages (UI setting only). The Finale UI appears to sync this with #fixedPerc.
    Evpu rightPgLeft{};                 ///< Horizontal position for right pages.
    Evpu rightPgBottom{};               ///< Vertical position for right pages.
    Cmper graphicCmper{};               ///< Graphic instance Cmper. A non-zero value indicates an embedded graphic whose payload is
                                        ///< only available if the caller loaded it into the @ref Document using
                                        ///< @ref musx::factory::DocumentFactory::CreateOptions. The embedded file may be found in the
                                        ///< @c /graphics/ subdirectory of the musx zip archive, named using this cmper (for example,
                                        ///< @c 3.png when @c graphicCmper is 3).

    /// @brief The assignment's odd/even setting, #displayType. Required by @ref PageAssignBase.
    PageAssignType getPageAssignType() const { return displayType; }

    constexpr static std::string_view XmlNodeName = "pageGraphicAssign"; ///< The XML node name for this type.
    static const xml::XmlElementArray<PageGraphicAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

/**
 * @class ShapeGraphicAssign
 * @brief Represents a graphic used by a specific @ref ShapeDef.
 *
 * The Cmper comes from a #ShapeDefInstructionType::ExternalGraphic instruction.
 *
 * This class is identified by the XML node name "shapeGraphicAssign".
 */
class ShapeGraphicAssign : public OthersBase
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (often 0 for score).
     * @param shareMode The sharing mode for this #ShapeGraphicAssign (often #ShareMode::All).
     * @param cmper The measure ID for this #ShapeGraphicAssign.
     * @param inci The 0-based incident. (Possibly always zero in this case.)
     */
    explicit ShapeGraphicAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper cmper, Inci inci)
        : OthersBase(document, partId, shareMode, cmper, inci)
    {
    }

    using HorizontalAlignment = options::TextOptions::HorizontalAlignment;  ///< Horizontal alignment options.
    using VerticalAlignment = options::TextOptions::VerticalAlignment;      ///< Vertical alignment options.

    uint32_t version{};     ///< Always 0x100, meaning perhaps "v1.0". (This was intended for tracking changes to the data format, but it was never used.)
    Evpu left{};            ///< Graphic left coordinate in Evpu
    Evpu bottom{};          ///< Graphic bottom coordinate in Evpu
    Evpu width{};           ///< Graphic width in Evpu
    Evpu height{};          ///< Graphic height in Evpu
    Cmper fDescId{};        ///< The Cmper of the assigned @ref others::FileDescription. (xml tag is `<fDescID>`)
    HorizontalAlignment hAlign{};  ///< Horizontal alignment of the placed graphic: possibly always `Left`. (xml tag is `<halign>`)
    VerticalAlignment vAlign{};    ///< Vertical alignment the placed graphic: possibly always `Top`. (xml tag is `<valign>`)
    bool fixedPerc{};       ///< UI setting to preserve aspect ratio. It may be meaningless here, since there is no UI setting for shape graphics.
    bool hidden{};          ///< Indicates the graphic is hidden from print/displayed as hidden: possibly always `false` for @ref ShapeGraphicAssign. (xml node is `<displayHidden>`)
    bool savedRecord{};     ///< Indicates a stored/saved record. (Used internally by Finale when a graphic is created.)
    Evpu origWidth{};       ///< Original (intrinsic) width in Evpu
    Evpu origHeight{};      ///< Original (intrinsic) height in Evpu
    Cmper graphicCmper{};   ///< Graphic instance Cmper. (See #others::PageGraphicAssign::graphicCmper for full explanation.)

    /// @brief Finds the assignment used by a ShapeDef ExternalGraphic instruction.
    /// @param document The document containing the shape and assignment.
    /// @param partId The requested part ID.
    /// @param graphicCmper The comparator stored by the ExternalGraphic instruction.
    /// @return The matching assignment, or an empty instance when none exists.
    static MusxInstance<ShapeGraphicAssign> findForGraphic(
        const DocumentPtr& document, Cmper partId, Cmper graphicCmper);

    constexpr static std::string_view XmlNodeName = "shapeGraphicAssign"; ///< The XML node name for this type.
    static const xml::XmlElementArray<ShapeGraphicAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

} //namespace others

namespace details {

/**
 * @class MeasureGraphicAssign
 * @brief Represents a graphic assignment anchored to a specific staff and measure.
 *
 * Cmper1 is the staff (staffId) @ref Cmper and Cmper2 is the measure @ref Cmper.
 *
 * Positions and sizes are Finale layout units: horizontal values use Evpu from the measure’s
 * left edge (system coordinates); vertical values use Evpu from the staff baseline.
 *
 * This class is identified by the XML node name "measGraphicAssign".
 */
class MeasureGraphicAssign : public DetailsBase
{
public:
    /**
     * @brief Constructor
     * @param document A weak pointer to the associated document.
     * @param partId The part that this is for (often 0 for score).
     * @param shareMode The sharing mode for this #MeasureGraphicAssign (often #ShareMode::All).
     * @param staffId The staff ID for this #MeasureGraphicAssign.
     * @param meas The measure ID for this #MeasureGraphicAssign.
     * @param inci The 0-based incident.
     */
    explicit MeasureGraphicAssign(const DocumentWeakPtr& document, Cmper partId, ShareMode shareMode, Cmper staffId, Cmper meas, Inci inci)
        : DetailsBase(document, partId, shareMode, staffId, meas, inci)
    {
    }

    using HorizontalAlignment = options::TextOptions::HorizontalAlignment;  ///< Horizontal alignment options.
    using VerticalAlignment = options::TextOptions::VerticalAlignment;      ///< Vertical alignment options.
    using PositionFrom = others::PageGraphicAssign::PositionFrom;           ///< Postion relative to page. (Meaningless here.)
    
    uint32_t version{};     ///< Always 0x100, meaning perhaps "v1.0". (This was intended for tracking changes to the data format, but it was never used.)
    Evpu left{};            ///< Graphic left coordinate in Evpu
    Evpu bottom{};          ///< Graphic bottom coordinate in Evpu
    Evpu width{};           ///< Graphic width in Evpu
    Evpu height{};          ///< Graphic height in Evpu
    Cmper fDescId{};        ///< The Cmper of the assigned @ref others::FileDescription. (xml tag is `<fDescID>`)
    bool hidden{};          ///< Indicates the graphic is hidden from print/displayed as hidden (xml node is `<displayHidden>`)
    HorizontalAlignment hAlign{}; ///< Horizontal alignment for left/all pages. Meaningless here, but sometimes provided. (xml tag is `<halign>`)
    VerticalAlignment vAlign{}; ///< Vertical alignment for left/all pages. Meaningless here, but sometimes provided. (xml tag is `<valign>`)
    PositionFrom posFrom{}; ///< Position reference for left/all pages. Meaningless here, but sometimes provided.
    bool fixedPerc{};       ///< If true, preserve aspect ratio. This is a UI setting. Use width/origWidth and height/origHeight for actual scaling.
    bool savedRecord{};     ///< Indicates a stored/saved record. (Used internally by Finale when a graphic is created.)
    Evpu origWidth{};       ///< Original (intrinsic) width in Evpu
    Evpu origHeight{};      ///< Original (intrinsic) height in Evpu
    Cmper graphicCmper{};   ///< Graphic instance Cmper. (See #others::PageGraphicAssign::graphicCmper for full explanation.)

    constexpr static std::string_view XmlNodeName = "measGraphicAssign"; ///< The XML node name for this type.
    static const xml::XmlElementArray<MeasureGraphicAssign>& xmlMappingArray(); ///< Required for musx::factory::FieldPopulator.
};

} //namespace details

} //namespace dom
} //namespace musx
