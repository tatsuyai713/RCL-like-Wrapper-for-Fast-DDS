#ifndef LWRCL_HPP_
#define LWRCL_HPP_

#include <functional>
#include <string>
#include <unordered_map>
#include <forward_list>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <atomic>

#include "fast_dds_header.hpp"
#include "signal_handler.hpp"

#include "publisher.hpp"
#include "subscriber.hpp"
#include "timer.hpp"

namespace lwrcl
{

  class Clock;

  class Node
  {
  public:
    Node(int domain_id);
    Node(std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant);
    virtual ~Node();
    std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> get_participant() const;

    template <typename T>
    Publisher<T> *create_publisher(MessageType *message_type, const std::string &topic, const dds::TopicQos &qos)
    {
      auto publisher = std::make_unique<Publisher<T>>(participant_.get(), message_type, std::string("rt/") + topic, qos);
      Publisher<T> *raw_ptr = publisher.get();
      publisher_list_.push_front(std::move(publisher));
      return raw_ptr;
    }

    template <typename T>
    Subscriber<T> *create_subscription(MessageType *message_type, const std::string &topic, const dds::TopicQos &qos,
                                       std::function<void(T *)> callback_function)
    {
      auto subscriber = std::make_unique<Subscriber<T>>(participant_.get(), message_type, std::string("rt/") + topic, qos, callback_function, channel_);
      Subscriber<T> *raw_ptr = subscriber.get();
      subscription_list_.push_front(std::move(subscriber));
      return raw_ptr;
    }

    template <typename T>
    Timer<T> *create_timer(T period, std::function<void()> callback_function)
    {
      auto timer = std::make_unique<Timer<T>>(period, callback_function, channel_);
      Timer<T> *raw_ptr = timer.get();
      timer_list_.push_front(std::move(timer));
      return raw_ptr;
    }
    virtual void spin();
    virtual void spin_some();
    virtual void stop_spin();
    virtual void shutdown();
    virtual Clock *get_clock();

  private:
    struct DomainParticipantDeleter
    {
        void operator()(eprosima::fastdds::dds::DomainParticipant* participant) const
        {
            if (participant != nullptr)
            {
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
            }
        }
    };

    std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant_;
    std::forward_list<std::unique_ptr<IPublisher>> publisher_list_;
    std::forward_list<std::unique_ptr<ISubscriber>> subscription_list_;
    std::forward_list<std::unique_ptr<ITimer>> timer_list_;
    Channel<ChannelCallback *> channel_;
    std::unique_ptr<Clock> clock_;
  };

  // lwrcl state
  bool ok(void);

  // Executor that manages and executes nodes in a single thread.
  class SingleThreadedExecutor
  {
  public:
    SingleThreadedExecutor();
    ~SingleThreadedExecutor();

    void add_node(Node *node);
    void remove_node(Node *node);
    void stop_spin();
    void spin();
    void spin_some();
    void shutdown();

  private:
    std::vector<Node *> nodes_; // List of nodes managed by the executor.
    std::mutex mutex_;          // Mutex for thread-safe access to the nodes list.
  };

  // Executor that manages and executes nodes, each in its own thread, allowing for parallel processing.
  class MultiThreadedExecutor
  {
  public:
    MultiThreadedExecutor();
    ~MultiThreadedExecutor();

    void add_node(Node *node);
    void remove_node(Node *node);
    void stop_spin();
    void spin();
    void spin_some();
    void shutdown();

  private:
    std::vector<Node *> nodes_;        // List of nodes managed by the executor.
    std::vector<std::thread> threads_; // Threads created for each node for parallel execution.
    std::mutex mutex_;                 // Mutex for thread-safe access to the nodes and threads lists.
  };

  class Duration;

  class Time
  {
  public:
    Time();
    Time(int64_t nanoseconds);
    Time(int32_t seconds, uint32_t nanoseconds);
    int64_t nanoseconds() const;
    double seconds() const;

    Time operator+(const Duration &rhs) const;
    Time operator-(const Duration &rhs) const;
    Duration operator-(const Time &rhs) const;

    bool operator==(const Time &rhs) const;
    bool operator!=(const Time &rhs) const;
    bool operator<(const Time &rhs) const;
    bool operator<=(const Time &rhs) const;
    bool operator>(const Time &rhs) const;
    bool operator>=(const Time &rhs) const;

  private:
    int64_t nanoseconds_;
  };

  class Duration
  {
  public:
    Duration();
    Duration(int64_t nanoseconds);
    Duration(int32_t seconds, uint32_t nanoseconds);

    template <typename Rep, typename Period>
    Duration(const std::chrono::duration<Rep, Period> &duration)
    {
      auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
      nanoseconds_ = nanos.count();
    }

    int64_t nanoseconds() const;
    double seconds() const;

    Duration operator+(const Duration &rhs) const;
    Duration operator-(const Duration &rhs) const;

    bool operator==(const Duration &rhs) const;
    bool operator!=(const Duration &rhs) const;
    bool operator<(const Duration &rhs) const;
    bool operator<=(const Duration &rhs) const;
    bool operator>(const Duration &rhs) const;
    bool operator>=(const Duration &rhs) const;

  private:
    int64_t nanoseconds_;
  };

  class Clock
  {
  public:
    enum class ClockType
    {
      SYSTEM_TIME
    };

  private:
    ClockType type_;

  public:
    explicit Clock(ClockType type = ClockType::SYSTEM_TIME);
    Time now();
    ClockType get_clock_type() const;
  };

  class Rate
  {
  private:
    Duration period_;
    std::chrono::steady_clock::time_point next_time_;

  public:
    explicit Rate(const Duration &period);
    void sleep();
  };
} // namespace lwrcl

#endif // LWRCL_HPP_