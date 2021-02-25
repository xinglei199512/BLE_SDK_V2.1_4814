#ifndef _OTA_IMAGE_H_
#define _OTA_IMAGE_H_
#include <stdint.h>


#define OTA_AVAILABLE_FLAG          0x12345678
#define OTA_IMAGE_HEADER_LENGTH     256

typedef struct
{
    uint32_t valid_flag;
    uint32_t crc32;         //without image header
    uint32_t image_length;  //without image header
} ota_image_header_struct_t;

typedef union
{
    ota_image_header_struct_t   s;
    uint8_t                     a[OTA_IMAGE_HEADER_LENGTH];
} ota_image_header_t;



#endif

