#ifndef CONFIG_CLIENT_H_
#define CONFIG_CLIENT_H_


#include "config_client_feature.h"
//#include "config_client_heartheat.h"
#include "config_client_key.h"
#include "config_client_misc.h"

#define CONFIGURATION_CLIENT_MODEL_ID 0x0001

typedef struct{
    model_client_base_t model;
}config_client_model_t;



#endif
