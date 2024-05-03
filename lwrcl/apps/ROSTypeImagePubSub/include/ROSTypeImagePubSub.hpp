#ifndef ROSTYPEIMAGEPUBLSUB_H_
#define ROSTYPEIMAGEPUBLSUB_H_

#include "lwrcl.hpp"
#include "sensor_msgs/msg/Image.h"
#include "sensor_msgs/msg/ImagePubSubTypes.h"
#include <functional>
#include <memory>
#include <string>
#include <yaml-cpp/yaml.h>

using namespace lwrcl;

#ifndef SENSOR_MSGS_MSG_IMAGETYPE_HPP
#define SENSOR_MSGS_MSG_IMAGETYPE_HPP
FAST_DDS_DATA_TYPE(sensor_msgs, msg, Image)
#endif // SENSOR_MSGS_MSG_IMAGETYPE_HPP

class ROSTypeImagePubSub : public Node {
public:
    ROSTypeImagePubSub(uint16_t domain_number);
    virtual ~ROSTypeImagePubSub();

    // Override init and run methods from Node
    bool init(const std::string& config_file_path);
    // void run() override;

    // Callback function to publish data
    void callbackPublish(int test);

    // Callback function to subscribe data
    void callbackSubscribe(sensor_msgs::msg::Image* message);

private:
    std::string publish_topic_name_;
    std::string subscribe_topic_name_;
    uint16_t interval_ms_;
    Publisher<sensor_msgs::msg::Image>* publisher_ptr_;
    Subscriber<sensor_msgs::msg::Image>* subscriber_ptr_;
    Timer<std::chrono::milliseconds>* timer_ptr_;
    std::function<void()> timer_callback_;
    sensor_msgs::msg::ImageType pub_message_type_;
    sensor_msgs::msg::ImageType sub_message_type_;
    int counter_;
};

#endif /* ROSTYPEIMAGEPUBLSUB_H_ */
