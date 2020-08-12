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
    auto chat = helper.create<std::string>({"chat"});
    helper.create<std::string>({"chien"});
    helper.create<std::string, Position>({"velociraptor"}, {25, 70});

    //Adding component, changing values
    auto position = helper.add<Position>(chat, {50, 50});
    position->x += 50;

    //System 1
    helper.for_each<std::string, Position>([&helper](auto entity, auto identity)
    {
        if(*identity == "chat")
        {
            auto position = helper.get<Position>(entity);
            position->x = 200;
            position->y = 70;

            return;
        }
    });

    //System 2
    for(const auto& it: helper.acquire<std::string, Position>())
    {
        if(*helper.get<std::string>(it) == "chat")
        {
            auto position = helper.get<Position>(it);
            position->x = 200;
            position->y = 70;
        }
    }

    //Removing a component, erasing an entity
    helper.remove<Position>(chat);
    helper.erase(chat);

    return 0;
}
```

***

### Help

Many functions you need for your ECS are provided in the helper class.

Functions | Description
------- | -----------
`components()` | Get registered components.
`entities()` | Get all the entities.
`create<Component>(identifier)` | Creates an entity with or without components, a defined identifier can be used, otherwise, by default, a new unique identifier will be generated.
`add<Component>(entity)` | Add a component to an entity.
`remove<Component>(entity)` | Remove a component from an entity.
`get<Component>(entity)` | Get a specified component of an entity.
`retrieve(entity)` | Retrieve pointers on every components associated to a specified entity.
`has<Component>(entity)` | Check if an entity has a specified component.
`erase(entity)` | Erase an entity.
`for_each<Component>(function)` | Execute a given function for each entities containing specified components.
`acquire<Component>()` | Get a vector of entities containing specified components.

***

### Performances

I have tried to make CNtity as fast as I could! Here is a little benchmark I have done (Intel Core i5-8300H CPU @ 2.30GHz) between CNtity and EntityX (another ECS library, most referenced on google for me). It's probably not accurate but it will help you to make yourself an idea about CNtity. [Here](https://github.com/swordfatih/CNtity/blob/master/benchmark.cpp) is the source code.


|   | Entities | Iterations | Probability | Create and add | `each` one component | `each` two component | `acquire` one component | `acquire` two component |
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| CNtity | 1'000 | 100 | 3 | 13 ms | 0 ms | 0 ms | 0 ms | 0 ms |
| EntityX | 1'000 | 100 | 3 | 0 ms | 0 ms | 1 ms | / | / |
| CNtity | 1'000 | 1'000'000 | 3 | 12 ms | 69 ms | 86 ms | 63 ms | 74 ms |
| EntityX | 1'000 | 1'000'000 | 3 | 0 ms | 7'523 ms | 15'550 ms | / | / |
| CNtity | 10'000 | 1'000'000 | 3 | 123 ms | 73 ms | 88 ms | 61 ms | 70 ms |
| EntityX | 10'000 | 1'000'000 | 3 | 0 ms | 76'136 ms | 172'437 ms | / | / |
| CNtity | 30'000 | 100'000 | 3 | 401 ms | 6 ms | 7 ms | 7 ms | 8 ms |
| EntityX | 30'000 | 100'000 | 3 | 2 ms | 24'571 ms | 54'553 ms | / | / |
| CNtity | 100'000 | 100'000 | 5 | 1054 ms | 7 ms | 8 ms | 7 ms | 7 ms |
| EntityX | 100'000 | 100'000 | 5 | 10 ms | 50'626 ms | 104'086 ms | / | / |
| CNtity | 10'000 | 1'000'000 | 1'000 | 91 ms | 63 ms | 89 ms | 77 ms | 75 ms |
| EntityX | 10'000 | 1'000'000 | 1'000 | 0 ms | 5'562 ms | 8'426 ms | / | / |
| CNtity | 100'000 | 1'000'000 | 1'000 | 800 ms | 63 ms | 91 ms | 74 ms | 77 ms |
| EntityX | 100'000 | 1'000'000 | 1'000 | 8 ms | 57'402 ms | 83'855 ms | / | / |
