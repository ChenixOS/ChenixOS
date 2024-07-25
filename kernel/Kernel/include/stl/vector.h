#pragma once

#include "types.h"
#include "string.h"

#define STEP_CAPACITY 10

// Vector和AnchorList的主要区别在于，Vector是基于数组扩容的方法，适用于对数据顺序敏感的地方
// AnchorList基于链表操作的方法，效率更高

template <typename T>
class Vector {
private:
    T* data;
    size_t size;
    size_t capacity;

    void resize(size_t new_capacity) {
        T* new_data = new T[new_capacity];
        if(data != nullptr) {
            kmemcpy(new_data, data, size * sizeof(T));
            delete [] data;
        }
        this->data = new_data;
        this->capacity = new_capacity;
    }
public:
    Vector() : data(nullptr), size(0), capacity(0) {}

    ~Vector() {
        delete [] data;
    }

    // 添加到末尾
    void push_back(const T& value) {
        if (size == capacity) {
            resize(capacity + STEP_CAPACITY);
        }
        data[size++] = value;
    }

    // 获取元素数量
    size_t count() const {
        return size;
    }

    // 访问元素
    T& operator[](size_t index) {
        return data[index];
    }

    const T& operator[](size_t index) const {
        return data[index];
    }

    size_t find(T value) {
        for(size_t i = 0;i < size;i++) {
            if(this->data[i] == value) {
                return i;
            }
        }
        return -1;
    }

    // 迭代器类
    class Iterator {
    private:
        const Vector<T>* vector;
        size_t index;

        friend class Vector;

    public:
        Iterator(const Vector<T>& vec, size_t idx) : vector(&vec), index(idx) {}

        T& operator*() const {
            return vector->data[index];
        }

        Iterator& operator++() {
            ++index;
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp(*this);
            ++index;
            return temp;
        }

        bool operator==(const Iterator& other) const {
            return vector == other.vector && index == other.index;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    };

    // begin方法
    Iterator begin() const {
        return Iterator(*this, 0);
    }

    // end方法
    Iterator end() const {
        return Iterator(*this, size);
    }
};