#ifndef FLEXIBLEARRAYMEMBER_H
#define FLEXIBLEARRAYMEMBER_H

namespace tower120::containers{

    template<class Header, class T>
    class FlexibleMemberArray {
        struct alignas(T) AlignedHeader : Header{};

        Header* self(){
            static_cast<Header*>(this);
        }

    protected:
        static Header* make(const int capacity) {
            Header *chunk = static_cast<Header *>(std::malloc(sizeof(AlignedHeader) + sizeof(T) * capacity));
            return chunk;
        }

        inline T *elements() {
            return static_cast<T *>(static_cast<void *>(reinterpret_cast<unsigned char *>(self()) +
                                                              sizeof(AlignedHeader)));
        }
    };


}

#endif //FLEXIBLEARRAYMEMBER_H
