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

#include "libhal-microsd/microsd.hpp"
#include "microsd_cmds.hpp"

namespace hal::microsd {

result<microsd_card> microsd_card::create(hal::spi& p_spi, hal::output_pin& p_cs)
{
  microsd_card sd(p_spi, p_cs);
  HAL_CHECK(sd.init());
  return sd;
}

hal::status microsd_card::init()
{
  HAL_CHECK(m_cs->level(false));
  // Send CMD0 to reset the card
  std::array<hal::byte, 6> reset_payload{ CMD0, 0x00, 0x00, 0x00, 0x00, 0x95 };
  HAL_CHECK(hal::write(*m_spi, reset_payload));
  HAL_CHECK(m_cs->level(true));

  HAL_CHECK(m_cs->level(false));
  // Send CMD8 to check voltage and card type
  std::array<hal::byte, 6> check_payload{ CMD8, 0x00, 0x00, 0x01, 0xAA, 0x87 };
  HAL_CHECK(hal::write(*m_spi, check_payload));
  HAL_CHECK(m_cs->level(true));

  // Initialize card with ACMD41
  HAL_CHECK(m_cs->level(false));

  std::array<hal::byte, 6> init_payload{ CMD55, 0x00, 0x00, 0x00, 0x00, 0x65 };
  HAL_CHECK(hal::write(*m_spi, init_payload));

  std::array<hal::byte, 6> init_payload2{
    ACMD41, 0x00, 0x00, 0x00, 0x00, 0x77
  };
  HAL_CHECK(hal::write(*m_spi, init_payload2));
  HAL_CHECK(m_cs->level(true));

  return hal::success();
}

// Reading a block
hal::result<std::array<hal::byte, 512>> microsd_card::read_block(uint32_t address)
{
  // Send CMD17 with the desired address
  std::array<hal::byte, 6> read_command{
    CMD17,
    static_cast<hal::byte>((address >> 24) & 0xFF),
    static_cast<hal::byte>((address >> 16) & 0xFF),
    static_cast<hal::byte>((address >> 8) & 0xFF),
    static_cast<hal::byte>(address & 0xFF),
    DUMMY_CRC
  };
  HAL_CHECK(m_cs->level(false));
  HAL_CHECK(hal::write(*m_spi, read_command));

  // Wait for the data token
  std::array<hal::byte, 1> token{};
  do {
    HAL_CHECK(hal::read(*m_spi, token));
  } while (token[0] == 0xFF);

  // Read the block data
  HAL_CHECK(hal::read(*m_spi, m_data));

  HAL_CHECK(m_cs->level(true));
  return m_data;
}

// Writing a block
hal::status microsd_card::write_block(uint32_t address, std::array<hal::byte, 512> data)
{
  std::array<hal::byte, 1> dummy_crc_arr = { DUMMY_CRC };
  // Send CMD24 with the desired address
  std::array<hal::byte, 6> write_command{
    CMD24,
    static_cast<hal::byte>((address >> 24) & 0xFF),
    static_cast<hal::byte>((address >> 16) & 0xFF),
    static_cast<hal::byte>((address >> 8) & 0xFF),
    static_cast<hal::byte>(address & 0xFF),
    DUMMY_CRC
  };

  HAL_CHECK(m_cs->level(false));
  HAL_CHECK(hal::write(*m_spi, write_command));

  // Send the data token indicating start of the block
  std::array<hal::byte, 1> start_token = { 0xFE };
  HAL_CHECK(hal::write(*m_spi, start_token));

  // Write the block data
  HAL_CHECK(hal::write(*m_spi, data));

  // Ignore card's data response token and send dummy CRC
  HAL_CHECK(hal::write(*m_spi, dummy_crc_arr));
  HAL_CHECK(hal::write(*m_spi, dummy_crc_arr));

  HAL_CHECK(m_cs->level(true));
}

}  // namespace hal::microsd
