#pragma once

#include <atomic>
#include <memory>
#include <thread>

template<typename T, template<typename U> class Atomic = std::atomic>
class LockFreeQueue {
  struct Node {
    T element_{};
    Atomic<Node *> next_{nullptr};

    explicit Node(T element, Node *next = nullptr)
        : element_(std::move(element)), next_(next) {
    }

    explicit Node() {
    }
  };

 public:
  explicit LockFreeQueue() {
    Node *dummy = new Node{};
    head_ = dummy;
    tail_ = dummy;
    to_remove_ = dummy;
    active_threads_.store(0);
  }

  ~LockFreeQueue() {
    while (to_remove_.load() != nullptr) {
      Node *tmp = to_remove_.load();
      to_remove_.store(tmp->next_.load());
      delete tmp;
    }
  }

  void Enqueue(T element) {
    active_threads_.fetch_add(1);
    Node *curr_tail;
    Node *new_node = new Node(element);
    while (true) {
      curr_tail = tail_.load();
      Node *curr_tail_next = curr_tail->next_.load();
      if (!curr_tail_next) {
        if (tail_.load()->next_.compare_exchange_weak(curr_tail_next, new_node)) {
          break;
        }
      } else {
        tail_.compare_exchange_weak(curr_tail, curr_tail_next);
      }
    }
    tail_.compare_exchange_weak(curr_tail, new_node);
    active_threads_.fetch_sub(1);
  }

  bool Dequeue(T &element) {
    active_threads_.fetch_add(1);
    Node *curr_head;
    while (true) {
      curr_head = head_.load();
      Node *curr_tail = tail_.load();
      Node *next = curr_head->next_.load();
      if (curr_head == head_.load()) {
        if (curr_head == curr_tail) {
          if (next == nullptr) {
            return false;
          }
          head_.compare_exchange_weak(curr_head, next);
        } else {
          if (head_.compare_exchange_weak(curr_head, next)) {
            element = next->element_;
            break;
          }
        }
      }
    }
    if (active_threads_.load() == 1) {
      Node *node = to_remove_.exchange(curr_head);
      while (node != curr_head) {
        Node *tmp = node;
        node = node->next_.load();
        delete tmp;
      }
    }
    active_threads_.fetch_sub(1);
    return true;
  }

 private:
  Atomic<Node *> head_{nullptr};
  Atomic<Node *> tail_{nullptr};
  Atomic<Node *> to_remove_{nullptr};
  Atomic<std::size_t> active_threads_{0};
};
