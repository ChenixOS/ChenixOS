#pragma once

#include "vector.h"
#include "string.h"

template <typename T>
struct KeyValuePair {
    const char* name;
    T value;
};

template <typename T>
class StringTable {
public:
    Vector<struct KeyValuePair<T>> keyValues;
    T defaultValue;
    
    void put(const char* name,T value) {
        keyValues.push_back({
            .name = name,
            .value = value
        });
    }

    T find(char* name) {
        if(name == nullptr)
            return defaultValue;
        for(size_t i = 0;i < keyValues.count();i++) {
            if(keyValues[i] == nullptr) {
                continue;
            }
            if(keyValues[i].name == nullptr) {
                continue;
            }
            if(kstrcmp(keyValues[i].name,name) == 0) {
                return keyValues[i].value;
            }
        }
        return defaultValue;
    }

    size_t count() {
        return keyValues.count();
    }

    Vector<struct KeyValuePair<T>>& pairs() {
        return this->keyValues;
    }
};
