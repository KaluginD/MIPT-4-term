//
// Created by Kalugin Dima on 21.03.17.
//

#pragma once

#include <condition_variable>
#include <iostream>
#include <atomic>

class Robot {
 public:
  explicit Robot() {
    which_step_ = false;
  }

  void StepLeft() {
    std::unique_lock<std::mutex> lock(mutex_);
    left_step_.wait(lock, [this] { return !which_step_;});
    std::cout << "left" << std::endl;
    which_step_ = true;
    right_step_.notify_one();
  }

  void StepRight() {
    std::unique_lock<std::mutex> lock(mutex_);
    right_step_.wait(lock, [this] { return which_step_;});
    std::cout << "right" << std::endl;
    which_step_ = false;
    left_step_.notify_one();
  }

 private:
  std::condition_variable left_step_;
  std::condition_variable right_step_;
  // last step: false - right step, true - left step
  bool which_step_;
  std::mutex mutex_;
};
