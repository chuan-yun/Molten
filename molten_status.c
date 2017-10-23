
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
#include "molten_status.h"

/* {{{ check status */
void mo_request_handle(mo_ctrl_t *mrt TSRMLS_DC)
{
    /* request for status */
    if (SG(request_info).request_uri != NULL 
        && SG(request_info).request_method != NULL
        && strstr(SG(request_info).request_uri, STATUS_URI) != NULL) {

        /* GET report status */
        if (strncmp(SG(request_info).request_method, "GET", sizeof("GET") - 1) == 0) {
            php_output_start_default(TSRMLS_C); 
            sapi_add_header_ex(ZEND_STRL("Expires: Thu, 01 Jan 1970 00:00:00 GMT"), 1, 1 TSRMLS_CC);
            sapi_add_header_ex(ZEND_STRL("Cache-Control: no-cache, no-store, must-revalidate, max-age=0"), 1, 1 TSRMLS_CC);
            sapi_add_header_ex(ZEND_STRL("Content-Type: text/plain"), 1, 1 TSRMLS_CC);
            SG(sapi_headers).http_response_code = 200;
            char *buf;
            mo_ctrl_serialize_msg(mrt, &buf);
            php_output_write(buf, strlen(buf) TSRMLS_CC); 
            efree(buf);
            php_output_end_all(TSRMLS_C);
    
            /* disable output after */ 
#if PHP_VERSION_ID > 50400
            php_output_set_status(PHP_OUTPUT_DISABLED TSRMLS_C);
#else
            php_output_set_status(PHP_OUTPUT_HANDLER_END TSRMLS_C);
#endif
        }

        /* POST update ctrl info */
        if (strncmp(SG(request_info).request_method, "POST", sizeof("POST") - 1) == 0) {
            int res = 0;

#if PHP_VERSION_ID < 50600
            res = mo_ctrl_update_sampling(SG(request_info).raw_post_data, mrt->mcm);
#elif PHP_VERSION_ID < 70000
            php_stream_rewind(SG(request_info).request_body);
            char *post_data;
            php_stream_copy_to_mem(SG(request_info).request_body, &post_data, PHP_STREAM_COPY_ALL, 0);
            res = mo_ctrl_update_sampling(post_data, mrt->mcm);
            efree(post_data);
#else
            php_stream_rewind(SG(request_info).request_body);
            zend_string *post_data = php_stream_copy_to_mem(SG(request_info).request_body, PHP_STREAM_COPY_ALL, 0);
            res = mo_ctrl_update_sampling(ZSTR_VAL(post_data), mrt->mcm);
            zend_string_free(post_data);

#endif
            php_output_start_default(TSRMLS_C); 
            sapi_add_header_ex(ZEND_STRL("Cache-Control: no-cache, no-store, must-revalidate, max-age=0"), 1, 1 TSRMLS_CC);
            sapi_add_header_ex(ZEND_STRL("Content-Type: text/plain"), 1, 1 TSRMLS_CC);
            if (res == -1) {
                SG(sapi_headers).http_response_code = 400;
                #define ERROR_OUTPUT    "[molten] update sampling error!!!"
                php_output_write(ERROR_OUTPUT, strlen(ERROR_OUTPUT) TSRMLS_CC); 
            } else {
                #define TRUE_OUTPUT    "[molten] update sampling success!!!"
                php_output_write(TRUE_OUTPUT, strlen(TRUE_OUTPUT) TSRMLS_CC); 
                SG(sapi_headers).http_response_code = 200;
            }

            php_output_end_all(TSRMLS_C);

            /* disable output after */ 
#if PHP_VERSION_ID > 50400
            php_output_set_status(PHP_OUTPUT_DISABLED TSRMLS_C);
#else
            php_output_set_status(PHP_OUTPUT_HANDLER_END TSRMLS_C);
#endif
        }
    }
}
/* }}} */
