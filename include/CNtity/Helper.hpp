/////////////////////////////////////////////////////////////////////////////////
//
// CNtity - Chats Noirs Entity Component System Helper
// Copyright (c) 2018 Fatih (accfldekur@gmail.com)
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
using Entity = int64_t; ///< Entities are only IDs

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
    /// \param type Components
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
            mComponents[std::type_index(typeid(Types)...)].emplace(std::make_pair(mCountEntity, types ...));
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
    template <typename Type>
    Type* add(Entity entity, const Type& type)
    {
        mComponents[typeid(Type)][entity] = type;

        return get<Type>(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove a component from a specified entity
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type>
    void remove(Entity entity)
    {
        mComponents[typeid(Type)].erase(entity);
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
            return mComponents[typeid(Type)].count(entity) > 0 && mComponents[std::type_index(typeid(Types)...)].count(entity) > 0;
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
            for(auto [entity, component]: mComponents[typeid(Type)])
            {
                func(entity, &std::get<Type>(component));
            }
        }
        else
        {
            std::type_index type(typeid(Type));
            smallest(type, std::type_index(typeid(Types)...));

            for(auto [entity, component]: mComponents[type])
            {
                if(mComponents[std::type_index(typeid(Types)...)].count(entity) > 0)
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
            entities.reserve(mComponents[typeid(Type)].size());
            for(const auto& entity: mComponents[typeid(Type)])
            {
                entities.emplace_back(entity.first);
            }
        }
        else
        {
            std::type_index type(typeid(Type));
            smallest(type, std::type_index(typeid(Types)...));

            entities.reserve(mComponents[type].size());

            for(const auto& entity: mComponents[typeid(Type)])
            {
                if(mComponents[std::type_index(typeid(Types)...)].count(entity.first) > 0)
                {
                    entities.emplace_back(entity.first);
                }
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
        for(auto [component, entities]: mComponents)
        {
            entities.erase(entity);
        }
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Compare a component to another on the amount of
    /// entities containing them
    ///
    ////////////////////////////////////////////////////////////
    void smallest(std::type_index& component, const std::type_index& type)
    {
        if(mComponents[type].size() < mComponents[component].size())
        {
            component = type;
        }
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    tsl::hopscotch_map<std::type_index, tsl::hopscotch_map<Entity, std::variant<Component, Components ...>>>    mComponents;        ///< Components
    uint64_t                                                                                                    mCountEntity = 0;   ///< Entity count

};

} // namespace CNtity

#endif // HELPER_HPP
