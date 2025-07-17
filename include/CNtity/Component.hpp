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
#include <vector>
#include <cstdint>

namespace CNtity
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using Entity = uint64_t;
using EntitySlice = uint32_t;
using size_type = std::size_t;

////////////////////////////////////////////////////////////
inline EntitySlice get_index(Entity entity)
{
    return static_cast<EntitySlice>(entity & static_cast<EntitySlice>(-1));
}

////////////////////////////////////////////////////////////
inline EntitySlice get_generation(Entity entity)
{
    return static_cast<EntitySlice>(entity >> (sizeof(EntitySlice) * 8));
}

////////////////////////////////////////////////////////////
class IComponentSet
{
public:
    virtual ~IComponentSet() = default;

    virtual void                 remove(Entity entity) = 0;
    virtual void                 clear() = 0;
    virtual bool                 has(Entity entity) const = 0;
    virtual void                 copy(Entity source, Entity destination) = 0;
    virtual size_type            size() const = 0;
    virtual std::vector<Entity>& entities() = 0;
};

////////////////////////////////////////////////////////////
template <typename Component>
class ComponentSet : public IComponentSet
{
public:
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
    bool has(Entity entity) const override
    {
        const size_type index = get_index(entity);
        return index < m_sparse.size() &&
               m_sparse[index] != invalid_index() &&
               m_dense[m_sparse[index]] == entity;
    }

    ////////////////////////////////////////////////////////////
    Component& get(Entity entity)
    {
        assert(has(entity) && "Entity does not exist in ComponentSet");
        return m_values[m_sparse[get_index(entity)]];
    }

    ////////////////////////////////////////////////////////////
    const Component& get(Entity entity) const
    {
        assert(has(entity) && "Entity does not exist in ComponentSet");
        return m_values[m_sparse[get_index(entity)]];
    }

    ////////////////////////////////////////////////////////////
    void clear() override
    {
        m_sparse = {};
        m_dense = {};
        m_values = {};
    }

    ////////////////////////////////////////////////////////////
    void copy(Entity source, Entity destination) override
    {
        if(has(source))
        {
            insert(destination, get(source));
        }
    }

    ////////////////////////////////////////////////////////////
    size_type size() const override
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
    std::vector<Entity>& entities() override
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
        size_type     index;
        ComponentSet* set;

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