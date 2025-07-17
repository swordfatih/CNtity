#include "CNtity/Helper.hpp"
#include "Components.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace CNtity;

TEST_CASE("View updates when components are added or removed", "[View]")
{
    Helper helper;
    auto   entity1 = helper.create(Position{1, 1}, Velocity{2, 2});
    auto   entity2 = helper.create(Position{3, 3});
    auto   view = helper.view<Position, Velocity>();

    int counter = 0;
    view.each([&](Entity, Position& p, Velocity& v)
    {
        ++counter;
        REQUIRE((p == Position{1, 1} || p == Position{3, 3}));
        REQUIRE(v == Velocity{2, 2});
    });
    REQUIRE(counter == 1);

    helper.add<Velocity>(entity2, Velocity{4, 4});
    counter = 0;
    view.each([&](Entity, Position& p, Velocity& v)
    {
        ++counter;
        REQUIRE((p == Position{1, 1} || p == Position{3, 3}));
    });
    REQUIRE(counter == 2);

    helper.remove<Velocity>(entity1);
    counter = 0;
    view.each([&](Entity, Position& p, Velocity& v)
    {
        ++counter;
        REQUIRE(p == Position{3, 3});
        REQUIRE(v == Velocity{4, 4});
    });
    REQUIRE(counter == 1);
}

TEST_CASE("View reflects entity duplication", "[View]")
{
    Helper helper;
    auto   entity1 = helper.create(Position{5, 6}, Velocity{1, 1});
    auto   entity2 = helper.duplicate(entity1);
    auto   view = helper.view<Position, Velocity>();

    int counter = 0;
    view.each([&](Entity, Position& p, Velocity& v)
    {
        ++counter;
        REQUIRE(p == Position{5, 6});
        REQUIRE(v == Velocity{1, 1});
    });
    REQUIRE(counter == 2);
}

TEST_CASE("View updates after removing entities", "[View]")
{
    Helper helper;
    auto   entity1 = helper.create(Position{1, 1}, Velocity{2, 2});
    auto   entity2 = helper.create(Position{2, 2}, Velocity{3, 3});
    auto   view = helper.view<Position, Velocity>();

    int counter = 0;
    view.each([&](Entity, Position& p, Velocity& v)
    {
        ++counter;
        REQUIRE((p == Position{1, 1} || p == Position{2, 2}));
    });
    REQUIRE(counter == 2);

    helper.remove(entity1);
    counter = 0;
    view.each([&](Entity, Position& p, Velocity& v)
    {
        ++counter;
        REQUIRE(p == Position{2, 2});
        REQUIRE(v == Velocity{3, 3});
    });
    REQUIRE(counter == 1);
}

TEST_CASE("View updates after adding new entities", "[View]")
{
    Helper helper;
    auto   view = helper.view<Position, Velocity>();

    REQUIRE(view.each().empty());

    auto entity1 = helper.create(Position{7, 7});
    REQUIRE(view.each().empty());

    helper.add<Velocity>(entity1, Velocity{1, 2});
    REQUIRE(view.each().size() == 1);
    REQUIRE(std::get<1>(view.each()[0]) == Position{7, 7});
    REQUIRE(std::get<2>(view.each()[0]) == Velocity{1, 2});
}

TEST_CASE("View handles multiple updates correctly", "[View]")
{
    Helper helper;
    auto   entity1 = helper.create(Position{1, 1}, Velocity{2, 2});
    auto   view = helper.view<Position, Velocity>();

    REQUIRE(view.each().size() == 1);

    helper.remove<Velocity>(entity1);
    REQUIRE(view.each().empty());

    helper.add<Velocity>(entity1, Velocity{5, 5});
    REQUIRE(view.each().size() == 1);
    REQUIRE(std::get<2>(view.each()[0]) == Velocity{5, 5});
}
