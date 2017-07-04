#pragma once

#include <algorithm>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <stack>
#include <queue>

template<class T, class Container = std::deque<T>>
class BlockingQueue {
 public:
  explicit BlockingQueue(const std::size_t &capacity)
      : capacity_{capacity},
        is_shut_{false} {
  }

  void Put(T &&element) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (is_shut_) {
      throw std::exception();
    }
    is_full_.wait(lock, [this] { return queue_.size() < capacity_; });
    if (is_shut_) {
      throw std::exception();
    }
    queue_.push_back(std::move(element));
    is_empty_.notify_one();
  }

  bool Get(T &result) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      if (is_shut_) {
        throw std::exception();
      }
      is_empty_.wait(lock);
    }
    result = std::move(queue_.front());
    queue_.pop_front();
    is_full_.notify_one();
    return true;
  }

  void Shutdown() {
    std::unique_lock<std::mutex> lock(mutex_);
    is_shut_ = true;
    is_empty_.notify_all();
    is_full_.notify_all();
  }

 private:
  Container queue_;
  std::size_t capacity_;
  std::mutex mutex_;
  std::condition_variable is_empty_;
  std::condition_variable is_full_;
  bool is_shut_;
};

template<class T>
class ThreadPool {
 public:
  ThreadPool()
      : queue_(std::numeric_limits<std::size_t>::max()) {
    std::size_t tmp = std::thread::hardware_concurrency();
    std::size_t num_threads = tmp != 0 ? tmp : 4;
    for (std::size_t i = 0; i < num_threads; ++i) {
      workers_.push_back(std::thread([this]() { this->TaskPerforming_(); }));
    }
  }

  explicit ThreadPool(const size_t num_threads)
      : queue_(std::numeric_limits<std::size_t>::max()) {
    for (std::size_t i = 0; i < num_threads; ++i) {
      workers_.push_back(std::thread([this]() { this->TaskPerforming_(); }));
    }
  }

  std::future<T> Submit(std::function<T()> task) {
    std::packaged_task<T()> curr_task(task);
    std::future<T> curr_future = curr_task.get_future();
    queue_.Put(std::move(curr_task));
    return std::move(curr_future);
  }

  void Shutdown() {
    queue_.Shutdown();
    for (auto &worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

 private:
  BlockingQueue<std::packaged_task<T()>> queue_;
  std::vector<std::thread> workers_;

  void TaskPerforming_() {
    std::packaged_task<T()> task;
    try {
      while (true) {
        queue_.Get(task);
        task();
      }
    }
    catch (std::exception &) {
      return;
    }
  }
};

class ParallelSort {
 public:
  explicit ParallelSort(std::size_t thread_number)
      : sorting_pools_(thread_number) {}

  ~ParallelSort() {
    sorting_pools_.Shutdown();
  }

  template<typename RandomAccessIterator, typename Compare>
  void Sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp) {
    std::stack<std::future<void>> futures;
    std::stack<RandomAccessIterator> beginning_stack;
    while (last - first > 64) {
      RandomAccessIterator middle = first + (first - last) / 2;
      auto future = sorting_pools_.Submit(std::bind(&ParallelSort::Sort<RandomAccessIterator, Compare>,
                                                    this, first, middle, comp));
      futures.emplace(std::move(future));
      beginning_stack.push(first);
      first = middle;
    }
    std::sort(first, last, comp);
    while (!beginning_stack.empty()) {
      futures.pop();
      std::inplace_merge(beginning_stack.top(), first, last, comp);
      first = beginning_stack.top();
      beginning_stack.pop();
    }
  }

 private:
  ThreadPool<void> sorting_pools_;
};