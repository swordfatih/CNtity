/////////////////////////////////////////////////////////////////////////////////
//
// CNtity - Chats Noirs Entity Component System Helper
// Copyright (c) 2018 - 2019 Fatih (accfldekur@gmail.com)
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

#ifndef HELPER_HPP
#define HELPER_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
//Standard
#include <cstdint>
#include "CNtity/tsl/hopscotch_map.h"
#include <vector>
#include <variant>
#include <functional>
#include <typeindex>
#include <algorithm>

namespace CNtity
{

////////////////////////////////////////////////////////////
using Entity = uint64_t; ///< Entities are only IDs

////////////////////////////////////////////////////////////
/// \brief Class that contains helper functions for an Entity
/// Component System architecture
///
////////////////////////////////////////////////////////////
template <typename Component, typename ... Components>
class Helper
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    Helper() : mComponents()
    {

    }

    ////////////////////////////////////////////////////////////
    /// \brief Default destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Helper()
    {

    }

    ////////////////////////////////////////////////////////////
    /// \brief Create entity without any component
    ///
    /// \return Created entity
    ///
    ////////////////////////////////////////////////////////////
    Entity create()
    {
        return ++mCountEntity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create entity with components
    ///
    /// \param type Component
    /// \param types Components
    ///
    /// \return Created entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    Entity create(const Type& type, const Types& ... types)
    {
        mComponents[typeid(Type)][++mCountEntity] = type;

        if constexpr (sizeof...(Types) != 0)
        {
            emplace_variadic<Type, Types ...>(type, types ...);
        }

        if(!mGroupings.empty())
        {
            mGroupings.clear();
        }

        return mCountEntity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Add components to a specified entity
    ///
    /// \param entity Entity
    /// \param type Component
    ///
    /// \return Pointer to the added component
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    Type* add(Entity entity, const Type& type, const Types& ... types)
    {
        mComponents[typeid(Type)][entity] = type;

        if constexpr (sizeof...(Types) != 0)
        {
            auto assign = [&](const auto& component)
            {
                mComponents[typeid(component)][entity] = component;
            };

            (assign(types), ...);
        }

        mGroupings.clear();

        return get<Type>(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove a component from a specified entity
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    void remove(Entity entity)
    {
        mComponents[typeid(Type)].erase(entity);

        if constexpr (sizeof...(Types) != 0)
        {
            (mComponents[typeid(Types)].erase(entity), ...);
        }

        mGroupings.clear();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get a component of a specified entity
    ///
    /// \param entity Entity
    ///
    /// \return Pointer to the component
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type>
    Type* get(Entity entity)
    {
        return std::get_if<Type>(&mComponents[typeid(Type)].at(entity));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if an entity contains specified components
    ///
    /// \param entity Entity
    ///
    /// \return True if the entity has the component
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    bool has(Entity entity)
    {
        if constexpr (sizeof...(Types) == 0)
        {
            return mComponents[typeid(Type)].count(entity) > 0;
        }
        else
        {
            return mComponents[typeid(Type)].count(entity) > 0 && (mComponents[typeid(Types)].count(entity), ...);
        }

        return false;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Execute a function for every entities that
    /// contain specified components
    ///
    /// \param func Function
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    void for_each(std::function<void(Entity, Type*)> func)
    {
        if constexpr (sizeof...(Types) == 0)
        {
            for(auto& [entity, component]: mComponents[typeid(Type)])
            {
                func(entity, const_cast<Type*>(&std::get<Type>(component)));
            }
        }
        else
        {
            uint_least32_t mask = bitmask<Type, Types...>();
            if(mGroupings.count(mask) == 0)
            {
                auto& grouping = mGroupings[mask];

                std::type_index type(typeid(Type));
                smallest<Type, Types...>(type);

                grouping.reserve(mComponents[type].size());

                for(auto& [entity, component]: mComponents[type])
                {
                    if(has<Type, Types...>(entity))
                    {
                        func(entity, &std::get<Type>(mComponents[typeid(Type)][entity]));
                        grouping.emplace_back(entity);
                    }
                }
            }
            else
            {
                for(const auto& entity: mGroupings[mask])
                {
                    func(entity, &std::get<Type>(mComponents[typeid(Type)][entity]));
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Acquire entities that contains specified
    /// components
    ///
    /// \return Vector of entities that contain specified components
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    std::vector<Entity> acquire()
    {
        std::vector<Entity> entities;

        if constexpr (sizeof...(Types) == 0)
        {
            if(mGroupings.count(1 << std::type_index(typeid(Type)).hash_code()) == 0)
            {
                entities.reserve(mComponents[typeid(Type)].size());

                for(const auto& entity: mComponents[typeid(Type)])
                {
                    entities.emplace_back(entity.first);
                }

                auto& grouping = mGroupings[1 << std::type_index(typeid(Type)).hash_code()];
                grouping.reserve(entities.size());
                grouping = entities;
            }
            else
            {
                return mGroupings[1 << std::type_index(typeid(Type)).hash_code()];
            }
        }
        else
        {
            uint_least32_t mask = bitmask<Type, Types...>();
            if(mGroupings.count(mask) == 0)
            {
                std::type_index type(typeid(Type));
                smallest<Type, Types...>(type);

                entities.reserve(mComponents[type].size());

                for(const auto& entity: mComponents[typeid(Type)])
                {
                    if(has<Type, Types...>(entity.first))
                    {
                        entities.emplace_back(entity.first);
                    }
                }

                auto& grouping = mGroupings[mask];
                grouping.reserve(entities.size());
                grouping = entities;
            }
            else
            {
                return mGroupings[mask];
            }
        }

        return entities;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Erase a specified entity
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    void erase(Entity entity)
    {
        for(auto& [component, entities]: mComponents)
        {
            const_cast<tsl::hopscotch_map<Entity, std::variant<Component, Components ...>>&>(entities).erase(entity);
        }

        for(auto& [group, entities]: mGroupings)
        {
            auto position = std::find(entities.begin(), entities.end(), entity);
            if(position != entities.end())
            {
                const_cast<std::vector<Entity>&>(entities).erase(position);
            }
        }
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Compare components on the amount of entities
    /// containing them
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    void smallest(std::type_index& component)
    {
        if(mComponents[typeid(Type)].size() < mComponents[component].size())
        {
            component = typeid(Type);
        }

        if constexpr (sizeof...(Types) != 0)
        {
            smallest<Types ...>(component);
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Add several components to the entities
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    void emplace_variadic(const Type& type, const Types& ... types)
    {
        mComponents[typeid(Type)].emplace(std::make_pair(mCountEntity, type));

        if constexpr (sizeof...(Types) != 0)
        {
            emplace_variadic<Types ...>(types ...);
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create a bitmask with components
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    uint_least32_t bitmask()
    {
        std::vector<uint_least32_t> hashCodes;
        hashCodes.reserve(sizeof...(Types) + 1);

        hashCodes.push_back(std::type_index(typeid(Type)).hash_code());

        if(sizeof...(Types) != 0)
        {
            (hashCodes.push_back(std::type_index(typeid(Types)).hash_code()), ...);
        }

        uint_least32_t mask = 0;
        for(const auto& it: hashCodes)
        {
            mask |= (1 << it);
        }

        return mask;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    tsl::hopscotch_map<std::type_index, tsl::hopscotch_map<Entity, std::variant<Component, Components ...>>>    mComponents;        ///< Components
    tsl::hopscotch_map<uint32_t, std::vector<Entity>>                                                           mGroupings;         ///< Groupings
    uint64_t                                                                                                    mCountEntity = 0;   ///< Entity count

};

} // namespace CNtity

#endif // HELPER_HPP
