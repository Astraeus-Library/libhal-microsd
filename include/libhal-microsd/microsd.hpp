// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <libhal-util/output_pin.hpp>
#include <libhal-util/spi.hpp>
#include <libhal/output_pin.hpp>
#include <libhal/spi.hpp>

namespace hal::microsd {
class microsd_card
{
public:
  static constexpr hal::byte kCommandBase = 0x40;
  static constexpr hal::byte DUMMY_CRC = 0x95;

  enum Command
  {
    kGarbage = 0xFF,            // "SendGarbage" command; instructs
                                // SendCommand() to send 0xFFFFFFFF as data
                                // and even set the checksum as 0xFF
    CMD0 = kCommandBase | 0,    // CMD0: reset the sd card (force it to go
                                // to the idle state)
    CMD1 = kCommandBase | 1,    // CMD1: starts an initiation of the card
    CMD8 = kCommandBase | 8,    // CMD8: request the sd card's support of
                                // the provided host's voltage ranges
    CMD9 = kCommandBase | 9,    // CMD9: request the sd card's CSD
                                // (card-specific data) register
    CMD12 = kCommandBase | 12,  // CMD12: terminates a multi-block read or
                                // write operation
    CMD13 = kCommandBase | 13,  // CMD13: get status register
    CMD16 = kCommandBase | 16,  // CMD16: change block length (only
                                // effective in SDSC cards; SDHC/SDXC
                                // cards are locked to 512-byte blocks)
    CMD17 = kCommandBase | 17,  // CMD17: read a single block of data
    CMD18 = kCommandBase | 18,  // CMD18: read many blocks of data until
                                // a "CMD12" frame is sent
    CMD24 = kCommandBase | 24,  // CMD24: write a single block of data
    CMD25 = kCommandBase | 25,  // CMD25: write many blocks of data until
                                // a "CMD12" frame is sent
    CMD32 = kCommandBase | 32,  // CMD32: set address of the start block
                                // for deletion
    CMD33 = kCommandBase | 33,  // CMD33: set address of the end block for
                                // deletion
    CMD38 = kCommandBase | 38,  // CMD38: begin deletion from the block
                                // range specified by the vector
                                // [DEL_FROM : DEL_TO]
    CMD55 = kCommandBase | 55,  // CMD55: signals the start of an
                                // application-specific command
    CMD58 = kCommandBase | 58,  // CMD58: request data from the
                                // operational conditions register
    CMD41 = kCommandBase | 41   // CMD41: application-specific version of
                                // CMD1 (must precede with CMD55)
  };

  [[nodiscard]] static result<microsd_card> create(hal::spi& p_spi,
                                                   hal::output_pin& p_cs);
  hal::status init();
  hal::result<std::array<hal::byte, 512>> read_block(
    uint32_t address,
    std::array<hal::byte, 512> data);
  hal::status write_block(uint32_t address, std::array<hal::byte, 512> data);

  hal::result<uint32_t> read_c_size();
  hal::result<float> GetCapacity();
  hal::result<std::array<hal::byte, 16>> read_csd_register();

private:
  explicit microsd_card(hal::spi& p_spi, hal::output_pin& p_cs)
    : m_spi(&p_spi)
    , m_cs(&p_cs)
  {
  }

  hal::status delay(int p_cycles);

  hal::spi* m_spi;
  hal::output_pin* m_cs;
  std::array<hal::byte, 1> m_wait{ 0xFF };
};
}  // namespace hal::microsd
