# CNtity
CNtity (Chats Noirs Entity) is a header-only helper library for an Entity Component System (ECS) implementation using C++17. It is actually in development by Fatih#6810 (accfldekur@gmail.com) from *Moonlit Black Cats*.
***

### How to use

CNtity is header-only, you just need a compiler that supports C++17 and to include `CNtity/Helper.hpp` after downloading files in the include directory.

**Example code**

An example code showing how maniable CNtity is!
```cpp
//CNtity
#include "CNtity/Helper.hpp"

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
    CNtity::Helper<Position, Velocity, Health, std::string> helper;

    //Creating entities
    auto chat = helper.create<std::string>("chat");
    helper.create<std::string>("chien");
    helper.create<std::string, Position>("velociraptor", {25, 70});

    //Adding component, changing values
    auto [position] = helper.add<Position>(chat, {50, 50});
    position.x += 50;

    //System 1
    helper.for_each<std::string, Position>({[](auto entity, auto tuple)
    {       
        auto [name, position] = tuple;

        if(name == "chat")
        {
            position.x += 10;
        }
    }});

    //System 2
    for(auto [entity, tuple]: helper.acquire<std::string, Position>())
    {
        auto [name, position] = tuple;

        if(name == "chat")
        {
            position.x += 200;
            position.y += 70;
        }
    }

    //Removing a component, erasing an entity
    helper.remove<Position>(chat);
    helper.erase(chat);

    return 0;
}
```