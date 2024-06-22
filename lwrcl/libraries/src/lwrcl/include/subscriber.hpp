#ifndef LWRCL_SUBSCRIBER_HPP_
#define LWRCL_SUBSCRIBER_HPP_

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

#include "fast_dds_header.hpp"
#include "channel.hpp"

namespace lwrcl
{
  template <typename T>
  class SubscriptionCallback : public ChannelCallback
  {
  public:
    SubscriptionCallback(std::function<void(T *)> callback_function, std::vector<std::shared_ptr<T>> *message_buffer)
        : callback_function_(callback_function), message_buffer_(message_buffer) {}

    ~SubscriptionCallback() = default;

    void invoke()
    {
      try
      {
        if (!message_buffer_->empty())
        {
          callback_function_(message_buffer_->front().get());
          message_buffer_->erase(message_buffer_->begin());
        }
        else
        {
          std::cerr << "Error: Vector is empty" << std::endl;
        }
      }
      catch (const std::exception &e)
      {
        std::cerr << "Exception during callback invocation: " << e.what() << std::endl;
      }
      catch (...)
      {
        std::cerr << "Unknown exception during callback invocation." << std::endl;
      }
    }

  private:
    std::function<void(T *)> callback_function_;
    std::vector<std::shared_ptr<T>> *message_buffer_;
  };

  template <typename T>
  class SubscriberListener : public dds::DataReaderListener
  {
  public:
    virtual ~SubscriberListener()
    {
    }

    void on_subscription_matched(dds::DataReader *, const dds::SubscriptionMatchedStatus &status) override
    {
      count = status.current_count;
    }

    void on_data_available(dds::DataReader *reader) override
    {
      T temp_instance;
      if (reader->take_next_sample(&temp_instance, &sample_info_) == ReturnCode_t::RETCODE_OK && sample_info_.valid_data)
      {
        auto data_ptr = std::make_shared<T>(temp_instance);
        message_ptr_buffer_.emplace_back(data_ptr);
        channel_.produce(subscription_callback_.get());
      }
    }

    SubscriberListener(MessageType *message_type, std::function<void(T *)> callback_function, Channel<ChannelCallback *> &channel)
        : message_type_(message_type), callback_function_(callback_function), channel_(channel)
    {
      subscription_callback_ = std::make_unique<SubscriptionCallback<T>>(callback_function_, &message_ptr_buffer_);
    }
    std::atomic<int32_t> count{0};

  private:
    MessageType *message_type_;
    std::function<void(T *)> callback_function_;
    Channel<ChannelCallback *> &channel_;
    std::vector<std::shared_ptr<T>> message_ptr_buffer_;
    std::unique_ptr<SubscriptionCallback<T>> subscription_callback_;
    dds::SampleInfo sample_info_;
  };

  class ISubscriber
  {
  public:
    virtual ~ISubscriber() = default;
    virtual int32_t get_publisher_count() = 0;
  };

  template <typename T>
  class Subscriber : public ISubscriber
  {
  public:
    Subscriber(dds::DomainParticipant *participant, MessageType *message_type, const std::string &topic,
               const dds::TopicQos &qos, std::function<void(T *)> callback_function,
               Channel<ChannelCallback *> &channel)
        : participant_(participant),
          listener_(message_type, callback_function, channel)
    {
      if (message_type->get_type_support().register_type(participant_) != ReturnCode_t::RETCODE_OK)
      {
        throw std::runtime_error("Failed to register message type");
      }
      
      dds::Topic* retrieved_topic = dynamic_cast<eprosima::fastdds::dds::Topic*>(participant->lookup_topicdescription(topic));
      if (retrieved_topic == nullptr)
      {
        topic_ = participant_->create_topic(topic, message_type->get_type_support().get_type_name(), qos);
        if (!topic_)
        {
          throw std::runtime_error("Failed to create topic");
        }
      }
      else
      {
        topic_ = retrieved_topic;
      }

      subscriber_ = participant_->create_subscriber(dds::SUBSCRIBER_QOS_DEFAULT);
      if (!subscriber_)
      {
        participant_->delete_topic(topic_);
        throw std::runtime_error("Failed to create subscriber");
      }
      dds::DataReaderQos reader_qos = dds::DATAREADER_QOS_DEFAULT;
      reader_qos.endpoint().history_memory_policy = rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
      reader_qos.history().depth = 10;
      reader_qos.reliability().kind = dds::RELIABLE_RELIABILITY_QOS;
      // reader_qos.durability().kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
      reader_qos.data_sharing().automatic();
      reader_ = subscriber_->create_datareader(topic_, reader_qos, &listener_);
      if (!reader_)
      {
        participant_->delete_subscriber(subscriber_);
        participant_->delete_topic(topic_);
        throw std::runtime_error("Failed to create datareader");
      }
    }

    ~Subscriber()
    {
      if (reader_ != nullptr)
      {
        subscriber_->delete_datareader(reader_);
      }
      if (subscriber_ != nullptr)
      {
        participant_->delete_subscriber(subscriber_);
      }
      if (topic_ != nullptr)
      {
        participant_->delete_topic(topic_);
      }
    }

    int32_t get_publisher_count()
    {
      return listener_.count.load();
    }

  private:
    dds::DomainParticipant *participant_;
    SubscriberListener<T> listener_;
    dds::Topic *topic_;
    dds::Subscriber *subscriber_;
    dds::DataReader *reader_;
  };

} // namespace lwrcl

#endif // LWRCL_SUBSCRIBER_HPP_