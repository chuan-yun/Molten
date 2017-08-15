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

#include "molten_stack.h"

#define STACK_ELE(stack, m) ((void *)((char *)(stack)->elements + (stack)->size * (m)))

void mo_stack_init(mo_stack *stack, int size, stack_dtor_func dtor)
{
    stack->max = 0;
    stack->size = size;
    stack->top = 0;
    stack->elements = NULL;
    stack->dtor = dtor;
}

int mo_stack_push(mo_stack *stack, void *element)
{
    if (stack->top >= stack->max) {
        stack->max += MO_STACK_BLOCK_SIZE;
        stack->elements = erealloc(stack->elements, (stack->max * stack->size));
    }
    memcpy(STACK_ELE(stack, stack->top), element, stack->size);
    return stack->top++;
}

void *mo_stack_top(mo_stack *stack)
{
    if (stack->top > 0) {
        return STACK_ELE(stack, stack->top - 1);
    } else {
        return NULL;
    }
}

void *mo_stack_element(mo_stack *stack, int offset)
{
    return STACK_ELE(stack, (stack->top - 1 - offset));
}

void *mo_stack_sec_element(mo_stack *stack)
{
    if (stack->top > 1) {
        return mo_stack_element(stack, 1);
    } else {
        return NULL;
    }
}

void mo_stack_del_top(mo_stack *stack)
{
    stack->dtor(STACK_ELE(stack, stack->top - 1));
    --stack->top;
}

void mo_stack_pop(mo_stack *stack, void *element)
{
    if (stack->top <= 0) {
        return;
    }

    if (element != NULL) {
        memcpy(element, STACK_ELE(stack, stack->top-1), stack->size);
    }
    mo_stack_del_top(stack);
}

int mo_stack_empty(mo_stack *stack)
{
    return stack->top == 0;  
}

int mo_stack_num(mo_stack *stack)
{
    return stack->top;
}

void mo_stack_destroy(mo_stack *stack)
{
    int i;
    if (stack->elements) {
        for (i = 0; i < stack->top; i++) {
            stack->dtor(stack->elements + stack->size * i);
        }
        efree(stack->elements);
        stack->max = 0;
        stack->top = 0;
        stack->dtor = NULL;
        stack->elements = NULL;
    }
}
