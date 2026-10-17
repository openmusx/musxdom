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
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <tuple>
#include <variant>
#include <stdexcept>
#include <functional>
#include <iterator>
#include <limits>
#include <typeindex>

#include "MusxInstance.h"

#include "BaseClasses.h"
#include "Others.h"
#include "Details.h"
#include "Entries.h"
#include "ShapeDesigner.h"
#include "SmartShape.h"
#include "Staff.h"

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
namespace bench {
/// @brief Gives access to the raw pools for benchmarking. This class is
/// only defined in the benchmarks.
template<class> class PoolAccessor;
}; // namespace bench
#endif

namespace musx {
namespace dom {

/// @brief Type trait to determine if a class in a given pool
/// @tparam Pool The pool type
/// @tparam T The class to query
template <typename Pool, typename T>
struct is_pool_type : std::false_type {};  // default: not valid

class OptionsPool;
class OthersPool;
class DetailsPool;
class EntryPool;
class TextsPool;

template <typename T>
struct is_pool_type<OptionsPool, T> : std::is_base_of<OptionsBase, T> {};
template <typename T>
struct is_pool_type<OthersPool, T> : std::is_base_of<OthersBase, T> {};
template <typename T>
struct is_pool_type<DetailsPool, T> : std::is_base_of<DetailsBase, T> {};
template <typename T>
struct is_pool_type<EntryPool, T> : std::is_same<Entry, T> {};
template <typename T>
struct is_pool_type<TextsPool, T> : std::is_base_of<TextsBase, T> {};

/// @brief Value shortcut for @ref is_pool_type
template <typename Pool, typename T>
inline constexpr bool is_pool_type_v = is_pool_type<Pool, T>::value;

/**
 * @class ObjectPool
 * @brief A pool that manages collections of `OthersBase` objects, organized by XML node names and `Cmper` values.
 *
 * This class provides functionality to store and retrieve objects derived from a `Base` object type,
 * It supports dynamic retrieval of objects cast to their specific derived types.
 *
 * @tparam ObjectBaseType the base type for the objects in the pool
 */
template <typename ObjectBaseType>
class ObjectPool
{
public:
        /** @brief shared pointer to `ObjectBaseType` */
    using ObjectPtr = std::shared_ptr<ObjectBaseType>;
    /** @brief key type for storing in pool */
    struct ObjectKey {
        std::type_index typeId;         ///< runtime identity of the object's concrete class.
        std::string_view nodeId;        ///< the identifier for this node. usually the XML node name.
        Cmper partId;                   ///< the part this item is associated with (or 0 for score).
        std::optional<Cmper> cmper1;    ///< optional cmper1 for Others, Texts, Details.
        std::optional<Cmper> cmper2;    ///< optional cmper2 for Details.
        // Use `int` instead of `Inci` to work around GCC spurious -Wmaybe-uninitialized warning
        std::optional<int> inci;        ///< optional inci for multi-inci classes

        /** @brief explicit constructor for optional parameters */
        ObjectKey(std::type_index t,
            std::string_view n,
            Cmper p,
            std::optional<Cmper> c1 = std::nullopt,
            std::optional<Cmper> c2 = std::nullopt,
            std::optional<int> i = std::nullopt) : typeId(t), nodeId(n), partId(p), cmper1(c1), cmper2(c2), inci(i)
        {
        }

        /** @brief comparison operator for std::map */
        bool operator<(const ObjectKey& other) const
        {
            if (typeId != other.typeId) {
                return typeId < other.typeId;
            }
            if (partId != other.partId) {
                return partId < other.partId;
            }
            if (cmper1 != other.cmper1) {
                return cmper1 < other.cmper1;
            }
            if (cmper2 != other.cmper2) {
                return cmper2 < other.cmper2;
            }
            return inci < other.inci;
        }

        /** @brief provides a description of the key for diagnostic purposes */
        std::string description() const
        {
            std::string result = std::string(nodeId) + " part " + std::to_string(partId);
            if (cmper1) {
                if (cmper2) {
                    result += " cmpers [" + std::to_string(cmper1.value()) + ", " + std::to_string(cmper2.value()) + "]";
                } else {
                    result += " cmper " + std::to_string(cmper1.value());
                }
            }
            if (inci) {
                result += " inci " + std::to_string(inci.value());
            }
            return result;
        }
    };

    using PoolMap = std::map<ObjectKey, ObjectPtr>;

private:
    template <typename T>
    std::shared_ptr<const T> bindWithPartId(std::shared_ptr<const T> obj, Cmper requestedPartId) const
    {
        if constexpr (std::is_base_of_v<OthersBase, T> || std::is_base_of_v<DetailsBase, T>) {
            if (obj && obj->getRequestedPartId() != requestedPartId) {
                return PartContextCloner::copyWithPartId(obj, requestedPartId);
            }
        }
        return obj;
    }

    inline static ObjectKey makeEndKey(const ObjectKey& key)
    {
        return ObjectKey{
            key.typeId,
            key.nodeId,
            key.partId,
            key.cmper1.value_or((std::numeric_limits<Cmper>::max)()),
            key.cmper2.value_or((std::numeric_limits<Cmper>::max)()),
            key.inci.value_or((std::numeric_limits<Inci>::max)())
        };
    }

    template <typename T>
    inline static MusxInstance<T> checkedStaticCast([[maybe_unused]]const ObjectKey& key, const ObjectPtr& p)
    {
        static_assert(std::is_base_of_v<ObjectBaseType, T>, "T must derive from ObjectBaseType");
        if constexpr (!std::is_same_v<T, ObjectBaseType>) {
            assert(T::XmlNodeName == key.nodeId && "nodeId/type mismatch: pool invariant violated");
        }
    #ifndef NDEBUG
        // Optional paranoia while refactoring:
        //assert(std::dynamic_pointer_cast<T>(p));
    #endif
        return std::static_pointer_cast<T>(p);
    }

    inline static bool logicalLess(const ObjectKey& a, const ObjectKey& b)
    {
        return std::tie(a.cmper1, a.cmper2, a.inci) < std::tie(b.cmper1, b.cmper2, b.inci);
    }

    inline static bool logicalEq(const ObjectKey& a, const ObjectKey& b)
    {
        return a.cmper1 == b.cmper1 && a.cmper2 == b.cmper2 && a.inci == b.inci;
    }

public:
    /** @brief virtual destructor */
    virtual ~ObjectPool() = default;

    /// @brief Runs object-local integrity validation for every stored source object.
    /// @details This is called by the document construction finalizer after all overlays are complete.
    void integrityCheckAll() const
    {
        for (const auto& [key, object] : m_pool) {
            (void)key;
            object->integrityCheck(object);
        }
    }

    /**
     * @brief Adds an `ObjectBaseType` object to the pool.
     *
     * @param key The key with which to store the object
     * @param object A shared pointer to the `ObjectBaseType` object to add.
     */
    void add(ObjectKey key, ObjectPtr object)
    {
        if (key.inci.has_value() && key.inci.value() != 0) {
            // Incis arrive in order, so the prior inci is normally the node inserted just before this one.
            // Checking only the predecessor reports a gap once rather than again for every inci after it.
            ObjectKey priorKey = key;
            priorKey.inci = key.inci.value() - 1;
            const bool priorIsHint = m_insertionHint && !((*m_insertionHint)->first < priorKey) && !(priorKey < (*m_insertionHint)->first);
            if (!priorIsHint && m_pool.find(priorKey) == m_pool.end()) {
                MUSX_INTEGRITY_ERROR("Node " + std::string(key.nodeId) + " has inci " + std::to_string(key.inci.value()) + " that is out of sequence.");
            }
        }
        auto shareModeIt = m_shareMode.find(key.nodeId);
        const auto objectShareMode = object->getShareMode();
        const auto priorSize = m_pool.size();
        typename PoolMap::iterator poolIt;
        if (m_insertionHint
            && (*m_insertionHint)->first.typeId == key.typeId
            && (*m_insertionHint)->first < key) {
            poolIt = m_pool.emplace_hint(
                std::next(*m_insertionHint), std::move(key), std::move(object));
        } else {
            poolIt = m_pool.emplace(std::move(key), std::move(object)).first;
        }
        const bool emplaced = m_pool.size() != priorSize;
        m_insertionHint = poolIt;
        if (!emplaced) {
            MUSX_INTEGRITY_ERROR("Attempted to add same key more than once: " + poolIt->first.description());
        }
        if (shareModeIt == m_shareMode.end()) {
            m_shareMode.emplace(poolIt->first.nodeId, objectShareMode);
        } else if (objectShareMode != shareModeIt->second && objectShareMode != EnigmaBase::ShareMode::All) {
            if (shareModeIt->second == EnigmaBase::ShareMode::All) {
                m_shareMode[poolIt->first.nodeId] = objectShareMode;
            } else {
                MUSX_INTEGRITY_ERROR("Share mode for added " + std::string(poolIt->first.nodeId) + " object [" + std::to_string(int(objectShareMode))
                    + "] does not match previous [" + std::to_string(int(shareModeIt->second)) + "]");
            }
        }
    }

    /**
     * @brief Retrieves a vector of objects of a specific type from the pool.
     *
     * This function may be used to
     * - retrieve a multi-inci array
     * - get all the objects of a specific type, regardless of @ref Cmper value(s), such as getting a vector
     * of all the @ref others::TextExpressionDef instances.
     *
     * @tparam T The derived type of `ObjectBaseType` to retrieve.
     * @param key The key value used to filter the objects.
     * @param requestedPartId The part for which this array was requested.
     * @return An MusxInstanceList of shared pointers to objects of type `T`.
     */
    template <typename T>
    MusxInstanceList<T> getArray(const ObjectKey& key, Cmper requestedPartId) const
    {
        MusxInstanceList<T> result(m_document, requestedPartId);

        auto rangeStart = m_pool.lower_bound(key);
        auto rangeEnd = m_pool.upper_bound(makeEndKey(key));

        for (auto it = rangeStart; it != rangeEnd; ++it) {
            auto typedPtr = bindWithPartId<T>(checkedStaticCast<T>(key, it->second), requestedPartId);
            result.push_back(typedPtr);
        }
        return result;
    }

    /**
     * @brief Retrieves a vector of objects of a specific type from the pool.
     *
     * This function may be used to
     * - retrieve a multi-inci array
     * - get all the objects of a specific type, regardless of @ref Cmper value(s), such as getting a vector
     * of all the @ref others::TextExpressionDef instances.
     *
     * @tparam T The derived type of `ObjectBaseType` to retrieve.
     * @param key The key value used to filter the objects.
     * @return An MusxInstanceList of shared pointers to objects of type `T`.
     */
    template <typename T>
    MusxInstanceList<T> getArrayForPart(const ObjectKey& key) const
    {
        EnigmaBase::ShareMode forShareMode = EnigmaBase::ShareMode::All;
        if (key.partId != SCORE_PARTID) {
            auto it = m_shareMode.find(key.nodeId);
            // If the nodeId is not found in m_shareMode, it means the document contains no instances,
            // and every path will return an empty vector. We can safely ignore this case.
            if (it != m_shareMode.end()) {
                forShareMode = it->second;
            }
        }

        if (key.partId == SCORE_PARTID || forShareMode == EnigmaBase::ShareMode::None) {
            return getArray<T>(key, key.partId);
        }
        ObjectKey scoreKey(key);
        scoreKey.partId = SCORE_PARTID;
        if (forShareMode == EnigmaBase::ShareMode::All) {
            return getArray<T>(scoreKey, key.partId);
        }

        const auto partStart = m_pool.lower_bound(key);
        const auto partEnd   = m_pool.upper_bound(makeEndKey(key));

        const auto scoreStart = m_pool.lower_bound(scoreKey);
        const auto scoreEnd   = m_pool.upper_bound(makeEndKey(scoreKey));

        auto partIt = partStart;
        auto scoreIt = scoreStart;

        MusxInstanceList<T> result(m_document, key.partId);
        auto pushInstance = [&](const auto& it) {
            auto typed = bindWithPartId<T>(checkedStaticCast<T>(key, it->second), key.partId);
            result.push_back(typed);
        };
        while (partIt != partEnd || scoreIt != scoreEnd) {
            if (scoreIt == scoreEnd) {
                pushInstance(partIt++);
                continue;
            }
            if (partIt == partEnd) {
                pushInstance(scoreIt++); // bind score record to requested part
                continue;
            }
            const ObjectKey& pk = partIt->first;
            const ObjectKey& sk = scoreIt->first;
            if (logicalEq(pk, sk)) {
                pushInstance(partIt++); // prefer part instance
                scoreIt++;
            } else if (logicalLess(pk, sk)) {
                pushInstance(partIt++);
            } else {
                pushInstance(scoreIt++); // score fallback
            }
        }
        return result;
    }

    /**
     * @brief Retrieves the first (and usually only) object of a specific type from the pool.
     * @warning The returned value is the source item from the pool and is guaranteed not to be a copy.
     * Because of this, it does not reflect the requested part id. External callers should not
     * directly call this function.
     *
     * @tparam T The derived type of `OthersBase` to retrieve.
     *           Must have a `constexpr static std::string_view XmlNodeName` member.
     * @param key The key value used to filter the objects.
     * @return A shared_ptr to the type or nullptr if none exists
     */
    template <typename T>
    MusxInstance<T> getSource(const ObjectKey& key) const
    {
        auto it = m_pool.find(key);
        if (it == m_pool.end()) {
            return nullptr;
        }
        return checkedStaticCast<T>(key, it->second);
    }

    /**
     * @brief Returns physically stored objects of one type across every source part.
     * @param cmper1 If present, restricts the result to this first comparator.
     * @param cmper2 If present, restricts the result to this second comparator.
     * @param inci If present, restricts the result to this incidence.
     * @return Source objects in key order, without sharing fallbacks or part-context copies.
     */
    template <typename T>
    std::vector<MusxInstance<T>> getAllSources(
        std::optional<Cmper> cmper1 = std::nullopt,
        std::optional<Cmper> cmper2 = std::nullopt,
        std::optional<Inci> inci = std::nullopt) const
    {
        const ObjectKey lowerKey{std::type_index(typeid(T)), T::XmlNodeName, SCORE_PARTID, cmper1, cmper2, inci};
        const auto upperKey = [&]() -> ObjectKey {
            auto result = makeEndKey(lowerKey);
            result.partId = (std::numeric_limits<Cmper>::max)();
            return result;
        }();
        std::vector<MusxInstance<T>> result;
        for (auto it = m_pool.lower_bound(lowerKey); it != m_pool.upper_bound(upperKey); ++it) {
            const auto& key = it->first;
            if (cmper1 && key.cmper1 != cmper1) continue;
            if (cmper2 && key.cmper2 != cmper2) continue;
            if (inci && key.inci != inci) continue;
            result.push_back(checkedStaticCast<T>(key, it->second));
        }
        return result;
    }

    /**
     * @brief Retrieves the first (and usually only) object of a specific type from the pool for a part
     * @warning The returned value is the source item from the pool and is guaranteed not to be a copy.
     * Because of this, it does not reflect the requested part id. External callers should not
     * directly call this function.
     *
     * If no part item exists, returns the score item.
     *
     * @tparam T The derived type of `OthersBase` to retrieve.
     *           Must have a `constexpr static std::string_view XmlNodeName` member.
     * @param key The key value used to filter the objects.
     * @return A shared_ptr to the type or nullptr if none exists
     */
    template <typename T>
    MusxInstance<T> getEffectiveSourceForPart(const ObjectKey& key) const
    {
        if (auto partVersion = getSource<T>(key)) {
            return partVersion;
        }
        if (key.partId == SCORE_PARTID) {
            // if this is already the score version, there is nothing to return.
            return nullptr;
        } else {
            // if this is for a part, do not search score for unshared types.
            auto it = m_shareMode.find(key.nodeId);
            if (it != m_shareMode.end() && it->second == EnigmaBase::ShareMode::None) {
                return nullptr;
            }
        }
        ObjectKey scoreKey(key);
        scoreKey.partId = SCORE_PARTID;
        return getSource<T>(scoreKey);
    }

    /**
     * @brief Retrieves the first (and usually only) object of a specific type from the pool for a part.
     *
     * If no part item exists, returns the score item.
     *
     * If the requestedPartId differs from the sourcePartId, a copy of the source is returned with the
     * requestedPartId set correctly.
     *
     * @tparam T The derived type of `OthersBase` to retrieve.
     *           Must have a `constexpr static std::string_view XmlNodeName` member.
     * @param key The key value used to filter the objects.
     * @return A shared_ptr to the type or nullptr if none exists
     */
    template <typename T>
    MusxInstance<T> getEffectiveForPart(const ObjectKey& key) const
    {
        return bindWithPartId<T>(getEffectiveSourceForPart<T>(key), key.partId);
    }

    /// @brief Returns an identifier not currently used in @p items.
    /// @details The identifier is one past the highest in use. When the highest possible identifier
    /// is in use, it is instead the lowest unused identifier above zero.
    /// @tparam T The type stored in @p items.
    /// @tparam IdGetter A callable that returns an item's @ref Cmper identifier.
    /// @param items The items whose identifiers are being counted, in ascending identifier order,
    /// as a pool's @c getArray returns them. An identifier may repeat.
    /// @param getId The callable used to obtain each identifier.
    /// @return The next free identifier, or std::nullopt when every identifier above zero is in use.
    template <typename T, typename IdGetter>
    static std::optional<Cmper> nextFreeCmper(
        const MusxInstanceList<T>& items, IdGetter getId)
    {
        constexpr Cmper maxCmper = (std::numeric_limits<Cmper>::max)();
        if (items.empty()) {
            return Cmper(1);
        }
        const Cmper highest = std::invoke(getId, *items.back());
        if (highest != maxCmper) {
            return Cmper(highest + 1);
        }
        // A gap may be an identifier a stale reference still names, so gaps are reused only once
        // the top of the space is taken. The items are in order, so the first skipped identifier
        // is the lowest free one.
        Cmper expected = 1;
        for (const auto& item : items) {
            const Cmper id = std::invoke(getId, *item);
            if (id > expected) {
                return expected;
            }
            if (id == expected) {
                if (expected == maxCmper) {
                    return std::nullopt;
                }
                ++expected;
            }
        }
        return expected;
    }

    /// @brief Constructs the object pool
    /// @param document THe document for this pool.
    /// @param knownShareModes Optional parameter that specifies known share modes for certain elements.
    /// These can be particurly important for EnigmaBase::ShareMode::None because there may be no parts containing them.
    ObjectPool(const DocumentWeakPtr& document, const std::unordered_map<std::string_view, dom::EnigmaBase::ShareMode>& knownShareModes = {})
        : m_document(document), m_shareMode(knownShareModes) {}

    /// @brief Copies the stored objects without retaining an iterator into the source pool.
    ObjectPool(const ObjectPool& other)
        : m_document(other.m_document), m_shareMode(other.m_shareMode), m_pool(other.m_pool) {}

    /// @brief Copies the stored objects without retaining an iterator into the source pool.
    ObjectPool& operator=(const ObjectPool& other)
    {
        if (this != &other) {
            m_document = other.m_document;
            m_shareMode = other.m_shareMode;
            m_pool = other.m_pool;
            m_insertionHint.reset();
        }
        return *this;
    }

    ObjectPool(ObjectPool&&) = default;
    ObjectPool& operator=(ObjectPool&&) = default;

private:
    DocumentWeakPtr m_document;
    std::unordered_map<std::string_view, dom::EnigmaBase::ShareMode> m_shareMode;

    PoolMap m_pool;
    std::optional<typename PoolMap::iterator> m_insertionHint;
};

/**
 * @class OptionsPool
 * @brief A pool that manages collections of `OptionsBase` objects that have no Cmper value.
 */
class OptionsPool
{
    ObjectPool<OptionsBase> m_pool;
    friend class bench::PoolAccessor<OptionsPool>;

public:
    /// @brief Constructor function
    OptionsPool(const DocumentWeakPtr& document) : m_pool(document) {}

    /** @brief Scalar version of #ObjectPool::add */
    void add(std::string_view nodeName, std::shared_ptr<OptionsBase> instance)
    {
        const EnigmaBase* basePtr = instance.get();
        if (basePtr->getSourcePartId() != SCORE_PARTID) {
            MUSX_INTEGRITY_ERROR("Options node " + std::string(nodeName) + " hase non-score part id [" + std::to_string(basePtr->getSourcePartId()) + "]");
        }
        const ObjectPool<OptionsBase>::ObjectKey key{
            std::type_index(typeid(*basePtr)), nodeName, basePtr->getSourcePartId()};
        m_pool.add(key, std::move(instance));
    }

    /// @brief Validates every option object. Called by document construction finalization.
    void integrityCheckAll() const { m_pool.integrityCheckAll(); }

    /** @brief Scalar version of #ObjectPool::getArray */
    template <typename T>
    MusxInstanceList<T> getArray() const
    {
        static_assert(is_pool_type_v<OptionsPool, T>, "Type T is not registered in OptionsPool");
        return m_pool.getArray<T>({ std::type_index(typeid(T)), T::XmlNodeName,
            SCORE_PARTID }, SCORE_PARTID);
    }

    /** @brief Get a single item out of the pool */
    template <typename T>
    MusxInstance<T> get() const
    {
        static_assert(is_pool_type_v<OptionsPool, T>, "Type T is not registered in OptionsPool");
        return m_pool.getSource<T>({ std::type_index(typeid(T)), T::XmlNodeName,
            SCORE_PARTID });
    }
};
/** @brief Shared `OptionsPool` pointer */
using OptionsPoolPtr = std::shared_ptr<OptionsPool>;

/**
 * @class OthersPool
 * @brief A pool that manages collections of `OthersBase` objects.
 */
class OthersPool
{
    ObjectPool<OthersBase> m_pool;
    friend class bench::PoolAccessor<OthersPool>;
    friend MusxInstance<others::StaffComposite> others::StaffComposite::createCurrent(const DocumentPtr& document, Cmper partId, StaffCmper staffId, MeasCmper measId, Edu eduPosition);

    MusxInstance<others::Staff> getRawStaff(Cmper partId, StaffCmper staffId)
    {
        return m_pool.getEffectiveSourceForPart<others::Staff>({
            std::type_index(typeid(others::Staff)), others::Staff::XmlNodeName,
            partId, staffId, std::nullopt, std::nullopt });
    }

public:
    /// @brief Constructor
    OthersPool(const DocumentWeakPtr& document) : m_pool(document, {
        { others::BeatChartElement::XmlNodeName, EnigmaBase::ShareMode::None },
        { others::StaffUsed::XmlNodeName, EnigmaBase::ShareMode::None },
        { others::SystemLock::XmlNodeName, EnigmaBase::ShareMode::None },
        { others::MultimeasureRest::XmlNodeName, EnigmaBase::ShareMode::None },
        { others::MultiStaffGroupId::XmlNodeName, EnigmaBase::ShareMode::None },
        { others::Page::XmlNodeName, EnigmaBase::ShareMode::None },
        { others::PartGlobals::XmlNodeName, EnigmaBase::ShareMode::None },
        { others::PartVoicing::XmlNodeName, EnigmaBase::ShareMode::None },
        { others::StaffSystem::XmlNodeName, EnigmaBase::ShareMode::None },
        { others::StaffStyleAssign::XmlNodeName, EnigmaBase::ShareMode::None },
        // add other known sharemode none items as they are identified.
    }) {}

    /** @brief OthersPool version of #ObjectPool::add */
    void add(std::string_view nodeName, std::shared_ptr<OthersBase> instance)
    {
        const OthersBase* instancePtr = instance.get();
        const ObjectPool<OthersBase>::ObjectKey key{
            std::type_index(typeid(*instancePtr)), nodeName,
            instance->getSourcePartId(), instance->getCmper(),
            std::nullopt, instance->getInci()};
        m_pool.add(key, std::move(instance));
    }

    /// @brief Validates every others object. Called by document construction finalization.
    void integrityCheckAll() const { m_pool.integrityCheckAll(); }

    /** @brief OthersPool version of #ObjectPool::getArray */
    template <typename T>
    MusxInstanceList<T> getArray(Cmper partId, std::optional<Cmper> cmper = std::nullopt) const
    {
        static_assert(is_pool_type_v<OthersPool, T>, "Type T is not registered in OthersPool");
        return m_pool.getArrayForPart<T>({ std::type_index(typeid(T)),
            T::XmlNodeName, partId, cmper });
    }

    /** @brief Get a single item out of the pool */
    template <typename T>
    MusxInstance<T> get(Cmper partId, Cmper cmper, std::optional<Inci> inci = std::nullopt) const
    {
        static_assert(is_pool_type_v<OthersPool, T>, "Type T is not registered in OthersPool");
        return m_pool.getEffectiveForPart<T>({ std::type_index(typeid(T)),
            T::XmlNodeName, partId, cmper, std::nullopt, inci });
    }

    /** @brief Returns every physically stored source object of type @p T. */
    template <typename T>
    std::vector<MusxInstance<T>> getAllSources() const
    {
        static_assert(is_pool_type_v<OthersPool, T>, "Type T is not registered in OthersPool");
        return m_pool.template getAllSources<T>();
    }

    /**
     * @brief Returns physically stored source objects for one comparator and optional incidence.
     * @param cmper The comparator to select.
     * @param inci If present, restricts the result to this incidence.
     * @return Source objects in part and incidence order, without sharing fallbacks or copies.
     */
    template <typename T>
    std::vector<MusxInstance<T>> getAllSources(
        Cmper cmper, std::optional<Inci> inci = std::nullopt) const
    {
        static_assert(is_pool_type_v<OthersPool, T>, "Type T is not registered in OthersPool");
        return m_pool.template getAllSources<T>(cmper, std::nullopt, inci);
    }

    /// @brief The cmper one past the highest currently used for @c T.
    ///
    /// Each type numbers independently, so a cmper free for one type says nothing about another.
    ///
    /// @tparam T The type whose cmpers are being counted.
    /// @param partId The part to search.
    /// @return The next free cmper, or std::nullopt when the cmper space for @c T is exhausted.
    template <typename T>
    std::optional<Cmper> nextFreeCmper(Cmper partId) const
    {
        static_assert(is_pool_type_v<OthersPool, T>, "Type T is not registered in OthersPool");
        return m_pool.nextFreeCmper(getArray<T>(partId), &OthersBase::getCmper);
    }
};
/** @brief Shared `OthersPool` pointer */
using OthersPoolPtr = std::shared_ptr<OthersPool>;

/**
 * @class DetailsPool
 * @brief A pool that manages collections of `DetailsBase` objects, organized by XML node names and `Cmper` values.
 *
 * Examples of `OneCmperBase` classes are @ref OthersBase and @ref TextsBase.
 */
class DetailsPool
{
    ObjectPool<DetailsBase> m_pool;
    friend class bench::PoolAccessor<DetailsPool>;

public:
    /// @brief Constructor
    DetailsPool(const DocumentWeakPtr& document) : m_pool(document, {
        { details::StaffGroup::XmlNodeName, EnigmaBase::ShareMode::None },
        { details::StaffSize::XmlNodeName, EnigmaBase::ShareMode::None },
        // add other known sharemode none items as they are identified.
    }) {}

    /** @brief DetailsPool version of #ObjectPool::add */
    void add(std::string_view nodeName, std::shared_ptr<DetailsBase> instance)
    {
        const DetailsBase* instancePtr = instance.get();
        const ObjectPool<DetailsBase>::ObjectKey key{
            std::type_index(typeid(*instancePtr)), nodeName,
            instance->getSourcePartId(), instance->getCmper1(),
            instance->getCmper2(), instance->getInci()};
        m_pool.add(key, std::move(instance));
    }

    /// @brief Validates every details object. Called by document construction finalization.
    void integrityCheckAll() const { m_pool.integrityCheckAll(); }

    /** @brief version of #ObjectPool::getArray for getting all of them */
    template <typename T, typename = std::enable_if_t<is_pool_type_v<DetailsPool, T>>>
    MusxInstanceList<T> getArray(Cmper partId) const
    { return m_pool.template getArrayForPart<T>({ std::type_index(typeid(T)), T::XmlNodeName, partId }); }

    /** @brief DetailsPool version of #ObjectPool::getArray */
    template <typename T, typename std::enable_if_t<!std::is_base_of_v<EntryDetailsBase, T>, int> = 0>
    MusxInstanceList<T> getArray(Cmper partId, Cmper cmper1, std::optional<Cmper> cmper2 = std::nullopt) const
    {
        static_assert(is_pool_type_v<DetailsPool, T>, "Type T is not registered in DetailsPool");
        return m_pool.template getArrayForPart<T>({ std::type_index(typeid(T)),
            T::XmlNodeName, partId, cmper1, cmper2 });
    }

    /** @brief EntryDetailsPool version of #ObjectPool::getArray */
    template <typename T, typename std::enable_if_t<std::is_base_of_v<EntryDetailsBase, T>, int> = 0>
    MusxInstanceList<T> getArray(Cmper partId, EntryNumber entnum) const
    {
        static_assert(is_pool_type_v<DetailsPool, T>, "Type T is not registered in DetailsPool");
        return m_pool.template getArrayForPart<T>({ std::type_index(typeid(T)),
            T::XmlNodeName, partId, Cmper(entnum >> 16), Cmper(entnum & 0xffff) });
    }

    /** @brief Get a single DetailsBase item out of the pool (not EntryDetailsBase) */
    template <typename T, typename std::enable_if_t<!std::is_base_of_v<EntryDetailsBase, T>, int> = 0>
    MusxInstance<T> get(Cmper partId, Cmper cmper1, Cmper cmper2, std::optional<Inci> inci = std::nullopt) const
    {
        static_assert(is_pool_type_v<DetailsPool, T>, "Type T is not registered in DetailsPool");
        return m_pool.getEffectiveForPart<T>({ std::type_index(typeid(T)),
            T::XmlNodeName, partId, cmper1, cmper2, inci });
    }

    /** @brief Returns every physically stored source object of type @p T. */
    template <typename T>
    std::vector<MusxInstance<T>> getAllSources() const
    {
        static_assert(is_pool_type_v<DetailsPool, T>, "Type T is not registered in DetailsPool");
        return m_pool.template getAllSources<T>();
    }

    /**
     * @brief Returns physically stored source objects for one comparator pair and optional incidence.
     * @param cmper1 The first comparator to select.
     * @param cmper2 The second comparator to select.
     * @param inci If present, restricts the result to this incidence.
     * @return Source objects in part and incidence order, without sharing fallbacks or copies.
     */
    template <typename T>
    std::vector<MusxInstance<T>> getAllSources(
        Cmper cmper1, Cmper cmper2, std::optional<Inci> inci = std::nullopt) const
    {
        static_assert(is_pool_type_v<DetailsPool, T>, "Type T is not registered in DetailsPool");
        return m_pool.template getAllSources<T>(cmper1, cmper2, inci);
    }

    /** @brief Get a single EntryDetailsBase item out of the pool */
    template <typename T, typename std::enable_if_t<std::is_base_of_v<EntryDetailsBase, T>, int> = 0>
    MusxInstance<T> get(Cmper partId, EntryNumber entnum, std::optional<Inci> inci = std::nullopt) const
    {
        static_assert(is_pool_type_v<DetailsPool, T>, "Type T is not registered in DetailsPool");
        return m_pool.getEffectiveForPart<T>({ std::type_index(typeid(T)),
            T::XmlNodeName, partId, Cmper(entnum >> 16), Cmper(entnum & 0xffff), inci });
    }

    /// @brief Returns the detail for a particular note
    /// @tparam T The type to retrieve (must be derived from @ref NoteDetailsBase)
    /// @tparam enable_if_t Enforces T as a base of @ref NoteDetailsBase
    /// @param noteInfo The note for which to get the note detail
    /// @param forPartId The part for which to get the note detail. If omitted, the @p noteInfo part is used.
    /// @return The instance associated with @p noteInfo or nullptr if none.
    template <typename T, typename std::enable_if_t<std::is_base_of_v<NoteDetailsBase, T>, int> = 0>
    MusxInstance<T> getForNote(const NoteInfoPtr& noteInfo, const std::optional<Cmper>& forPartId = std::nullopt)
    {
        static_assert(is_pool_type_v<DetailsPool, T>, "Type T is not registered in DetailsPool");
        auto details = getArray<T>(
            forPartId.value_or(noteInfo.getEntryInfo().getFrame()->getRequestedPartId()),
            noteInfo.getEntryInfo()->getEntry()->getEntryNumber()
        );
        for (const auto& detail : details) {
            if (detail->getNoteId() == noteInfo->getNoteId()) {
                return detail;
            }
        }
        return nullptr;
    }
};

/** @brief Shared `DetailsPool` pointer */
using DetailsPoolPtr = std::shared_ptr<DetailsPool>;

/** @brief Entry pool */
class EntryPool // uses different implementation than other pools for more efficient access
{
public:
    /// @brief Constructor function
    EntryPool(const DocumentWeakPtr& document) : m_document(document) {}

    /** @brief Returns whether the pool contains no entries. */
    bool empty() const { return m_pool.empty(); }

    /** @brief Add an entry to the EntryPool. (Used by the factory.) */
    void add(EntryNumber entryNumber, std::shared_ptr<Entry> instance)
    {
        auto [it, emplaced] = m_pool.emplace(entryNumber, std::move(instance));
        if (!emplaced) {
            MUSX_INTEGRITY_ERROR("Entry number " + std::to_string(entryNumber) + " added twice.");
        }
    }

    /// @brief Validates every entry in entry-number order.
    void integrityCheckAll() const
    {
        std::vector<EntryNumber> entryNumbers;
        entryNumbers.reserve(m_pool.size());
        for (const auto& [entryNumber, entry] : m_pool) {
            (void)entry;
            entryNumbers.push_back(entryNumber);
        }
        std::sort(entryNumbers.begin(), entryNumbers.end());
        for (const auto entryNumber : entryNumbers) {
            const auto& entry = m_pool.at(entryNumber);
            entry->integrityCheck(entry);
        }
    }

    /// @brief Iterates every stored entry once, including entries not referenced by a frame.
    /// @tparam Iterator A callable that accepts an entry and returns a bool.
    /// @param iterator Called for each entry; return false to stop iteration.
    /// @return True if all entries were visited, or false if iteration stopped early.
    /// @note Iteration order is unspecified.
    template <typename Iterator>
    bool iterateRawEntries(Iterator&& iterator) const
    {
        for (const auto& [entryNumber, entry] : m_pool) {
            (void)entryNumber;
            if (!iterator(entry)) {
                return false;
            }
        }
        return true;
    }

    /** @brief Get an entry from the EntryPool. */
    MusxInstance<Entry> get(EntryNumber entryNumber) const
    {
        const auto it = m_pool.find(entryNumber);
        if (it == m_pool.end()) {
            return nullptr;
        }
        return it->second;
    }

private:
    DocumentWeakPtr m_document;
    std::unordered_map<EntryNumber, std::shared_ptr<Entry>> m_pool;

    friend class bench::PoolAccessor<EntryPool>;
};
/** @brief Shared `EntryPool` pointer */
using EntryPoolPtr = std::shared_ptr<EntryPool>;

/** @brief Text pool */
class TextsPool
{
    ObjectPool<TextsBase> m_pool;
    friend class bench::PoolAccessor<TextsPool>;

public:
    /// @brief Constructor fundtion
    TextsPool(const DocumentWeakPtr& document) : m_pool(document) {}

    /** @brief Texts version of #ObjectPool::add */
    void add(std::string_view nodeName, std::shared_ptr<TextsBase> instance)
    {
        const EnigmaBase* basePtr = instance.get();
        if (basePtr->getSourcePartId() != SCORE_PARTID) {
            MUSX_INTEGRITY_ERROR("Texts node " + std::string(nodeName) + " hase non-score part id [" + std::to_string(basePtr->getSourcePartId()) + "]");
        }
        const TextsBase* instancePtr = instance.get();
        const ObjectPool<TextsBase>::ObjectKey key{
            std::type_index(typeid(*instancePtr)), nodeName,
            basePtr->getSourcePartId(), instance->getTextNumber()};
        m_pool.add(key, std::move(instance));
    }

    /// @brief Validates every text object. Called by document construction finalization.
    void integrityCheckAll() const { m_pool.integrityCheckAll(); }

    /** @brief Texts version of #ObjectPool::getArray */
    template <typename T>
    MusxInstanceList<T> getArray(std::optional<Cmper> cmper = std::nullopt) const
    {
        static_assert(is_pool_type_v<TextsPool, T>, "Type T is not registered in TextsPool");
        return m_pool.getArray<T>({ std::type_index(typeid(T)), T::XmlNodeName,
            SCORE_PARTID, cmper }, SCORE_PARTID);
    }

    /** @brief Get a single item out of the pool */
    template <typename T>
    MusxInstance<T> get(Cmper cmper) const
    {
        static_assert(is_pool_type_v<TextsPool, T>, "Type T is not registered in TextsPool");
        return m_pool.getSource<T>({ std::type_index(typeid(T)), T::XmlNodeName,
            SCORE_PARTID, cmper, std::nullopt, std::nullopt });
    }

    /// @brief The text number one past the highest currently used for @c T.
    ///
    /// Each text type numbers independently, so a number free for one type says nothing about
    /// another.
    ///
    /// @tparam T The text type whose numbers are being counted.
    /// @return The next free text number, or std::nullopt when the number space for @c T is
    /// exhausted.
    template <typename T>
    std::optional<Cmper> nextFreeCmper() const
    {
        static_assert(is_pool_type_v<TextsPool, T>, "Type T is not registered in TextsPool");
        return m_pool.nextFreeCmper(getArray<T>(), &TextsBase::getTextNumber);
    }
};
/** @brief Shared `OthersPool` pointer */
using TextsPoolPtr = std::shared_ptr<TextsPool>;

} // namespace dom
} // namespace musx
