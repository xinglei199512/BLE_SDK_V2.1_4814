#include "aes_ecb_blocked.h"
#include "log.h"
#include "reg_blecore.h"
#include "plf.h"
#include "sys_sleep.h"
#include "clk_gate.h"
#include "ll.h"

extern blemac_stat_t mac_status;

static void memcpy_rev(void * restrict p_dst, const void * restrict p_src, uint8_t length)
{
    uint8_t * src = (uint8_t *) p_src;
    uint8_t * dst = (uint8_t *) p_dst;
    
    dst += (length - 1);
    while(length --)
    {
        *dst = * src;
        dst--;
        src ++;
    }
}


void aes_ecb_bloked_operation(const uint8_t * key, const uint8_t * input, uint8_t * output , aes_operation_t mode)
{
    uint8_t operation_done = 0;
    uint8_t backup_encrypted[16];
    blemac_stat_t mac_status_tmp;
    
    //prepare clock
    GLOBAL_INT_DISABLE();
    mac_status_tmp = mac_status;
    GLOBAL_INT_RESTORE();
    if(mac_status_tmp == sleep_low_power_clk)
    {
        ble_soft_wakeup();
    }
    //operation
    while(operation_done == 0)
    {
        //disable irq
        GLOBAL_INT_DISABLE();
        //if aes is free
        if(ble_aes_start_getf() == 0)
        {
            //back up em reg
            em_rd(backup_encrypted, EM_BLE_ENC_CIPHER_OFFSET, ENC_DATA_LEN  );
            uint32_t irq_stat = ble_intstat_get();
            
            // copy the key in the register dedicated for the encryption
            ble_aeskey31_0_set  (co_read32p(&key[ 0]));
            ble_aeskey63_32_set (co_read32p(&key[ 4]));
            ble_aeskey95_64_set (co_read32p(&key[ 8]));
            ble_aeskey127_96_set(co_read32p(&key[12]));
            // copy data from sys ram to em
            em_wr(input, EM_BLE_ENC_PLAIN_OFFSET, EM_BLE_ENC_LEN);
            // set the pointer on the data to encrypt.
            ble_aesptr_set(EM_BLE_ENC_PLAIN_OFFSET);
            // start the encryption
            ble_aes_mode_setf(mode);
            ble_aes_start_setf(BLE_AES_START_BIT);
            
            //wait for complete
            while(ble_aes_start_getf());
            // copy data from em to sys ram
            em_rd(output, EM_BLE_ENC_CIPHER_OFFSET, ENC_DATA_LEN);
            //set flag
            operation_done = 1;
            
            //restore em reg
            em_wr(backup_encrypted, EM_BLE_ENC_CIPHER_OFFSET, ENC_DATA_LEN  );
            if(!(irq_stat & BLE_CRYPTINTSTAT_BIT))
            {
                ble_intack_clear(BLE_CRYPTINTSTAT_BIT);
                NVIC_ClearPendingIRQ(BLE_MAC_IRQn);
            }
        }
        //enable irq
        GLOBAL_INT_RESTORE();  
    } 
}

void aes_ecb_bloked_operation_rev(const uint8_t * key, const uint8_t * input, uint8_t * output , aes_operation_t mode)
{
    uint8_t rev_key   [16];
    uint8_t rev_input [16];
    uint8_t rev_output[16];
    
    memcpy_rev(rev_key   , key   , 16);
    memcpy_rev(rev_input , input , 16);
    aes_ecb_bloked_operation(rev_key , rev_input , rev_output , mode);
    memcpy_rev(output , rev_output , 16);
}


