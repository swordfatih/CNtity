/////////////////////////////////////////////////////////////////////////////////
//
// CNtity - Chats Noirs Entity Component System Helper
// Copyright (c) 2018 - 2023 Fatih (accfldekur@gmail.com)
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
#include <typeindex>
#include <set>
#include <map>
#include <any>

//stduuid
#include "CNtity/stduuid/uuid.h"

namespace CNtity
{

////////////////////////////////////////////////////////////
using Entity = uuids::uuid;     ///< Entities are only unique identifiers

////////////////////////////////////////////////////////////
template <typename ... Components>
class View;

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
    const std::set<Entity>& entities()
    {
        return m_entities;
    }

    ////////////////////////////////////////////////////////////
    template <typename ... Components>
    std::vector<std::tuple<Entity, Components&...>> entities()
    {
        std::vector<std::tuple<Entity, Components&...>> entities;

        for(auto& [entity, _]: m_components[smallest<Components...>()])
        {
            if(has<Components...>(entity))
            {
                entities.push_back(std::tuple_cat(std::make_tuple(entity), get<Components...>(entity)));
            }
        }

        return entities;
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
    /// \brief Erase a specified entity
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    void erase(const Entity& entity)
    {
        for(auto& [component, entities]: m_components)
        {
            const_cast<std::map<Entity, std::any>&>(entities).erase(entity);
            notify(component);
        }

        m_entities.erase(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if a given entity identifier exists
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    bool match(const Entity& entity)
    {
        return m_entities.count(entity);
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
        ((m_components[typeid(components)][entity] = components), ...);
        
        (notify(typeid(Components)), ...);

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
        
        (notify(typeid(Components)), ...);
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
    /// \brief Visit a component of a specified entity by its
    /// runtime index for given possible types. This function 
    /// adds a new component to the entity if it doesn't already
    /// exist
    ///
    /// \param entity Entity
    /// \param index Index of the component
    /// \param visitor Visitor function
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Components, typename Function>
    void visit(const Entity& entity, std::type_index index, Function visitor)
    {
        ([this, &entity, &visitor, &index]
        {
            if(typeid(Components).name() == index.name() && has<Components>(entity))
            {
                visitor(std::get<0>(get<Components>(entity)));
            }
        }(), ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Visit all components associated to a specified
    /// entity for given possible types
    ///
    /// \param entity Entity
    /// \param visitor Visitor function
    ///
    ////////////////////////////////////////////////////////////
    template <typename ... Components, typename Function>
    void visit(const Entity& entity, Function visitor)
    {
        ([this, &entity, &visitor]
        {
            if(auto tuple = get_if<Components>(entity))
            {
                visitor(std::get<0>(*tuple));
            }
        }(), ...);
    }

    ////////////////////////////////////////////////////////////
    template <typename ... Components>
    View<Components...> view()
    {
        View<Components...> view(*this);

        (view.observe(m_views[typeid(Components)].emplace_back()), ...);

        return view;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Copy components of an entity to another
    ///
    /// \brief source Source entity
    /// \brief destination Destination entity
    ///
    ////////////////////////////////////////////////////////////
    Entity duplicate(const Entity& source, Entity destination = {})
    {
        if(destination.is_nil() || !match(destination))
        {
            destination = create();
        }

        for(auto& [component, entities]: m_components)
        {
            if(entities.count(source))
            {
                auto& map = const_cast<std::map<Entity, std::any>&>(entities);
                map.emplace(std::make_pair(destination, map[source]));
                
                notify(component);
            }
        }

        return destination;
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
    template <typename ... Components>
    void notify(const std::type_index& component)
    {
        auto& observers = m_views[component];
        for(auto it = observers.begin(); it != observers.end();)
        {
            if ((*it).expired()) 
            {
                it = observers.erase(it);
            }
            else if (auto observer = it->lock())
            {
                *observer = true;
                ++it;
            }
        }
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::map<std::type_index, std::map<Entity, std::any>>   m_components;   ///< Components
    std::set<Entity>                                                  m_entities;     ///< Entities
    std::map<std::type_index, std::vector<std::weak_ptr<bool>>>       m_views;        ///< Views 
};

////////////////////////////////////////////////////////////
template <typename ... Components>
class View
{
public:
    ////////////////////////////////////////////////////////////
    View(Helper& helper): m_helper(helper), m_update(std::make_shared<bool>(true))
    {
    }

    ////////////////////////////////////////////////////////////
    void update()
    {
        m_entities = std::move(m_helper.entities<Components...>());
        *m_update = false;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Execute a callback for every entities that
    /// contain specified components
    ///
    /// \param callback Callback
    ///
    ////////////////////////////////////////////////////////////
    template <typename Function>
    void each(Function&& callback)
    {
        if(*m_update)
        {
            update();
        }
            
        for(auto&& components: m_entities)
        {
            std::apply(callback, components);
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Acquire entities that contains specified
    /// components to iterate on
    ///
    /// \return Map of entities with specified components
    ///
    ////////////////////////////////////////////////////////////
    std::vector<std::tuple<Entity, Components&...>>& each()
    {
        if(*m_update)
        {
            update();
        }

        return m_entities;
    }

    ////////////////////////////////////////////////////////////
    void observe(std::weak_ptr<bool>& observer)
    {
        observer = m_update;
    }

private:
    Helper& m_helper;
    std::vector<std::tuple<Entity, Components&...>> m_entities;
    std::shared_ptr<bool> m_update;
};

} // namespace CNtity

#endif // HELPER_HPP
