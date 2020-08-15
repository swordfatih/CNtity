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
    auto&& chat = helper.create<std::string>({"chat"});
    helper.create<std::string>({"chien"});
    helper.create<std::string, Position>({"velociraptor"}, {25, 70});

    //Adding component, changing values
    auto&& position = helper.add<Position>(chat, {50, 50});
    position->x += 50;

    //System 1
    helper.for_each<std::string, Position>([&helper](auto entity, auto identity)
    {
        if(*identity == "chat")
        {
            auto&& position = helper.get<Position>(entity);
            position->x = 200;
            position->y = 70;

            return;
        }
    });

    //System 2
    for(auto&& [entity, identity]: helper.acquire<std::string, Position>())
    {
        if(std::get<std::string>(*identity) == "chat")
        {
            auto&& position = helper.get<Position>(entity);
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
`match(identifier)` | Check if an entity matchs with a given identifier
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

My goal id to make CNtity as fast as I can! Here is a little benchmark I have done (Intel Core i5-8300H CPU @ 2.30GHz) between CNtity and EntityX (another ECS library, most referenced on google for me). It's probably not accurate but it will help you to make yourself an idea about CNtity. [Here](https://github.com/swordfatih/CNtity/blob/master/benchmark.cpp) is the source code.


|   | Entities | Iterations | Probability | Create and add | `each` one component | `each` two component | `acquire` one component | `acquire` two component |
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| CNtity | 1'000 | 100 | 3 | 0 ms | 0 ms | 0 ms | 0 ms | 0 ms |
| EntityX | 1'000 | 100 | 3 | 0 ms | 0 ms | 1 ms | / | / |
| CNtity | 1'000 | 1'000'000 | 3 | 5 ms | 822 ms | 794 ms | 452 ms | 613 ms |
| EntityX | 1'000 | 1'000'000 | 3 | 0 ms | 7'232 ms | 15'455 ms | / | / |
| CNtity | 10'000 | 1'000'000 | 3 | 45 ms | 9'596 ms | 9'329 ms | 6'601 ms | 8'329 ms |
| EntityX | 10'000 | 1'000'000 | 3 | 0 ms | 75'593 ms | 156'918 ms | / | / |
| CNtity | 30'000 | 100'000 | 3 | 152 ms | 6'765 ms | 6'629 ms | 5'714 ms | 6'067 ms |
| EntityX | 30'000 | 100'000 | 3 | 2 ms | 22'821 ms | 46'955 ms | / | / |
| CNtity | 100'000 | 100'000 | 5 | 390 ms | 13'926 ms | 13'762 ms | 11'551 ms | 12'374 ms |
| EntityX | 100'000 | 100'000 | 5 | 9 ms | 47'783 ms | 96'316 ms | / | / |
| CNtity | 10'000 | 1'000'000 | 1'000 | 40 ms | 54 ms | 61 ms | 43 ms | 50 ms |
| EntityX | 10'000 | 1'000'000 | 1'000 | 0 ms | 5'427 ms | 6'653 ms | / | / |
| CNtity | 100'000 | 1'000'000 | 1'000 | 322 ms | 245 ms | 237 ms | 137 ms | 185 ms |
| EntityX | 100'000 | 1'000'000 | 1'000 | 9 ms | 56'767 ms | 65'578 ms | / | / |
