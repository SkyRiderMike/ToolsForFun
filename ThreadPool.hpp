#ifndef INCLUDE_OKVIS_THREAD_POOL_HPP_
#define INCLUDE_OKVIS_THREAD_POOL_HPP_

#include <condition_variable>
#include <future>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>
#include <glog/logging.h>

/**
 * @brief This class manages multiple threads and fills them with work.
 */
class ThreadPool
{
 public:
  /// \brief Constructor. Launches some amount of workers.
  /// \param[in] numThreads The number of threads in the pool.
  ThreadPool(size_t numThreads);

  /// \brief Destructor. This joins all threads.
  ~ThreadPool();

  /// \brief Enqueue work for the thread pool
  ///
  /// Pass in a function and its arguments to enqueue work in the thread pool
  /// \param[in] function A function pointer to be called by a thread.
  /// \param args Function arguments.
  /// \returns A std::future that will return the result of calling function.
  ///          If this function is called after the thread pool has been stopped,
  ///          it will return an uninitialized future that will return
  ///          future.valid() == false
  template<class Function, class ... Args>
  std::future<typename std::result_of<Function(Args...)>::type>
  enqueue(Function&& function, Args&&... args);

  /// \brief Stop the thread pool. This method is non-blocking.
  void stop()
  {
    stop_ = true;
  }

  /// \brief This method blocks until the queue is empty.
  void waitForEmptyQueue() const;
 private:
  /// \brief Run a single thread.
  void run();
  /// Need to keep track of threads so we can join them.
  std::vector<std::thread> workers_;
  /// The task queue.
  std::queue<std::function<void()>> tasks_;
  /// A mutex to protect the list of tasks.
  mutable std::mutex tasks_mutex_;
  /// A condition variable for worker threads.
  mutable std::condition_variable tasks_condition_;
  /// A condition variable to support waitForEmptyQueue().
  mutable std::condition_variable wait_condition_;
  /// A counter of active threads
  unsigned active_threads_;
  /// A signal to stop the threads.
  volatile bool stop_;
};

// Enqueue work for the thread pool.
template<class Function, class ... Args>
std::future<typename std::result_of<Function(Args...)>::type> ThreadPool::enqueue(
    Function&& function, Args&&... args)
{
  typedef typename std::result_of<Function(Args...)>::type return_type;
  // Don't allow enqueueing after stopping the pool.
  if (stop_) {
    LOG(ERROR)<< "enqueue() called on stopped ThreadPool";
    // An empty future will return valid() == false.
    return std::future<typename std::result_of<Function(Args...)>::type>();
  }

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<Function>(function), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(tasks_mutex_);
    tasks_.push([task]() {(*task)();});
  }
  tasks_condition_.notify_one();
  return res;
}

}  // namespace okvis