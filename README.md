# CNtity

CNtity is a type-safe header-only library which provides simple and useful features for an Entity Component System (ECS) application using C++17.

---

## How to install

CNtity is a header-only library, you just need a compiler that supports C++17 and to include `CNtity/Helper.hpp` after downloading files in the include directory.

## Getting started

CNtity is thought to be very intuitive and simple to use.

You will be able to easily do everything, from managing a single entity and its components, to iterate through every entities associated to components inside your systems.

### Create and manage an entity and its components

Here are the basic functions to create an entity, add components to it, get its components and modify the values of the components.

```cpp
struct Health { int max, current };
struct Position { float x, y };

CNtity::Helper helper;

// create a cat with components, these can be of any type
auto cat = helper.create<std::string, Health, int>("cat", {100, 80}, 0);

// add a component afterwards
auto [position] = helper.add<Position>(cat, {50, 50});
position.x += 50;

// get the components associated to an entity
if(auto values = helper.get_if<Health, std::string>(cat))
{
    auto [health, name] = *values; // access components inside the tuple
    health.current += 5;
}
```

### Iterate over entities

You will for sure want to retrieve all the entities containing one or more components. To do so, you can create a View which will be automatically, or not, updated by the helper.

```cpp
CNtity::View<std::string, Position> view{helper}; // same as: auto view = helper.view<std::string, Position>();

// for each callback
view.each([](auto entity, auto& position) {
    position.x += 10;
});

// range based loop
for(auto [entity, position]: view.each())
{
    position.x += 10;
}
```

### Visit components, (de)serialization

Sometimes, you need to do an operation on many components of an entity in a generic way, for example to serialize it.

You may do this by visiting the entity on selected components in a type-safe manner. The component types that the entity doesn't have will be ignored.

**Index components**
```cpp
struct Position
{
    float x, y;

    std::string serialize() {};
    void deserialize(std::string data) {};
};

struct Health
{
    float x, y;

    std::string serialize() {};
    void deserialize(std::string data) {};
};

helper.index<Position, Health>("position", "health"); // associate an unique ID to components for (de)serialization
```

**Serialization**
```cpp
helper.visit<Position, Health>(cat, [](auto component, auto index)
{
    auto data = component.serialize(); // save data and index to a file
});
```

**Deserialization**
```cpp
std::string index, data; // load data and index from a file

helper.visit<Position, Health>(cat, index, [&data](auto component)
{
    component.deserialize(data);
});
```

### Full example

Checkout the following example codes, which can also be found inside the `examples` folder, to get an overview of most features.

Ofcourse, you will be able to make more complex programs (e.g, creating classes for your systems).
CNtity's best point is that it isn't forcing some "architecture" on your application.
