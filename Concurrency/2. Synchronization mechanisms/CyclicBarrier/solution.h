//
// Created by Kalugin Dima on 21.03.17.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

template<class ConditionVariable = std::condition_variable>
class CyclicBarrier {
 public:
  explicit CyclicBarrier(const size_t num_threads)
    : num_threads_{num_threads},
      num_threads_waiting_{0},
      era_{0} {
  }

  void Pass() {
    std::unique_lock<std::mutex> lock(mutex_);
    ++num_threads_waiting_;
    if (num_threads_waiting_ == num_threads_) {
      ++era_;
      num_threads_waiting_ = 0;
      all_threads_arrived_cv_.notify_all();
    } else {
      std::size_t curr_era = era_;
      while (num_threads_waiting_ < num_threads_ && curr_era == era_) {
        all_threads_arrived_cv_.wait(lock);
      }
    }
  }

 private:
  const size_t num_threads_;
  size_t num_threads_waiting_;
  std::size_t era_;
  ConditionVariable all_threads_arrived_cv_;
  std::mutex mutex_;
};
