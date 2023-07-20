#pragma once
#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "log_duration.h"


using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
private:

public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Bucket
    {
        std::mutex mtx;
        std::map<Key, Value> map;

        void Erase(Key key)
        {
            map.erase(key);
        }
    };


    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;

        Access(const Key& key, Bucket& bucket)
            : guard(bucket.mtx)
            , ref_to_value(bucket.map[key]) {
        }
    };

    explicit ConcurrentMap(size_t bucket_count)
        : all_parts_(bucket_count) {
    }

    Access operator[](const Key& key) {
        auto& bucket = all_parts_[static_cast<uint64_t>(key) % all_parts_.size()];
        return { key, bucket };
    }

    void Erase(Key key)
    {
        all_parts_[static_cast<uint64_t>(key) % all_parts_.size()].Erase(key);
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for (auto& [mtx, map] : all_parts_) {
            std::lock_guard g(mtx);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

private:
    std::vector<Bucket> all_parts_;
};