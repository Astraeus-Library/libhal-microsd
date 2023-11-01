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

    p_settings = hal::spi::settings{
    .clock_rate = 400.0_kHz,
  };
  *m_spi->configure(p_settings);

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
  uint32_t address, std::array<hal::byte, 512> data)
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

    HAL_CHECK(hal::read(*m_spi, data));
  HAL_CHECK(m_cs->level(true));
  return data;
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
  delay(10);
  HAL_CHECK(hal::write(*m_spi, start_token));
  HAL_CHECK(hal::write(*m_spi, data));
    delay(10);
  HAL_CHECK(hal::write(*m_spi, dummy_crc_arr));
  HAL_CHECK(hal::write(*m_spi, dummy_crc_arr));
  HAL_CHECK(m_cs->level(true));

  return hal::success();
}

hal::result<std::array<hal::byte, 16>> microsd_card::read_csd_register()
{
    std::array<hal::byte, 16> csd_register = {};  // Initialize with zeros

    // Prepare CMD9 command with appropriate arguments and CRC
    std::array<hal::byte, 6> cmd9{
        Command::CMD9,
        0x00,  // Argument (usually zero for CMD9)
        0x00,
        0x00,
        0x00,
        0x95  // Placeholder CRC (replace with actual CRC if needed)
    };

    // Send CMD9 to SD card
    delay(1);
    HAL_CHECK(m_cs->level(false));
    HAL_CHECK(hal::write(*m_spi, cmd9));
    delay(1);

        // Read data into buffer until start token is found
    std::array<hal::byte, 1> m_check;
    do {
        HAL_CHECK(hal::read(*m_spi, m_check));
    } while (m_check[0] != 0xFE);

    HAL_CHECK(hal::read(*m_spi, csd_register));
    HAL_CHECK(m_cs->level(true));

    return csd_register;
}

hal::result<uint32_t> microsd_card::read_c_size()
{
    auto csd_register = HAL_CHECK(read_csd_register());
    uint32_t c_size = (static_cast<uint32_t>(csd_register[6]) << 24) | 
                      (static_cast<uint32_t>(csd_register[7]) << 16) | 
                      (static_cast<uint32_t>(csd_register[8]) << 8)  |
                      (static_cast<uint32_t>(csd_register[9]));  // Added this line

    c_size = c_size & 0x3FFFFF;  // Adjust this mask as necessary to fit your data
    return c_size;
}

hal::result<float> microsd_card::GetCapacity()
{
    auto c_size = HAL_CHECK(read_c_size());

    float capacity = static_cast<float>((c_size + 1) * 512);  // in KBytes
    capacity /= 1024;  // Convert to MBytes
    capacity /= 1024;  // Convert to GBytes

    return capacity;  // This returns the capacity in GBytes
}

// ------------------------------- High Level Functions -------------------------------

// Add read, write, and erase functions here






}  // namespace hal::microsd
