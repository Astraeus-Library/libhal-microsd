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

#include <libhal-lpc40/output_pin.hpp>
#include <libhal-lpc40/spi.hpp>
#include <libhal-util/serial.hpp>
#include <libhal-util/spi.hpp>
#include <libhal-util/steady_clock.hpp>

#include "../hardware_map.hpp"
#include <libhal-microsd/microsd.hpp>

hal::status application(hardware_map& p_map)
{
  using namespace std::chrono_literals;
  using namespace hal::literals;

  auto& console = *p_map.console;
  auto& clock = *p_map.clock;

  auto spi2 = HAL_CHECK(hal::lpc40::spi::get(2));
  auto chip_select = HAL_CHECK(hal::lpc40::output_pin::get(1, 8));

  hal::print(console, "Starting MicroSD Application...\n");
  (void)hal::delay(clock, 200ms);
  auto micro_sd = HAL_CHECK(hal::microsd::microsd_card::create(spi2, chip_select));
  (void)hal::delay(clock, 200ms);
  std::array<unsigned char, 512> write_data = {0x69, 0x45, 0x22, 0x55};
  std::array<unsigned char, 512> read_buffer;

while(true) {
    HAL_CHECK(micro_sd.write_block(0x00000000, write_data));
    // print thr data being written
    (void)hal::delay(clock, 200ms);

    auto block = HAL_CHECK(micro_sd.read_block(0x00000000));

    // Print first N bytes of read data for verification
    hal::print(console, "Read Data:\n");
    for (size_t i = 0; i < 512; i++) {  // Just an example, adjust '10' as per your requirements.
        hal::print<128>(console, "Data[%d]: %x\n", i, block[i]);
    }

    (void)hal::delay(clock, 500ms);
}
  return hal::success();
}