#include <vector>

template<typename T>
class FIFOBuffer {
public:
    FIFOBuffer(size_t maxSize = 0) : maxSize_(maxSize), head_(0), tail_(0), count_(0) {
        if (maxSize_ > 0) {
            cache_.resize(maxSize_);
        }
    }

    void push(const T& value) {
        if (maxSize_ > 0 && count_ >= maxSize_) {
            // 缓存区已满，移除最前面的元素
            head_ = (head_ + 1) % maxSize_;
            --count_;
        } else if (maxSize_ == 0 && count_ >= cache_.size()) {
            // 如果没有最大大小限制，但缓存区已满，扩展缓存区
            cache_.push_back(value);
            tail_ = cache_.size() - 1;
        } else {
            // 添加新元素
            if (count_ < maxSize_) {
                cache_[tail_] = value;
                tail_ = (tail_ + 1) % maxSize_;
            } else {
                cache_.push_back(value);
                tail_ = cache_.size() - 1;
            }
            ++count_;
        }
    }

    bool pop(T& value) {
        if (count_ == 0) {
            return false; // 缓存区为空
        }
        value = cache_[head_];
        head_ = (head_ + 1) % maxSize_;
        --count_;
        return true;
    }

    bool empty() const {
        return count_ == 0;
    }

    size_t size() const {
        return count_;
    }

private:
    std::vector<T> cache_;
    size_t maxSize_;
    size_t head_; // 队列头
    size_t tail_; // 队列尾
    size_t count_; // 当前元素数量
};