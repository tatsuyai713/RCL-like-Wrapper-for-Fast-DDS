#ifndef ROSTYPEIMAGEPUBLSUBMONO_H_
#define ROSTYPEIMAGEPUBLSUBMONO_H_

#include "lwrcl.hpp"
#include "sensor_msgs/msg/Image.h"
#include "sensor_msgs/msg/ImagePubSubTypes.h"
#include <functional>
#include <memory>
#include <string>
#include <yaml-cpp/yaml.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


using namespace lwrcl;

#ifndef SENSOR_MSGS_MSG_IMAGETYPE_HPP
#define SENSOR_MSGS_MSG_IMAGETYPE_HPP
FAST_DDS_DATA_TYPE(sensor_msgs, msg, Image)
#endif // SENSOR_MSGS_MSG_IMAGETYPE_HPP

class ROSTypeImagePubSubMono : public Node {
public:
    ROSTypeImagePubSubMono(uint16_t domain_number);
    virtual ~ROSTypeImagePubSubMono();

    // Override init and run methods from Node
    bool init(const std::string& config_file_path);
    // void run() override;

    // Callback function to subscribe data
    void callbackSubscribe(sensor_msgs::msg::Image* message);

private:
    std::string publish_topic_name_;
    std::string subscribe_topic_name_;
    uint16_t interval_ms_;
    Publisher<sensor_msgs::msg::Image>* publisher_ptr_;
    Subscriber<sensor_msgs::msg::Image>* subscriber_ptr_;
    Timer<std::chrono::milliseconds>* timer_ptr_;
    sensor_msgs::msg::ImageType pub_message_type_;
    sensor_msgs::msg::ImageType sub_message_type_;
    int counter_;
    std::shared_ptr<sensor_msgs::msg::Image> gray_msg_;
};

#endif /* ROSTYPEIMAGEPUBLSUBMONO_H_ */
