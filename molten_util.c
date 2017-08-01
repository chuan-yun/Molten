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
#include "molten_util.h"

#if RAND_MAX/256 >= 0xFFFFFFFFFFFFFF
  #define LOOP_COUNT 1
#elif RAND_MAX/256 >= 0xFFFFFF
  #define LOOP_COUNT 2
#elif RAND_MAX/256 >= 0x3FFFF
  #define LOOP_COUNT 3
#elif RAND_MAX/256 >= 0x1FF
  #define LOOP_COUNT 4
#else
  #define LOOP_COUNT 5
#endif

#define MAX_SPANS       65534
#define MAX_SPANS_LEN   5

/* rand uini64 */
uint64_t rand_uint64(void) 
{
    uint64_t r = 0;
    int i = 0;
    struct timeval tv;
    int seed = gettimeofday(&tv, NULL) == 0 ? tv.tv_usec * getpid() : getpid();
    srandom(seed);
    for (i = LOOP_COUNT; i > 0; i--) {
      r = r*(RAND_MAX + (uint64_t)1) + random();
    }
    return r;
}

/* check is hit or not */
int check_hit_ratio(long base)
{
    struct timeval tv;
    int seed = gettimeofday(&tv, NULL) == 0 ? tv.tv_usec * getpid() : getpid();
    srandom(seed);
    int num = random();
    
    /* the remainder 0 is choose by self */
    if (num % base == 0) {
        return 1;
    } else {
        return 0;
    }
}

/* bin to hex */
void b2hex(char **output, const unsigned char *input, int input_len)
{
    static const char hexits[17] = "0123456789abcdef";
    int i;
    *output = (char *)emalloc(input_len * 2 + 1); 
    for (i = 0; i < input_len; i++) {
        *(*output + i*2) = hexits[*(input+i) >> 4];
        *(*output + i*2 + 1) = hexits[*(input+i) & 0x0F];
    }
    *(*output + input_len *2) = '\0';
}

/* bin2hex64 */
void bin2hex64(char **output, const uint64_t *input) 
{
    b2hex(output, (char *)input, 8); 
}

/* random 64 and change to hex */
void rand64hex(char **output) 
{
    uint64_t num = rand_uint64();
    return bin2hex64(output, &num);
}

/* build span id random */
void build_span_id_random(char **span_id, char *parent_span_id, int span_count)
{
    rand64hex(span_id);
}

/* span id change to 1.1.x.x 
 * if the parent span id 1.1.1, the span id is 1.1.1.1, the next span id is 1.1.1.2
 * */
/* so we will build our span id contact the parent_span_id */
void build_span_id_level(char **span_id, char *parent_span_id, int span_count)
{
    if (parent_span_id != NULL) {
        /* len = . + \0 + max_spans_len + parent_len */
        int len = 2 + MAX_SPANS_LEN + strlen(parent_span_id); 

        *span_id  = emalloc(len);
        memset(*span_id, 0x00, len);
        sprintf(*span_id, "%s.%d", parent_span_id, span_count);
        (*span_id)[len-1] = '\0';
    } else {
        *span_id = estrdup("1");
    }
}
