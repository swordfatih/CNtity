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
#include "Entity.hpp"

#include <cassert>
#include <unordered_map>
#include <utility>
#include <vector>

namespace CNtity
{

////////////////////////////////////////////////////////////
using size_type = std::size_t;

////////////////////////////////////////////////////////////
template <typename Component>
class SparseSet
{
public:
    ////////////////////////////////////////////////////////////
    bool insert(Entity entity, const Component& component)
    {
        const size_t index = Entities::index(entity);

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
    void remove(Entity entity)
    {
        assert(has(entity) && "Entity does not exist in SparseSet");

        const size_type dense_index = m_sparse[Entities::index(entity)];
        const size_type last_index = m_dense.size() - 1;

        if(dense_index != last_index)
        {
            const auto last_entity = m_dense[last_index];

            m_dense[dense_index] = last_entity;
            m_values[dense_index] = std::move(m_values[last_index]);

            m_sparse[Entities::index(last_entity)] = dense_index;
        }

        m_dense.pop_back();
        m_values.pop_back();
        m_sparse[Entities::index(entity)] = invalid_index();
    }

    ////////////////////////////////////////////////////////////
    bool has(Entity entity) const
    {
        const size_type index = Entities::index(entity);
        return index < m_sparse.size() &&
               m_sparse[index] != invalid_index() &&
               m_dense[m_sparse[index]] == entity;
    }

    ////////////////////////////////////////////////////////////
    Component& get(Entity entity)
    {
        assert(has(entity) && "Entity does not exist in SparseSet");
        return m_values[m_sparse[Entities::index(entity)]];
    }

    ////////////////////////////////////////////////////////////
    const Component& get(Entity entity) const
    {
        assert(has(entity) && "Entity does not exist in SparseSet");
        return m_values[m_sparse[Entities::index(entity)]];
    }

    ////////////////////////////////////////////////////////////
    void clear()
    {
        m_sparse = {};
        m_dense = {};
        m_values = {};
    }

    ////////////////////////////////////////////////////////////
    size_type size() const
    {
        return m_values.size();
    }

    ////////////////////////////////////////////////////////////
    bool empty() const
    {
        return m_values.empty();
    }

    ////////////////////////////////////////////////////////////
    const std::vector<Entity>& entities() const
    {
        return m_dense;
    }

    ////////////////////////////////////////////////////////////
    std::vector<Entity>& entities()
    {
        return m_dense;
    }

    ////////////////////////////////////////////////////////////
    const std::vector<Component>& values() const
    {
        return m_values;
    }

    ////////////////////////////////////////////////////////////
    std::vector<Component>& values()
    {
        return m_values;
    }

    ////////////////////////////////////////////////////////////
    struct Iterator
    {
        size_type  index;
        SparseSet* set;

        bool operator!=(const Iterator& other) const { return index != other.index; }
        void operator++() { ++index; }
        auto operator*() { return std::tie(set->m_dense[index], set->m_values[index]); }
    };

    ////////////////////////////////////////////////////////////
    Iterator begin()
    {
        return {0, this};
    }

    ////////////////////////////////////////////////////////////
    Iterator end()
    {
        return {m_dense.size(), this};
    }

private:
    ////////////////////////////////////////////////////////////
    static constexpr size_type invalid_index()
    {
        return static_cast<size_type>(-1);
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::vector<size_type> m_sparse;
    std::vector<Entity>    m_dense;
    std::vector<Component> m_values;
};

} // namespace CNtity