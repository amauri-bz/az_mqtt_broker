#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include "az_worker_pool_intf.hpp"

namespace AzMqttBroker {

class WorkerPool : public WorkerPoolIntf {
private:
    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mtx;
    std::condition_variable cv;
    bool stop = false;

public:
    WorkerPool(size_t num_threads);

    // Implementação da interface: Enfileira uma tarefa genérica
    void enqueue(std::function<void()> task);

    ~WorkerPool();

private:
    void worker_loop(std::stop_token st);
};

}