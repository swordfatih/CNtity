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

#include <cstdint>
#include <vector>

namespace CNtity
{

////////////////////////////////////////////////////////////
using Entity = uint64_t;
using EntitySlice = uint32_t;

class Entities
{
public:
    ////////////////////////////////////////////////////////////
    Entity create()
    {
        if(!m_depot.empty())
        {
            auto index = m_depot.back();
            m_depot.pop_back();

            return make(index, m_generations[index]);
        }

        auto index = m_generations.size();
        m_generations.push_back(0);

        return make(index, 0);
    }

    ////////////////////////////////////////////////////////////
    bool has(Entity entity) const
    {
        return index(entity) < m_generations.size() && m_generations[index(entity)] == Entities::generation(entity);
    }

    ////////////////////////////////////////////////////////////
    void remove(Entity entity)
    {
        if(index(entity) < m_generations.size())
        {
            m_generations[index(entity)]++;
            m_depot.push_back(index(entity));
        }
    }

    ////////////////////////////////////////////////////////////
    static EntitySlice invalid()
    {
        return static_cast<EntitySlice>(-1);
    }

    ////////////////////////////////////////////////////////////
    static EntitySlice index(Entity entity)
    {
        return static_cast<EntitySlice>(entity & static_cast<EntitySlice>(-1));
    }

    ////////////////////////////////////////////////////////////
    static EntitySlice generation(Entity entity)
    {
        return static_cast<EntitySlice>(entity >> (sizeof(EntitySlice) * 8));
    }

    ////////////////////////////////////////////////////////////
    static Entity make(EntitySlice index, EntitySlice generation)
    {
        return (static_cast<Entity>(generation) << (sizeof(EntitySlice) * 8)) | index;
    }

private:
    ////////////////////////////////////////////////////////////
    std::vector<EntitySlice> m_generations; ///< Generation per index
    std::vector<EntitySlice> m_depot;       ///< Available indexes
};


} // namespace CNtity