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
#include "libhal-microsd/ff.h"

// Global microSD card instance
hal::microsd::microsd_card sd;

hal::status application(hardware_map& p_map) {
    using namespace std::chrono_literals;
    using namespace hal::literals;

    auto& console = *p_map.console;
    auto& clock = *p_map.clock;

    // Initialize SPI and Chip Select
    auto spi2 = HAL_CHECK(hal::lpc40::spi::get(2));
    auto chip_select = HAL_CHECK(hal::lpc40::output_pin::get(1, 8));

    hal::print(console, "Starting MicroSD Application...\n");
    (void)hal::delay(clock, 200ms);

    // Create the microsd_card instance
    auto micro_sd_result = hal::microsd::microsd_card::create(spi2, chip_select);

    // Assign the created microSD card to the global instance
    sd = micro_sd_result.value();

    // Filesystem operations
    FATFS fs;   // Filesystem object
    FIL file;   // File object
    UINT written, read;
    char buffer[100] = "Hello World!";
    char readBuffer[100];

    // Mount the filesystem
    if (f_mount(&fs, "", 0) == FR_OK) {
        // Open or create a file
        if (f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
            // Write to the file
            f_write(&file, buffer, strlen(buffer), &written);
            f_close(&file);
        }

        // Open the file for reading
        if (f_open(&file, "test.txt", FA_READ) == FR_OK) {
            // Read the file
            f_read(&file, readBuffer, sizeof(readBuffer), &read);
            f_close(&file);
        }

        // Unmount the filesystem
        f_mount(NULL, "", 0);
    }

    // Your additional operations...

    return hal::success();
}
