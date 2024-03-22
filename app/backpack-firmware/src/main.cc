/* Copyright (c) 2024 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */

#define PW_LOG_MODULE_NAME "codelab"

#include "hello.h"
#include "pw_log/log.h"

int main() {
  pw::StringBuffer<128> buffer;
  pw::InlineString<6> greeting = "Hello";
  pw::InlineString<6> name = "World";
  app::WelcomeMessage(buffer, greeting, name);
  PW_LOG_INFO("%s", buffer.c_str());
  
  uint32_t count = 0;

  while (true) {
    k_sleep(K_MSEC(1000));
    PW_LOG_INFO("Hello toanpi %d", count++);
  }
  return 0;
}
