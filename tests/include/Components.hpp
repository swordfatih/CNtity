#pragma once

struct Position
{
    int  x{}, y{};
    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
};

struct Velocity
{
    float vx{}, vy{};
    bool  operator==(const Velocity& other) const { return vx == other.vx && vy == other.vy; }
};

struct Health
{
    int  hp{};
    bool operator==(const Health& other) const { return hp == other.hp; }
};