#ifndef SPI_TEST_H_
#define SPI_TEST_H_

void spim1_init(void);

void spis_init(void);

void spim_spis_test(void (*spim_cb)(uint8_t,uint8_t),void (*spis_cb)(uint8_t,uint8_t));


#endif
