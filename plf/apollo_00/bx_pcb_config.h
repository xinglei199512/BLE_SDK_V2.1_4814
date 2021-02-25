#ifndef BX_PCB_CONFIG_H_
#define BX_PCB_CONFIG_H_
#include "bx_sys_config.h"
enum gpio_pad_vdd_type
{
    VDD_1V8 = 0,
    VDD_3V
};
enum io_config_mask
{
    NORMAL_MODE_IE = 0x1,
    SLEEP_MODE_IE = 0x2,
    UTILITY_IO_EN = 0x4,
    SLEEP_RET_OUT_EN = 0x8,
    SLEEP_RET_OUT_H = 0x10,
};

#if (defined(DEBUGGER_ATTACHED)&&(DEBUGGER_ATTACHED==1))
#define SWDCLK      (UTILITY_IO_EN | NORMAL_MODE_IE|SLEEP_MODE_IE)
#define SWDIO       (UTILITY_IO_EN | NORMAL_MODE_IE|SLEEP_MODE_IE)
#else
#define SWDCLK      (UTILITY_IO_EN | NORMAL_MODE_IE)
#define SWDIO       (UTILITY_IO_EN | NORMAL_MODE_IE)
#endif
#ifndef FLASH_SLEEP_POWER_OFF
#define QSPI_CS     (SLEEP_RET_OUT_H |SLEEP_RET_OUT_EN | UTILITY_IO_EN)
#define QSPI_CLK    (SLEEP_RET_OUT_EN | UTILITY_IO_EN)
#define QSPI_D0     (SLEEP_RET_OUT_EN|UTILITY_IO_EN | NORMAL_MODE_IE)
#define QSPI_D1     (SLEEP_RET_OUT_EN|UTILITY_IO_EN | NORMAL_MODE_IE)
#define QSPI_D2     (SLEEP_RET_OUT_EN|UTILITY_IO_EN | NORMAL_MODE_IE)
#define QSPI_D3     (UTILITY_IO_EN | NORMAL_MODE_IE)
#else
#define QSPI_CS     (UTILITY_IO_EN)
#define QSPI_CLK   (UTILITY_IO_EN)
#define QSPI_D0     (UTILITY_IO_EN | NORMAL_MODE_IE)
#define QSPI_D1     (UTILITY_IO_EN | NORMAL_MODE_IE)
#define QSPI_D2     (UTILITY_IO_EN | NORMAL_MODE_IE)
#define QSPI_D3     (UTILITY_IO_EN | NORMAL_MODE_IE)
#endif
#define SPIM_CS         (SLEEP_RET_OUT_H |SLEEP_RET_OUT_EN |UTILITY_IO_EN)
#define SPIM_CLK        (SLEEP_RET_OUT_EN | UTILITY_IO_EN)
#define SPIM_MOSI   (SLEEP_RET_OUT_EN | UTILITY_IO_EN)
#define SPIM_MISO   (UTILITY_IO_EN | NORMAL_MODE_IE)
#define SPIS_CS            (UTILITY_IO_EN | NORMAL_MODE_IE)
#define SPIS_CLK          (UTILITY_IO_EN | NORMAL_MODE_IE)
#define SPIS_MOSI      (UTILITY_IO_EN | NORMAL_MODE_IE)
#define SPIS_MISO      (SLEEP_RET_OUT_EN | UTILITY_IO_EN)
#define IIC_SCL             (UTILITY_IO_EN | NORMAL_MODE_IE)
#define IIC_SDA           (UTILITY_IO_EN | NORMAL_MODE_IE)
#define UART_TXD    (SLEEP_RET_OUT_H|SLEEP_RET_OUT_EN|UTILITY_IO_EN)
#define UART_RXD     (UTILITY_IO_EN|NORMAL_MODE_IE)
#define UART_CTS      (UTILITY_IO_EN |NORMAL_MODE_IE)
#define UART_RTS     (SLEEP_RET_OUT_H|SLEEP_RET_OUT_EN|UTILITY_IO_EN)
#define GENERAL_PURPOSE_IO     (NORMAL_MODE_IE)
#define GPIO_OUTPUT     (NORMAL_MODE_IE)
#define GPIO_INPUT      (NORMAL_MODE_IE)
#define GPIO_DISABLED (0)
#define EXTERNAL_INTR_IO (UTILITY_IO_EN|SLEEP_MODE_IE|NORMAL_MODE_IE)

#define PAD0_GPIO_VDD VDD_3V
#define PAD1_GPIO_VDD VDD_3V
#define PAD2_GPIO_VDD VDD_3V
#define PAD3_GPIO_VDD VDD_3V

#endif

