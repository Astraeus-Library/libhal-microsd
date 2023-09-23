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

#include <libhal-util/bit.hpp>
#include <libhal/units.hpp>

namespace hal::microsd {

static constexpr hal::byte CMD0 = 0x40;
static constexpr hal::byte CMD8 = 0x48;
static constexpr hal::byte CMD55 = 0x77;
static constexpr hal::byte CMD17 = 0x51; 
static constexpr hal::byte CMD24 = 0x58; 
static constexpr hal::byte DUMMY_CRC = 0xFF;
static constexpr hal::byte ACMD41 = 0x69;

}  // namespace hal::microsd