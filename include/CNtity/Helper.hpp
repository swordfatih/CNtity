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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SparseSet.hpp"

// #include <algorithm>
#include <iostream>
#include <optional>
#include <string>

namespace CNtity
{

////////////////////////////////////////////////////////////
template <typename... Components>
class View;

////////////////////////////////////////////////////////////
/// \brief Wrapper class to associate a string to a type
///
////////////////////////////////////////////////////////////
template <typename Component>
class Index : public std::string
{
    using std::string::string;
};

////////////////////////////////////////////////////////////
/// \brief Class that contains helper functions for an Entity
/// Component System architecture
///
////////////////////////////////////////////////////////////
template <typename... Types>
class Helper
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    Helper() : m_components{}, m_entities{}
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Default destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Helper()
    {
    }

    // ////////////////////////////////////////////////////////////
    // /// \brief Get registered components
    // ///
    // /// \return type_index of every registered components in an
    // /// array
    // ///
    // ////////////////////////////////////////////////////////////
    // std::vector<std::type_index> components() const
    // {
    //     std::vector<std::type_index> components;
    //     components.reserve(std::tuple_size<m_components>);

    //     std::apply(m_components, strs);
    //     for(const auto& [key, _]: m_components)
    //     {

    //         components.push_back(key);
    //     }

    //     return components;
    // }

    ////////////////////////////////////////////////////////////
    /// \brief Get all the entities associated to given components
    ///
    /// \tparam Components to look for entities in
    ///
    /// \return Unique identifier of every entities in a vector
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    std::vector<std::tuple<Entity, Components&...>> entities()
    {
        auto& min_pool = std::get<SparseSet<std::tuple_element_t<0, std::tuple<Components...>>>>(m_components).entities();

        std::vector<std::tuple<Entity, Components&...>> result;
        result.reserve(min_pool.size());

        for(auto& entity: min_pool)
        {
            if(has<Components...>(entity))
            {
                result.emplace_back(entity, pool<Components>().get(entity)...);
            }
        }

        return result;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create an entity and add components to it
    ///
    /// \param components Components to add
    ///
    /// \return Created entity
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    Entity create(const Components&... components)
    {
        auto entity = m_entities.create();

        add(entity, components...);

        return entity;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if a given entity identifier exists
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    bool match(Entity entity) const
    {
        return m_entities.has(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Add components to a specified entity
    ///
    /// \param entity Entity
    /// \param components Instances of components
    /// \tparam Components to add
    ///
    /// \return Tuple containing added components
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    std::tuple<Components&...> add(Entity entity, const Components&... components)
    {
        (pool<Components>().insert(entity, components), ...);
        // (notify(typeid(Components)), ...);

        return get<Components...>(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove components from a specified entity
    ///
    /// \param entity Entity to remove components from
    /// \tparam Components to remove
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    void remove(Entity entity)
    {
        (pool<Components>().remove(entity), ...);
        // (notify(typeid(Components)), ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove a specified entity and all its components
    ///
    /// \param entity Entity
    ///
    ////////////////////////////////////////////////////////////
    void remove(Entity entity)
    {
        std::apply([&entity](auto&&... pools)
        {
            ((pools.has(entity) ? pools.remove(entity) : void()), ...);
        }, m_components);

        m_entities.remove(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Remove specified components from all entities
    ///
    /// \tparam Components to remove
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    void remove()
    {
        (pool<Components>().clear(), ...);
        // (notify(typeid(Components)), ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if an entity contains specified components
    ///
    /// \param entity Entity
    /// \tparam Components to look for
    ///
    /// \return True if the entity has the component
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    bool has(Entity entity) const
    {
        return (pool<Components>().has(entity) && ...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get components associated to an entity
    ///
    /// Throws an exception if the entity doesn't have the
    /// specified components.
    ///
    /// \param entity The entity to look components for
    /// \tparam Components to look for
    ///
    /// \return Tuple of components
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components>
    std::tuple<Components&...> get(Entity entity)
    {
        return std::tie(pool<Components>().get(entity)...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get components associated to an entity
    ///
    /// \param entity The entity to search look for
    /// \tparam Components to look for
    ///
    /// \return An optional containing a tuple of components if
    /// found, nullopt if not found
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
    /// Throws an exception if the entity doesn't have the
    /// specified components.
    ///
    /// \param entity The entity to look components for
    /// \tparam Component to look for
    ///
    /// \return Reference to the component
    ///
    ////////////////////////////////////////////////////////////
    template <typename Component>
    Component& one(Entity entity)
    {
        return pool<Component>().get(entity);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Visit components associated to a specified
    /// entity for given possible components
    ///
    /// You may give more or less component types than what the
    /// entity really has.
    ///
    /// \param entity Entity
    /// \param visitor Visitor function
    /// \tparam Possible components to visit
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Components, typename Function>
    void visit(Entity entity, Function visitor)
    {
        (([&]
        {
            if(auto tuple = get_if<Components>(entity))
            {
                visitor(std::get<0>(*tuple));
            }
        }()),
         ...);
    }

    // ////////////////////////////////////////////////////////////
    // /// \brief Visit a component of an entity by its index
    // ///
    // /// Visit a component of an entity by its index for given
    // /// possible types.
    // /// Adds a new component to the entity if it doesn't have
    // /// a given component. The component has to be default
    // /// constructible.
    // /// This function is useful for (de)serialization.
    // ///
    // /// \param entity Entity
    // /// \param index Index of the component
    // /// \param visitor Visitor function
    // /// \tparam Possible components to visit
    // ///
    // /// \see index
    // ///
    // ////////////////////////////////////////////////////////////
    // template <typename... Components, typename Function>
    // void visit(Entity entity, const std::string& index, Function visitor)
    // {
    //     ([this, &entity, &visitor, &index]
    //     {
    //         if(m_indexes.count(index) && typeid(Components).name() == m_indexes.at(index).name())
    //         {
    //             if(!has<Components>(entity))
    //             {
    //                 add<Components>(entity, {});
    //             }

    //             visitor(std::get<0>(get<Components>(entity)));
    //         }
    //     }(), ...);
    // }

    // ////////////////////////////////////////////////////////////
    // /// \brief Associate indexes to components for (de)serialization
    // ///
    // /// \param index Indexes to associate, in the same orders as
    // /// components
    // /// \tparam Components to associate
    // ///
    // ////////////////////////////////////////////////////////////
    // template <typename... Components>
    // void index(const Index<Components>&... index)
    // {
    //     ((m_indexes.emplace(std::make_pair(index, std::type_index(typeid(Components))))), ...);
    // }

    ////////////////////////////////////////////////////////////
    /// \brief Copy components of an entity to another
    ///
    /// \param source Source entity
    /// \param destination Destination entity, creates a new one
    /// if not specified.
    ///
    ////////////////////////////////////////////////////////////
    Entity duplicate(Entity source)
    {
        auto destination = create();

        std::apply([&source, &destination](auto&&... pools)
        {
            (([&]
            {
                if(pools.has(source))
                {
                    pools.insert(destination, pools.get(source));
                    // notify(id);
                }
            }()),
             ...);
        }, m_components);

        return destination;
    }

    // ////////////////////////////////////////////////////////////
    // /// \brief Add view to views-to-notify lists
    // ///
    // /// Create an observer weak pointer for every
    // /// components associated to a view.
    // ///
    // ////////////////////////////////////////////////////////////
    // template <typename... Components>
    // void subscribe(View<Entity, Components...>& view)
    // {
    //     (view.observe(m_views[typeid(Components)].emplace_back()), ...);
    // }

    // ////////////////////////////////////////////////////////////
    // /// \brief Create a view associated to the current helper
    // ///
    // /// Shortcut function to create a view associated to
    // /// the current helper instance observing specified
    // /// components.
    // ///
    // /// \code
    // /// auto view = helper.view<std::string>();
    // /// \endcode
    // ///
    // ////////////////////////////////////////////////////////////
    // template <typename... Components>
    // View<Entity, Components...> view()
    // {
    //     return View<Entity, Components...>(*this);
    // }

private:
    ////////////////////////////////////////////////////////////
    template <typename Component>
    SparseSet<Component>& pool()
    {
        return std::get<SparseSet<Component>>(m_components);
    }

    ////////////////////////////////////////////////////////////
    template <typename Component>
    const SparseSet<Component>& pool() const
    {
        return std::get<SparseSet<Component>>(m_components);
    }

    // ////////////////////////////////////////////////////////////
    // /// \brief Random generator using the standard library
    // ///
    // /// Used for the generation of UUID.
    // ///
    // ////////////////////////////////////////////////////////////
    // static std::mt19937& random_generator()
    // {
    //     static std::mt19937 random_generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    //     return random_generator;
    // }

    // ////////////////////////////////////////////////////////////
    // /// \brief Notify all views observing given components
    // ///
    // /// \param component Type index of the component.
    // ///
    // ////////////////////////////////////////////////////////////
    // template <typename Component>
    // void notify()
    // {
    //     auto& observers = m_views[component];
    //     for(auto it = observers.begin(); it != observers.end();)
    //     {
    //         if((*it).expired())
    //         {
    //             it = observers.erase(it);
    //         }
    //         else if(auto observer = it->lock())
    //         {
    //             *observer = true;
    //             ++it;
    //         }
    //     }
    // }

    // ////////////////////////////////////////////////////////////
    // /// \brief Get the string index associated to a component
    // /// by it's type
    // ///
    // /// \tparam Component to get index for
    // ///
    // /// \return Optional index
    // ///
    // ////////////////////////////////////////////////////////////
    // template <typename Component>
    // std::optional<std::string> index()
    // {
    //     for(auto [index, type]: m_indexes)
    //     {
    //         if(type == typeid(Component))
    //         {
    //             return std::make_optional(index);
    //         }
    //     }

    //     return std::nullopt;
    // }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::tuple<SparseSet<Types>...> m_components; ///< Component pools
    Entities                        m_entities;   ///< Entities
    // SparseSet<Entity, bool>                 m_entities; ///< Entities
    // std::map<std::type_index, std::vector<std::weak_ptr<bool>>> m_views;    ///< Views
    // std::map<std::string, std::type_index>                      m_indexes;  ///< Component indexes
};

// ////////////////////////////////////////////////////////////
// /// \brief Wrapper class to store and iterate over entities
// /// which contain specified components
// ///
// /// \tparam Components for entities to have
// ///
// ////////////////////////////////////////////////////////////
// template <typename Entity, typename... Components>
// class View
// {
// public:
//     ////////////////////////////////////////////////////////////
//     /// \brief Constructor to associate the view to a helper
//     ///
//     /// \param helper The associated helper
//     /// \param subscribe If false, the helper wont update this view
//     /// automatically. Defaults to true.
//     ///
//     ////////////////////////////////////////////////////////////
//     View(Helper<Entity>& helper, bool subscribe = true) : m_helper(helper), m_update(std::make_shared<bool>(true))
//     {
//         if(subscribe)
//         {
//             m_helper.subscribe(*this);
//         }
//     }

//     ////////////////////////////////////////////////////////////
//     /// \brief Default destructor
//     ///
//     ////////////////////////////////////////////////////////////
//     ~View()
//     {
//     }

//     ////////////////////////////////////////////////////////////
//     /// \brief Retrieve entities from the helper
//     ///
//     ////////////////////////////////////////////////////////////
//     void update()
//     {
//         m_entities = std::move(m_helper.entities<Components...>());
//         *m_update = false;
//     }

//     ////////////////////////////////////////////////////////////
//     /// \brief Execute a callback for every entities that
//     /// contain specified components
//     ///
//     /// \param callback Function to call for each entities
//     ///
//     ////////////////////////////////////////////////////////////
//     template <typename Function>
//     void each(Function&& callback)
//     {
//         if(*m_update)
//         {
//             update();
//         }

//         for(auto&& components: m_entities)
//         {
//             std::apply(callback, components);
//         }
//     }

//     ////////////////////////////////////////////////////////////
//     /// \brief Get entities that contains specified components
//     ///
//     /// \return Array of tuples containing the entity's identifier
//     /// and specified components
//     ///
//     ////////////////////////////////////////////////////////////
//     std::vector<std::tuple<Entity, Components&...>>& each()
//     {
//         if(*m_update)
//         {
//             update();
//         }

//         return m_entities;
//     }

//     ////////////////////////////////////////////////////////////
//     /// \brief Update a weak pointer with the update boolean
//     ///
//     /// \param observer Weak pointer to update
//     ///
//     ////////////////////////////////////////////////////////////
//     void observe(std::weak_ptr<bool>& observer)
//     {
//         observer = m_update;
//     }

// private:
//     ////////////////////////////////////////////////////////////
//     // Member data
//     ////////////////////////////////////////////////////////////
//     Helper<Entity>&                                 m_helper;
//     std::vector<std::tuple<Entity, Components&...>> m_entities;
//     std::shared_ptr<bool>                           m_update;
// };

} // namespace CNtity