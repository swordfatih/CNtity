//CNtity
#include "CNtity/Helper.hpp"
#include <iostream>

////////////////////////////////////////////////////////////
struct Position
{
    float x;
    float y;

    void serialize()
    {
        std::cout << "serialize Position: " << x << " " << y << std::endl;
    }
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

    void serialize()
    {
        std::cout << "serialize Health: " << max << " " << current << std::endl;
    }
};

////////////////////////////////////////////////////////////
/// \brief Class to check if a given class has a member
/// function signed serialize()
///
////////////////////////////////////////////////////////////
template<typename, typename T>
class check_serializer;

////////////////////////////////////////////////////////////
/// \brief Specialization of check_serializer
///
////////////////////////////////////////////////////////////
template<typename C, typename Ret, typename... Args>
class check_serializer<C, Ret(Args...)>
{
    template<typename T> static constexpr auto check_serialize(T*) -> typename std::is_same<decltype(std::declval<T>().serialize(std::declval<Args>()...)), Ret>::type;
    template<typename T> static constexpr auto check_deserialize(T*) -> typename std::is_same<decltype(std::declval<T>().deserialize(std::declval<Args>()...)), Ret>::type;

    template<typename> static constexpr std::false_type check_serialize(...);
    template<typename> static constexpr std::false_type check_deserialize(...);

    typedef decltype(check_serialize<C>(0)) type_serialize;
    typedef decltype(check_deserialize<C>(0)) type_deserialize;

public:
    static constexpr bool value_serialize = type_serialize::value;
    static constexpr bool value_deserialize = type_deserialize::value;
};

////////////////////////////////////////////////////////////
template <typename Type>
constexpr bool has_serializer(Type& object)
{
    return check_serializer<Type, void()>::value_serialize;
}

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

    //Duplicate a component
    auto chat_copy = helper.duplicate(chat);
    helper.add<std::string>(chat_copy, "copie de chat");

    //Entities
    std::cout << helper.entities().size() << std::endl;
    std::cout << helper.entities<Position>().size() << std::endl;

    //Visit components of an entity
    helper.visit<Position, std::string, Health>(chat, [](auto component)
    {
        if constexpr(has_serializer(component)) 
        {
            component.serialize();
        }
    });

    //System 1
    helper.each<std::string, Position>([&helper](auto entity, auto name, auto& position)
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