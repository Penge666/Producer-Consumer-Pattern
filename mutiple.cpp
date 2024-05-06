#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

class Worker
{
public:
    Worker(int max_queue_size) : max_queue_size_(max_queue_size) {}

    void Consumer()
    {
        for (;;)
        {
            std::unique_lock<std::mutex> ul(mutex_);
            cv_.wait(ul, [this]()
                     { return !queue_.empty(); });
            int data = queue_.front();
            queue_.pop();
            std::cout << "consumer " << std::this_thread::get_id() << ":consume data: " << data << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            ul.unlock();
            cv_.notify_one();
        }
    }

    void Producer()
    {
        for (;;)
        {
            std::unique_lock<std::mutex> ul(mutex_);
            cv_.wait(ul, [this]()
                     { return queue_.size() < max_queue_size_; });

            int data = rand() % 100;
            queue_.push(data);
            std::cout << "produce data: " << data << std::endl;
            ul.unlock();
            cv_.notify_all();
        }
    }

    void Join(int num_consumers)
    {
        std::vector<std::thread> consumer_threads;
        std::thread producer_thread(&Worker::Producer, this);
        for (int i = 0; i < num_consumers; i++)
        {
            consumer_threads.emplace_back(&Worker::Consumer, this);
        }
        for (auto &thread : consumer_threads)
        {
            thread.join();
        }
        producer_thread.join();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;

    std::queue<int> queue_;
    int max_queue_size_;
};

int main()
{
    Worker w{10};
    w.Join(2);

    return 0;
}
