#pragma once

#include <functional>
#include <vector>
#include <thread>

template <class Task>
void hello_world(std::function<void(const Task&)> func, const std::vector<Task>& tasks) {
    std::vector<std::thread> threads(tasks.size());
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        threads[i] = std::thread(func, tasks[i]);
    }
    for (auto& thread : threads) {
        thread.join();
    }
};