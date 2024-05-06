# C++并发编程之condition_variable

这个仓库主记录了生产者和消费者模式中经典的案例。

**1.CPU轮询等待版单生产者单消费者：**（basic.cpp）

该版本使用了简单的轮询机制，生产者不断地检查消费者是否已经消费完数据。这种模式简单直接，但效率较低，因为生产者在没有数据时仍然在忙等待。

**实现**

```c++
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
```

**2.等待通知版单生产者单消费者：**（single.cpp）

该版本引入了等待通知机制，生产者在没有数据时会等待消费者的通知。这种模式避免了忙等待，提高了效率，同时减少了资源消耗。

**实现**

在这个版本我们借助condition_variable，完成多线程之间的同步操作。

```c++
std::condition_variable cv_;
bool ready_{false};
```

生产者会不断生成一个随机数并将其存储在 data_ 变量中，然后将 ready_ 标志设置为 true，表示有可用的数据。接着，它通知等待的消费者线程，然后自己等待消费者线程处理完数据。

```c++
cv_.wait(ul, [this]() { return !ready_; });
```

消费者将会在一个无限循环中等待生产者通知数据的可用性。当 ready_ 标志为 true 时，它会从 data_ 变量中获取数据并进行处理，然后将 ready_ 标志设置为 false，表示数据已经被消费。接着，它通知生产者线程，然后自己等待生产者线程生成新的数据。

```c++
cv_.wait(ul, [this]() { return ready_; });
```

**3.等待通知版单生产者多消费者：**（mutiple.cpp）

在这个版本中，引入了多个消费者，它们共享生产者的数据。生产者在产生数据后，通知所有消费者进行处理。

**实现**

前面的版本中，我们还是单生产，单个消费者，如何做到多个消费者抢占消费？

此时需要引入队列，我们将任务丢到队列中去，随后多个消费者进行消费即可，与上述等待条件不同点在于队列的状态。

对于生产者：如果队列大小未达到 max_queue_size_ 的限制，如果队列已满，则生产者线程将等待消费者线程从队列中取走一些数据。

```c++
cv_.wait(ul, [this]() { return queue_.size() < max_queue_size_; }); 
```

对于消费者：队列有数据就消费，否则等待。

```c++
cv_.wait(ul, [this]() { return !queue_.empty(); });
```

**4.等待通知版多生产者多消费者：**（mutiple_mutiple.cpp）

这个版本支持多个生产者和多个消费者，生产者之间和消费者之间共享数据。生产者在产生数据后，通知所有消费者进行处理。

**实现**

对于这个版本比较简单，基于第三个版本继续优化，创建n个生产者线程即可。

**5.单生产者多消费者并行版：**（mutiple_sync.cpp）

在这个版本中，引入了并行处理机制，多个消费者可以同时处理数据。生产者产生数据后，多个消费者并行处理，提高了整体处理速度。

**实现**

对于以上版本有个比较大的问题，当生产者生产的数据到达上限时，消费者此时在消费，而生产者并没有动起来，它在等待消费者消费完才能进行，如何让生产者与消费者同时运转呢？

改进点在于使用多个cv，来回切换通知。

```c++
std::condition_variable cv_producer_;
std::condition_variable cv_consumer_;
```

**6.多生产者多消费者并行版：**（mutiple_mutiple_sync.cpp）

这个版本支持多个生产者和多个消费者，并且允许并行处理。多个生产者并行产生数据，多个消费者并行处理数据，提高了整体并发能力。

**实现**

基于5进行改造，支持多个生产者即可。

**7.支持Lambda回调的优雅停止版：**（mutiple_mutiple_stop.cpp）

在这个版本中，引入了Lambda回调函数，用于优雅地停止并发处理。可以通过调用回调函数来停止生产者和消费者的处理，并进行清理工作。

**实现**

简单来说，就是加回调函数



运行【比较简朴~】

```c++
g++  mutiple_mutiple_stop.cpp -o mutiple_mutiple_stop -lpthread
```



本文学习自：https://github.com/LHCyGan/Concurrent_Programming