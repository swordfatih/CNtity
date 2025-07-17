#include "CNtity/Helper.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace CNtity;

TEST_CASE("Entities can be created and are unique", "[Entity]")
{
    Helper helper;

    Entity entity1 = helper.create();
    Entity entity2 = helper.create();

    REQUIRE(helper.match(entity1));
    REQUIRE(helper.match(entity2));
    REQUIRE(entity1 != entity2);
}

TEST_CASE("Removed entities are invalid", "[Entity]")
{
    Helper helper;
    Entity entity = helper.create();

    REQUIRE(helper.match(entity));

    helper.remove(entity);

    REQUIRE_FALSE(helper.match(entity));
}

TEST_CASE("Entities can be reused after removal", "[Entity]")
{
    Helper helper;

    Entity entity1 = helper.create();
    helper.remove(entity1);

    Entity entity2 = helper.create();

    REQUIRE_FALSE(helper.match(entity1));
    REQUIRE(CNtity::get_index(entity1) == CNtity::get_index(entity2));
    REQUIRE_FALSE(CNtity::get_generation(entity1) == CNtity::get_generation(entity2));
    REQUIRE(helper.match(entity2));
}

TEST_CASE("Adding components to entities works", "[Entity][components]")
{
    Helper helper;

    Entity entity = helper.create();
    helper.add<int>(entity, 42);

    REQUIRE(helper.has<int>(entity));
    REQUIRE(helper.one<int>(entity) == 42);
}

TEST_CASE("Removing components from entities works", "[Entity][components]")
{
    Helper helper;

    Entity entity = helper.create();
    helper.add<int>(entity, 99);

    REQUIRE(helper.has<int>(entity));

    helper.remove<int>(entity);

    REQUIRE_FALSE(helper.has<int>(entity));
}
