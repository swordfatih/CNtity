// CNtity
#include "CNtity/Helper.hpp"

#include <iostream>
#include <map>

////////////////////////////////////////////////////////////
struct Position
{
    float x, y;

    std::string to_string()
    {
        return std::to_string(x) + " " + std::to_string(y);
    }

    void from_string(std::string string)
    {
        std::stringstream ss(string);
        ss >> string;
        x = std::stof(string);
        ss >> string;
        y = std::stof(string);
    }
};

////////////////////////////////////////////////////////////
struct Health
{
    int max, current;

    std::string to_string()
    {
        return std::to_string(max) + " " + std::to_string(current);
    }

    void from_string(std::string string)
    {
        std::stringstream ss(string);
        ss >> string;
        max = std::stoi(string);
        ss >> string;
        current = std::stoi(string);
    }
};

////////////////////////////////////////////////////////////
std::vector<std::pair<std::string, std::string>> serialize(CNtity::Helper& helper, const CNtity::Entity& entity)
{
    std::vector<std::pair<std::string, std::string>> components;

    helper.visit<Position, Health>(entity, [&helper, &components](auto component, auto index)
    {
        if(index)
        {
            components.push_back(std::make_pair(index.value(), component.to_string()));
        }
    });

    return components;
}

////////////////////////////////////////////////////////////
CNtity::Entity deserialize(CNtity::Helper& helper, const std::vector<std::pair<std::string, std::string>>& components)
{
    auto entity = helper.create();

    for(auto [index, data]: components)
    {
        helper.visit<Position, Health>(entity, index, [&data](auto& component)
        {
            component.from_string(data);
        });
    }

    return entity;
}

////////////////////////////////////////////////////////////
int main()
{
    CNtity::Helper helper;
    helper.index<Position, Health>("position", "health");

    auto entity = helper.create<Position, Health>({5.2, 10.3}, {80, 100});

    auto components = serialize(helper, entity);

    helper.remove(entity);

    // [...] write to file, load from file

    entity = deserialize(helper, components);

    if(auto tuple = helper.get_if<Position, Health>(entity))
    {
        // print in a generic way
        std::apply([](auto&&... args)
        {
            ((std::cout << args.to_string() << std::endl), ...);
        }, *tuple);
    }

    return 0;
}