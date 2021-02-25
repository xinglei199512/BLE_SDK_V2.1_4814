#ifndef VENDOR_MODELS_MSG_HANDLER_H_
#define VENDOR_MODELS_MSG_HANDLER_H_

#if(MESH_MODEL_TMALL_MODEL_SERVER)
#include "tmall_model_msg_handler.h"
#endif

#if(MESH_MODEL_LR_MODEL_SERVER||MESH_MODEL_LR_MODEL_CLIENT)
#include "lianrui_model_msg_handler.h"
#endif

#if(MESH_MODEL_TC_MODEL_SERVER||MESH_MODEL_TC_MODEL_CLIENT)
#include "tc_model_msg_handler.h"
#endif

#endif
