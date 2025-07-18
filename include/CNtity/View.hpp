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

#include "Helper.hpp"

#include <functional>
#include <stdexcept>

namespace CNtity
{

////////////////////////////////////////////////////////////
/// \brief Wrapper class to store and iterate over entities
/// which contain specified components
///
/// The View *stores* a snapshot of the entities (and component
/// references) locally inside an internal vector. The snapshot
/// is (re)built lazily on first access after it becomes
/// invalid.
///
/// A shared boolean flag (\c m_update) marks the snapshot as
/// dirty. When constructed with \p subscribe = true the Helper
/// will keep a weak reference to that flag (through observe())
/// so it can mark the view for update when the underlying
/// entity/component composition changes.
///
/// Access patterns:
/// - each(F&&) : applies a callback to every tuple
/// - each()    : returns the vector for manual iteration
/// - range-based for : via begin()/end() iterators
///
/// \tparam Components Component types entities must have
///
////////////////////////////////////////////////////////////
template <typename... Components>
class View
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Constructor to associate the view to a helper
    ///
    /// Creates a view bound to \p helper. If \p subscribe is
    /// true the view registers itself so that it will be marked
    /// dirty automatically when relevant changes occur.
    ///
    /// \param helper The associated Helper
    /// \param subscribe If false, the helper won't update this
    ///                  view automatically. Defaults to true.
    ///
    ////////////////////////////////////////////////////////////
    View(Helper& helper, bool subscribe = true)
        : m_helper(helper), m_update(std::make_shared<bool>(true))
    {
        if(subscribe)
        {
            m_helper.subscribe(*this);
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Default destructor
    ////////////////////////////////////////////////////////////
    ~View() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve entities from the helper (rebuild view)
    ///
    /// Rebuilds the internal snapshot of (Entity, Components&...)
    /// tuples by querying the helper, then marks the view as
    /// up-to-date.
    ///
    ////////////////////////////////////////////////////////////
    void update()
    {
        m_entities = std::move(m_helper.entities<Components...>());
        *m_update = false;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Execute a callback for every entity (old-style)
    ///
    /// Lazily updates the snapshot if needed, then applies
    /// \p callback to each stored tuple. The callback is
    /// invoked with (Entity, Components&...) exactly as stored
    /// in the tuple.
    ///
    /// \tparam Function Callable type
    /// \param callback Function to call for each entity tuple
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
    /// \brief Get entities directly (old-style)
    ///
    /// Returns the internal snapshot vector (lazily updated if
    /// dirty). The tuples contain (Entity, Components&...).
    ///
    /// \return Reference to internal vector of tuples
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
    /// \brief Iterator to allow range-based for loops directly
    ///
    /// Provides forward iteration over the cached snapshot of
    /// tuples. Does not perform validity checks—snapshot is
    /// guaranteed up-to-date by begin()/end().
    ///
    ////////////////////////////////////////////////////////////
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::tuple<Entity, Components&...>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        ////////////////////////////////////////////////////////
        /// \brief Construct iterator from underlying vector iterator
        ///
        /// \param it Iterator into the internal storage vector
        ///
        ////////////////////////////////////////////////////////
        Iterator(typename std::vector<value_type>::iterator it) : m_it(it) {}

        ////////////////////////////////////////////////////////
        /// \brief Inequality comparison
        ///
        /// \param other Other iterator
        /// \return True if underlying iterators differ
        ///
        ////////////////////////////////////////////////////////
        bool operator!=(const Iterator& other) const { return m_it != other.m_it; }

        ////////////////////////////////////////////////////////
        /// \brief Pre-increment
        ///
        /// \return *this advanced to next element
        ///
        ////////////////////////////////////////////////////////
        Iterator& operator++()
        {
            ++m_it;
            return *this;
        }

        ////////////////////////////////////////////////////////
        /// \brief Dereference
        ///
        /// \return Reference to current (Entity, Components&...) tuple
        ///
        ////////////////////////////////////////////////////////
        value_type& operator*() { return *m_it; }

    private:
        typename std::vector<value_type>::iterator m_it; ///< Underlying vector iterator
    };

    ////////////////////////////////////////////////////////////
    /// \brief Begin iterator (for range-based for)
    ///
    /// Ensures snapshot is up-to-date before returning.
    ///
    /// \return Iterator to first tuple
    ///
    ////////////////////////////////////////////////////////////
    Iterator begin()
    {
        if(*m_update)
        {
            update();
        }
        return Iterator(m_entities.begin());
    }

    ////////////////////////////////////////////////////////////
    /// \brief End iterator (for range-based for)
    ///
    /// Ensures snapshot is up-to-date before returning.
    ///
    /// \return Iterator past-the-end
    ///
    ////////////////////////////////////////////////////////////
    Iterator end()
    {
        if(*m_update)
        {
            update();
        }
        return Iterator(m_entities.end());
    }

    ////////////////////////////////////////////////////////////
    /// \brief Update a weak pointer with the update boolean
    ///
    /// Assigns \p observer to reference the shared dirty flag.
    /// Helper can later lock this weak_ptr to flag the view for
    /// a rebuild.
    ///
    /// \param observer Weak pointer to receive the update flag
    ///
    ////////////////////////////////////////////////////////////
    void observe(std::weak_ptr<bool>& observer)
    {
        observer = m_update;
    }

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Helper&                                         m_helper;   ///< Helper providing entity/component access
    std::vector<std::tuple<Entity, Components&...>> m_entities; ///< Cached snapshot of (Entity, Components&...)
    std::shared_ptr<bool>                           m_update;   ///< Shared dirty flag (true = needs rebuild)
};

} // namespace CNtity