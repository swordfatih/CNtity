#include <iostream>
#include <chrono>
#include "CNtity/Helper.hpp"
#include <entityx/entityx.h>

class Timer
{
public:
	Timer(const std::string& string) : mStart(std::chrono::high_resolution_clock::now()), mString(string)
    {

    }

	~Timer()
	{
		auto end = std::chrono::high_resolution_clock::now();
		std::cout << mString << ' ' << std::chrono::duration_cast<std::chrono::milliseconds>(end - mStart).count() << " ms\n";
	}

private:
    std::chrono::high_resolution_clock::time_point mStart;
	std::string mString;
};

int main()
{
    auto cntity = [](int entityCount, int iterationCount, int probability)
    {
        using namespace CNtity;
        Helper<int, std::string> helper;
        std::cout << "CNtity for | entity: " << entityCount << " iteration: " << iterationCount << std::endl;

        {
            Timer timer("Add entities: ");
            for(int i = 0; i < entityCount; ++i)
            {
                auto ent = helper.create();

                if(i % probability == 0)
                {
                    helper.add<int>(ent, i);
                }

                helper.add<std::string>(ent, "chat");
            }
        }

        {
            Timer timer("For_each entities one component: ");
            std::uint64_t sum = 0;
            for(int i = 0; i < iterationCount; ++i)
            {
                helper.for_each<int>([&](auto ent, auto num)
                {
                    sum += *num;
                });
            }
        }

        {
            Timer timer("For_each entities two component: ");
            std::uint64_t sum = 0;
            for(int i = 0; i < iterationCount; ++i)
            {
                helper.for_each<int, std::string>([&](auto ent, auto num)
                {
                    sum += *num;
                });
            }
        }

        {
            Timer timer("Acquire entities one component: ");
            std::uint64_t sum = 0;
            for(int i = 0; i < iterationCount; ++i)
            {
                for(auto&& [entity, num]: helper.acquire<int>())
                {
                    sum += std::get<int>(num);
                };
            }
        }

        {
            Timer timer("Acquire entities two component: ");
            std::uint64_t sum = 0;
            for(int i = 0; i < iterationCount; ++i)
            {
                for(auto&& [entity, num]: helper.acquire<int, std::string>())
                {
                    sum += std::get<int>(num);
                };
            }
        }

        std::cout << "____________________________" << std::endl;
    };

    auto entityx = [](int entityCount, int iterationCount, int probability)
    {
        using namespace entityx;
        entityx::EntityX manager;

        std::cout << "EntityX | entity: " << entityCount << " iteration: " << iterationCount << std::endl;

        {
            Timer timer("Add entities: ");
            for(int i = 0; i < entityCount; ++i)
            {
                auto ent = manager.entities.create();
                if(i % probability == 0)
                {
                    ent.assign<int>(i);
                }

                ent.assign<std::string>("chat");
            }
        }

        {
            Timer timer("For_each entities one component: ");
            std::uint64_t sum = 0;
            for(int i = 0; i < iterationCount; ++i)
            {
                manager.entities.each<int>([&sum](auto ent, auto num)
                {
                    sum += num;
                });
            }
        }

        {
            Timer timer("For_each entities two component: ");
            std::uint64_t sum = 0;
            for(int i = 0; i < iterationCount; ++i)
            {
                manager.entities.each<int, std::string>([&sum](auto ent, auto num, auto str)
                {
                    sum += num;
                });
            }
        }

        std::cout << "____________________________" << std::endl;
    };

  //Library     Entity      Iteration   Probability
  ///////////////////////////////////////////////////////
    cntity(     1000,       100,        3);
    entityx(    1000,       100,        3);
  ///////////////////////////////////
    cntity(     1000,       1000000,    3);
    entityx(    1000,       1000000,    3);
  ///////////////////////////////////
    cntity(     10000,      1000000,    3);
    entityx(    10000,      1000000,    3);
  ///////////////////////////////////
    cntity(     30000,      100000,     3);
    entityx(    30000,      100000,     3);
  ///////////////////////////////////
    cntity(     100000,     100000,     5);
    entityx(    100000,     100000,     5);
  ///////////////////////////////////
    cntity(     10000,      1000000,    1000);
    entityx(    10000,      1000000,    1000);
  ///////////////////////////////////
    cntity(     100000,     1000000,    1000);
    entityx(    100000,     1000000,    1000);
  ///////////////////////////////////

    return 0;
}
