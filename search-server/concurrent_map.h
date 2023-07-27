#pragma once
#pragma once
#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <mutex>






template <typename Key, typename Value>
class ConcurrentMap {
private:
    struct Bucket {
        std::mutex mutex_;
        std::map<Key, Value> Data_;
    };

    std::vector<Bucket> shelving_;

public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        std::lock_guard<std::mutex> lock_guard_mutex;
        Value& ref_to_value;

        Access(const Key& key, Bucket& bucket) : lock_guard_mutex(bucket.mutex_)
            , ref_to_value(bucket.Data_[key]) {}
    };

    explicit ConcurrentMap(size_t shelving_count) : shelving_(shelving_count) {}

    Access operator[](const Key& key) {

        auto& temp = shelving_[uint64_t(key) % shelving_.size()];
        return { key,
                temp };
    }

    std::map<Key, Value> BuildOrdinaryMap() {

        std::map<Key, Value> result;

        for (auto& [mutex_, Data_] : shelving_) {
            std::lock_guard lock_guard_mutex(mutex_);
            result.insert(Data_.begin(), Data_.end());
        }

        return result;
    }

    void erase(const Key& key) {
        auto& temp = shelving_[static_cast<uint64_t>(key) % shelving_.size()];
        std::lock_guard lock_guard_mutex(temp.mutex_);
        temp.Data_.erase(key);
    }
};
