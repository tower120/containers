#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <GrowOnlyChuckedArray.h>
#include "Benchmark.h"

using namespace tower120::containers;

void test_simple_emplace(){
    struct T{
        int i;
        T(int i) :i(i) {}
        ~T(){
            std::cout << "~T " << i << std::endl;
        }
    };

    GrowOnlyChuckedArray<T> list;

    const int size = 10;
    for(int i=0;i<size;++i){
        list.emplace(i);
    }

    list.iterate([&](const T& i){
        if (i.i == 9) list.emplace(999);
       std::cout << i.i << std::endl;
    });
}


void test_threaded_emplace(){
    struct T{
        int i;
        T(int i) :i(i) {}
        ~T(){
            std::cout << "~T " << i << std::endl;
        }
    };

    GrowOnlyChuckedArray<T> list;

    const int thread_count = 8;
    const int size = 10;

    std::vector<std::thread> threads;
    for(int i=0;i<thread_count;++i){
        threads.emplace_back(std::thread([&, start = i*size](){
            for(int i=0;i<size;++i){
                list.emplace(start + i);
            }
        }));
    }
    for(auto& thread:threads) thread.join();

    auto show_sum = [](auto& list){
        int sum = 0;
        list.iterate([&](const T &i) {
            sum += i.i;
        });
        std::cout << sum << std::endl;
    };
    show_sum(list);

    // test move
    auto list2 = std::move(list);
    show_sum(list2);
}

void test_forrange(){
    GrowOnlyChuckedArray<int> list;

    list.emplace(1);

    for(int i: list){
        std::cout << i << std::endl;
    }
}

int main() {
    //test_simple_emplace();
    //test_threaded_emplace();

    //test_forrange();

    Benchmark().benchmark();

#ifdef _MSC_VER
	char ch;
	std::cin >> ch;
#endif

    return 0;
}