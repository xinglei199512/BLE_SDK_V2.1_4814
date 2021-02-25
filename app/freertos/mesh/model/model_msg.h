
#ifndef MODEL_MSG_H_
#define MODEL_MSG_H_
#include "mesh_env.h"
#include "mesh_model.h"
#include "access_rx_process.h"

typedef void (*model_msg_handler_t)(mesh_elmt_t *,model_base_t *,access_pdu_rx_t *);
typedef const struct
{
    model_msg_handler_t hdl;
    uint32_t model_id;
}msg_handler_model_t;

#define ONE_OCTET_OPCODE_GEN(offset,msg) ((offset)+(msg))
#define TWO_OCTETS_OPCODE_GEN(offset,msg) (((offset)+(msg))|0x8000)
#define THREE_OCTETS_OPCODE_GEN(offset,begin,shift,msg) (((((offset)+(msg)) << (shift)) + begin)|0xc00000)

enum Byte_Opcode {
    ONE_BYPE_OPCODE = 0,
    TWO_BYPE_OPCODE = 1,
    THREE_BYPE_OPCODE = 2,
};

typedef const struct
{
    msg_handler_model_t *ptr;
    union {
        struct {
            uint16_t offset;
            uint8_t num;
        }one_two_opcode;
        struct {
            uint8_t offset;
            uint8_t num;
            uint16_t company_id;
        }three_opcode;
    }u;
}msg_map_t;
typedef const struct
{
    msg_map_t *msg_map;
    uint16_t size;
}msg_map_array_t;
msg_handler_model_t *mesh_sig_msg_handler_search(uint8_t *access_payload);

#endif /*MODEL_MSG_H_ */
