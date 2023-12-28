#include <iostream>
#include <chrono>
#include "CNtity/Helper.hpp"

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
        Helper helper;
        std::cout << "CNtity | entity: " << entityCount << " iteration: " << iterationCount << std::endl;

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

            auto view = helper.view<int>();

            for(int i = 0; i < iterationCount; ++i)
            {
                view.each([&](auto ent, auto num)
                {
                    sum += num;
                });
            }
        }

        {
            Timer timer("For_each entities two component: ");
            std::uint64_t sum = 0;

            auto view = helper.view<int, std::string>();
            
            for(int i = 0; i < iterationCount; ++i)
            {
                view.each([&](auto ent, auto num, auto name)
                {
                    sum += num;
                });
            }
        }

        {
            Timer timer("Acquire entities one component: ");
            std::uint64_t sum = 0;

            auto view = helper.view<int>();

            for(int i = 0; i < iterationCount; ++i)
            {
                for(auto [entity, num]: view.each())
                {
                    sum += num;
                };
            }
        }

        {
            Timer timer("Acquire entities two component: ");
            std::uint64_t sum = 0;

            auto view = helper.view<int, std::string>();

            for(int i = 0; i < iterationCount; ++i)
            {
                for(auto [entity, num, _]: view.each())
                {
                    sum += num;
                };
            }
        }

        std::cout << "____________________________" << std::endl;
    };

  //Library     Entity      Iteration   Probability
  ///////////////////////////////////////////////////////
    cntity(     1000,       100,        3);
  ///////////////////////////////////
    cntity(     1000,       1000000,    3);
  ///////////////////////////////////
    cntity(     10000,      1000000,    3);
  ///////////////////////////////////
    cntity(     30000,      100000,     3);
  ///////////////////////////////////
    cntity(     100000,     100000,     5);
  ///////////////////////////////////
    cntity(     10000,      1000000,    1000);
  ///////////////////////////////////
    cntity(     100000,     1000000,    1000);
  ///////////////////////////////////

    return 0;
}