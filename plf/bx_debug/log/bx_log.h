#ifndef BX_LOG_H_
#define BX_LOG_H_
#include "bx_dbg.h"

#define GLOBAL_OUTPUT_LVL     LVL_DBG

#if !defined(LOG_TAG)
    #define LOG_TAG                "NO_TAG"
#endif /* !defined(LOG_TAG) */

#if !defined(LOG_LVL)
    #define LOG_LVL                LVL_DBG
#endif /* !defined(LOG_LVL) */


#if (LOG_LVL >= LVL_DBG) && (GLOBAL_OUTPUT_LVL >= LVL_DBG)
    #define bxlog_d(TAG, ...)           rtt_output(LVL_DBG, true,"D/"TAG":"__VA_ARGS__)
#else
    #define bxlog_d(TAG, ...)
#endif

#if (LOG_LVL >= LVL_INFO) && (GLOBAL_OUTPUT_LVL >= LVL_INFO)
    #define bxlog_i(TAG, ...)           rtt_output(LVL_INFO,true,"I/"TAG":"__VA_ARGS__)
#else
    #define bxlog_i(TAG, ...)
#endif

#if (LOG_LVL >= LVL_WARN) && (GLOBAL_OUTPUT_LVL >= LVL_WARN)
    #define bxlog_w(TAG, ...)           rtt_output(LVL_WARN,true,"W/"TAG":"__VA_ARGS__)
#else
    #define bxlog_w(TAG, ...)
#endif 

#if (LOG_LVL >= LVL_ERROR) && (GLOBAL_OUTPUT_LVL >= LVL_ERROR)
    #define bxlog_e(TAG, ...)           rtt_output(LVL_ERROR,true,"E/"TAG":"__VA_ARGS__)
#else
    #define bxlog_e(TAG, ...)
#endif 

#define LOG_D(...)  bxlog_d(LOG_TAG,__VA_ARGS__)
#define LOG_I(...)  bxlog_i(LOG_TAG,__VA_ARGS__)
#define LOG_W(...)  bxlog_w(LOG_TAG,__VA_ARGS__)
#define LOG_E(...)  bxlog_e(LOG_TAG,__VA_ARGS__)
#define LOG_RAW(...)  rtt_output(LVL_DBG,false, __VA_ARGS__)
//#define LOG_HEX

#endif
