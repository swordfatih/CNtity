#include "CNtity/Helper.hpp"
#include "Components.hpp"

#include <algorithm>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace CNtity;

TEST_CASE("Helper starts empty", "[Helper][construction]")
{
    Helper helper;

    REQUIRE(helper.entities().empty());
    REQUIRE(helper.components().empty());
}

TEST_CASE("Create() creates entities and populates components", "[Helper][create][add]")
{
    Helper helper;

    Entity entity1 = helper.create(Position{1, 2}, Velocity{3.f, 4.f});

    SECTION("Entity ID is returned and stored")
    {
        REQUIRE(helper.match(entity1));
        REQUIRE_FALSE(helper.entities().empty());
    }

    SECTION("Components are present and retrievable")
    {
        REQUIRE(helper.has<Position, Velocity>(entity1));

        auto [pos, vel] = helper.get<Position, Velocity>(entity1);
        REQUIRE(pos == Position{1, 2});
        REQUIRE(vel == Velocity{3.f, 4.f});


        auto maybe = helper.get_if<Position, Velocity>(entity1);
        REQUIRE(maybe.has_value());
        auto [pos2, vel2] = *maybe;
        REQUIRE(pos2 == pos);
        REQUIRE(vel2 == vel);
    }

    SECTION("Non‑existent component is reported correctly")
    {
        REQUIRE_FALSE(helper.has<Health>(entity1));
        REQUIRE_FALSE(helper.get_if<Health>(entity1).has_value());
    }
}

TEST_CASE("Add(), remove() and has() operate as expected", "[Helper][add][remove][has]")
{
    Helper helper;
    Entity entity = helper.create();

    REQUIRE(helper.has<Position>(entity) == false);

    auto& pos = std::get<0>(helper.add<Position>(entity, Position{5, 6}));
    REQUIRE(pos == Position{5, 6});
    REQUIRE(helper.has<Position>(entity));

    std::get<0>(helper.add<Position>(entity, Position{7, 8}));
    REQUIRE(helper.one<Position>(entity) == Position{7, 8});

    helper.remove<Position>(entity);
    REQUIRE_FALSE(helper.has<Position>(entity));

    REQUIRE_NOTHROW(helper.remove<Velocity>(entity));
}

TEST_CASE("remove<Components>() clears entire pools", "[Helper][remove_pool]")
{
    Helper helper;
    auto   entity1 = helper.create(Position{1, 1});
    auto   entity2 = helper.create(Position{2, 2});

    REQUIRE(helper.components().size() == 1);
    REQUIRE(helper.has<Position>(entity1));
    REQUIRE(helper.has<Position>(entity2));

    helper.remove<Position>();

    REQUIRE(helper.components().empty());
    REQUIRE_FALSE(helper.has<Position>(entity1));
    REQUIRE_FALSE(helper.has<Position>(entity2));
}

TEST_CASE("duplicate() clones an entity and its components", "[Helper][duplicate]")
{
    Helper helper;
    Entity original = helper.create(Position{10, 20}, Velocity{1.f, 2.f});

    Entity clone = helper.duplicate(original);
    REQUIRE(clone != original);
    REQUIRE(helper.has<Position, Velocity>(clone));

    auto [posClone, velClone] = helper.get<Position, Velocity>(clone);
    REQUIRE(posClone == Position{10, 20});
    REQUIRE(velClone == Velocity{1.f, 2.f});


    helper.one<Position>(clone).x = 99;
    REQUIRE(helper.one<Position>(original).x == 10);
    REQUIRE(helper.one<Position>(clone).x == 99);
}

TEST_CASE("remove(Entity) eliminates entity and all its components", "[Helper][remove_entity]")
{
    Helper helper;
    Entity entity = helper.create(Position{3, 3}, Velocity{0.f, 0.f});

    REQUIRE(helper.match(entity));
    helper.remove(entity);

    REQUIRE_FALSE(helper.match(entity));
    REQUIRE_FALSE(helper.has<Position>(entity));
    REQUIRE(helper.entities().empty());
}

TEST_CASE("components() returns all registered component typeids", "[Helper][components]")
{
    Helper helper;
    auto   entity1 = helper.create(Position{1, 1});
    auto   entity2 = helper.create();
    helper.add<Velocity>(entity2, Velocity{2.f, 2.f});

    auto vec = helper.components();

    REQUIRE(vec.size() == 2);
    REQUIRE(std::find(vec.begin(), vec.end(), std::type_index(typeid(Position))) != vec.end());
    REQUIRE(std::find(vec.begin(), vec.end(), std::type_index(typeid(Velocity))) != vec.end());
}

TEST_CASE("entities<>() returns tuples of matching entities & components", "[Helper][entities_template]")
{
    Helper helper;

    auto entity1 = helper.create(Position{1, 1}, Velocity{1.f, 1.f});
    auto entity2 = helper.create(Position{2, 2});

    auto list = helper.entities<Position, Velocity>();
    REQUIRE(list.size() == 1);
    auto [entity, pos, vel] = list.front();
    REQUIRE(entity == entity1);
    REQUIRE(pos == Position{1, 1});
    REQUIRE(vel == Velocity{1.f, 1.f});
}

TEST_CASE("index() associates string identifiers to component types", "[Helper][index]")
{
    constexpr Index<Position> kPosIdx{"pos"};
    constexpr Index<Velocity> kVelIdx{"vel"};
    constexpr Index<Health>   kHpIdx{"hp"};

    Helper helper;
    helper.index(kPosIdx, kVelIdx, kHpIdx);

    Entity entity = helper.create(Position{4, 5});

    int counter = 0;
    helper.visit<Position, Velocity, Health>(entity, [&](auto& comp, std::optional<std::string> idx)
    {
        ++counter;

        if constexpr(std::is_same_v<std::decay_t<decltype(comp)>, Position>)
        {
            REQUIRE(comp == Position{4, 5});
            REQUIRE(idx.has_value());
            REQUIRE(*idx == kPosIdx);
        }
    });
    REQUIRE(counter == 1);

    helper.visit<Health>(entity, [](auto& h)
    {
        REQUIRE(h.hp == 100);
        h.hp = 42;
    });
    REQUIRE_FALSE(helper.has<Health>(entity));
}

TEST_CASE("pool<Component>() lazily creates a pool and maintains size", "[Helper][pool]")
{
    Helper helper;

    auto& posPool = helper.pool<Position>();
    REQUIRE(posPool.size() == 0);

    auto entity = helper.create(Position{8, 8});
    REQUIRE(posPool.size() == 1);

    helper.remove(entity);
    REQUIRE(posPool.size() == 0);
}

TEST_CASE("one() returns reference to single component", "[Helper][one]")
{
    Helper helper;
    auto   entity = helper.create(Velocity{5.f, 6.f});

    auto& vel = helper.one<Velocity>(entity);
    REQUIRE(vel == Velocity{5.f, 6.f});

    vel.vx = 9.f;
    REQUIRE(helper.one<Velocity>(entity).vx == Catch::Approx(9.f));
}