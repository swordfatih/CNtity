// CNtity
#include "CNtity/Helper.hpp"

////////////////////////////////////////////////////////////
struct Position
{
    float x, y;

    std::string to_string()
    {
        return "Health: " + std::to_string(x) + "/" + std::to_string(y) + ")";
    }
};

////////////////////////////////////////////////////////////
struct Health
{
    int max, current;

    std::string to_string()
    {
        return "Health: " + std::to_string(max) + "/" + std::to_string(current) + ")";
    }
};

////////////////////////////////////////////////////////////
int main()
{
    // Helper
    CNtity::Helper helper;

    // Creating entities
    auto chat = helper.create<std::string, Health>("chat", {100, 80});
    helper.create<std::string>("chien");
    helper.create<std::string, Position>("velociraptor", {25, 70});

    // Adding component, changing values
    auto [position] = helper.add<Position>(chat, {50, 50});
    position.x += 50;

    // Duplicate an entity
    auto clone = helper.duplicate(chat);
    helper.add<std::string>(clone, "clone de chat");

    // Visit components of an entity
    helper.visit<Position, Health>(chat, [](auto component, auto index)
    {
        component.to_string();
    });

    // View
    // or: auto view = helper.view<std::string, Position>();
    CNtity::View<std::string, Position> view{helper};

    // System 1
    view.each([](auto entity, auto name, auto& position)
    {
        position.x += 10;
    });

    // System 2
    for(auto [entity, name, position]: view.each())
    {
        if(name == "chat")
        {
            position.x += 10;
            break;
        }
    }

    // Get 1
    if(auto values = helper.get_if<Health, Position>(chat))
    {
        auto [health, position] = *values;
        health.current += 5;
    }

    // Get 2
    if(helper.has<Health, Position>(chat))
    {
        auto [health, position] = helper.get<Health, Position>(chat);
        health.current += 5;
    }

    helper.remove<Position>(chat); // Removes component from an entity
    helper.remove<std::string>();  // Removes component from all entities
    helper.remove(chat);           // Removes an entity

    return 0;
}