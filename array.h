#ifndef ARRAY_H
#define ARRAY_H

#include "types.h"


template<typename T, size_t size>
class Array{
private:
    T data[size];
    size_t size_;

public:
    Array(): size_(0){}

    bool Add(const T &item)
    {
        if (size_ < size)
        {
            data[size_++] = item;
            return true;
        }
        return false;
    }

    template <typename HT>
    bool Remove(size_t index, HT &hashTable)
    {
        if (index >= size_)
            return false;

        size_t movedIndex = size_ - 1;
        if (index == movedIndex)
        {
            --size_;
            return true;
        }

        data[index] = data[movedIndex];
        --size_;

        hashTable.fixIndex(movedIndex, index);
        return true;
    }

    T &operator[](size_t index) { return data[index]; }
    const T &operator[](size_t index) const { return data[index]; }

    size_t Size() const { return size_; }
    size_t GetCapacity() const { return size; }
};


extern Array<Patient, 1000> PatientArray;
extern Array<Appointment, 1000> AppointmentArray;

#endif
