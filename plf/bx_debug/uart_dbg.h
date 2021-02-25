/**
 ****************************************************************************************
 *
 * @file app_printf.c
 *
 * @brief Application debug API
 *
 * Copyright (C) Quintic 2012-2013
 *
 * $Rev: 1.0 $
 *
 ****************************************************************************************
 */
#ifndef _UART_DBG_H_
#define _UART_DBG_H_
#include <stdarg.h>


#define QN_DBG_PRINT 1


#define QSPRINTF qsprintf

#define QPRINTF qprintf

#define QTRACE qtrace



/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/*
 ****************************************************************************************
 * @brief QN sprintf function
 ****************************************************************************************
 */
//int qsprintf(char *buf, const char *fmt, va_list args);
extern int qsprintf(char *buf, const char *fmt, va_list args);


/*
 ****************************************************************************************
 * @brief QN printf function
 ****************************************************************************************
 */
//int qprintf(const char *fmt, ... );


extern int qprintf(const char *fmt, ... );

//extern int qsprintf(char *buf, const char *fmt, va_list args);


extern void uart_log_init(void);


#endif

