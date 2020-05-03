#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <string>

#include "osi_layers/physical.h"
#include "crc.h"
#include "osi_layers/network.h"

#include "utilities.h"
#include "test_unit_base.h"

using ::testing::Return;

void generic_transmit_byte(const uint8_t payload) { static_cast<void>(payload); }
uint8_t generic_receive_byte() { return 0xFF; }

template <typename PhysicalLayer = Physical, typename CRCFunctions = CRC>
class MockDatalink : public Datalink<PhysicalLayer, CRCFunctions>
{
public:
    MockDatalink(PhysicalLayer &&physical) : Datalink<>{std::forward<PhysicalLayer>(physical)} {}
    MOCK_METHOD(CommunicationStatus, TransmitWithAcknowledge, (const Payload &, uint8_t retransmit_count), (const override));
    MOCK_METHOD(Payload, ReceiveWithAcknowledge, (), (const override));
};

class FixtureWithMock : public ::testing::Test
{

public:
    virtual void SetUp() override
    {
        payloadified_data_with_to_from_crc_ = Payload{data_.c_str(), static_cast<uint8_t>(data_.length())};
        payloadified_data_with_to_from_crc_.data[payloadified_data_with_to_from_crc_.size++] = kOwnId;
        payloadified_data_with_to_from_crc_.data[payloadified_data_with_to_from_crc_.size++] = kSourceId;
        payloadified_data_with_to_from_crc_ = crc_.append_crc_to_payload(payloadified_data_with_to_from_crc_);
    }

protected:
    MockDatalink<> datalink_{Physical{generic_transmit_byte, generic_receive_byte}};
    Network<> sut_{kOwnId, datalink_};

    Payload payloadified_data_with_to_from_crc_;
    CRC crc_;
    std::string data_{"abcd"};
};

TEST_F(FixtureWithMock, NetworkTransmitToWorks_WhenTypical)
{

    ON_CALL(datalink_, TransmitWithAcknowledge).WillByDefault(Return(CommunicationStatus::Acknowledge));

    auto actual = sut_.TransmitTo(payloadified_data_with_to_from_crc_, kDestinationId);

    EXPECT_EQ(actual, CommunicationStatus::Acknowledge);
}

class Fixture : public UnitBase
{
public:
    static void generic_transmit_byte(const uint8_t payload)
    {

        if (transmitted_.size == kPosSourceIdInPayload + kSizeofLength && payload == kOwnId)
        {
            to_field_matches_own_ = true;
        }
        transmitted_.data[transmitted_.size++] = payload;
    }

    static uint8_t generic_receive_byte()
    {
        if (to_field_matches_own_)
        {
            return lookup_map_[call_count_++];
        }
        return 0xFF;
    }

protected:
    virtual void SetUp() override
    {

        UnitBase::SetUp();
        call_count_ = 0;
        to_field_matches_own_ = false;
        lookup_map_ = {
            {0, payload_size_byte_count + kCRCSize},
            {1, payloadified_acknowledge_.data[0]},
            {2, payloadified_acknowledge_.data[1]},
            {3, payloadified_acknowledge_.data[2]},

            {4, payload_size_byte_count + kCRCSize},
            {5, payloadified_negative_acknowledge_.data[0]},
            {6, payloadified_negative_acknowledge_.data[1]},
            {7, payloadified_negative_acknowledge_.data[2]},
        };
    }
    Network<> sut_{kOwnId, Datalink<>{Physical{generic_transmit_byte, generic_receive_byte}}};
    static uint8_t call_count_;
    static bool to_field_matches_own_;
};

uint8_t Fixture::call_count_;
bool Fixture::to_field_matches_own_;

TEST_F(Fixture, NetworkTransmitToWorks_WhenTypical)
{

    sut_.TransmitTo(payloadified_data_, kDestinationId);

    ASSERT_EQ(transmitted_.data[kPosDestinationIdInPayload + kSizeofLength], kDestinationId);
    ASSERT_EQ(transmitted_.data[kPosSourceIdInPayload + kSizeofLength], kOwnId);
}

TEST_F(Fixture, NetworkTransmitToResultAcknowledge_WhenTypical)
{
    CommunicationStatus result = sut_.TransmitTo(payloadified_data_, kDestinationId);

    ASSERT_EQ(result, CommunicationStatus::Acknowledge);
}
