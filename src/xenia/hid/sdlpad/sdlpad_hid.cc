/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2019 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/hid/sdlpad/sdlpad_hid.h"

#include "xenia/hid/sdlpad/sdlpad_input_driver.h"

namespace xe {
namespace hid {
namespace sdlpad {

std::unique_ptr<InputDriver> Create(xe::ui::Window* window) {
  return std::make_unique<SDLPadInputDriver>(window);
}

}  // namespace sdlpad
}  // namespace hid
}  // namespace xe
