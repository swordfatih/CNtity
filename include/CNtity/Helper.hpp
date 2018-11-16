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
#include <unordered_map>
#include <vector>
#include <variant>
#include <functional>

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
        ++mCount;
        mComponents[mCount];

        return mCount;
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
    Entity create(Type type, Types ... types)
    {
        ++mCount;
        mComponents[mCount].push_back(type);

        if constexpr (sizeof...(Types) != 0)
        {
            mComponents[mCount].push_back(types ...);
        }

        return mCount;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Add a component to a specified entity
    ///
    /// \param entity Entity
    /// \param type Component
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type>
    Type* add(Entity entity, Type type)
    {
        mComponents.at(entity).push_back(type);
        return &std::get<Type>(mComponents.at(entity).back());

        return nullptr;
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
        if(has<Type>(entity))
        {
            for(auto& it: mComponents.at(entity))
            {
                int i = &it - &mComponents.at(entity)[0];

                if(std::holds_alternative<Type>(it))
                {
                    mComponents.at(entity).erase(mComponents.at(entity).begin() + i);
                    return;
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get a component of a specified entity
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type>
    Type* get(Entity entity)
    {
        for(auto& it: mComponents.at(entity))
        {
            if(auto component = std::get_if<Type>(&it))
            {
                return component;
            }
        }

        return nullptr;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if an entity contains specified components
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    bool has(Entity entity)
    {

        if constexpr (sizeof...(Types) == 0)
        {
            for(auto& it: mComponents.at(entity))
            {
                if(std::holds_alternative<Type>(it))
                {
                    return true;
                }
            }
        }
        else
        {
            uint16_t size = sizeof...(Types);

            for(auto& it: mComponents.at(entity))
            {
                if(std::holds_alternative<Type>(it) || std::holds_alternative<Types ...>(it))
                {
                    --size;
                }

                if(size == 0)
                {
                    return true;
                }
            }
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
            for(auto& [entity, components]: mComponents)
            {
                if(has<Type>(entity))
                {
                    func(entity, get<Type>(entity));
                }
            }
        }
        else
        {
            for(auto& [entity, components]: mComponents)
            {
                if(has<Type, Types ...>(entity))
                {
                    func(entity, get<Type>(entity));
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Acquire entities that contains specified
    /// components
    ///
    /// \return Vector of entity containing specified components
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    std::vector<Entity> acquire()
    {
        std::vector<Entity> entities;

        if constexpr (sizeof...(Types) == 0)
        {
            for(auto& [entity, components]: mComponents)
            {
                if(has<Type, Types ...>(entity))
                {
                    entities.push_back(entity);
                }
            }
        }
        else
        {
            for(auto& [entity, components]: mComponents)
            {
                if(has<Type, Types ...>(entity))
                {
                    entities.push_back(entity);
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
        mComponents.erase(entity);
    }

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::unordered_map<Entity, std::vector<std::variant<Component, Components ...>>> mComponents;   ///< Components
    uint64_t mCount = 0;                                                                                 ///< Entity count

};

} // namespace CNtity

#endif // HELPER_HPP
