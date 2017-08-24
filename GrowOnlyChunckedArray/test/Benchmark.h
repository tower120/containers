#ifndef GROW_ONLY_CHUNCKED_ARRAY_BENCHMARK_H
#define GROW_ONLY_CHUNCKED_ARRAY_BENCHMARK_H

#include <GrowOnlyChuckedArray.h>
#include <functional>
#include <iostream>
#include <deque>
#include <vector>

using namespace tower120::containers;

struct Benchmark{

    template<class T>
    struct BenchmarkChuckedArray {
        GrowOnlyChuckedArray<T> list;

        void fill(int count) {
            for (int i = 0; i < count; i++) {
                list.emplace([=]() { return i; });
            }
        }

        int traverse() {
            int sum = 0;
            

			/*
            list.iterate([&](const auto& value) {
                sum += value();
            });
			*/
            
			
            for(const auto& value : list){
                sum += value();
            }
			
			
            return sum;
        }
    };


    template<class T>
    struct BenchmarkVector {
        std::deque<T> list;

        void fill(int count) {
            for (int i = 0; i < count; i++) {
                list.emplace_back([=]() { return i; });
            }
        }

        int traverse() {
            int sum = 0;

            for (const T& value : list) {
                sum += value();
            }

            return sum;
        }
    };

    void benchmark() {
        using namespace std::chrono;

        const int times = 100;
        const int count = 100'000;
        using T = std::function<int()>;
        using TimeUnit = microseconds;



        auto benchmark_all = [&](auto& container) {
            {
                high_resolution_clock::time_point t1 = high_resolution_clock::now();
                container.fill(count);
                high_resolution_clock::time_point t2 = high_resolution_clock::now();
                auto duration = duration_cast<TimeUnit>(t2 - t1).count();

                std::cout << "Filled in: " << duration << std::endl;
            }
            {
                high_resolution_clock::time_point t1 = high_resolution_clock::now();

                int res = 0;
                for (int i = 0; i < times; ++i) {
                    res += container.traverse();
                }
                high_resolution_clock::time_point t2 = high_resolution_clock::now();
                auto duration = duration_cast<TimeUnit>(t2 - t1).count();

                std::cout << "Traversed in: " << duration << " (" << res << ")" << std::endl;
            }
        };

        BenchmarkVector<T> vec;
        std::cout << "Vector" << std::endl;
        benchmark_all(vec);
        std::cout << std::endl;

        BenchmarkChuckedArray<T> arr;
        std::cout << "Chuncked Array" << std::endl;
        benchmark_all(arr);
        std::cout << std::endl;
    }
};

#endif //GROW_ONLY_CHUNCKED_ARRAY_BENCHMARK_H