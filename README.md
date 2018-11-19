# CNtity
CNtity (Chats Noirs Entity) is a header-only helper library for an Entity Component System (ECS) implementation using C++17. It is actually in development by Fatih#6810 (accfldekur@gmail.com) from *Moonlit Black Cats*.
***

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
struct Identity
{
    std::string name;
};

////////////////////////////////////////////////////////////
int main()
{
    //Helper
    CNtity::Helper<Position, Velocity, Health, Identity> helper;

    //Creating entities
    CNtity::Entity chat = helper.create<Identity>({"chat"});
    helper.create<Identity>({"chien"});
    helper.create<Identity, Position>({"velociraptor"}, {25, 70});

    //Adding component, changing values
    Position* position = helper.add<Position>(chat, {50, 50});
    position->x += 50;

    //System 1
    helper.for_each<Identity, Position>([&helper](CNtity::Entity entity, Identity* identity)
    {
        if(identity->name == "chat")
        {
            Position* position = helper.get<Position>(entity);
            position->x = 200;
            position->y = 70;

            return;
        }
    });

    //System 2
    for(auto it: helper.acquire<Identity, Position>())
    {
        if(helper.get<Identity>(it)->name == "chat")
        {
            Position* position = helper.get<Position>(it);
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

Functions | Description
------- | -----------
`create()` | Creates an entity with or without components.
`add()` | Add a component to an entity.
`remove()` | Remove a component from an entity.
`get()` | Get a specified component of an entity.
`has()` | Check if an entity has a specified component.
`for_each()` | Execute a given function for each entities containing specified components.
`acquire()` | Get a vector of entities containing specified components.
`erase()` | Erase an entity.

***

**Performances**

I have tried to make CNtity as fast as I could! Here is a little benchmark I have done with my old computer I code with* (Intel Core i3-4160T) between CNtity and EntityX (another ECS library, most referenced on google for me). It's probably not accurate but will help you to make an idea about CNtity. [Here](https://github.com/swordfatih/CNtity/blob/master/benchmark.cpp) is the source code. 

*\*will do it again with my gamer setup*

|   | Entities | Iterations | Probability | `each` one component | `each` two component | `acquire` one component | `acquire` two component |
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| CNtity | 1'000 | 100 | 3 |  |  |  |  |
| EntityX | 1'000 | 100 | 3 |  |  | X | X |
| CNtity | 1'000 | 1'000'000 | 3 |  |  |  |  |
| EntityX | 1'000 | 1'000'000 | 3 |  |  | X | X |
| CNtity | 10'000 | 1'000'000 | 3 |  |  |  |  |
| EntityX | 10'000 | 1'000'000 | 3 |  |  | X | X |
| CNtity | 30'000 | 100'000 | 3 |  |  |  |  |
| EntityX | 30'000 | 100'000 | 3 |  |  | X | X |
| CNtity | 100'000 | 100'000 | 5 |  |  |  |  |
| EntityX | 100'000 | 100'000 | 5 |  |  | X | X |
| CNtity | 10'000 | 1'000'000 | 1'000 |  |  |  |  |
| EntityX | 10'000 | 1'000'000 | 1'000 |  |  | X | X |
| CNtity | 100'000 | 1'000'000 | 1'000 |  |  |  |  |
| EntityX | 100'000 | 1'000'000 | 1'000 |  |  | X | X |
