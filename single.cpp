#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std;
class Worker
{
public:
    void Consumer()
    {
        while (1)
        {
            unique_lock<mutex> lock(mutex_);
            cv_.wait(lock, [this]()
                     { return ready_; });
            cout << "消费data:" << data_ << endl;
            ready_ = false;
            lock.unlock();
            cv_.notify_one();
        }
    }
    void Producer()
    {
        while (1)
        {
            unique_lock<mutex> lock(mutex_);
            // 这里理解就是希望怎么样才能往下走
            cv_.wait(lock, [this]()
                     { return !ready_; });
            data_ = rand() % 100;
            cout << "生产data:" << data_ << endl;
            ready_ = true;
            lock.unlock();
            cv_.notify_one();
        }
    }
    void Join()
    {
        std::thread consumer_thread(&Worker::Consumer, this);
        std::thread producer_thread(&Worker::Producer, this);
        consumer_thread.join();
        producer_thread.join();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool ready_{false};
    int data_{0};
};

int main()
{
    Worker w;
    w.Join();
    return 0;
}
