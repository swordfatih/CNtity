# CNtity

CNtity is a type-safe, header-only library which provides simple and useful features for building an **Entity Component System (ECS)** in C++17.

---

## How to install

CNtity is a header-only library, so installation is straightforward.

1. Download the files and place them in your include folder.
2. Include the main header in your project: `#include <CNtity/Helper.hpp>`

## Getting started

CNtity is designed to be intuitive and simple to use.

You can do everything from managing a single entity and its components to iterating through thousands of entities inside your systems.

### Create and manage an entity and its components

Here are the basic functions to create an entity, add components to it, and retrieve or modify those components.

```cpp
struct Health { int max, current; };
struct Position { float x, y; };

CNtity::Helper helper;

// create an entity (a "cat") with components (any type is allowed)
auto cat = helper.create<std::string, Health, int>("cat", {100, 80}, 0);

// add a component afterwards
auto [position] = helper.add<Position>(cat, {50, 50});
position.x += 50; // modify the component directly

// safely get components (if they exist)
if (auto values = helper.get_if<Health, std::string>(cat))
{
    auto [health, name] = *values;
    health.current += 5;
}
```

### Iterate over entities

You will likely want to iterate through all entities containing one or more specific components.
You can do this by creating a `View`.

```cpp
auto view = helper.view<std::string, Position>();
// same as: CNtity::View<std::string, Position> view{helper};

// for each callback
view.each([](auto entity, auto& position) {
    position.x += 10;
});

// range-based loop
for(auto [entity, position]: view)
{
    position.x += 10;
}
```

> All components are accessed by reference, so modifications are applied directly.

### Visit components (generic operations and serialization)

Sometimes, you need to operate on multiple components of an entity in a generic way, for example for saving or loading them.

You can visit components in a type-safe manner. Components that the entity doesn't have are simply ignored.

**Index component**

Before serializing, you can associate components with unique string identifiers.

```cpp
struct Position
{
    float x, y;

    std::string serialize();
    void deserialize(const std::string& data);
};

struct Health
{
    int max, current;

    std::string serialize();
    void deserialize(const std::string& data);
};

helper.index<Position, Health>("position", "health");
```

**Serialization**

```cpp
helper.visit<Position, Health>(cat, [](auto& component, auto index)
{
    auto data = component.serialize(); // save data with its index
});
```

**Deserialization**

```cpp
std::string index, data; // loaded from file

helper.visit<Position, Health>(cat, index, [&data](auto& component)
{
    component.deserialize(data);
});
```

Full example
Check out the examples inside the examples folder for more complete use cases.
You can build them with CMake by enabling the option:

```bash
cmake -DCNTITY_BUILD_EXAMPLES=ON ..
```

---

CNtity's best point is that it doesn't enforce any strict "architecture" on your application.
You are free to organize your systems and components however you want.