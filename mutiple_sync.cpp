#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

class Worker
{
public:
    Worker(int max_num) : max_num_(max_num) {}
    void Consumer()
    {
        while (1)
        {
            unique_lock<mutex> lock(consumer_mutex_);
            cv_consumer_.wait(lock, [this]()
                              { return !q.empty(); });
            auto num = q.front();
            q.pop();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            cout << "消费者：" << this_thread::get_id() << " num:" << num << endl;
            lock.unlock();
            cv_producer_.notify_one();
        }
    }
    void Producer()
    {
        while (1)
        {
            unique_lock<mutex> lock(producer_mutex_);
            cv_producer_.wait(lock, [this]()
                              { return q.size() < max_num_; });
            int num = rand() % 100;
            cout << "生产者：" << this_thread::get_id() << " num:" << num << endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            q.push(num);
            lock.unlock();
            cv_consumer_.notify_one();
        }
    }
    void Join(int num_consumers)
    {
        vector<thread> consumer_thread;
        thread producer_thread(&Worker::Producer, this);
        for (int i = 0; i < num_consumers; i++)
        {
            consumer_thread.emplace_back(&Worker::Consumer, this);
        }
        producer_thread.join();
        for (auto &thread : consumer_thread)
        {
            thread.join();
        }
        return;
    }

private:
    queue<int> q;
    int max_num_;
    mutex producer_mutex_;
    mutex consumer_mutex_;
    condition_variable cv_producer_;
    condition_variable cv_consumer_;
};

int main()
{
    Worker worker(5);
    worker.Join(2);
    return 0;
}
