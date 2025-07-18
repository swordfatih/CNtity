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

#include "ComponentSet.hpp"

namespace CNtity
{

////////////////////////////////////////////////////////////
/// \brief Manages allocation, recycling and validation of entities
///
/// Entity identifiers are composed of two slices (index
/// and generation) packed together. Destroyed entity indices
/// are recycled; their generation counters are incremented
/// so stale identifiers can be detected as invalid.
///
////////////////////////////////////////////////////////////
class EntitySet
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create (allocate) a new entity
    ///
    /// Reuses an index from the depot if available; otherwise
    /// appends a new index with generation 0. The resulting
    /// identifier is inserted into an internal ComponentSet
    /// for fast existence queries.
    ///
    /// \return Newly created entity identifier
    ///
    ////////////////////////////////////////////////////////////
    Entity create()
    {
        Entity entity;

        if(m_depot.empty())
        {
            auto index = m_generations.size();
            m_generations.push_back(0);
            entity = make_entity(index, 0);
        }
        else
        {
            auto index = m_depot.back();
            m_depot.pop_back();
            entity = make_entity(index, m_generations[index]);
        }

        m_entities.insert(entity, {});

        return entity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if an entity currently exists
    ///
    /// \param entity Entity identifier
    ///
    /// \return True if the entity is alive (allocated and not removed)
    ///
    ////////////////////////////////////////////////////////////
    bool has(Entity entity) const
    {
        return m_entities.has(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Destroy (remove) an entity
    ///
    /// Increments the generation counter associated with the
    /// entity's index, recycles the index by pushing it into
    /// the depot, and erases the entity from the internal set.
    /// Passing an already invalid / stale entity is ignored.
    ///
    /// \param entity Entity identifier
    ///
    ////////////////////////////////////////////////////////////
    void remove(Entity entity)
    {
        if(get_index(entity) < m_generations.size())
        {
            m_generations[get_index(entity)]++;
            m_depot.push_back(get_index(entity));
            m_entities.remove(entity);
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get immutable view of living entities
    ///
    /// \return Const reference to dense vector of entities
    ///
    ////////////////////////////////////////////////////////////
    const std::vector<Entity>& entities() const
    {
        return m_entities.entities();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get mutable view of living entities
    ///
    /// \return Reference to dense vector of entities
    ///
    ////////////////////////////////////////////////////////////
    std::vector<Entity>& entities()
    {
        return m_entities.entities();
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Pack index and generation into an Entity identifier
    ///
    /// \param index Entity index
    /// \param generation Generation counter
    ///
    /// \return Packed entity identifier
    ///
    ////////////////////////////////////////////////////////////
    static Entity make_entity(EntitySlice index, EntitySlice generation)
    {
        return (static_cast<Entity>(generation) << (sizeof(EntitySlice) * 8)) | index;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::vector<EntitySlice> m_generations; ///< Generation per index (increments on recycle)
    std::vector<EntitySlice> m_depot;       ///< Recycled (free) indices
    ComponentSet<uint8_t>    m_entities;    ///< Set of currently alive entities (value payload unused)
};

} // namespace CNtity