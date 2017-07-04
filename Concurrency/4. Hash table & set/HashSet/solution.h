//
// Created by Kalugin Dima on 06.04.17.
//

#pragma once

#include <algorithm>
#include <atomic>
#include <forward_list>
#include <mutex>
#include <vector>

template<typename T, class H = std::hash<T>>
class StripedHashSet {
 public:
  StripedHashSet(std::size_t concurrency_level = 1,
                 std::size_t growth_factor = 2,
                 double max_load_factor = 5.)
      : hash_table_{growth_factor * concurrency_level},
        stripes_{concurrency_level},
        size_{0},
        growth_factor_{growth_factor},
        max_load_factor_{max_load_factor} {
  }

  bool Insert(const T &element);
  bool Remove(const T &element);
  bool Contains(const T &element);
  std::size_t Size() const {
    return size_.load();
  }

 private:
  std::vector<std::forward_list<T>> hash_table_;
  std::vector<std::mutex> stripes_;
  std::atomic<std::size_t> size_;
  std::size_t growth_factor_;
  double max_load_factor_;
  H hash_function_;
  void Resize_();

  std::size_t GetBasketNum_(const T &element) const {
    return hash_function_(element) % hash_table_.size();
  }
  std::size_t GetStripeNum_(const T &element) const {
    return hash_function_(element) % stripes_.size();
  }
  bool IsFilled_() const {
    return size_ >= max_load_factor_ * hash_table_.size();
  }
};

template<typename T, class H>
bool StripedHashSet<T, H>::Insert(const T &element) {
  const std::size_t stripe = GetStripeNum_(element);
  std::unique_lock<std::mutex> lock(stripes_[stripe]);
  if (IsFilled_()) {
    lock.unlock();
    Resize_();
    lock.lock();
  }
  const std::size_t hash = GetBasketNum_(element);
  const auto result = std::find(hash_table_[hash].begin(), hash_table_[hash].end(), element);
  if (result != hash_table_[hash].end()) {
    return false;
  }
  hash_table_[hash].push_front(element);
  size_.fetch_add(1);
  return true;
};

template<typename T, class H>
bool StripedHashSet<T, H>::Remove(const T &element) {
  const std::size_t stripe = GetStripeNum_(element);
  std::unique_lock<std::mutex> lock(stripes_[stripe]);
  const std::size_t hash = GetBasketNum_(element);
  const auto result = std::find(hash_table_[hash].begin(), hash_table_[hash].end(), element);
  if (result == hash_table_[hash].end()) {
    return false;
  }
  hash_table_[hash].remove(element);
  size_.fetch_sub(1);
  return true;
};

template<typename T, class H>
bool StripedHashSet<T, H>::Contains(const T &element) {
  std::size_t stripe = GetStripeNum_(element);
  std::unique_lock<std::mutex> lock(stripes_[stripe]);
  const std::size_t hash = GetBasketNum_(element);
  const auto result = std::find(hash_table_[hash].begin(), hash_table_[hash].end(), element);
  return result != hash_table_[hash].end();
};

template<typename T, class H>
void StripedHashSet<T, H>::Resize_() {
  std::vector<std::unique_lock<std::mutex>> locks;
  locks.push_back(std::unique_lock<std::mutex>(stripes_[0]));
  if (!IsFilled_()) {
    return;
  }
  for (std::size_t i = 1; i < stripes_.size(); ++i) {
    locks.push_back(std::unique_lock<std::mutex>(stripes_[i]));
  }
  const std::size_t new_capacity = growth_factor_ * hash_table_.size();
  std::vector<std::forward_list<T>> new_hash_table_(new_capacity);
  for (const auto& hash_list: hash_table_) {
    for (const T& element: hash_list) {
      std::size_t new_hash = hash_function_(element) % new_capacity;
      new_hash_table_[new_hash].push_front(element);
    }
  }
  hash_table_ = new_hash_table_;
};

template<typename T, class H = std::hash<T>>
using ConcurrentSet = StripedHashSet<T, H>;