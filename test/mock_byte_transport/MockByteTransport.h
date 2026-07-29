#pragma once

#include <byte_transport/Transport.h>

namespace pendarlab::app::mavlink_hub::test
{
  class MockByteTransport : public pendarlab::lib::comm::byte_transport::Transport
  {
    public:
    virtual int read(unsigned char* buf, unsigned int buf_size) override;
    virtual int write(const unsigned char* buf, unsigned int length) override; 
  };
} // namespace pendarlab::app::mavlink_hub::test
