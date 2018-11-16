# CNtity
CNtity (Chats Noirs Entity) is a header-only helper library for an implementation of an Entity Component System (ECS) in C++17. It is actually in development by Fatih#6810 (accfldekur@gmail.com) from *Moonlit Black Cats*.
***
**Example code**
An example code showing how CNtity is maniable!
```
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
    helper.create<Identity>({"velociraptor"});

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
