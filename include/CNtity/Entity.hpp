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

#include "Component.hpp"

namespace CNtity
{


class Entities
{
public:
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
    bool
    has(Entity entity) const
    {
        return m_entities.has(entity);
    }

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
    const std::vector<Entity>& entities() const
    {
        return m_entities.entities();
    }

    ////////////////////////////////////////////////////////////
    std::vector<Entity>& entities()
    {
        return m_entities.entities();
    }

private:
    ////////////////////////////////////////////////////////////
    static Entity make_entity(EntitySlice index, EntitySlice generation)
    {
        return (static_cast<Entity>(generation) << (sizeof(EntitySlice) * 8)) | index;
    }

    ////////////////////////////////////////////////////////////
    std::vector<EntitySlice> m_generations; ///< Generation per index
    std::vector<EntitySlice> m_depot;       ///< Available indexes
    ComponentSet<uint8_t>    m_entities;    ///< Existing entities
};

} // namespace CNtity