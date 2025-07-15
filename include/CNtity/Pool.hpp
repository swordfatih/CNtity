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

#ifndef POOL_HPP
#define POOL_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SparseSet.hpp"

#include <any>
#include <map>
#include <set>
#include <typeindex>

namespace CNtity
{

////////////////////////////////////////////////////////////
template <typename Entity>
struct BasePool
{
    using RemoveFn = void (*)(BasePool<Entity>*, const Entity&);
    using HasFn = bool (*)(BasePool<Entity>*, const Entity&);
    using ClearFn = void (*)(BasePool<Entity>*);
    using CloneFn = void (*)(BasePool<Entity>*, const Entity&, const Entity&);
    using SizeFn = size_type (*)(BasePool<Entity>*);
    using EntitiesFn = std::vector<Entity>& (*)(BasePool<Entity>*);

    RemoveFn   remove_fn{};
    ClearFn    clear_fn{};
    HasFn      has_fn{};
    CloneFn    clone_fn{};
    SizeFn     size_fn{};
    EntitiesFn entities_fn{};

    void                 remove(const Entity& entity) { remove_fn(this, entity); }
    void                 clear() { clear_fn(this); }
    bool                 has(const Entity& entity) { return has_fn(this, entity); }
    void                 clone(const Entity& source, const Entity& destination) { clone_fn(this, source, destination); }
    size_type            size() { return size_fn(this); }
    std::vector<Entity>& entities() { return entities_fn(this); }
};

////////////////////////////////////////////////////////////
template <typename Entity, typename Component>
struct Pool final : BasePool<Entity>
{
    SparseSet<Entity, Component> storage;

    auto& get_storage(BasePool<Entity>* self)
    {
        return static_cast<Pool*>(self)->storage;
    }

    Pool()
    {
        this->remove_fn = [](BasePool<Entity>* self, const Entity& entity)
        {
            static_cast<Pool*>(self)->storage.remove(entity);
        };

        this->clear_fn = [](BasePool<Entity>* self)
        {
            static_cast<Pool*>(self)->storage = {};
        };

        this->has_fn = [](BasePool<Entity>* self, const Entity& entity)
        {
            return static_cast<Pool*>(self)->storage.has(entity);
        };

        this->clone_fn = [](BasePool<Entity>* self, const Entity& source, const Entity& destination)
        {
            auto& storage = static_cast<Pool*>(self)->storage;

            if(storage.has(source))
            {
                storage.insert(destination, storage.get(source));
            }
        };

        this->size_fn = [](BasePool<Entity>* self)
        {
            return static_cast<Pool*>(self)->storage.size();
        };

        this->entities_fn = [](BasePool<Entity>* self) -> std::vector<Entity>&
        {
            return static_cast<Pool*>(self)->storage.entities();
        };
    }
};

} // namespace CNtity

#endif // POOL_HPP