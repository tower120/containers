#ifndef CHUNCKED_ARRAY_CHUNCKEDARRAY_H
#define CHUNCKED_ARRAY_CHUNCKEDARRAY_H


#include "../../GrowOnlyChunckedArray/src/GrowOnlyChuckedArray.h"

#include <atomic>
#include <shared_mutex>

namespace tower120::containers{

    template<class T, bool multithreaded = true>
    class ChunckedArray{

        enum class State{alive, deleted, destroyed};

        struct Element{
            std::atomic<State> state{ State::alive };
            T value;

            operator T&(){
                return value;
            }
            T& operator*(){
                return value;
            }


            template<class Arg0, class ...Args,
                    typename = std::enable_if_t<
                            !std::is_same<std::decay_t<Arg0>, Element>::value
                    >
            >
            Element(Arg0&& arg, Args&&...args)
                    :value(std::forward<Arg0>(arg), std::forward<Args>(args)...) {}

        };
        GrowOnlyChuckedArray<Element> elements;

        using MaintanceLock = std::shared_mutex;    // use RWSpinLock<unsigned short> here
        MaintanceLock maintanceLock;

        std::atomic<bool> have_deleted{false};

        void do_delete_element(Element& element){
            // clear head
            while(elements.back().state != State::alive){
                elements.pop_back();

                if (elements.back() == element){
                    // reached target element
                    // don't move, just pop_back
                    elements.pop_back();
                    return;
                }
            }

            element.value = std::move(elements.back().value);
            element.state = State::alive;
            elements.pop_back();
        }
    public:
        template<class ...Args>
        void emplace(Args&&...args){
            std::shared_lock<MaintanceLock> l(maintanceLock);

            elements.emplace(std::forward<Args>(args)...);
        }

        // element must be valid!
        // call under iterate / for-loop only
        void erase(Element* element){
            std::shared_lock<MaintanceLock> l(maintanceLock);

            element->state = State::deleted;
            have_deleted = true;
        }

        template<class Closure>
        void iterate_elements(Closure&& closure){
            bool maintance_pass = false;
            if (have_deleted){
                const bool locked = maintanceLock.try_lock();
                if (locked) {
                    maintance_pass = true;
                } else{
                    maintanceLock.lock_shared();
                }
            }

            elements.iterate([&](Element& element){
                if (element.state != State::alive){
                    if (maintance_pass){
                        do_delete_element(element);
                    }
                    return;
                }
                closure(element);
            });

            if (maintance_pass){
                maintanceLock.unlock();
            } else {
                maintanceLock.unlock_shared();
            }
        }

        template<class Closure>
        void iterate(Closure&& closure) {
            iterate_elements([&](Element& element){
                closure(element.value);
            });
        }

        // TODO: implement compacting for range
        template<bool return_values = false>
        struct Range{
            GrowOnlyChuckedArray<Element>* elements;

            Range(Range<!return_values>&& other)
                :elements(other.elements)
            {
                other.elements = nullptr;
            }
            Range(Range&& other)
                :elements(other.elements)
            {
                other.elements = nullptr;
            }

            Range(const Range&) = delete;

            Range(GrowOnlyChuckedArray<Element>& elements)
                :elements(&elements)
            {
                // lock_shared
            }

            ~Range(){
                if (!elements) return;      //moved
                // unlock_shared
            }

            struct iterator : GrowOnlyChuckedArray<Element>::iterator{
                using Base = typename GrowOnlyChuckedArray<Element>::iterator;
                using Base::Base;

                iterator(Base&& base)
                    :Base(std::move(base))
                {}

                auto& operator*(){
                    if constexpr (return_values){
                        return Base::operator*().value;
                    } else {
                        return Base::operator*();
                    }
                }

                auto& operator++() {
                    while(true) {
                        Base::operator++();
                        if (Base::chunk == nullptr) {
                            return *this;
                        }
                        if (Base::operator*().state == State::alive) {
                            return *this;
                        }
                    }
                }
            };

            auto begin(){
                return iterator{ elements->begin() };
            }

            auto end() const{
                return elements->end();
            }

            auto values(){
                return Range<true>{std::move(*this)};
            }
        };

        Range<false> range(){
            return {elements};
        }

    };

}

#endif //CHUNCKED_ARRAY_CHUNCKEDARRAY_H
