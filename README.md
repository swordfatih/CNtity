# CNtity
CNtity (Chats Noirs Entity) is a header-only helper library for an Entity Component System (ECS) implementation using C++17. It is actually in development by Fatih#6810 (accfldekur@gmail.com) from *Moonlit Black Cats*.
***

### How to use

CNtity is header-only, you just need a compiler that supports C++17 and to include `CNtity/Helper.hpp` after downloading files in the include directory.

**Example code**

An example code showing how CNtity is maniable!
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
`create<Component>()` | Creates an entity with or without components.
`add<Component>(entity)` | Add a component to an entity.
`remove<Component>(entity)` | Remove a component from an entity.
`get<Component>(entity)` | Get a specified component of an entity.
`has<Component>(entity)` | Check if an entity has a specified component.
`erase(entity)` | Erase an entity.
`for_each<Component>(function)` | Execute a given function for each entities containing specified components.
`acquire<Component>()` | Get a vector of entities containing specified components.

***

### Performances

I have tried to make CNtity as fast as I could! Here is a little benchmark I have done on my old computer I code with* (Intel Core i3-4160T) between CNtity and EntityX (another ECS library, most referenced on google for me). It's probably not accurate but it will help you to make an idea about CNtity. [Here](https://github.com/swordfatih/CNtity/blob/master/benchmark.cpp) is the source code. 

*\*will do it again with my gamer setup*

|   | Entities | Iterations | Probability | `each` one component | `each` two component | `acquire` one component | `acquire` two component |
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| CNtity | 1'000 | 100 | 3 | 0 ms | 0 ms | 0 ms | 0 ms |
| EntityX | 1'000 | 100 | 3 | 0 ms | 1 ms | / | / |
| CNtity | 1'000 | 1'000'000 | 3 | 2'523 ms | 6'117 ms | 5'333 ms | 5'156 ms |
| EntityX | 1'000 | 1'000'000 | 3 | 11'738 ms | 22'208 ms | / | / |
| CNtity | 10'000 | 1'000'000 | 3 | 23'081 ms | 60'542 ms | 53'160 ms | 51'339 ms |
| EntityX | 10'000 | 1'000'000 | 3 | 113'041 ms | 219'806 ms | / | / |
| CNtity | 30'000 | 100'000 | 3 | 7'627 ms | 18'743 ms | 16'038 ms | 15'480 ms |
| EntityX | 30'000 | 100'000 | 3 | 33'709 ms | 67'798 ms | / | / |
| CNtity | 100'000 | 100'000 | 5 | 15'195 ms | 36'691 ms | 32'948 ms | 31'123 ms |
| EntityX | 100'000 | 100'000 | 5 | 72'244 ms | 145'751 ms | / | / |
| CNtity | 10'000 | 1'000'000 | 1'000 | 136 ms | 243 ms | 278 ms | 288 ms |
| EntityX | 10'000 | 1'000'000 | 1'000 | 9'160 ms | 8'377 ms | / | / |
| CNtity | 100'000 | 1'000'000 | 1'000 | 790 ms | 2'205 ms | 2'053 ms | 2'018 ms |
| EntityX | 100'000 | 1'000'000 | 1'000 | 94'978 ms | 86'477 ms | / | / |
