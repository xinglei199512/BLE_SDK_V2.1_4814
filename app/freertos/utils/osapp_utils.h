#ifndef OSAPP_UTILS_H_
#define OSAPP_UTILS_H_
#include <stdint.h>
#include <stdarg.h>
#include "compiler.h"
#include "gattm_task.h"
#include "gattc_task.h"
#include "co_list.h"
#include "ke_msg.h"
#include "gap.h"

/*
 * MACROS
 ****************************************************************************************
 */
/* (ad_type1, ad_data1, ad_data_length1), (ad_type2, ad_data2, ad_data_length2) ... */
uint8_t *adv_data_pack(uint8_t *buf,uint8_t field_nums,...);
#define ADV_DATA_PACK(buf,field_nums,...) (adv_data_pack((buf),(field_nums),__VA_ARGS__) - (buf))

#define TO_2BYTES_ARRAY(u16) {(u16)&0xff,(u16)>>8&0xff}

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/// osapp utils params type
typedef enum
{
    OSAPP_UTILS_TYPE_GET_BD_ADDR,
    OSAPP_UTILS_TYPE_GET_IRK,
}osapp_utils_param_type_t;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef union
{
    /// Identity Resolving Key
    struct gap_sec_key irk;
    /// Device BD Identity Address
    struct gap_bdaddr addr;
}osapp_utils_param_t;


//extract ad_type content
uint8_t adv_data_extract(uint8_t **buff , uint8_t ad_type);

/**
 ****************************************************************************************
 * @brief  tools for random generate.
 *
 * @param[out] p_out      Pointer to the random data.
 * @param[in]  len        generate random length in byte.
 *
 ****************************************************************************************
 */
extern void osapp_utils_random_generate(uint8_t *p_out,uint8_t len);
/**
 ****************************************************************************************
 * @brief   Is all of the array elements set to a value.
 *
 * @param[in]   buf - buffer to check
 * @param[in]   val - value to check each array element for
 * @param[in]   len - length to check
 *
 * @return  TRUE if all "val", FALSE otherwise
 ****************************************************************************************
 */
extern bool osapp_utils_is_buffer_set( uint8_t *buf, uint8_t val, uint8_t len );
/**
 ****************************************************************************************
 * @brief   Func osapp_utils_set_dev_init
 *
 * @param[in] role           The role of the device.
 * @param[in] addr_type      Address type of the device 0=public/1=private random
 *
 *                           /// Device Address is a Public Static address
 *                           GAPM_CFG_ADDR_PUBLIC        = 0,
 *                           /// Device Address is a Private Static address
 *                           GAPM_CFG_ADDR_PRIVATE       = 1,
 *                           /// Device Address generated using host-based Privacy feature
 *                           GAPM_CFG_ADDR_HOST_PRIVACY  = 2,
 *                           /// Device Address generated using controller-based Privacy feature
 *                           GAPM_CFG_ADDR_CTNL_PRIVACY  = 4,
 *
 ****************************************************************************************
 */
extern void osapp_utils_set_dev_init(enum gap_role role,uint8_t addr_type);
/**
 ****************************************************************************************
 * @brief   Func osapp_utils_get_params
 *
 * @param[in]  type           The type osapp utils params type.
 * @param[out] p_param        The point of the params.
 *
 ****************************************************************************************
 */
extern bool osapp_utils_get_params(osapp_utils_param_type_t type,osapp_utils_param_t *p_param);
/**
 ****************************************************************************************
 * @brief  tools for printf to segger rtt log data.(temp buff use malloc)
 *
 * @param[in]  data      Pointer to the data to be printf to segger rtt log.
 * @param[in]  length    The length of data to be printf.
 *
 ****************************************************************************************
 */
extern void osapp_utils_log_hex_data(const uint8_t * data , uint16_t length);
/**
 ****************************************************************************************
 * @brief  used to set max_mtu.
 *
 * @param[in]  max_mtu      the max mtu
 *
 ****************************************************************************************
 */
extern void osapp_utils_set_dev_mtu(uint16_t max_mtu);


/**
 ****************************************************************************************
 * @brief  tools for printf to segger rtt log data.(temp buff use malloc)
 *
 * @param[in]  data      Pointer to the data to be printf to segger rtt log.
 * @param[in]  length    The length of data to be printf.
 *
 ****************************************************************************************
 */
extern void log_hex_data(const uint8_t * data , uint16_t length);

/**
 ****************************************************************************************
 * @brief  tools for memory reverse and copy.
 *
 * @param[in]  p_src      Pointer to the memcpy source data.
 * @param[out] p_dst      Pointer to the memcpy destination data.
 * @param[in]  length     The length of data to be memcpy.
 *
 * @note  Source and destination addresses cannot have an intersection.
 *
 ****************************************************************************************
 */
extern void memcpy_rev(void * restrict p_dst, const void * restrict p_src, uint8_t length);

/**
 ****************************************************************************************
 * @brief  tools for reverse the src data.
 *
 * @param[in]  p_src      Pointer to the memcpy source data.
 * @param[in]  length     The length of data to be reverse.
 *
 ****************************************************************************************
 */
extern void reverse_self(void * src , uint8_t length);

/**
 ****************************************************************************************
 * @brief  tools for random generate.
 *
 * @param[out] p_out      Pointer to the random data.
 * @param[in]  len        generate random length in byte.
 *
 ****************************************************************************************
 */
extern void tools_random_generate(uint8_t *p_out,uint8_t len);

/**
 ****************************************************************************************
 * @brief  tools for get the gap public bd address.
 *
 * @param[out] p_out_addr      Pointer to the gap public bd address data.
 *
 ****************************************************************************************
 */
extern void tools_gap_public_bd_addr_get(uint8_t *p_out_addr);

void output_rf_debug_pin(void);


#endif
