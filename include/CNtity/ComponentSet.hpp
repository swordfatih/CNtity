/////////////////////////////////////////////////////////////////////////////////
//
// CNtity - Chats Noirs Entity Component System Helper
// Copyright (c) 2018 - 2025 Fatih (accfldekur@gmail.com)
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <cassert>
#include <cstdint>
#include <vector>

namespace CNtity
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Entity identifier type
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using Entity = uint64_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Slice type used to store either index or generation part of an Entity
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using EntitySlice = uint32_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Unsigned size / capacity related type alias
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using size_type = std::size_t;

////////////////////////////////////////////////////////////
/// \brief Extract the index part from a packed Entity
///
/// The entity identifier is assumed to be composed of two
/// EntitySlice values: lower bits for index, higher bits for
/// generation.
///
/// \param entity Packed entity identifier
///
/// \return Index portion
///
////////////////////////////////////////////////////////////
inline EntitySlice get_index(Entity entity)
{
    return static_cast<EntitySlice>(entity & static_cast<EntitySlice>(-1));
}

////////////////////////////////////////////////////////////
/// \brief Extract the generation part from a packed Entity
///
/// \param entity Packed entity identifier
///
/// \return Generation portion
///
////////////////////////////////////////////////////////////
inline EntitySlice get_generation(Entity entity)
{
    return static_cast<EntitySlice>(entity >> (sizeof(EntitySlice) * 8));
}

////////////////////////////////////////////////////////////
/// \brief Interface of a homogeneous component storage
///
/// Provides polymorphic access used by Helper to manipulate
/// different component pools without knowing their concrete
/// component type.
///
////////////////////////////////////////////////////////////
class IComponentSet
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Virtual destructor
    ////////////////////////////////////////////////////////////
    virtual ~IComponentSet() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Remove a component for an entity if present
    ///
    /// \param entity Entity identifier
    ///
    ////////////////////////////////////////////////////////////
    virtual void remove(Entity entity) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Remove all components (reset container)
    ////////////////////////////////////////////////////////////
    virtual void clear() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether an entity has a component in this set
    ///
    /// \param entity Entity identifier
    ///
    /// \return True if present
    ///
    ////////////////////////////////////////////////////////////
    virtual bool has(Entity entity) const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Copy component from one entity to another
    ///
    /// Copies the component attached to \p source into
    /// \p destination (overwriting or inserting as needed).
    ///
    /// \param source Source entity
    /// \param destination Destination entity
    ///
    ////////////////////////////////////////////////////////////
    virtual void copy(Entity source, Entity destination) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Number of stored components/entities
    ///
    /// \return Count
    ///
    ////////////////////////////////////////////////////////////
    virtual size_type size() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Access list of entities stored in this set
    ///
    /// \return Reference to internal entity vector
    ///
    ////////////////////////////////////////////////////////////
    virtual std::vector<Entity>& entities() = 0;
};

////////////////////////////////////////////////////////////
/// \brief Sparse-set component container
///
/// Stores components of a single type in a densely packed
/// array while providing O(1) insertion, lookup and removal.
/// Uses three arrays:
/// - m_sparse: maps entity index -> dense position or invalid
/// - m_dense:  dense array of entity identifiers
/// - m_values: parallel array of Component instances
///
/// Removal keeps arrays compact by swapping with last.
///
////////////////////////////////////////////////////////////
template <typename Component>
class ComponentSet : public IComponentSet
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Insert or replace a component for an entity
    ///
    /// \param entity Entity identifier
    /// \param component Component instance to insert
    ///
    /// \return True if inserted new component, false if replaced
    ///
    ////////////////////////////////////////////////////////////
    bool insert(Entity entity, const Component& component)
    {
        const size_t index = get_index(entity);

        if(has(entity))
        {
            m_values[m_sparse[index]] = component;
            return false;
        }

        if(index >= m_sparse.size())
        {
            m_sparse.resize(index + 1, invalid_index());
        }

        m_sparse[index] = m_dense.size();
        m_dense.push_back(entity);
        m_values.push_back(component);

        return true;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove component for an entity if present
    ///
    /// \param entity Entity identifier
    ///
    ////////////////////////////////////////////////////////////
    void remove(Entity entity) override
    {
        if(!has(entity))
        {
            return;
        }

        const size_type dense_index = m_sparse[get_index(entity)];
        const size_type last_index = m_dense.size() - 1;

        if(dense_index != last_index)
        {
            const auto last_entity = m_dense[last_index];

            m_dense[dense_index] = last_entity;
            m_values[dense_index] = std::move(m_values[last_index]);

            m_sparse[get_index(last_entity)] = dense_index;
        }

        m_dense.pop_back();
        m_values.pop_back();
        m_sparse[get_index(entity)] = invalid_index();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if a component for an entity exists
    ///
    /// \param entity Entity identifier
    ///
    /// \return True if present
    ///
    ////////////////////////////////////////////////////////////
    bool has(Entity entity) const override
    {
        const size_type index = get_index(entity);
        return index < m_sparse.size() &&
               m_sparse[index] != invalid_index() &&
               m_dense[m_sparse[index]] == entity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get mutable component reference
    ///
    /// \param entity Entity identifier
    ///
    /// \return Reference to component
    ///
    /// \pre Entity must have the component (asserted)
    ///
    ////////////////////////////////////////////////////////////
    Component& get(Entity entity)
    {
        assert(has(entity) && "Entity does not exist in ComponentSet");
        return m_values[m_sparse[get_index(entity)]];
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get const component reference
    ///
    /// \param entity Entity identifier
    ///
    /// \return Const reference to component
    ///
    /// \pre Entity must have the component (asserted)
    ///
    ////////////////////////////////////////////////////////////
    const Component& get(Entity entity) const
    {
        assert(has(entity) && "Entity does not exist in ComponentSet");
        return m_values[m_sparse[get_index(entity)]];
    }

    ////////////////////////////////////////////////////////////
    /// \brief Clear all stored components and entities
    ////////////////////////////////////////////////////////////
    void clear() override
    {
        m_sparse = {};
        m_dense = {};
        m_values = {};
    }

    ////////////////////////////////////////////////////////////
    /// \brief Copy component from source entity to destination
    ///
    /// \param source Source entity
    /// \param destination Destination entity
    ///
    ////////////////////////////////////////////////////////////
    void copy(Entity source, Entity destination) override
    {
        if(has(source))
        {
            insert(destination, get(source));
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Reserve capacity for components / entities
    ///
    /// \param capacity Anticipated number of components
    ///
    ////////////////////////////////////////////////////////////
    void reserve(size_type capacity)
    {
        m_dense.reserve(capacity);
        m_values.reserve(capacity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Number of stored components
    ///
    /// \return Count
    ///
    ////////////////////////////////////////////////////////////
    size_type size() const override
    {
        return m_values.size();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if no components are stored
    ///
    /// \return True if empty
    ///
    ////////////////////////////////////////////////////////////
    bool empty() const
    {
        return m_values.empty();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get const reference to entity list (dense order)
    ///
    /// \return Const vector reference
    ///
    ////////////////////////////////////////////////////////////
    const std::vector<Entity>& entities() const
    {
        return m_dense;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get mutable reference to entity list (dense order)
    ///
    /// \return Vector reference
    ///
    ////////////////////////////////////////////////////////////
    std::vector<Entity>& entities() override
    {
        return m_dense;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get const reference to component storage
    ///
    /// \return Const vector reference
    ///
    ////////////////////////////////////////////////////////////
    const std::vector<Component>& values() const
    {
        return m_values;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get mutable reference to component storage
    ///
    /// \return Vector reference
    ///
    ////////////////////////////////////////////////////////////
    std::vector<Component>& values()
    {
        return m_values;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Forward iterator over (Entity, Component) pairs
    ///
    /// Dereferencing returns a std::tie(entity, component).
    ///
    ////////////////////////////////////////////////////////////
    struct Iterator
    {
        size_type     index; ///< Current dense index
        ComponentSet* set;   ///< Pointer to parent container

        ////////////////////////////////////////////////////////
        /// \brief Inequality comparison
        ////////////////////////////////////////////////////////
        bool operator!=(const Iterator& other) const { return index != other.index; }

        ////////////////////////////////////////////////////////
        /// \brief Pre-increment
        ////////////////////////////////////////////////////////
        void operator++() { ++index; }

        ////////////////////////////////////////////////////////
        /// \brief Dereference to (Entity, Component&) tuple
        ////////////////////////////////////////////////////////
        auto operator*() { return std::tie(set->m_dense[index], set->m_values[index]); }
    };

    ////////////////////////////////////////////////////////////
    /// \brief Begin iterator
    ///
    /// \return Iterator at first element
    ///
    ////////////////////////////////////////////////////////////
    Iterator begin()
    {
        return {0, this};
    }

    ////////////////////////////////////////////////////////////
    /// \brief End iterator
    ///
    /// \return Iterator past last element
    ///
    ////////////////////////////////////////////////////////////
    Iterator end()
    {
        return {m_dense.size(), this};
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Sentinel invalid index value
    ///
    /// \return Value representing an invalid sparse entry
    ///
    ////////////////////////////////////////////////////////////
    static constexpr size_type invalid_index()
    {
        return static_cast<size_type>(-1);
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::vector<size_type> m_sparse; ///< Sparse: entity index -> dense position or invalid
    std::vector<Entity>    m_dense;  ///< Dense array of entity identifiers
    std::vector<Component> m_values; ///< Dense array of component values (parallel to m_dense)
};

} // namespace CNtity