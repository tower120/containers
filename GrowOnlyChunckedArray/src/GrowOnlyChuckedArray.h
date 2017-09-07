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

            // use SpinLock instead reserved_size + has_next ?

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

		struct end_iterator {};
        struct iterator{
            Chunk* chunk;
            std::size_t chunk_size;
            std::size_t index;

            iterator(Chunk* chunk, std::size_t chunk_size, std::size_t index)
                : chunk(chunk)
                , chunk_size(chunk_size)
                , index(index)
            {}

            T& operator*(){
                return chunk->elements()[index];
            }

            iterator& operator++(){
                index++;
                if (index == chunk_size /*chunk->built_size*/){           // or read from chunk->built_size
                    // move to prev chunk
                    chunk = chunk->prev;
                    chunk_size = chunk ? static_cast<std::size_t>(chunk->built_size) : 0;
                    index = 0;
                    return *this;
                }

                return *this;
            }

			bool operator==(const end_iterator&) const {
				return chunk == nullptr;
			}
			bool operator!=(const end_iterator&) const {
				return chunk != nullptr;
			}

            /*bool operator==(const iterator& other) const{
                return chunk == other.chunk && index == other.index;
            }
            bool operator!=(const iterator& other) const{
                return !operator==(other);
            }*/
       };

        iterator begin(){
            return {
                head
                , head ? static_cast<std::size_t>(static_cast<Chunk*>(head)->built_size) : 0
                , 0
            };
        }
		end_iterator end() const {
			return {};
		}
        /*iterator end() const {
            return {nullptr, 0, 0};
        }*/


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
                for(unsigned int i=0;i < size /*chunk->built_size*/;++i){
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

        T& back(){
            Chunk* head = this->head;
            return head->elements()[head->built_size];
        }
        void pop_back(){
            Chunk* chunk = this->head;

            back().~T();
            chunk->built_size--;
            chunk->reserved_size--;

            if(chunk->built_size == 0){
                Chunk* prev = chunk->prev;
                    delete chunk;
                head = prev;
            }
        }

        ~GrowOnlyChuckedArray(){
            clear();
        }
    };

	template<class T, bool multithreaded>
	typename GrowOnlyChuckedArray<T, multithreaded>::Chunk* GrowOnlyChuckedArray<T, multithreaded>::zero_size_chunk = new (0) typename GrowOnlyChuckedArray<T, multithreaded>::Chunk;
}

#endif //GROWONLYCHUCKEDARRAY_H