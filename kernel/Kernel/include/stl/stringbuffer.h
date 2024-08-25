#pragma once

#include "types.h"
#include "string.h"

class StringBuffer {
private:
    char* buffer;
    size_t capacity;

    void resize(size_t newCapacity) {
        char* newBuffer = new char[newCapacity];
        memcpy(newBuffer, buffer, strlen(buffer) + 1);
        delete[] buffer;
        buffer = newBuffer;
        capacity = newCapacity;
    }

public:
    // 构造函数
    StringBuffer() : capacity(10) {
        buffer = new char[capacity];
        buffer[0] = '\0';
    }

    // 析构函数
    ~StringBuffer() {
        delete[] buffer;
    }

    // 向缓冲区中写入字符串
    bool append(const char* str) {
        size_t len = strlen(str);
        if (len + strlen(buffer) + 1 > capacity) {
            resize(capacity == 0 ? 1 : capacity + len + 2);
        }
        memcpy(buffer + strlen(buffer), str, len + 1);
        return true;
    }

    void putc(char ch) {
        char buffer[] = { ch, 0 };
        this->append((const char*)&buffer);
    }

    void append(int num) {
        char buf[50];
        append(itoa(num, buf, 10));
    }

    void format(const char* format, ...);

    void copy(StringBuffer* src) {
        append(src->c_str());
    }

    // 从缓冲区中读取字符串
    const char* c_str() const {
        return buffer;
    }

    // 复制一个新的字符串出来
    char* to_string() {
        int len = strlen(buffer);
        char* result = new char[len + 1];
        memcpy(result,this->buffer,len);
        return result;
    }

    // 获取缓冲区中当前字符串的长度
    size_t count() const {
        return strlen(buffer);
    }
};