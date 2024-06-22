#include "ROSTypeImagePubSubMono.hpp"
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <string>

SIGNAL_HANDLER_DEFINE()

int main(int argc, char **argv) 
{
    SIGNAL_HANDLER_INIT()
    std::cout << "Starting ROSTypeImagePubSubExecutorTest" << std::endl;

    // Buffer to hold the path to the current executable
    char buf[PATH_MAX] = {0};
    std::string exePath, configPath;
    std::string configPath1, configPath2;

    // Attempt to read the symbolic link '/proc/self/exe' to find the path of the current executable
    ssize_t count = readlink("/proc/self/exe", buf, sizeof(buf));
    if (count != -1) {
        exePath = std::string(buf, count); // Convert to string and assign
        size_t pos = exePath.rfind('/'); // Find the last occurrence of '/'
        if (pos != std::string::npos) {
            configPath = exePath.substr(0, pos + 1); // Extract the path without executable name
        }
    } else {
        std::cerr << "Failed to determine the path of the executable." << std::endl;
        return 1;
    }

    // Initialize Executor
    MultiThreadedExecutor executor;

    // Initialize and run the ROS-like node
    ROSTypeImagePubSubMono rcl_like_node1(0);
    configPath1 = configPath + "config/config1.yaml"; // Append the relative path of the config file
    std::cout << "Using config file at: " << configPath1 << std::endl;
    
    if (rcl_like_node1.init(configPath1)) {
        executor.add_node(&rcl_like_node1);
    } else {
        std::cerr << "Failed to initialize the ROSTypeImagePubSubMono." << std::endl;
        return 1;
    }
    ROSTypeImagePubSubMono rcl_like_node2(0);
    configPath2 = configPath + "config/config2.yaml"; // Append the relative path of the config file
    std::cout << "Using config file at: " << configPath2 << std::endl;
    
    if (rcl_like_node2.init(configPath2)) {
        executor.add_node(&rcl_like_node2);
    } else {
        std::cerr << "Failed to initialize the ROSTypeImagePubSubMono." << std::endl;
        return 1;
    }

    // Wait for the ROS-like node to finish
    executor.spin();

    return 0;
}
