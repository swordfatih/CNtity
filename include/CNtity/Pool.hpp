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

#include <vector>

namespace CNtity
{

////////////////////////////////////////////////////////////
struct BasePool
{
    virtual ~BasePool() = default;

    virtual void                 remove(Entity entity) = 0;
    virtual void                 clear() = 0;
    virtual bool                 has(Entity entity) = 0;
    virtual void                 copy(Entity source, Entity destination) = 0;
    virtual size_type            size() const = 0;
    virtual std::vector<Entity>& entities() = 0;
};

////////////////////////////////////////////////////////////
template <typename Component>
struct Pool final : BasePool
{
    ComponentSet<Component> storage;

    void                 remove(Entity entity) override { storage.remove(entity); }
    void                 clear() override { storage = {}; }
    bool                 has(Entity entity) override { return storage.has(entity); }
    void                 copy(Entity source, Entity destination) override { storage.copy(source, destination); }
    size_type            size() const override { return storage.size(); }
    std::vector<Entity>& entities() override { return storage.entities(); }
};

} // namespace CNtity