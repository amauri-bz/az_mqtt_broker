#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "../include/az_worker_pool.hpp"
#include "../include/az_thread_pool_intf.hpp"

namespace AzMqttBroker {

WorkerPool::WorkerPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this](std::stop_token st) {
            this->worker_loop(st);
        });
    }
}

WorkerPool::~WorkerPool() {
    {
        std::lock_guard lock(queue_mtx);
        stop = true;
    }
    cv.notify_all();
}

void WorkerPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard lock(queue_mtx);
        tasks.push(std::move(task));
    }
    cv.notify_one();
}

void WorkerPool::worker_loop(std::stop_token st) {
    while (!st.stop_requested()) {
        std::function<void()> task;
        {
            std::unique_lock lock(queue_mtx);
            cv.wait(lock, [this, &st] { 
                return stop || !tasks.empty() || st.stop_requested(); 
            });

            if (stop || st.stop_requested()) return;

            task = std::move(tasks.front());
            tasks.pop();
        }
        if (task) task();
    }
}

}