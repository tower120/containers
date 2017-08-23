#ifndef GROWONLYCHUCKEDARRAY_H
#define GROWONLYCHUCKEDARRAY_H

#include <memory>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <vector>

#include "../../FlexibleArrayMember/src/FlexibleArrayMember.h"

namespace tower120::containers{

    template<class T, bool multithreaded = true>
    class GrowOnlyChuckedArray{
        static constexpr const std::size_t intial_capacity = 4;

        struct Chunk : FlexibleArrayMember<Chunk, T>{
            Chunk* prev = nullptr;      // const

            using Size = std::conditional_t<multithreaded, std::atomic<unsigned int>, unsigned int>;

            unsigned int capacity;      // const

            Size reserved_size{0};
            Size built_size{0};

            using HasNext = std::conditional_t<multithreaded, std::atomic<bool>, bool>;
            HasNext has_next{false};

            static void* operator new(std::size_t, unsigned int capacity) {
                Chunk* self = FlexibleArrayMember<Chunk, T>::make(capacity);
                self->capacity = capacity;
                return self;
            }

            using FlexibleArrayMember<Chunk, T>::elements;

            ~Chunk(){
                if (prev) prev->has_next = false;
                for(unsigned int i=0;i<built_size;++i){
                    elements()[i].~T();
                }
            }
        };

        using Head = std::conditional_t<multithreaded, std::atomic<Chunk*>, Chunk*>;
        Head head{nullptr};

        std::pair<Chunk*, std::size_t> reserve_emplace(){
            while(true) {
                Chunk *head = this->head;

                if (head->reserved_size >= head->capacity) {
                    // just spin
                    if (head->has_next) continue;
                    if (head->has_next.exchange(true)) continue;        // aka spinlock

                    // sequenced execution from here

                    // grow
                    Chunk *chunk = new (head->capacity*2) Chunk;
                    chunk->prev = head;
                    chunk->reserved_size = 1;

                    this->head = chunk;

                    return {chunk, 0};
                }

                const auto index = head->reserved_size++;
                if (index >= head->capacity) {
                    head->reserved_size--;
                    continue;
                }

                return {head, index};
            }
        };

		static Chunk* zero_size_chunk;

    public:
        GrowOnlyChuckedArray(){}

        // Move
        GrowOnlyChuckedArray(GrowOnlyChuckedArray&& other)
            :head(static_cast<Chunk*>(other.head))
        {
            other.head = nullptr;
        }
        GrowOnlyChuckedArray& operator=(GrowOnlyChuckedArray&& other){
            head = static_cast<Chunk*>(other.head);
            other.head = nullptr;
            return *this;
        }


        // TODO: implement copy
        GrowOnlyChuckedArray(const GrowOnlyChuckedArray& other) = delete;
        GrowOnlyChuckedArray& operator=(GrowOnlyChuckedArray& other) = delete;

        // thread safe
        template<class ...Args>
        void emplace(Args&&...args){
            Chunk *chunk;
            std::size_t index;

            auto build = [&](){
                new (&chunk->elements()[index]) T(std::forward<Args>(args)...);
                chunk->built_size++;
            };

            if(!head){
                Chunk* chunk_null{nullptr};
                if (head.compare_exchange_strong(chunk_null, zero_size_chunk)){
                    chunk = new (intial_capacity) Chunk;
                    chunk->reserved_size = 1;

                    head = chunk;

                    index = 0;
                    build();
                    return;
                }
            }

            while(head == zero_size_chunk){
                // spin while not zero_size_chunk
            }
            std::tie(chunk, index) = reserve_emplace();
            build();
        }

        template<class Closure>
        void iterate(Closure&& closure){
            Chunk *chunk = head;

            while (chunk) {
				const unsigned int size = chunk->built_size;
                for(unsigned int i=0;i < size;++i){
                    closure(chunk->elements()[i]);
                }

                chunk = chunk->prev;
            }
        }

        bool empty() const{
            const Chunk* const chunk = head;
            return !chunk || chunk->built_size == 0;
        }

        // non thread-safe
        void clear(){
            Chunk *chunk = head;

            while (chunk) {
                Chunk* prev = chunk->prev;
                    delete chunk;
                chunk = prev;
            }

            head = nullptr;
        }

        ~GrowOnlyChuckedArray(){
            clear();
        }
    };

	template<class T, bool multithreaded>
	typename GrowOnlyChuckedArray<T, multithreaded>::Chunk* GrowOnlyChuckedArray<T, multithreaded>::zero_size_chunk = new (0) typename GrowOnlyChuckedArray<T, multithreaded>::Chunk;
}

#endif //GROWONLYCHUCKEDARRAY_H