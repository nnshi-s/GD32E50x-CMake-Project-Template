/*!
    \file    main.c
    \brief   transmit/receive data using DMA interrupt

    \version 2020-12-09, V1.0.0, firmware for GD32E50x
    \version 2021-03-23, V1.2.0, firmware for GD32E50x
*/

/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include <stdio.h>
#include "gd32e50x.h"
  
#define USART5_RDATA_ADDRESS      (uint32_t)&USART5_RDATA(USART5)
#define USART5_TDATA_ADDRESS      (uint32_t)&USART5_TDATA(USART5)
#define ARRAYNUM(arr_nanme)       (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))

__IO FlagStatus g_transfer_complete = RESET;
uint8_t rxbuffer[10];
uint8_t txbuffer[] = "\n\rUSART DMA interrupt receive and transmit example, please input 10 bytes:\n\r";

void usart5_config(void);
void nvic_config(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    dma_parameter_struct dma_init_struct;
    /* enable DMA1 clock */
    rcu_periph_clock_enable(RCU_DMA1);
    /* initilize the com */
    usart5_config();
    /*configure DMA1 interrupt*/
    nvic_config();

    /* initialize DMA channel4 */
    dma_deinit(DMA1, DMA_CH4);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)txbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = ARRAYNUM(txbuffer);
    dma_init_struct.periph_addr = USART5_TDATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA1, DMA_CH4, &dma_init_struct);

    /* initialize DMA channel2 */
    dma_deinit(DMA1, DMA_CH2);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)rxbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 10;
    dma_init_struct.periph_addr = USART5_RDATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA1, DMA_CH2, &dma_init_struct);

    /* configure DMA mode */
    dma_circulation_disable(DMA1, DMA_CH4);
    dma_memory_to_memory_disable(DMA1, DMA_CH4);
    dma_circulation_disable(DMA1, DMA_CH2);
    dma_memory_to_memory_disable(DMA1, DMA_CH2);

    /* USART DMA enable for reception */
    usart_dma_receive_config(USART5, USART_DENR_ENABLE);
    /* enable DMA channel2 transfer complete interrupt */
    dma_interrupt_enable(DMA1, DMA_CH2, DMA_INT_FTF);
    /* enable DMA channel2 */
    dma_channel_enable(DMA1, DMA_CH2);
    /* USART DMA enable for transmission */
    usart_dma_transmit_config(USART5, USART_DENT_ENABLE);
    /* enable DMA channel4 transfer complete interrupt */
    dma_interrupt_enable(DMA1, DMA_CH4, DMA_INT_FTF);
    /* enable DMA channel4 */
    dma_channel_enable(DMA1, DMA_CH4);

    /* waiting for the transfer to complete*/
    while(RESET == g_transfer_complete);

    g_transfer_complete = RESET;

    /* waiting for the transfer to complete*/
    while(RESET == g_transfer_complete);

    printf("\n\r%s\n\r", rxbuffer);
    while(1);
}

/*!
    \brief      configure USART5
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usart5_config(void)
{
    /* enable USART5 GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);
    /* enable AF clock */
    rcu_periph_clock_enable(RCU_AF);
    /* enable USART5 clock */
    rcu_periph_clock_enable(RCU_USART5);

    /* connect port to USART5_Tx */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    /* connect port to USART5_Rx */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_7);

    /* configure USART5 Tx Rx as alternate function */
    gpio_afio_port_config(AFIO_PC6_USART5_CFG, ENABLE);
    gpio_afio_port_config(AFIO_PC7_USART5_CFG, ENABLE);

    /* USART configure */
    usart_deinit(USART5);
    usart_baudrate_set(USART5, 115200U);
    usart_receive_config(USART5, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART5, USART_TRANSMIT_ENABLE);

    usart_enable(USART5);
}

/*!
    \brief      configure DMA interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nvic_config(void)
{
    /*configure DMA1_CH2 interrupt*/
    nvic_irq_enable(DMA1_Channel2_IRQn, 0U, 0U);

#if (defined(GD32E50X_HD) || defined(GD32E50X_XD))
    /*configure DMA1_CH4 interrupt*/
    nvic_irq_enable(DMA1_Channel3_Channel4_IRQn, 2U, 0U);
#elif (defined(GD32E50X_CL))
    /*configure DMA1_CH4 interrupt*/
    nvic_irq_enable(DMA1_Channel4_IRQn, 2U, 0U);
#endif
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART5, (uint8_t)ch);
    while(RESET == usart5_flag_get(USART5, USART5_FLAG_TBE));
    return ch;
}