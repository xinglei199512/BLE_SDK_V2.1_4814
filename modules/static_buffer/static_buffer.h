#ifndef STATIC_BUFFER_H_
#define STATIC_BUFFER_H_
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "co_math.h"
typedef struct{
    uint8_t *buf;
    uint8_t *ref_cnt;
    uint16_t element_size;
    uint16_t buf_length;
    uint16_t remaining_size;
}static_buffer_t;

 
#define DEF_ARRAY_BUF(buf_hdl,type,size) \
    static __attribute__((aligned(4))) uint8_t _##buf_hdl##_array[(size)*sizeof(type)]; \
    static uint8_t _##buf_hdl##_array_ref_cnt[(size)]; \
    static_buffer_t buf_hdl = { \
        .buf = _##buf_hdl##_array, \
        .ref_cnt = _##buf_hdl##_array_ref_cnt, \
        .element_size = sizeof(type), \
        .buf_length = (size), \
        .remaining_size = (size),\
    }

#define DECLARE_ARRAY_BUF_TYPE(type_name,data_type,size) \
    typedef struct \
    { \
        static_buffer_t buf_hdl; \
        data_type buf[size]; \
        uint8_t ref_cnt[size]; \
    }type_name;

#define INIT_ARRAY_BUF_TYPE_VAR(var_ptr) \
        do{\
            (var_ptr)->buf_hdl.buf = (uint8_t *)(var_ptr)->buf; \
            (var_ptr)->buf_hdl.ref_cnt = (var_ptr)->ref_cnt; \
            (var_ptr)->buf_hdl.element_size = sizeof((var_ptr)->buf[0]); \
            (var_ptr)->buf_hdl.buf_length = sizeof((var_ptr)->buf)/sizeof((var_ptr)->buf[0]); \
            (var_ptr)->buf_hdl.remaining_size = sizeof((var_ptr)->buf)/sizeof((var_ptr)->buf[0]); \
        }while(0)

#define GET_ARRAY_BUF_TYPE_HDL(var_ptr) (&(var_ptr)->buf_hdl)

#define FOR_EACH_ALLOCATED_ITEM(buf_hdl,ptr,sentences_block) \
    for(ptr = (void *)(buf_hdl).buf; (uint8_t *)ptr < &(buf_hdl).buf[(buf_hdl).buf_length*(buf_hdl).element_size];ptr = (void *)((uint8_t *)ptr + (buf_hdl).element_size)) \
    { \
        if((buf_hdl).ref_cnt[((uint8_t *)ptr-(buf_hdl).buf)/(buf_hdl).element_size]) \
        { \
           sentences_block; \
        } \
    }

uint16_t array_buf_element_size(static_buffer_t *buf_hdl);

uint16_t array_buf_available_size(static_buffer_t *buf_hdl);

bool array_buf_has_element(static_buffer_t *buf_hdl,void *ptr);
#define array_buf_is_elem_handle_valid(buf_hdl,elem_hdl) array_buf_has_element(buf_hdl,elem_hdl)

uint16_t array_buf_idx_get(static_buffer_t *buf_hdl,void *ptr);

void *array_buf_element_get(static_buffer_t *buf_hdl,uint16_t idx);

void *array_buf_alloc_idx(static_buffer_t *buf_hdl,uint16_t idx);

void *array_buf_calloc_idx(static_buffer_t *buf_hdl,uint16_t idx);

void *array_buf_alloc(static_buffer_t *buf_hdl);

void *array_buf_calloc(static_buffer_t *buf_hdl);//alloc and memset 0

uint8_t array_buf_retain(static_buffer_t *buf_hdl,void *ptr);

bool array_buf_release(static_buffer_t *buf_hdl,void *ptr);

uint8_t array_buf_ref_cnt(static_buffer_t *buf_hdl,void *ptr);


#endif
