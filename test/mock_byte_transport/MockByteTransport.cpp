#include "mock_byte_transport/MockByteTransport.h"

#include <mavlink/common/mavlink.h>
#include <chrono>
#include <thread>

namespace pendarlab::app::mavlink_hub::test
{
  int MockByteTransport::read(unsigned char* buf, unsigned int buf_size)
  {
    auto time_to_wake_up = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
    std::this_thread::sleep_until(time_to_wake_up);

    return buf_size;
  }

  int MockByteTransport::write(const unsigned char* buf, unsigned int length)
  {
    return length;
  }
} // namespace pendarlab::app::mavlink_hub::test
