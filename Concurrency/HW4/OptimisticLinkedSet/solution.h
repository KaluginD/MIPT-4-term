//
// Created by Kalugin Dima on 12.04.17.
//

#pragma once

#include "arena_allocator.h"

#include <vector>
#include <atomic>
#include <limits>
#include <mutex>

template<typename T>
struct ElementTraits {
  static T Min() {
    return std::numeric_limits<T>::min();
  }
  static T Max() {
    return std::numeric_limits<T>::max();
  }
};

class SpinLock {
 public:
  explicit SpinLock()
      : locked_{false} {}

  void Lock() {
    while (locked_.exchange(true)) {
      std::this_thread::yield();
    }
  }
  void Unlock() {
    locked_.store(false);
  }
  // adapters for BasicLockable concept
  void lock() {
    Lock();
  }
  void unlock() {
    Unlock();
  }
 private:
  std::atomic<bool> locked_;
};

template<typename T>
class OptimisticLinkedSet {
 private:
  struct Node {
    T element_;
    std::atomic<Node *> next_;
    SpinLock lock_{};
    std::atomic<bool> marked_{false};

    Node(const T &element, Node *next = nullptr)
        : element_(element),
          next_(next) {
    }
  };

  struct Edge {
    Node *pred_;
    Node *curr_;

    Edge(Node *pred, Node *curr)
        : pred_(pred),
          curr_(curr) {
    }
  };

 public:
  explicit OptimisticLinkedSet(ArenaAllocator &allocator)
      : allocator_{allocator},
        size_{0} {
    CreateEmptyList();
  }

  bool Insert(const T &element);

  bool Remove(const T &element);

  bool Contains(const T &element);

  size_t Size() const {
    return size_;
  }

 private:
  void CreateEmptyList() {
    head_ = allocator_.New<Node>(ElementTraits<T>::Min());
    head_->next_ = allocator_.New<Node>(ElementTraits<T>::Max());
  }

  Edge Locate(const T &element) const {
    Node *pred = head_;
    Node *next = head_->next_;
    while (next->element_ < element) {
      pred = next;
      next = next->next_;
    }
    return Edge{pred, next};
  }

  bool Validate(const Edge &edge) const {
    Node *pred = edge.pred_;
    Node *curr = edge.curr_;
    return (pred->next_ == curr && !pred->marked_ && !curr->marked_);
  }

 private:
  ArenaAllocator &allocator_;
  Node *head_{nullptr};
  std::size_t size_;
};

template<typename T>
bool OptimisticLinkedSet<T>::Insert(const T &element) {
  while (true) {
    Edge edge = Locate(element);
    Node *pred = edge.pred_;
    Node *curr = edge.curr_;
    std::unique_lock<SpinLock> pred_lock(pred->lock_);
    std::unique_lock<SpinLock> curr_lock(curr->lock_);
    if (Validate(Edge(pred, curr))) {
      if (curr->element_ == element) {
        return false;
      }
      Node *new_node = new Node(element, curr);
      pred->next_.store(new_node);
      ++size_;
      return true;
    }
  }
}

template<typename T>
bool OptimisticLinkedSet<T>::Remove(const T &element) {
  while (true) {
    Edge edge = Locate(element);
    Node *pred = edge.pred_;
    Node *curr = edge.curr_;
    std::unique_lock<SpinLock> pred_lock(pred->lock_);
    std::unique_lock<SpinLock> curr_lock(curr->lock_);
    if (Validate(Edge(pred, curr))) {
      if (curr->element_ != element) {
        return false;
      }
      curr->marked_.store(true);
      pred->next_.store(curr->next_);
      --size_;
      return true;
    }
  }
}

template<typename T>
bool OptimisticLinkedSet<T>::Contains(const T &element) {
  Node *curr = head_;
  while (curr->element_ < element) {
    curr = curr->next_;
  }
  return (curr->element_ == element && !curr->marked_);
}

template<typename T> using ConcurrentSet = OptimisticLinkedSet<T>;
