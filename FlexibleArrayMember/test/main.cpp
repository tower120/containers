#include <iostream>
#include <memory>
#include <new>

#include <FlexibleArrayMember.h>

using namespace tower120::containers;

struct Chunk : public FlexibleArrayMember<Chunk, int>{
    int capacity;

    static void* operator new(std::size_t sz, int capacity) {
        Chunk* self = make(capacity);
        self->capacity = capacity;
        return self;
    }

     using FlexibleArrayMember::elements;
};

int main() {
    std::unique_ptr<Chunk> chunk {new (30) Chunk};

    for(int i=0;i<chunk->capacity;i++){
        chunk->elements()[i] = i;
    }

    for(int i=0;i<chunk->capacity;i++){
        std::cout << chunk->elements()[i] << std::endl;
    }

    return 0;
}