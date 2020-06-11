#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <string>

#include <containers/static_string.h>

#define private public // For testing purposes only

#include "osi_layers/physical.h"
#include "crc.h"
#include "osi_layers/session.h"

#include "utilities.h"
#include "unit/test_unit_base.h"

class Fixture : public UnitBase
{
public:
    static void generic_transmit_byte(const uint8_t payload)
    {
        static_cast<void>(payload);
    }

    static uint8_t generic_receive_byte()
    {
        static uint8_t call_count{0};

        if (call_count++ > 0)
        {
            return payloadified_data_with_segment_to_from_crc_.data[call_count - kSizeofLength - 1];
        }
        else
        {
            return payloadified_data_with_segment_to_from_crc_.size;
        }
    }

protected:
    virtual void SetUp() override
    {
        data_ = std::string{"User"} + " " + "Pass";
        UnitBase::SetUp();
    }
    Session<> sut_{Transport<>{Network<>{kOwnId, {Datalink<>{Physical{generic_transmit_byte, generic_receive_byte}}}}}, {"User"}, {"Pass"}, kPort};
};

TEST_F(Fixture, LoginSuccess_WhenTypical)
{
    TString expected = "OK \xBE\xEF";
    sut_.transport_.network_.datalink_.retransmit_count_ = kRetransmitCountInCaseOfNoAcknowledge;

    auto actual = sut_.Receive(kSourceId, kPort);

    ASSERT_EQ(actual, expected);
}
