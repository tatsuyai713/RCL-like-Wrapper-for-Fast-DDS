#ifndef RCL_LIKE_WRAPPER_PUBLISHER_HPP_
#define RCL_LIKE_WRAPPER_PUBLISHER_HPP_

#include <atomic>
#include <string>

#include "fast_dds_header.hpp"
namespace lwrcl
{

  class PublisherListener : public dds::DataWriterListener
  {
  public:
    void on_publication_matched(dds::DataWriter *, const dds::PublicationMatchedStatus &status) override
    {
      count = status.current_count;
    }

    std::atomic<int32_t> count{0};
  };

  class IPublisher
  {
  public:
    virtual ~IPublisher() = default;
    virtual int32_t get_subscriber_count() = 0;
  };
  template <typename T>
  class Publisher : public IPublisher
  {
  public:
    Publisher(dds::DomainParticipant *participant, MessageType *message_type, const std::string &topic,
              const dds::TopicQos &qos)
        : participant_(participant), message_type_(message_type), topic_(nullptr), publisher_(nullptr), writer_(nullptr)
    {
      if (message_type_->get_type_support().register_type(participant_) != ReturnCode_t::RETCODE_OK)
      {
        throw std::runtime_error("Failed to register message type");
      }
      topic_ = participant_->create_topic(topic, message_type_->get_type_support().get_type_name(), qos);
      if (!topic_)
      {
        throw std::runtime_error("Failed to create topic");
      }
      publisher_ = participant_->create_publisher(dds::PUBLISHER_QOS_DEFAULT);
      if (!publisher_)
      {
        participant_->delete_topic(topic_); // Cleanup on failure
        throw std::runtime_error("Failed to create publisher");
      }
      dds::DataWriterQos writer_qos = dds::DATAWRITER_QOS_DEFAULT;
      writer_ = publisher_->create_datawriter(topic_, writer_qos, &listener_);
      if (!writer_)
      {
        participant_->delete_publisher(publisher_); // Cleanup on failure
        participant_->delete_topic(topic_);         // Cleanup on failure
        throw std::runtime_error("Failed to create datawriter");
      }
    }

    ~Publisher()
    {
      if (writer_ != nullptr)
      {
        publisher_->delete_datawriter(writer_);
      }
      if (publisher_ != nullptr)
      {
        participant_->delete_publisher(publisher_);
      }
      if (topic_ != nullptr)
      {
        participant_->delete_topic(topic_);
      }
    }

    void publish(T *message) const
    {
      writer_->write(message);
    }

    int32_t get_subscriber_count()
    {
      return listener_.count;
    }

  private:
    dds::DomainParticipant *participant_;
    MessageType *message_type_;
    dds::Topic *topic_;
    dds::Publisher *publisher_;
    dds::DataWriter *writer_;
    PublisherListener listener_;
  };
} // namespace lwrcl

#endif // RCL_LIKE_WRAPPER_PUBLISHER_HPP_