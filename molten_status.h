/**
 * Copyright 2017 chuan-yun silkcutKs <silkcutbeta@gmail.com>
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

#ifndef MOLTEN_STATUS_H
#define MOLTEN_STATUS_H

#include "php.h"
#include "SAPI.h"

#include "molten_ctrl.h"
#include "php7_wrapper.h"
#include "main/php_streams.h"

/* status uri */
#define STATUS_URI      "/molten/status"

void mo_request_handle(mo_ctrl_t *mrt TSRMLS_DC);
#endif
