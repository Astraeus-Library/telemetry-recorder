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

#include <libhal-armcortex/dwt_counter.hpp>
#include <libhal-armcortex/startup.hpp>
#include <libhal-armcortex/system_control.hpp>
#include <libhal-lpc40/clock.hpp>
#include <libhal-lpc40/constants.hpp>
#include <libhal-lpc40/uart.hpp>
#include <libhal-util/as_bytes.hpp>

#include "../hardware_map.hpp"

hal::status initialize_processor()
{
  // Handled by picolibc's crt0.s
  return hal::success();
}

hal::result<hardware_map> initialize_platform()
{
  using namespace hal::literals;

  // Set the MCU to the maximum clock speed
  HAL_CHECK(hal::lpc40::clock::maximum(12.0_MHz));

  // Create a hardware counter
  auto& clock = hal::lpc40::clock::get();
  auto cpu_frequency = clock.get_frequency(hal::lpc40::peripheral::cpu);
  static hal::cortex_m::dwt_counter counter(cpu_frequency);

  static std::array<hal::byte, 64> uart0_buffer{};
  // Get and initialize UART0 for UART based logging
  static auto uart0 = HAL_CHECK(hal::lpc40::uart::get(0,
                                                      uart0_buffer,
                                                      hal::serial::settings{
                                                        .baud_rate = 115200,
                                                      }));

      static auto uart1 = HAL_CHECK(hal::lpc40::uart::get(1,
                                                      uart1_buffer,
                                                      hal::serial::settings{
                                                        .baud_rate = 38400,
                                                      }));

  static auto i2c = HAL_CHECK((hal::lpc40::i2c::get(2,
                                                    hal::i2c::settings{
                                                      .clock_rate = 100.0_kHz,
                                                    })));

  return hardware_map{
    .console = &uart0,
    .xbee = &uart1,
    .clock = &counter,
    .i2c = &i2c,
    .reset = []() { hal::cortex_m::reset(); },
  };
}
