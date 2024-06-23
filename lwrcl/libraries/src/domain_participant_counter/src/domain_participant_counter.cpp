#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp> // 追加
#include <fastrtps/rtps/common/Guid.h>
#include <iostream>
#include <unordered_set>
#include <mutex>
#include <thread>

using namespace eprosima::fastdds::dds;

// カスタムハッシュ関数
struct GUIDHash
{
    std::size_t operator()(const eprosima::fastrtps::rtps::GUID_t& guid) const noexcept
    {
        return std::hash<std::string>()(std::string(reinterpret_cast<const char*>(guid.guidPrefix.value), 12) +
                                        std::string(reinterpret_cast<const char*>(guid.entityId.value), 4));
    }
};

// GUIDの比較関数
struct GUIDEqual
{
    bool operator()(const eprosima::fastrtps::rtps::GUID_t& lhs, const eprosima::fastrtps::rtps::GUID_t& rhs) const noexcept
    {
        return std::memcmp(&lhs, &rhs, sizeof(eprosima::fastrtps::rtps::GUID_t)) == 0;
    }
};

class CustomParticipantListener : public DomainParticipantListener
{
public:
    void on_participant_discovery(
            DomainParticipant* participant,
            eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
        {
            participants_.insert(info.info.m_guid);
            std::cout << "Participant discovered: " << info.info.m_guid << std::endl;
        }
        else if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
        {
            participants_.erase(info.info.m_guid);
            std::cout << "Participant removed: " << info.info.m_guid << std::endl;
        }

        std::cout << "Number of participants: " << participants_.size() << std::endl;
    }

private:
    std::unordered_set<eprosima::fastrtps::rtps::GUID_t, GUIDHash, GUIDEqual> participants_;
    std::mutex mutex_;
};

int main(int argc, char** argv)
{
    DomainParticipantQos participant_qos;
    participant_qos.name("MyParticipant");

    CustomParticipantListener listener;
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, participant_qos, &listener, StatusMask::all());

    if (participant == nullptr)
    {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return 1;
    }

    // Run the application and process discovery events
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    DomainParticipantFactory::get_instance()->delete_participant(participant);
    return 0;
}
