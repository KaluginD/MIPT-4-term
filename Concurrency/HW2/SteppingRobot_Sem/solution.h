//
// Created by Kalugin Dima on 21.03.17.
//

#pragma once

#include <condition_variable>
#include <iostream>

class Semaphore {
 public:
  explicit Semaphore(std::size_t throudhput) {
    throudhput_ = throudhput;
  }

  void Enter() {
    std::unique_lock<std::mutex> lock(mutex_);
    --throudhput_;
    if(throudhput_ < 0) {
      cv_.wait(lock);
    }
  }

  void Leave() {
    std::unique_lock<std::mutex> lock(mutex_);
    ++throudhput_;
    if(throudhput_ < 1) {
      cv_.notify_one();
    }
  }

 private:
  int throudhput_;
  std::condition_variable cv_;
  std::mutex mutex_;
};

class Robot {
 public:
  explicit Robot()
      : left_step_{1},
        right_step_{0} {
  }

  void StepLeft() {
    left_step_.Enter();
    std::cout << "left" << std::endl;
    right_step_.Leave();
  }

  void StepRight() {
    right_step_.Enter();
    std::cout << "right" << std::endl;
    left_step_.Leave();
  }

 private:
  Semaphore left_step_;
  Semaphore right_step_;
};
