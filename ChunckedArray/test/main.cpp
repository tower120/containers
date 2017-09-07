#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "../src/ChunckedArray.h"

using namespace tower120::containers;

void test_emplace(){
    ChunckedArray<int> list;

    for(int i=0; i<10;++i) {
        list.emplace(i);
    }
    list.iterate_elements([&](auto& element){
        if (element.value == 9){
            list.erase(&element);
        }
    });

    list.iterate([](int i){
        std::cout << i << std::endl;
    });

    std::cout << "for range" << std::endl;
    for(int i : list.range().values()){
        std::cout << i << std::endl;
    }
}


int main() {
    test_emplace();

#ifdef _MSC_VER
	char ch;
	std::cin >> ch;
#endif

    return 0;
}