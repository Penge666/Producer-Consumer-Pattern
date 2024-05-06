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
            data_ = rand() % 100;
            cout << "生产data:" << data_ << endl;
            ready_ = true;
            lock.unlock();
            while (ready_)
            {
                this_thread::sleep_for(chrono::seconds(1));
            }
        }
    }
    void Producer()
    {
        while (1)
        {
            while (!ready_)
            {
                this_thread::sleep_for(chrono::seconds(1));
            }
            unique_lock<mutex> lock(mutex_);
            cout << "消费data:" << data_ << endl;
            ready_ = false;
            lock.unlock();
        }
    }
    void join()
    {
        thread consumer_thread(&Worker::Consumer, this);
        thread produce_thread(&Worker::Producer, this);
        consumer_thread.join();
        produce_thread.join();
    }

private:
    mutex mutex_;
    int data_{0};
    bool ready_{false};
};

int main()
{
    Worker worker;
    worker.join();
    return 0;
}