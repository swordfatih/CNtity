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

#ifndef SPARSESET_HPP
#define SPARSESET_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
// Standard
#include <cassert>
#include <unordered_map>
#include <utility>
#include <vector>

namespace CNtity
{
    
////////////////////////////////////////////////////////////
using size_type = std::size_t;

////////////////////////////////////////////////////////////
template <typename Entity, typename Component>
class SparseSet
{
public:
    ////////////////////////////////////////////////////////////
    void insert(const Entity& entity, const Component& component)
    {
        if(auto it = m_sparse.find(entity); it != m_sparse.end())
        {
            m_components[it->second] = component;
        }
        else
        {
            const size_type index = m_components.size();
            m_dense.push_back(entity);
            m_components.push_back(component);
            m_sparse.emplace(entity, index);
        }
    }

    ////////////////////////////////////////////////////////////
    void remove(const Entity& entity)
    {
        auto it = m_sparse.find(entity);

        if(it == m_sparse.end())
        {
            return;
        }

        const size_type index = it->second;
        const Entity&   lastEntity = m_dense.back();

        m_dense[index] = lastEntity;
        m_components[index] = m_components.back();

        m_sparse[lastEntity] = index;
        m_dense.pop_back();
        m_components.pop_back();
        m_sparse.erase(it);
    }

    ////////////////////////////////////////////////////////////
    bool has(const Entity& entity) const
    {
        return m_sparse.find(entity) != m_sparse.end();
    }

    ////////////////////////////////////////////////////////////
    Component& get(const Entity& entity)
    {
        assert(has(entity) && "Entity does not exist in SparseSet");
        return m_components[m_sparse.at(entity)];
    }

    ////////////////////////////////////////////////////////////
    const Component& get(const Entity& entity) const
    {
        assert(has(entity) && "Entity does not exist in SparseSet");
        return m_components[m_sparse.at(entity)];
    }

    ////////////////////////////////////////////////////////////
    size_type size() const
    {
        return m_components.size();
    }

    ////////////////////////////////////////////////////////////
    bool empty() const
    {
        return m_components.empty();
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
    const std::vector<Component>& components() const
    {
        return m_components;
    }

    ////////////////////////////////////////////////////////////
    std::vector<Component>& components()
    {
        return m_components;
    }

    ////////////////////////////////////////////////////////////
    struct Iterator
    {
        size_type  index;
        SparseSet* set;

        bool operator!=(const Iterator& other) const { return index != other.index; }
        void operator++() { ++index; }
        auto operator*() { return std::tie(set->m_dense[index], set->m_components[index]); }
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
    // Member data
    ////////////////////////////////////////////////////////////
    std::unordered_map<Entity, size_type> m_sparse;
    std::vector<Entity>                   m_dense;
    std::vector<Component>                m_components;
};

} // namespace CNtity

#endif // SPARSESET_HPP