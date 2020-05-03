#pragma once

#include <cstdint>
#include "config.h"
#include "payload.h"

using TVoidUint8 = void (*)(const uint8_t);
using TUint8Void = uint8_t (*)();

class Physical
{
public:
  Physical(TVoidUint8 on_transmit, TUint8Void on_receive) : on_transmit_byte_{on_transmit}, on_receive_byte_{on_receive} {}
  Physical(Physical &&other)
  {
    on_transmit_byte_ = other.on_transmit_byte_;
    on_receive_byte_ = other.on_receive_byte_;
  }
  Physical(const Physical &other)
  {
    on_transmit_byte_ = other.on_transmit_byte_;
    on_receive_byte_ = other.on_receive_byte_;
  }

  void Transmit(const Payload &payload) const;
  Payload Receive() const;

  virtual ~Physical() = default;
  Physical &operator=(const Physical &other) = delete;
  Physical &operator=(Physical &&other) = delete;

private:
  TVoidUint8 on_transmit_byte_;
  TUint8Void on_receive_byte_;
};