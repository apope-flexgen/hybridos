#include <rigtorp/MPMCQueue.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <iostream>

class IO_Task {
    // ... other members ...
public:
    void operator()() {
        // put the code that should be executed by the thread here
    }
};

struct QueueWrapper {
    rigtorp::MPMCQueue<void(*)(IO_Task*)> setQueue;
    rigtorp::MPMCQueue<void(*)(IO_Task*)> getQueue;
    IO_Task* ioTask;
    bool* shouldStop;
    std::condition_variable* cv;
    std::mutex* mutex;
    bool* ready;
    int sets;
    int gets;

    QueueWrapper(size_t size) : setQueue(size), getQueue(size) {}
};

std::mutex output_mutex;

void worker(QueueWrapper& queues) {
    std::unique_lock<std::mutex> lock(*queues.mutex);
    queues.cv->wait(lock, [&queues]{ return *(queues.ready); });
    lock.unlock();
    int sets  = 0;
    int gets  = 0;
    while (true) {
        void(*task)(IO_Task*) = nullptr;
        if (queues.setQueue.try_pop(task)) {
            task(queues.ioTask);
            sets++;
        } else if (queues.getQueue.try_pop(task)) {
            task(queues.ioTask);
            gets++;
        }

        if (*(queues.shouldStop)) {
            break;
        }
    }
    std::lock_guard<std::mutex> lock2(output_mutex); 
    std::cout << " Thread done; sets " << sets << " Gets "<<gets<<std::endl;
}

int main() {
    size_t queueSize = 10;
    IO_Task ioTask;
    bool shouldStop = false;
    std::condition_variable cv;
    std::mutex mutex;
    bool ready = false;

    QueueWrapper queues(queueSize);
    queues.ioTask = &ioTask;
    queues.shouldStop = &shouldStop;
    queues.cv = &cv;
    queues.mutex = &mutex;
    queues.ready = &ready;

    int numThreads = 4;
    std::vector<std::thread> workerThreads;
    for (int i = 0; i < numThreads; i++) {
        workerThreads.push_back(std::thread(worker, std::ref(queues)));
    }
    // Enqueue tasks
    for (int i = 0; i < 10; i++) {
        void(*setTask)(IO_Task*) = [](IO_Task* ioTask) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            // code for setting a value
        };
        queues.setQueue.try_push(setTask);

        void(*getTask)(IO_Task*) = [](IO_Task* ioTask) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            // code for getting a value
        };
        queues.getQueue.try_push(getTask);
    }

    // Start all threads at once
    {
        std::unique_lock<std::mutex> lock(mutex);
        ready = true;
        cv.notify_all();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Stop worker threads
    {
        std::unique_lock<std::mutex> lock(mutex);
        shouldStop = true;
    }
    for (auto& t : workerThreads) {
        t.join();
    }

    return 0;
}
