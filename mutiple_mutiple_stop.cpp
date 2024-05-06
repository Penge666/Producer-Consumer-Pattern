#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <functional>
using namespace std;

class Worker
{
public:
    Worker(int max_num) : max_num_(max_num) {}
    void Consumer()
    {
        while (1)
        {
            if (flag == 1)
            {
                break;
            }
            unique_lock<mutex> lock(consumer_mutex_);
            // if (stop_func_())
            // {
            //     int flag = 1;
            //     break;
            // }
            cv_consumer_.wait(lock, [this]()
                              { return !q.empty(); });
            auto num = q.front();
            q.pop();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            cout << "消费者：" << this_thread::get_id() << " num:" << num << endl;
            lock.unlock();
            if (flag == 0)
                cv_producer_.notify_one();
            if (flag == 1)
            {
                break;
            }
        }
        // return;
    }
    void Producer()
    {
        while (1)
        {
            unique_lock<mutex> lock(producer_mutex_);
            if (stop_func_())
            {
                int flag = 1;
                break;
            }
            cv_producer_.wait(lock, [this]()
                              { return q.size() < max_num_; });
            int num = rand() % 100;
            sum++;
            cout << "生产者：" << this_thread::get_id() << " num:" << num << endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            q.push(num);
            lock.unlock();

            cv_consumer_.notify_one();
        }
    }
    void Join(int num_consumers, int num_producer)
    {
        vector<thread> consumer_thread;
        vector<thread> producer_thread;
        for (int i = 0; i < num_producer; i++)
        {
            producer_thread.emplace_back(&Worker::Producer, this);
        }
        for (int i = 0; i < num_consumers; i++)
        {
            consumer_thread.emplace_back(&Worker::Consumer, this);
        }
        for (auto &thread : producer_thread)
        {
            thread.join();
        }
        for (auto &thread : consumer_thread)
        {
            thread.join();
        }
        return;
    }
    void setStop_fun(function<bool()> stop_func)
    {
        stop_func_ = stop_func;
    }
    int Sum()
    {
        return sum;
    }

private:
    queue<int> q;
    int max_num_;
    mutex producer_mutex_;
    mutex consumer_mutex_;
    condition_variable cv_producer_;
    condition_variable cv_consumer_;
    function<bool()> stop_func_;
    int sum{0};
    int flag{0};
};
int main()
{
    Worker worker(5);
    int NUM = 3;
    auto func = [&]()
    {
        return worker.Sum() > NUM;
    };
    worker.setStop_fun(func);
    worker.Join(2, 3);
    return 0;
}
