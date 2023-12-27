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
#include <tuple>
#include <unordered_set>

//tsl
#include "CNtity/tsl/hopscotch_map.h"

//stduuid
#include "CNtity/stduuid/uuid.h"

namespace CNtity
{

////////////////////////////////////////////////////////////
using Entity = uuids::uuid;     ///< Entities are only unique identifiers

////////////////////////////////////////////////////////////
/// \brief Class that contains helper functions for an Entity
/// Component System architecture
///
////////////////////////////////////////////////////////////
template <typename ... Components>
class Helper
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    Helper() : m_components(), m_entities()
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
        entities.reserve(m_entities.size());

        for(auto&& entity: m_entities)
        {
            entities.push_back(entity);
        }

        return std::move(entities);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if an entity matchs with a given identifier
    ///
    /// \param identifier Unique identifier
    ///
    /// \return True if the identifier exists
    ///
    ////////////////////////////////////////////////////////////
    bool match(const uuids::uuid& identifier)
    {
        return m_entities.count(identifier);
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
        if(identifier.is_nil() || match(identifier))
        {
            identifier = uuids::uuid_random_generator{random_generator()}();
        }

        m_entities.emplace(identifier);

        return identifier;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create entity with components
    ///
    /// \param types Components
    ///
    /// \return Created entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Types>
    Entity create(const Types& ... types)
    {
        Entity&& entity = create();

        add(entity, types ...);

        return entity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create entity with components using a defined
    /// identifier
    ///
    /// \param identifier Defined identifier
    /// \param types Components
    ///
    /// \return Created entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Types>
    Entity create(const uuids::uuid& identifier, const Types& ... types)
    {
        Entity&& entity = create(identifier);

        add(entity, types ...);

        return entity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Add components to a specified entity
    ///
    /// \param entity Entity
    /// \param types Components
    ///
    /// \return Pointer to the added component
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Types>
    std::tuple<Types&...> add(const Entity& entity, const Types& ... types)
    {
        (m_components[typeid(types)].insert(std::make_pair(entity, types)), ...);

        return get<Types...>(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove a component from a specified entity
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Types>
    void remove(const Entity& entity)
    {
        (m_components[typeid(Types)].erase(entity), ...);
    }

    ////////////////////////////////////////////////////////////
    template <typename ... Types>
    std::tuple<Types&...> get(const Entity& entity)
    {
        return std::tie(std::get<Types>(m_components[typeid(Types)].at(entity))...);   
    }

    ////////////////////////////////////////////////////////////
    template <typename ... Types>
    std::optional<std::tuple<Types&...>> get_if(const Entity& entity)
    {
        if(!has<Types...>(entity))
        {
            return std::nullopt;
        }

        return std::make_optional(get<Types...>(entity));   
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
    std::variant<Components...>& get_by_index(const Entity& entity, std::type_index index)
    {
        return m_components[index][entity];
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
    std::vector<std::variant<Components ...>*> retrieve(const Entity& entity)
    {
        std::vector<std::variant<Components ...>*> components;

        for(auto& [component, entities]: m_components)
        {
            if(entities.count(entity))
            {
                components.push_back(const_cast<std::variant<Components ...>*>(&entities.at(entity)));
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
    template <typename ... Types>
    bool has(const Entity& entity)
    {
        return (m_components[typeid(Types)].count(entity) && ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Execute a callback for every entities that
    /// contain specified components
    ///
    /// \param callback Callback
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Types, typename Function>
    void each(Function callback)
    {
        std::type_index type(typeid(std::tuple_element_t<0, std::tuple<Types&...>>));
        smallest<Types...>(type);

        for(auto&& [entity, variant]: m_components[type])
        {
            if(has<Types...>(entity))
            {
                std::apply([&callback, &entity](Types& ... args) 
                {
                    callback(entity, args...);
                }, get<Types...>(entity));
            }
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Acquire entities that contains specified
    /// components
    ///
    /// \return Map of entities with specified components
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Types>
    std::vector<std::tuple<Entity, Types&...>> each()
    {
        std::vector<std::tuple<Entity, Types&...>> values;

        std::type_index type(typeid(std::tuple_element_t<0, std::tuple<Types&...>>));
        smallest<Types...>(type);

        for(auto&& [entity, variant]: m_components[type])
        {
            if(has<Types...>(entity))
            {
                values.push_back(std::tuple_cat(std::make_tuple(entity), get<Types...>(entity)));
            }
        }

        return values;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Erase a specified entity
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    void erase(const Entity& entity)
    {
        for(auto& [component, entities]: m_components)
        {
            const_cast<tsl::hopscotch_map<Entity, std::variant<Components ...>>&>(entities).erase(entity);
        }

        m_entities.erase(entity);
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
        if(m_components[typeid(Type)].size() < m_components[component].size())
        {
            component = typeid(Type);
        }

        if constexpr (sizeof...(Types) != 0)
        {
            smallest<Types ...>(component);
        }
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
    tsl::hopscotch_map<std::type_index, tsl::hopscotch_map<Entity, std::variant<Components ...>>>    m_components;    ///< Components
    std::unordered_set<Entity>                                                                       m_entities;      ///< Entities
};

} // namespace CNtity

#endif // HELPER_HPP
