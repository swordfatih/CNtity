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

### Visit components of an entity and serialization

Sometimes, you need to do an operation on many components of an entity in a generic way, for example for serialization.

You may do this by visiting the entity on selected components in a type-safe manner. The types of components the entity doesn't have will be ignored.

```cpp
struct Position
{
    float x, y;

    void serialize() {};
};

struct Health
{
    float x, y;

    void serialize() {};
};

helper.visit<Position, Health>(cat, [](auto component)
{
    component.serialize();
    // this is safe, compilation will fail if all the components don't have the "serialize" member
});
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

### Full example

Checkout the following example code, which can also be found inside the `examples/getting-started` folder, to get an overview of most features.

Ofcourse, you will be able to make more complex programs (e.g, creating classes for your systems).
CNtity's best point is that it isn't imposing any architecture to your application.
