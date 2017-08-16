/**
 * Copyright 2017 chuan-yun silkcutks <silkcutbeta@gmail.com>
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

#ifndef MOLTEN_COMMON_H
#define MOLTEN_COMMON_H

#ifdef MOLTEN_DEBUG
#define MOLTEN_ERROR(format, ...) fprintf(stderr, "[MOLTEN] [file:%s] [line:%d]" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define MOLTEN_ERROR(format, ...)
#endif

#endif
