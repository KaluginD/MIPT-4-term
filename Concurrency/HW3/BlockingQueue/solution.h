//
// Created by Kalugin Dima on 28.03.17.
//

#pragma once

#include<queue>
#include <mutex>
#include <condition_variable>

template <class T, class Container = std::deque<T>>
class BlockingQueue {
 public:
  explicit BlockingQueue(const std::size_t& capacity)
      : capacity_{capacity},
        is_shut_{false} {
  }

  void Put(T&& element) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (is_shut_) {
      throw std::exception();
    }
    is_full_.wait(lock, [this] {return queue_.size() < capacity_;});
    queue_.push_back(std::move(element));
    is_empty_.notify_all();
  }

  bool Get(T& result) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (is_shut_ && queue_.empty()) {
      return false;
    }
    is_empty_.wait(lock, [this] {return queue_.size() > 0;});
    result = std::move(queue_.front());
    queue_.pop_front();
    is_full_.notify_all();
    return true;
  }

  void Shutdown() {
    std::unique_lock<std::mutex> lock(mutex_);
    is_shut_ = true;
    is_full_.notify_all();
    is_empty_.notify_all();
  }

 private:
  Container queue_;
  std::size_t capacity_;
  std::mutex mutex_;
  std::condition_variable is_empty_;
  std::condition_variable is_full_;
  bool is_shut_;
};