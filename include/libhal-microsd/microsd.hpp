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
  [[nodiscard]] static result<microsd_card> create(hal::spi& p_spi,
                                              hal::output_pin& p_cs);
  hal::status init();
  hal::result<std::array<hal::byte, 512>> read_block(uint32_t address);
  hal::status write_block(uint32_t address, std::array<hal::byte, 512> data);

private:
  explicit microsd_card(hal::spi& p_spi, hal::output_pin& p_cs)
    : m_spi(&p_spi)
    , m_cs(&p_cs)
  {
  }

  hal::spi* m_spi;
  hal::output_pin* m_cs;
  std::array<hal::byte, 512> m_data{};
};
}  // namespace hal::microsd
