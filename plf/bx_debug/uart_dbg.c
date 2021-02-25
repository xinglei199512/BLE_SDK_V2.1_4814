/**
 ****************************************************************************************
 *
 * @file uart_printf.c
 *
 * @brief Application PRINTF API
 *
 * Copyright (C) Quintic 2012-2013
 *
 * $Rev: 1.0 $
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
//#include "uart_env.h"
//#include "uart.h"

#include "bx_config.h"
#include <stdio.h>
#include <stdarg.h>
#include "stdint.h"
#include "stdbool.h"


//#include "osapp_config.h"
//#include "osapp_task.h"
//#include "bx_dbg.h"
#include "log.h"

#include "field_manipulate.h"
#include "dw_apb_uart_typedef.h"
#include "app_uart.h"

#include "uart_dbg.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#if QN_DBG_PRINT

//static void uart1_write_log_callback(void *dummy,uint8_t status);

//#define QN_PRINTF(value,length)           app_uart_write(&uart1_inst.inst, (uint8_t *)(value),(length),uart1_write_log_callback,NULL) //  uart_printf(QN_DEBUG_UART, (a))
//#define QN_PRINTF(value,length)           app_uart_write(&uart1_inst.inst, (uint8_t *)(value),(length),NULL,NULL) //  uart_printf(QN_DEBUG_UART, (a))
#define QN_PRINTF(value,length)           uart_printf((value), (length)) //  uart_printf(QN_DEBUG_UART, (a))

#define ZEROPAD 1               // Pad with zero
#define SIGN    2               // Unsigned/signed long
#define PLUS    4               // Show plus
#define SPACE   8               // Space if plus
#define LEFT    16              // Left justified
#define SPECIAL 32              // 0x
#define LARGE   64              // Use 'ABCDEF' instead of 'abcdef'

#define is_digit(c) ((c) >= '0' && (c) <= '9')

uint8_t uart_log_enable = 0;

static char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";



void uart_printf(void *buf, uint32_t length)
{
	uint8_t *temp_buf = (uint8_t*)buf;
	reg_uart_t *reg = (reg_uart_t*) 0x20144000;
	while(length > 0)
	{
		 while (FIELD_RD(reg,USR,UART_TFNF)==Transmit_FIFO_Not_Full)
	        {
	            // Put a byte in the FIFO
	            reg->RBR_THR_DLL = *temp_buf;
		     temp_buf++;
		     length--;
	            if (length == 0)
	            {
	                // Exit loop
	                break;
	            }
	        }
	}
	
}

//static bool uart1_tx_log_busy = false;

static app_uart_inst_t uart1_inst = UART_INSTANCE(1);


static size_t strnlen(const char *s, size_t count)
{
    const char *sc;
    for (sc = s; *sc != '\0' && count--; ++sc);
    return sc - s;
}

static int skip_atoi(const char **s)
{
    int i = 0;
    while (is_digit(**s)) i = i*10 + *((*s)++) - '0';
    return i;
}

static char *number(char *str, long num, int base, int size, int precision, int type)
{
    char c, sign, tmp[66];
    char *dig = digits;
    int i;

    if (type & LARGE)
        dig = upper_digits;
    if (type & LEFT)
        type &= ~ZEROPAD;
    if (base < 2 || base > 36)
        return 0;

    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;

    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
            size--;
        }
        else if (type & PLUS)
        {
            sign = '+';
            size--;
        }
        else if (type & SPACE)
        {
            sign = ' ';
            size--;
        }
    }

    if (type & SPECIAL)
    {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }

    i = 0;

    if (num == 0)
        tmp[i++] = '0';
    else
    {
        while (num != 0)
        {
            tmp[i++] = dig[((unsigned long) num) % (unsigned) base];
            num = ((unsigned long) num) / (unsigned) base;
        }
    }

    if (i > precision)
        precision = i;
    size -= precision;
    if (!(type & (ZEROPAD | LEFT)))
        while (size-- > 0) *str++ = ' ';
    if (sign)
        *str++ = sign;

    if (type & SPECIAL)
    {
        if (base == 8)
            *str++ = '0';
        else if (base == 16)
        {
            *str++ = '0';
            *str++ = digits[33];
        }
    }

    if (!(type & LEFT))
        while (size-- > 0)
            *str++ = c;
    while (i < precision--)
        *str++ = '0';
    while (i-- > 0)
        *str++ = tmp[i];
    while (size-- > 0)
        *str++ = ' ';

    return str;
}

static char *eaddr(char *str, unsigned char *addr, int size, int precision, int type)
{
    char tmp[24];
    char *dig = digits;
    int i, len;

    if (type & LARGE)
        dig = upper_digits;
    len = 0;
    for (i = 0; i < 6; i++)
    {
        if (i != 0)
            tmp[len++] = ':';
        tmp[len++] = dig[addr[i] >> 4];
        tmp[len++] = dig[addr[i] & 0x0F];
    }

    if (!(type & LEFT))
        while (len < size--)
            *str++ = ' ';

    for (i = 0; i < len; ++i)
        *str++ = tmp[i];

    while (len < size--)
        *str++ = ' ';

    return str;
}

static char *iaddr(char *str, unsigned char *addr, int size, int precision, int type)
{
    char tmp[24];
    int i, n, len;
    len = 0;

    for (i = 0; i < 4; i++)
    {
        if (i != 0)
            tmp[len++] = '.';
        n = addr[i];

        if (n == 0)
            tmp[len++] = digits[0];
        else
        {
            if (n >= 100)
            {
                tmp[len++] = digits[n / 100];
                n = n % 100;
                tmp[len++] = digits[n / 10];
                n = n % 10;
            }
            else if (n >= 10)
            {
                tmp[len++] = digits[n / 10];
                n = n % 10;
            }

            tmp[len++] = digits[n];
        }
    }

    if (!(type & LEFT))
        while (len < size--)
            *str++ = ' ';

    for (i = 0; i < len; ++i)
        *str++ = tmp[i];

    while (len < size--)
        *str++ = ' ';

    return str;
}

/**
 ****************************************************************************************
 * @brief QN sprintf function
 *
 ****************************************************************************************
 */
int qsprintf(char *buf, const char *fmt, va_list args)
{
    int len;
    unsigned long num;
    int i, base;
    char *str;
    char *s;

    int flags;            // Flags to number()

    int field_width;      // Width of output field
    int precision;        // Min. # of digits for integers; max number of chars for from string
    int qualifier;        // 'h', 'l', or 'L' for integer fields

    for (str = buf; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            *str++ = *fmt;
            continue;
        }

        // Process flags
        flags = 0;
        repeat:
        fmt++; // This also skips first '%'

        switch (*fmt)
        {
            case '-':
                flags |= LEFT;
                goto repeat;
            case '+':
                flags |= PLUS;
                goto repeat;
            case ' ':
                flags |= SPACE;
                goto repeat;
            case '#':
                flags |= SPECIAL;
                goto repeat;
            case '0':
                flags |= ZEROPAD;
                goto repeat;
        }

        // Get field width
        field_width = -1;
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*')
        {
            fmt++;
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        // Get the precision
        precision = -1;
        if (*fmt == '.')
        {
            ++fmt;
            if (is_digit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*')
            {
                ++fmt;
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        // Get the conversion qualifier
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
        {
            qualifier = *fmt;
            fmt++;
        }

        // Default base
        base = 10;

        switch (*fmt)
        {
            case 'c':
                if (!(flags & LEFT))
                    while (--field_width > 0)
                        *str++ = ' ';
                *str++ = (unsigned char)
                    va_arg(args, int);
                while (--field_width > 0)
                    *str++ = ' ';
                continue;

            case 's':
                s = va_arg(args, char *);
                if (!s)
                    s = "<NULL>";
                len = strnlen(s, precision);
                if (!(flags & LEFT))
                    while (len < field_width--)
                        *str++ = ' ';
                for (i = 0; i < len; ++i)
                    *str++ = *s++;
                while (len < field_width--)
                    *str++ = ' ';
                continue;

            case 'p':
                if (field_width == -1)
                {
                    field_width = 2 * sizeof(void *);
                    flags |= ZEROPAD;
                }
                str = number(str, (unsigned long) va_arg(args, void *), 16, field_width, precision, flags);
                continue;

            case 'n':
                if (qualifier == 'l')
                {
                    long *ip = va_arg(args, long *);
                    *ip = (str - buf);
                }
                else
                {
                    int *ip = va_arg(args, int *);
                    *ip = (str - buf);
                }
                continue;

            case 'A':
                flags |= LARGE;
                if (qualifier == 'l')
                    str = eaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
                else
                    str = iaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
                continue;
                
            case 'a':
                if (qualifier == 'l')
                    str = eaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
                else
                    str = iaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
                continue;

            // Integer number formats - set up the flags and "break"
            case 'o':
                base = 8;
                break;

            case 'X':
                flags |= LARGE;
                base = 16;
                break;
                
            case 'x':
                base = 16;
                break;

            case 'd':
            case 'i':
                flags |= SIGN;
                break;
            case 'u':
            break;

            default:
                if (*fmt != '%')
                    *str++ = '%';

                if (*fmt)
                    *str++ = *fmt;
                else
                    --fmt;
            continue;
        }

        if (qualifier == 'l' || qualifier == 'L' )
        {
            num = va_arg(args, unsigned long);
        }
        /*
        else if (qualifier == 'h')
        {
            if (flags & SIGN)
                num = va_arg(args, int); // Derek, shouble be short
            else
                num = va_arg(args, unsigned int); // Derek, shouble be unsigned short
        }
        */
        else
        {
            if (flags & SIGN)
                num = va_arg(args, int);
            else
                num = va_arg(args, unsigned int);
        }
        str = number(str, num, base, field_width, precision, flags);
    }

    *str = '\0';
    return str - buf;
}

// used by QPRINTF()
char print_buff[128];

/**
 ****************************************************************************************
 * @brief QN printf function
 *
 ****************************************************************************************
 */
int qprintf(const char *fmt, ... )
{
	if (!uart_log_enable)
		return 1;
//	while( true == uart1_tx_log_busy );
	memset(print_buff,0,sizeof(print_buff));
	va_list args;
	va_start(args, fmt);
	qsprintf(print_buff, fmt, args);
	va_end(args);
//	uart1_tx_log_busy = true;
	QN_PRINTF((uint8_t *)print_buff,strlen(print_buff));

    return 0;
}


void uart_log_init(void)
{
    uart1_inst.param.baud_rate = UART_BAUDRATE_115200;
    uart1_inst.param.tx_pin_no = 12;
    uart1_inst.param.rx_pin_no = 13;
    uart1_inst.param.flow_control_en = 0;
    uart1_inst.param.auto_flow_control = 0;
	uart1_inst.param.rx_dma = 0;
	uart1_inst.param.tx_dma = 0;
	uart1_inst.param.parity_en = 0;
    app_uart_init(&uart1_inst.inst);
			uart_log_enable = 1;
			NVIC_DisableIRQ(UART1_IRQn); // disable uart1 irq
}

//static void uart1_write_log_callback(void *dummy,uint8_t status)
//{
//	uart1_tx_log_busy = false;
//}

/**
 ****************************************************************************************
 * @brief Trace data with length and format
 *
 ****************************************************************************************
 */
void qtrace(uint8_t *data, uint16_t len, bool dir, uint8_t fmt)
{
    if (len == 0)
    {
        QPRINTF("NULL");
    }
    else
    {
        for (uint16_t i = 0; i < len; i++)
        {
            switch (fmt)
            {
                case 0: // %c
                default:
                    QPRINTF("%c", dir==0 ? data[i] : data[len-i-1]);
                    break;
                case 1: // %d
                    QPRINTF("%d", dir==0 ? data[i] : data[len-i-1]);
                    break;
                case 2: // %x
                    QPRINTF("%x", dir==0 ? data[i] : data[len-i-1]);
                    break;
            }
        }
    }
}

#endif // QN_DBG_PRINT

