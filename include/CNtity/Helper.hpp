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
#include <any>
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
        std::vector<std::type_index> components;
        components.reserve(m_components.size());

        for(const auto& [key, _]: m_components) 
        {
            components.push_back(key);
        }

        return components;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get all the entities
    ///
    /// \return unique identifier of every entities in a set
    ///
    ////////////////////////////////////////////////////////////
    const std::unordered_set<Entity>& entities()
    {
        return m_entities;
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
    Entity create(Entity identifier = {})
    {
        if(identifier.is_nil() || m_entities.count(identifier))
        {
            identifier = uuids::uuid_random_generator{random_generator()}();
        }

        m_entities.emplace(identifier);

        return identifier;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create entity with components
    ///
    /// \param components Components
    ///
    /// \return Created entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Components>
    Entity create(const Components& ... components)
    {
        Entity entity = create();

        add(entity, components ...);

        return entity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create entity with components using a defined
    /// identifier
    ///
    /// \param identifier Defined identifier
    /// \param components Components
    ///
    /// \return Created entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Components>
    Entity create(Entity identifier, const Components& ... components)
    {
        Entity entity = create(identifier);

        add(entity, components ...);

        return entity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Add components to a specified entity
    ///
    /// \param entity Entity
    /// \param components Components
    ///
    /// \return Pointer to the added component
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Components>
    std::tuple<Components&...> add(const Entity& entity, const Components& ... components)
    {
        (m_components[typeid(components)].insert(std::make_pair(entity, components)), ...);

        return get<Components...>(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove a component from a specified entity
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Components>
    void remove(const Entity& entity)
    {
        (m_components[typeid(Components)].erase(entity), ...);
    }

    ////////////////////////////////////////////////////////////
    template <typename ... Components>
    std::tuple<Components&...> get(const Entity& entity)
    {
        return std::tie(std::any_cast<Components&>(m_components[typeid(Components)].at(entity))...);   
    }

    ////////////////////////////////////////////////////////////
    template <typename ... Components>
    std::optional<std::tuple<Components&...>> get_if(const Entity& entity)
    {
        return has<Components...>(entity) ? std::make_optional(get<Components...>(entity)) : std::nullopt;   
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
    std::any get(const Entity& entity, std::type_index index)
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
    std::vector<std::reference_wrapper<std::any>> get(const Entity& entity)
    {
        std::vector<std::reference_wrapper<std::any>> components;

        for(auto& [component, entities]: m_components)
        {
            if(entities.count(entity))
            {
                components.push_back(const_cast<tsl::hopscotch_map<Entity, std::any>&>(entities).at(entity));
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
    template <typename ... Components>
    bool has(const Entity& entity)
    {
        return (m_components[typeid(Components)].count(entity) && ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Execute a callback for every entities that
    /// contain specified components
    ///
    /// \param callback Callback
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Components, typename Function>
    void each(Function callback)
    {
        for(auto& [entity, variant]: m_components[smallest<Components...>()])
        {
            if(has<Components...>(entity))
            {
                std::apply([&callback, &entity](Components& ... components) 
                {
                    callback(entity, components...);
                }, get<Components...>(entity));
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
    template <typename ... Components>
    std::vector<std::tuple<Entity, Components&...>> each()
    {
        std::vector<std::tuple<Entity, Components&...>> values;

        for(auto& [entity, variant]: m_components[smallest<Components...>()])
        {
            if(has<Components...>(entity))
            {
                values.push_back(std::tuple_cat(std::make_tuple(entity), get<Components...>(entity)));
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
            const_cast<tsl::hopscotch_map<Entity, std::any>&>(entities).erase(entity);
        }

        m_entities.erase(entity);
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Get the component containing the least amount of
    /// entities
    ///
    ////////////////////////////////////////////////////////////
    template<typename ... Components>
    std::type_index smallest()
    {
        std::type_index component(typeid(std::tuple_element_t<0, std::tuple<Components&...>>));
        ((component = m_components[typeid(Components)].size() < m_components[component].size() ? typeid(Components) : component), ...);
        return component;
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
    tsl::hopscotch_map<std::type_index, tsl::hopscotch_map<Entity, std::any>>    m_components;    ///< Components
    std::unordered_set<Entity>                                                   m_entities;      ///< Entities
};

} // namespace CNtity

#endif // HELPER_HPP
