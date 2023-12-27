//CNtity
#include "CNtity/Helper.hpp"
#include <iostream>

////////////////////////////////////////////////////////////
struct Position
{
    float x;
    float y;
};

////////////////////////////////////////////////////////////
struct Velocity
{
    float x;
    float y;
};

////////////////////////////////////////////////////////////
struct Health
{
    int max;
    int current;
};

////////////////////////////////////////////////////////////
int main()
{
    //Helper
    CNtity::Helper helper;

    //Creating entities
    auto chat = helper.create<std::string, Health, int>("chat", {100, 80}, 10);
    helper.create<std::string>("chien");
    helper.create<std::string, Position>("velociraptor", {25, 70});

    //Adding component, changing values
    auto [position] = helper.add<Position>(chat, {50, 50});
    position.x += 50;

    std::any_cast<Health&>(helper.get(chat)[0].get()) = {50, 50};

    //System 1
    helper.each<std::string, Position, int>([&helper](auto entity, auto name, auto& position, auto _)
    {       
        if(name == "chat")
        {
            position.x += 10;
        }

        std::cout << name << std::endl;  
    });

    //System 2
    for(auto [entity, name, position]: helper.each<std::string, Position>())
    {
        if(name == "chat")
        {
            position.x += 200; 
            position.y += 70;
        }

        std::cout << name << ' ' << position.x << ' ' << position.y << std::endl;  
    }

    if(auto values = helper.get_if<Health, Position>(chat))
    {
        auto [health, position] = *values;
        health.current += 10;
        position.x += 5;
        std::cout << health.current << std::endl;
    }

    if(helper.has<Health, Position>(chat))
    {
        auto [health, position] = helper.get<Health, Position>(chat);
        health.current += 5;
        std::cout << health.current << std::endl; 
    }

    //Removing a component, erasing an entity
    helper.remove<Position>(chat);
    helper.erase(chat);

    return 0;
}