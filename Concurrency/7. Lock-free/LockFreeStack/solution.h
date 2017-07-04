#pragma once

#include <atomic>
#include <thread>

template<typename T>
class LockFreeStack {
  struct Node {
    T element;
    std::atomic<Node *> next;
    Node(T new_element)
        : element(new_element),
          next(nullptr) {
    }
  };

 public:
  explicit LockFreeStack() {}

  ~LockFreeStack() {
    Node *node = to_remove_.load();
    while (node != nullptr) {
      Node *next = node->next.load();
      delete node;
      node = next;
    }
    node = top_.load();
    while (node != nullptr) {
      Node *next = node->next.load();
      delete node;
      node = next;
    }
  }

  void Push(T element) {
    Node *new_node = new Node(element);
    Node *curr_top = top_.load();
    new_node->next.store(curr_top);
    while (!top_.compare_exchange_weak(curr_top, new_node)) {
      new_node->next.store(curr_top);
    }
  }

  bool Pop(T &element) {
    Node *curr_top = top_.load();
    while (true) {
      if (!curr_top) {
        return false;
      }
      if (top_.compare_exchange_weak(curr_top, curr_top->next.load())) {
        element = curr_top->element;
        Node *deleted = to_remove_.load();
        curr_top->next.store(deleted);
        while (!to_remove_.compare_exchange_weak(deleted, curr_top)) {
          curr_top->next.store(deleted);
        }
        return true;
      }
    }
  }

 private:
  std::atomic<Node *> top_{nullptr};
  std::atomic<Node *> to_remove_{nullptr};
};

template<typename T>
using ConcurrentStack = LockFreeStack<T>;