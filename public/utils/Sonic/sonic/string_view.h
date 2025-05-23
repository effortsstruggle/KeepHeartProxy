/*
 * Copyright 2022 ByteDance Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#if __cplusplus >= 201703L
#include <string_view>
namespace sonic_json {
using StringView = std::string_view;
}  // namespace sonic_json
#else
#include "Sonic/thirdparty/string-view-lite/string_view.h"
namespace sonic_json {
using StringView = nonstd::string_view;
}  // namespace sonic_json
#endif
