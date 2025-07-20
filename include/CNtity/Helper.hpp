/////////////////////////////////////////////////////////////////////////////////
//
// CNtity - Chats Noirs Entity Component System Helper
// Copyright (c) 2018 - 2025 swordfatih
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
#include "EntitySet.hpp"

#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace CNtity
{

////////////////////////////////////////////////////////////
template <typename... Components>
class View;

////////////////////////////////////////////////////////////
/// \brief Wrapper class to associate a string to a type
///
/// Used to bind a textual index (e.g. for (de)serialization)
/// to a component type through Helper::index().
///
/// \tparam Component Component type to be indexed
///
////////////////////////////////////////////////////////////
template <typename Component>
class Index : public std::string_view
{
    using std::string_view::string_view;
};

////////////////////////////////////////////////////////////
/// \brief Core utility / facade for the ECS storage
///
/// Holds:
/// - Component pools (one per distinct component type)
/// - The entity allocator / recycler (EntitySet)
/// - Index mappings (string -> component type) for (de)serialization
/// - View subscriptions (weak flags marking cached views dirty)
///
/// Provides:
/// - Creation / destruction of entities
/// - Addition / removal / querying of components
/// - Visiting components by type or by string index
/// - Duplication of entities
/// - Construction of snapshot or filtering views
///
/// \note Not thread-safe.
///
////////////////////////////////////////////////////////////
class Helper
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ////////////////////////////////////////////////////////////
    Helper() : m_pools(), m_entities()
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Default destructor
    ////////////////////////////////////////////////////////////
    ~Helper() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ////////////////////////////////////////////////////////////
    Helper(const Helper&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ////////////////////////////////////////////////////////////
    Helper& operator=(const Helper&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Default move constructor
    ////////////////////////////////////////////////////////////
    Helper(Helper&&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Default move assignment
    ////////////////////////////////////////////////////////////
    Helper& operator=(Helper&&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Get registered components
    ///
    /// \return A vector containing the std::type_index of every
    /// registered component pool
    ///
    ////////////////////////////////////////////////////////////
    std::vector<std::type_index> components() const
    {
        std::vector<std::type_index> components;
        components.reserve(m_pools.size());

        for(const auto& [key, _]: m_pools)
        {
            components.push_back(key);
        }

        return components;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get (and lazily create) the component pool for a type
    ///
    /// Lazily allocates a ComponentSet<Component> if it does not
    /// already exist, then returns a reference to it.
    ///
    /// \tparam Component Component type whose pool is requested
    ///
    /// \return Reference to the ComponentSet<Component>
    ///
    ////////////////////////////////////////////////////////////
    template <typename Component>
    ComponentSet<Component>& pool()
    {
        auto& pool = m_pools[typeid(Component)];

        if(!pool)
        {
            pool = std::make_unique<ComponentSet<Component>>();
        }

        return *static_cast<ComponentSet<Component>*>(pool.get());
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get all the entities
    ///
    /// \return Constant reference to the vector containing the
    /// unique identifiers of all entities
    ///
    ////////////////////////////////////////////////////////////
    const std::vector<Entity>& entities()
    {
        return m_entities.entities();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create an entity and add components to it
    ///
    /// \tparam Components Component types to add
    /// \param components Component instances to add
    ///
    /// \return The created entity identifier
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    Entity create(const Components&... components)
    {
        Entity entity = m_entities.create();

        if constexpr(sizeof...(Components) > 0)
        {
            add(entity, components...);
        }

        return entity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if a given entity identifier exists
    ///
    /// \param entity Entity identifier
    ///
    /// \return True if the entity currently exists, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    bool match(Entity entity)
    {
        return m_entities.has(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Add components to a specified entity
    ///
    /// Existing components of the same types are replaced /
    /// overwritten according to the underlying ComponentSet
    /// semantics.
    ///
    /// Also marks subscribed views for those component types
    /// as dirty.
    ///
    /// \tparam Components Component types to add
    /// \param entity Entity identifier
    /// \param components Instances of the components to add
    ///
    /// \return Tuple containing references to the added components
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    std::tuple<Components&...> add(Entity entity, const Components&... components)
    {
        (pool<Components>().insert(entity, components), ...);
        (notify(typeid(Components)), ...);

        return get<Components...>(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove components from a specified entity
    ///
    /// Silently does nothing for missing components.
    ///
    /// Marks subscribed views observing these component types
    /// as dirty.
    ///
    /// \tparam Components Component types to remove
    /// \param entity Entity identifier
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    void remove(Entity entity)
    {
        (pool<Components>().remove(entity), ...);
        (notify(typeid(Components)), ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove a specified entity and all its components
    ///
    /// Marks all views for every component pool touched.
    ///
    /// \param entity Entity identifier
    ///
    ////////////////////////////////////////////////////////////
    void remove(Entity entity)
    {
        for(auto& [id, pool]: m_pools)
        {
            pool->remove(entity);
            notify(id);
        }

        m_entities.remove(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove specified component types from all entities
    ///
    /// Destroys the entire pools for the provided component types.
    /// Subscribed views for these pools are marked dirty.
    ///
    /// \tparam Components Component types to remove globally
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    void remove()
    {
        (m_pools.erase(typeid(Components)), ...);
        (notify(typeid(Components)), ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if an entity contains all specified components
    ///
    /// \tparam Components Component types to test
    /// \param entity Entity identifier
    ///
    /// \return True only if the entity has *all* the requested
    /// component types
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    bool has(Entity entity)
    {
        return (pool<Components>().has(entity) && ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get components associated to an entity
    ///
    /// \tparam Components Component types to retrieve
    /// \param entity Entity identifier
    ///
    /// \return Tuple of references to the components
    ///
    /// \throws (Implementation defined) if the entity does not
    /// have one of the requested components
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    std::tuple<Components&...> get(Entity entity)
    {
        return std::tie(pool<Components>().get(entity)...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Try to get components associated to an entity
    ///
    /// \tparam Components Component types to retrieve
    /// \param entity Entity identifier
    ///
    /// \return std::optional containing a tuple of component
    /// references if all are present, std::nullopt otherwise
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    std::optional<std::tuple<Components&...>> get_if(Entity entity)
    {
        return has<Components...>(entity) ? std::make_optional(get<Components...>(entity)) : std::nullopt;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get one component associated to an entity
    ///
    /// \tparam Component Component type to retrieve
    /// \param entity Entity identifier
    ///
    /// \return Reference to the component
    ///
    /// \throws (Implementation defined) if the entity does not
    /// have the component
    ///
    ////////////////////////////////////////////////////////////
    template <typename Component>
    Component& one(Entity entity)
    {
        return pool<Component>().get(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Visit components associated to a specified entity
    /// for given possible component types
    ///
    /// If the entity has a component among the candidate types,
    /// invokes the visitor either as visitor(Component&) or
    /// visitor(Component&, optionalIndexString) depending on
    /// invocability.
    ///
    /// \tparam Components Candidate component types
    /// \tparam Function Visitor callable
    /// \param entity Entity identifier
    /// \param visitor Callable
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components, typename Function>
    void visit(Entity entity, Function visitor)
    {
        ([this, &entity, &visitor]
        {
            if(auto tuple = get_if<Components>(entity))
            {
                if constexpr(std::is_invocable_v<Function, Components&>)
                {
                    visitor(std::get<0>(*tuple));
                }
                else
                {
                    visitor(std::get<0>(*tuple), index<Components>());
                }
            }
        }(), ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Visit a component of an entity by its string index
    ///
    /// For each candidate type, if its registered string index
    /// matches \p index ensures the component exists (default
    /// constructed if absent) then invokes visitor(Component&).
    ///
    /// \tparam Components Candidate component types
    /// \tparam Function Visitor callable
    /// \param entity Entity identifier
    /// \param index String index
    /// \param visitor Callable
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components, typename Function>
    void visit(Entity entity, const std::string& index, Function visitor)
    {
        ([this, &entity, &visitor, &index]
        {
            if(m_indexes.count(index) && typeid(Components).name() == m_indexes.at(index).name())
            {
                if(!has<Components>(entity))
                {
                    add<Components>(entity, {});
                }

                visitor(std::get<0>(get<Components>(entity)));
            }
        }(), ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Associate string indexes to component types
    ///
    /// Registers mappings used for reflective / serialized
    /// access through the index-based visit().
    ///
    /// \tparam Components Component types
    /// \param index Index tokens (string views)
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    void index(const Index<Components>&... index)
    {
        ((m_indexes.emplace(std::make_pair(index, std::type_index(typeid(Components))))), ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Copy components of an entity to a newly created entity
    ///
    /// Only components present on the source entity are copied.
    /// Subscribed views for copied component types are marked
    /// dirty.
    ///
    /// \param source Source entity
    ///
    /// \return Destination entity with copied components
    ///
    ////////////////////////////////////////////////////////////
    Entity duplicate(Entity source)
    {
        auto destination = create();

        for(auto& [id, pool]: m_pools)
        {
            if(pool->has(source))
            {
                pool->copy(source, destination);
                notify(id);
            }
        }

        return destination;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create a view associated to the current helper
    ///
    /// Returns a snapshot-style view (new View<Components...>).
    ///
    /// \tparam Components Component types required by the view
    ///
    /// \return View object
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    View<Components...> view()
    {
        return View<Components...>(*this);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Subscribe a view to this helper so it gets
    /// notified automatically when components change
    ///
    /// Adds a weak pointer entry for each observed component
    /// type; when those component pools change the view is
    /// marked dirty.
    ///
    /// \tparam Components Component types observed by the view
    /// \param view View to subscribe
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    void subscribe(View<Components...>& view)
    {
        (view.observe(m_views[typeid(Components)].emplace_back()), ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get all the entities associated to given components
    ///
    /// Builds a vector of (Entity, Components&...) for entities
    /// that possess all requested component types, iterating
    /// from the smallest pool.
    ///
    /// \tparam Components Component types required
    ///
    /// \return Vector of tuples
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    std::vector<std::tuple<Entity, Components&...>> entities()
    {
        std::vector<std::tuple<Entity, Components&...>> entities;

        auto& smallest = smallest_pool<Components...>();
        entities.reserve(smallest.size());

        for(auto& entity: smallest.entities())
        {
            if(has<Components...>(entity))
            {
                entities.emplace_back(entity, pool<Components>().get(entity)...);
            }
        }

        return entities;
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Get the component pool containing the least
    /// number of entities among the provided component types
    ///
    /// Used to optimize multi-component iteration.
    ///
    /// \tparam Components Component types to compare
    ///
    /// \return Reference to smallest IComponentSet
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    IComponentSet& smallest_pool()
    {
        IComponentSet* smallest = &pool<std::tuple_element_t<0, std::tuple<Components...>>>();
        ((smallest = pool<Components>().size() < smallest->size() ? &pool<Components>() : smallest), ...);
        return *smallest;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Notify all views observing given component type
    ///
    /// Cleans up expired weak pointers and sets the dirty flag
    /// for active observers so that their snapshots are
    /// rebuilt lazily.
    ///
    /// \param component Component type_index key
    /// \param entity (Unused) entity that triggered the change
    ///
    ////////////////////////////////////////////////////////////
    void notify(const std::type_index& component)
    {
        auto& observers = m_views[component];
        for(auto it = observers.begin(); it != observers.end();)
        {
            if((*it).expired())
            {
                it = observers.erase(it);
            }
            else if(auto observer = it->lock())
            {
                *observer = true;
                ++it;
            }
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the string index associated to a component type
    ///
    /// \tparam Component Component type
    ///
    /// \return Optional string index
    ///
    ////////////////////////////////////////////////////////////
    template <typename Component>
    std::optional<std::string> index()
    {
        for(auto [index, type]: m_indexes)
        {
            if(type == typeid(Component))
            {
                return std::make_optional(index);
            }
        }

        return std::nullopt;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::unordered_map<std::type_index, std::unique_ptr<IComponentSet>>   m_pools;    ///< Component pools keyed by typeid
    EntitySet                                                             m_entities; ///< Entity allocator / storage
    std::unordered_map<std::string, std::type_index>                      m_indexes;  ///< String index -> component type
    std::unordered_map<std::type_index, std::vector<std::weak_ptr<bool>>> m_views;    ///< Component type -> observers (dirty flags)
};

} // namespace CNtity

////////////////////////////////////////////////////////////
#include "View.hpp"