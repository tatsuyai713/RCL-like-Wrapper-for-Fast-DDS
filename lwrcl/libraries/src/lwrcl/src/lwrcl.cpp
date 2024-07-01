#include <csignal>
#include <cstring>
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <algorithm>
#include <functional>
#include <memory>
#include <chrono>
#include <vector>
#include "lwrcl.hpp" // The main header file for the lwrcl namespace

namespace lwrcl
{
  class Node;
}

class HandlerRegistry
{
public:
  std::vector<lwrcl::Node *> nodes_;
  std::mutex registry_mutex;

  void add_node(lwrcl::Node *node)
  {
    std::lock_guard<std::mutex> lock(registry_mutex);
    nodes_.push_back(node);
  }

  void remove_node(lwrcl::Node *node)
  {
    std::lock_guard<std::mutex> lock(registry_mutex);
    nodes_.erase(std::remove(nodes_.begin(), nodes_.end(), node), nodes_.end());
  }

  void notify_all()
  {
    std::lock_guard<std::mutex> lock(registry_mutex);
    for (auto *node : nodes_)
    {
      if (node)
      {
        node->stop_spin();
      }
      else
      {
        std::cerr << "node pointer is invalid!" << std::endl;
      }
    }
  }
};

static HandlerRegistry &get_global_registry()
{
  static HandlerRegistry global_registry;
  return global_registry;
}

// Global flag to control the stopping of the application, e.g., in response to SIGINT
std::atomic_bool global_stop_flag{false};

// Function to handle SIGINT signals for graceful application termination
void lwrcl_signal_handler(int signal)
{
  if (signal == SIGINT || signal == SIGTERM)
  {
    global_stop_flag = true;
    get_global_registry().notify_all();
  }
}

// Begin namespace for the lwrcl functionality
namespace lwrcl
{
  SingleThreadedExecutor::SingleThreadedExecutor() {}

  SingleThreadedExecutor::~SingleThreadedExecutor()
  {
    stop_spin();
  }

  void SingleThreadedExecutor::add_node(Node *node)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (node != nullptr)
    {
      nodes_.push_back(node);
    }
    else
    {
      std::cerr << "Error: Node pointer is null, cannot add to executor." << std::endl;
    }
  }

  void SingleThreadedExecutor::remove_node(Node *node)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (node != nullptr)
    {
      nodes_.erase(std::remove(nodes_.begin(), nodes_.end(), node), nodes_.end());
    }
  }

  void SingleThreadedExecutor::stop_spin()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &node : nodes_)
    {
      if (node)
      {
        node->stop_spin();
      }
      else
      {
        std::cerr << "node pointer is invalid!" << std::endl;
      }
    }
  }

  void SingleThreadedExecutor::spin()
  {
    while (!global_stop_flag.load())
    {
      std::lock_guard<std::mutex> lock(mutex_);
      for (auto node : nodes_)
      {
        if (node)
        {
          node->spin_some();
        }
        else
        {
          std::cerr << "node pointer is invalid!" << std::endl;
        }
      }
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  }

  void SingleThreadedExecutor::spin_some()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto node : nodes_)
    {
      if (node)
      {
        node->spin_some();
      }
      else
      {
        std::cerr << "node pointer is invalid!" << std::endl;
      }
    }
  }

  void SingleThreadedExecutor::shutdown()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_spin();
  }

  MultiThreadedExecutor::MultiThreadedExecutor() {}

  MultiThreadedExecutor::~MultiThreadedExecutor()
  {
    stop_spin();
  }

  void MultiThreadedExecutor::add_node(Node *node)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (node != nullptr)
    {
      nodes_.push_back(node);
    }
    else
    {
      std::cerr << "Error: Node pointer is null, cannot add to executor." << std::endl;
    }
  }

  void MultiThreadedExecutor::remove_node(Node *node)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (node != nullptr)
    {
      nodes_.erase(std::remove(nodes_.begin(), nodes_.end(), node), nodes_.end());
    }
  }

  void MultiThreadedExecutor::stop_spin()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto node : nodes_)
    {
      if (node)
      {
        node->stop_spin();
      }
      else
      {
        std::cerr << "node pointer is invalid!" << std::endl;
      }
    }

    for (auto &thread : threads_)
    {
      if (thread.joinable())
      {
        thread.join();
      }
    }
    threads_.clear();
  }

  void MultiThreadedExecutor::spin()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto node : nodes_)
    {
      threads_.emplace_back([this, node]()
                            {
        if (!node)
        {
          std::cerr << "node pointer is invalid!" << std::endl;
        }
        else
        {
          node->spin();
        } });
    }

    for (auto &thread : threads_)
    {
      if (thread.joinable())
      {
        thread.join();
      }
    }
  }

  void MultiThreadedExecutor::spin_some()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto node : nodes_)
    {
      if (node)
      {
        node->spin_some();
      }
      else
      {
        std::cerr << "node pointer is invalid!" << std::endl;
      }
    }
  }

  void MultiThreadedExecutor::shutdown()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_spin();
  }

  Time::Time() : nanoseconds_(0) {}
  Time::Time(int64_t nanoseconds) : nanoseconds_(nanoseconds) {}
  Time::Time(int32_t seconds, uint32_t nanoseconds) : nanoseconds_(static_cast<int64_t>(seconds) * 1000000000 + nanoseconds) {}
  int64_t Time::nanoseconds() const { return nanoseconds_; }
  double Time::seconds() const { return static_cast<double>(nanoseconds_) / 1e9; }
  Time Time::operator+(const Duration &rhs) const { return Time(nanoseconds_ + rhs.nanoseconds()); }
  Time Time::operator-(const Duration &rhs) const { return Time(nanoseconds_ - rhs.nanoseconds()); }
  Duration Time::operator-(const Time &rhs) const { return Duration(nanoseconds_ - rhs.nanoseconds_); }
  bool Time::operator==(const Time &rhs) const { return nanoseconds_ == rhs.nanoseconds_; }
  bool Time::operator!=(const Time &rhs) const { return !(*this == rhs); }
  bool Time::operator<(const Time &rhs) const { return nanoseconds_ < rhs.nanoseconds_; }
  bool Time::operator<=(const Time &rhs) const { return nanoseconds_ <= rhs.nanoseconds_; }
  bool Time::operator>(const Time &rhs) const { return nanoseconds_ > rhs.nanoseconds_; }
  bool Time::operator>=(const Time &rhs) const { return nanoseconds_ >= rhs.nanoseconds_; }

  Duration::Duration() : nanoseconds_(0) {}
  Duration::Duration(int64_t nanoseconds) : nanoseconds_(nanoseconds) {}
  Duration::Duration(int32_t seconds, uint32_t nanoseconds) : nanoseconds_(static_cast<int64_t>(seconds) * 1000000000 + nanoseconds) {}
  int64_t Duration::nanoseconds() const { return nanoseconds_; }
  double Duration::seconds() const { return static_cast<double>(nanoseconds_) / 1e9; }
  Duration Duration::operator+(const Duration &rhs) const { return Duration(nanoseconds_ + rhs.nanoseconds()); }
  Duration Duration::operator-(const Duration &rhs) const { return Duration(nanoseconds_ - rhs.nanoseconds()); }
  bool Duration::operator==(const Duration &rhs) const { return nanoseconds_ == rhs.nanoseconds_; }
  bool Duration::operator!=(const Duration &rhs) const { return !(*this == rhs); }
  bool Duration::operator<(const Duration &rhs) const { return nanoseconds_ < rhs.nanoseconds_; }
  bool Duration::operator<=(const Duration &rhs) const { return nanoseconds_ <= rhs.nanoseconds_; }
  bool Duration::operator>(const Duration &rhs) const { return nanoseconds_ > rhs.nanoseconds_; }
  bool Duration::operator>=(const Duration &rhs) const { return nanoseconds_ >= rhs.nanoseconds_; }

  // Clock implementation
  Clock::Clock(ClockType type) : type_(type) {}
  Time Clock::now()
  {
    switch (type_)
    {
    case ClockType::SYSTEM_TIME:
      return Time(std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count());
    default:
      throw std::runtime_error("Unsupported clock type.");
    }
  }
  Clock::ClockType Clock::get_clock_type() const { return type_; }

  // Rate implementation
  Rate::Rate(const Duration &period) : period_(period), next_time_(std::chrono::steady_clock::now() + std::chrono::nanoseconds(period.nanoseconds())) {}
  void Rate::sleep()
  {
    auto now = std::chrono::steady_clock::now();
    if (now >= next_time_)
    {
      auto periods_missed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - next_time_) / std::chrono::nanoseconds(period_.nanoseconds()) + 1;
      next_time_ += periods_missed * std::chrono::nanoseconds(period_.nanoseconds());
    }
    std::this_thread::sleep_until(next_time_);
    next_time_ += std::chrono::nanoseconds(period_.nanoseconds());
  }

  Node::Node(int domain_id) : clock_(std::make_unique<Clock>())
  {
    dds::DomainParticipantQos participant_qos = dds::PARTICIPANT_QOS_DEFAULT;

    // Create a descriptor for the new transport.
    auto udp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
    udp_transport->sendBufferSize = 4194304;
    udp_transport->receiveBufferSize = 4194304;
    udp_transport->non_blocking_send = true;

    // Link the Transport Layer to the Participant.
    participant_qos.transport().user_transports.push_back(udp_transport);

    // Increase the sending buffer size
    participant_qos.transport().send_socket_buffer_size = 4194304;
    // Increase the receiving buffer size
    participant_qos.transport().listen_socket_buffer_size = 4194304;

    // eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Info);

    auto participant_factory = eprosima::fastdds::dds::DomainParticipantFactory::get_instance();
    participant_factory->load_XML_profiles_file("/opt/fast-dds/fastdds.xml");

    participant_ = std::shared_ptr<eprosima::fastdds::dds::DomainParticipant>(
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domain_id, participant_qos),
    DomainParticipantDeleter());
    if (!participant_)
    {
      throw std::runtime_error("Failed to create domain participant");
    }

    get_global_registry().add_node(this);
  }

  Node::Node(std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> participant) : clock_(std::make_unique<Clock>()), participant_(participant)
  {
    if (!participant_)
    {
      throw std::runtime_error("Failed to create domain participant");
    }

    get_global_registry().add_node(this);
  }

  Node::~Node()
  {
    publisher_list_.clear();
    subscription_list_.clear();
    timer_list_.clear();
  }

  std::shared_ptr<eprosima::fastdds::dds::DomainParticipant> Node::get_participant() const
  {
    return participant_;
  }

  void Node::spin()
  {
    while (!channel_.is_closed() && !global_stop_flag.load())
    {
      ChannelCallback *callback;
      while (channel_.consume(callback))
      {
        if (callback)
        {
          callback->invoke();
        }
      }
    }
    channel_.close();
  }

  void Node::spin_some()
  {
    bool event_processed = false;

    do
    {
      event_processed = false;

      ChannelCallback *callback;
      while (channel_.consume_nowait(callback))
      {
        callback->invoke();
        event_processed = true;
      }
    } while (event_processed);
  }

  void Node::stop_spin()
  {
    channel_.close();
  }

  void Node::shutdown()
  {
    channel_.close();
  }

  Clock* Node::get_clock()
  {
    return clock_.get();
  }

  bool ok()
  {
    return !global_stop_flag.load();
  }

} // namespace lwrcl
