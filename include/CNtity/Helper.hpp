/////////////////////////////////////////////////////////////////////////////////
//
// CNtity - Chats Noirs Entity Component System Helper
// Copyright (c) 2018 - 2020 Fatih (accfldekur@gmail.com)
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
#include <vector>
#include <variant>
#include <functional>
#include <typeindex>
#include <algorithm>

//tsl
#include "CNtity/tsl/hopscotch_map.h"

//stduuid
#include "CNtity/stduuid/uuid.h"

namespace CNtity
{

////////////////////////////////////////////////////////////
using Entity = uuids::uuid;     ///< Entities are only unique identifiers
using Mask = uint_least32_t;    ///< Bitmask

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
    /// \brief Get registered components
    ///
    /// \return type_index of every registered components in an
    /// array
    ///
    ////////////////////////////////////////////////////////////
    std::vector<std::type_index> components() const
    {
        std::vector<std::type_index> indexes;

        indexes.push_back(typeid(Component));
        (indexes.push_back(typeid(Components)), ...);

        return indexes;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get all the entities
    ///
    /// \return unique identifier of every entities in an array
    ///
    ////////////////////////////////////////////////////////////
    std::vector<Entity>&& entities()
    {
        std::vector<Entity> entities;
        entities.reserve(mEntities.size());

        for(auto&& [entity, mask]: mEntities)
        {
            entities.push_back(entity);
        }

        return std::move(entities);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create entity without any component
    ///
    /// \param identifier Use a defined identifier for the new
    /// entity. If none is specified, if the identifier is
    /// invalid or if it is already used, a new unique identifier
    /// will be generated.
    ///
    /// \return Created entity
    ///
    ////////////////////////////////////////////////////////////
    Entity create(uuids::uuid identifier = {})
    {
        Entity entity;

        if(identifier.is_nil() || mEntities.count(identifier) != 0)
        {
            while(entity.is_nil() || mEntities.count(entity) != 0)
            {
                entity = uuids::uuid_random_generator{random_generator()}();
            }
        }
        else
        {
            entity = identifier;
        }

        mEntities[entity] = 0;

        return std::move(entity);
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
        Entity entity = create();

        add(entity, type, types ...);

        return std::move(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create entity with components using a defined
    /// identifier
    ///
    /// \param identifier Defined identifier
    /// \param type Component
    /// \param types Components
    ///
    /// \return Created entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    Entity&& create(uuids::uuid identifier, const Type& type, const Types& ... types)
    {
        Entity entity = create(identifier);

        add(entity, type, types ...);

        return std::move(entity);
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
        switch_grouping<Type, Types...>(entity);

        mComponents[typeid(Type)][entity] = type;

        if constexpr (sizeof...(Types) != 0)
        {
            auto assign = [&](const auto& component)
            {
                mComponents[typeid(component)][entity] = component;
            };

            (assign(types), ...);
        }

        return get<Type>(std::move(entity));
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
        switch_grouping<Type, Types...>(entity);

        mComponents[typeid(Type)].erase(entity);

        if constexpr (sizeof...(Types) != 0)
        {
            (mComponents[typeid(Types)].erase(entity), ...);
        }
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
    /// \brief Get a component of a specified entity by its
    /// index. This function adds a new component to the entity
    /// if it doesn't already exist. This function is useful for
    /// serialization and is supposed to be used with std::visit
    ///
    /// This function works only if all the components are
    /// default constructible. Or at least don't require
    /// parameters for construction
    ///
    /// \param entity Entity
    /// \param index Index of the component
    ///
    /// \return Pointer to the component as variant
    ///
    ////////////////////////////////////////////////////////////
    std::variant<Component, Components...>* get_by_index(Entity entity, std::type_index index)
    {
        if(mComponents[index].count(entity) == 0)
        {
            tsl::hopscotch_map<size_t, std::variant<Component, Components...>> indexes;

            indexes.emplace(std::type_index(typeid(Component)).hash_code(), Component{});
            (indexes.emplace(std::type_index(typeid(Components)).hash_code(), Components{}), ...);

            mComponents[index][entity] = indexes[index.hash_code()];
        }

        return &mComponents[index].at(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve every components associated to a specified
    /// entity
    ///
    /// \param entity Entity
    ///
    /// \return Pointers to every components associated to the
    /// entity
    ///
    ////////////////////////////////////////////////////////////
    std::vector<std::variant<Component, Components ...>*> retrieve(Entity entity)
    {
        std::vector<std::variant<Component, Components ...>*> components;

        for(auto& [component, entities]: mComponents)
        {
            if(entities.count(entity) > 0)
            {
                components.push_back(const_cast<std::variant<Component, Components ...>*>(&entities.at(entity)));
            }
        }

        return components;
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
        for(auto&& entity: mGroupings[bitmask<Type, Types...>()])
        {
            func(entity, &std::get<Type>(mComponents[typeid(Type)].at(entity)));
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
        return mGroupings[bitmask<Type, Types...>()];
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

        mGroupings.erase(mEntities.at(entity));
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
    void emplace_variadic(Entity entity, const Type& type, const Types& ... types)
    {
        mComponents[typeid(Type)].emplace(std::make_pair(entity, type));

        if constexpr (sizeof...(Types) != 0)
        {
            emplace_variadic<Types ...>(entity, types ...);
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create a bitmask with components
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename ... Types>
    Mask bitmask() const
    {
        std::vector<Mask> hashCodes;
        hashCodes.reserve(sizeof...(Types) + 1);

        hashCodes.push_back(std::type_index(typeid(Type)).hash_code());

        if(sizeof...(Types) != 0)
        {
            (hashCodes.push_back(std::type_index(typeid(Types)).hash_code()), ...);
        }

        Mask mask = 0;
        for(const auto& it: hashCodes)
        {
            mask |= (1 << it);
        }

        return mask;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Switch entity to a new grouping
    ///
    ////////////////////////////////////////////////////////////
    template <typename Type, typename... Types>
    Mask&& switch_grouping(Entity entity)
    {
        if(mEntities.at(entity) != 0)
        {
            auto&& grouping = mGroupings[mEntities.at(entity)];
            grouping.erase(std::find(grouping.begin(), grouping.end(), entity));
        }

        auto&& mask = bitmask<Type, Types...>();
        mGroupings[mask].push_back(entity);
        return std::move(mEntities.at(entity) = mask);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Random generator using the standard library
    ///
    ////////////////////////////////////////////////////////////
    static std::mt19937& random_generator()
    {
        static std::mt19937 random_generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        return random_generator;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    tsl::hopscotch_map<std::type_index, tsl::hopscotch_map<Entity, std::variant<Component, Components ...>>>    mComponents;        ///< Components
    tsl::hopscotch_map<Mask, std::vector<Entity>>                                                               mGroupings;         ///< Groupings
    tsl::hopscotch_map<Entity, Mask>                                                                            mEntities;          ///< Entities

};

} // namespace CNtity

#endif // HELPER_HPP
