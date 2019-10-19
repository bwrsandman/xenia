/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2019 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_HID_SDLPAD_SDLPAD_HID_H_
#define XENIA_HID_SDLPAD_SDLPAD_HID_H_

#include <memory>

#include "xenia/hid/input_system.h"

namespace xe {
namespace hid {
namespace sdlpad {

std::unique_ptr<InputDriver> Create(xe::ui::Window* window);

}  // namespace sdlpad
}  // namespace hid
}  // namespace xe

#endif  // XENIA_HID_SDLPAD_SDLPAD_HID_H_
