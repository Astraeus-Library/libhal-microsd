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

result<microsd_card> microsd_card::create(hal::spi& p_spi,
                                          hal::output_pin& p_cs)
{
  microsd_card sd(p_spi, p_cs);
  HAL_CHECK(sd.init());
  return sd;
}

hal::status microsd_card::init()
{
  using namespace hal::literals;

hal::spi::settings p_settings = hal::spi::settings{
    .clock_rate = 100.0_kHz,
  };
  *m_spi->configure(p_settings);

  // Step 4 to 6: Send CMD55 and ACMD41 in a loop until the card is ready.
  std::array<hal::byte, 6> cmd0{ Command::CMD0, 0x00, 0x00, 0x00, 0x00, 0x95 };
  std::array<hal::byte, 6> cmd1{ Command::CMD1, 0x50, 0x00, 0x00, 0x00, 0x95 };
  std::array<hal::byte, 6> cmd8{ Command::CMD8, 0x00, 0x00, 0x01, 0xAA, 0x87 };
  std::array<hal::byte, 6> cmd55{Command::CMD55, 0x00, 0x00, 0x00, 0x00, 0x65};
  std::array<hal::byte, 6> cmd58{Command::CMD58, 0x00, 0x00, 0x00, 0x00, 0x00};
  std::array<hal::byte, 6> acmd41{Command::CMD41, 0x40, 0x00, 0x00, 0x00, 0x77};
  std::array<hal::byte, 6> cmd9{ Command::CMD9, 0x00, 0x00, 0x00, 0x00, 0x95 };
  

  // Step 1: Power up initialization. Provide at least 74 clock cycles with CS
  // high.
    delay(100);
  // Step 2: Send CMD0 until a valid response is received.

    HAL_CHECK(m_cs->level(false));
    HAL_CHECK(hal::write(*m_spi, cmd0));
    delay(10);
    HAL_CHECK(hal::write(*m_spi, cmd8));
    delay(10);
    HAL_CHECK(hal::write(*m_spi, cmd58));
    delay(10);
    HAL_CHECK(hal::write(*m_spi, cmd55));
    delay(10);
    HAL_CHECK(hal::write(*m_spi, acmd41));

    std::array<hal::byte, 3> crc{};
    HAL_CHECK(hal::read(*m_spi, crc));

    if (crc[0] == 0x00 && crc[1] == 0x00 && crc[2] == 0x00) {
        HAL_CHECK(hal::write(*m_spi, cmd58));
        delay(10);
    }
    else {
        HAL_CHECK(hal::write(*m_spi, cmd55));
        delay(10);
        HAL_CHECK(hal::write(*m_spi, acmd41));
        delay(10);
    }

    HAL_CHECK(hal::write(*m_spi, cmd58));
    delay(10);
    HAL_CHECK(m_cs->level(true));

//     p_settings = hal::spi::settings{
//     .clock_rate = 400.0_kHz,
//   };
//   *m_spi->configure(p_settings);

  // TODO: Check and handle the response.

  return hal::success();
}

hal::status microsd_card::delay(int p_cycles)
{
    HAL_CHECK(m_cs->level(false));
  for (int i = 0; i < p_cycles; ++i) {
    HAL_CHECK(hal::write(*m_spi, m_wait));
  }
  HAL_CHECK(m_cs->level(true));
HAL_CHECK(m_cs->level(false));

  return hal::success();
}

// Reading a block
hal::result<std::array<hal::byte, 512>> microsd_card::read_block(
  uint32_t address)
{
  using namespace hal::literals;

  // Prepare CMD17 command with the desired address
  std::array<hal::byte, 6> read_command{
    CMD17,
    static_cast<hal::byte>((address >> 24) & 0xFF),
    static_cast<hal::byte>((address >> 16) & 0xFF),
    static_cast<hal::byte>((address >> 8) & 0xFF),
    static_cast<hal::byte>(address & 0xFF),
    DUMMY_CRC
  };

    delay(10);
  HAL_CHECK(m_cs->level(false));

  // Use the write_then_read function for the entire transaction
    HAL_CHECK(hal::write(*m_spi, read_command));
    delay(10);

        // Read data into buffer until start token is found
    std::array<hal::byte, 1> m_check;
    do {
        HAL_CHECK(hal::read(*m_spi, m_check));
    } while (m_check[0] != 0xFE);

    HAL_CHECK(hal::read(*m_spi, m_data));
//   HAL_CHECK(hal::write_then_read(*m_spi, read_command, m_data));

//   // Read CRC bytes (assuming 2 CRC bytes, you can adjust as necessary)
//   std::array<hal::byte, 2> crc{};
//   HAL_CHECK(hal::read(*m_spi, crc));

  HAL_CHECK(m_cs->level(true));
  return m_data;
}

// Writing a block
hal::status microsd_card::write_block(uint32_t address,
                                      std::array<hal::byte, 512> data)
{
  using namespace hal::literals;

  std::array<hal::byte, 1> dummy_crc_arr = { DUMMY_CRC };
  std::array<hal::byte, 6> write_command{
    CMD24,
    static_cast<hal::byte>((address >> 24) & 0xFF),
    static_cast<hal::byte>((address >> 16) & 0xFF),
    static_cast<hal::byte>((address >> 8) & 0xFF),
    static_cast<hal::byte>(address & 0xFF),
    DUMMY_CRC
  };

  std::array<hal::byte, 1> start_token = { 0xFE };

  HAL_CHECK(m_cs->level(false));
  HAL_CHECK(hal::write(*m_spi, write_command));
  delay(1);
  HAL_CHECK(hal::write(*m_spi, start_token));
  HAL_CHECK(hal::write(*m_spi, data));
    delay(10);
  HAL_CHECK(hal::write(*m_spi, dummy_crc_arr));
  HAL_CHECK(hal::write(*m_spi, dummy_crc_arr));

  // Deactivate chip select
  HAL_CHECK(m_cs->level(true));

  return hal::success();
}

}  // namespace hal::microsd
