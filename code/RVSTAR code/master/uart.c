#include <gd32vf103v_rvstar.h>

extern uint8_t txbuffer[7];
extern uint8_t rxbuffer[5];

void UART2_init(void);
void DMA0_Channel1_IRQHandler(void);

void UART2_init(void)
{
    
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART2);
    rcu_periph_clock_enable(RCU_AF);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, 115200U);
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);
    usart_parity_config(USART2, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART2, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART2, USART_CTS_DISABLE);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);

    dma_parameter_struct dma_init_struct;
    
    /* tx */
    /* enable DMA0 clock */
    rcu_periph_clock_enable(RCU_DMA0);    
    /* deinitialize DMA channel1(USART0 tx) */
    dma_deinit(DMA0, DMA_CH1);
    dma_struct_para_init(&dma_init_struct);
    
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = (uint32_t)txbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 7;
    dma_init_struct.periph_addr = (uint32_t)&USART_DATA(USART2);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH1, &dma_init_struct);
  
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH1);
    dma_memory_to_memory_disable(DMA0, DMA_CH1);

    /* USART DMA0 enable for transmission */
    usart_dma_transmit_config(USART2, USART_DENT_ENABLE);
    /* enable DMA0 channel1 transfer complete interrupt */
    dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF);

    /* rx */
    /* enable DMA0 clock */
    rcu_periph_clock_enable(RCU_DMA0);
    /* deinitialize DMA channel2 (USART0 rx) */
    dma_deinit(DMA0, DMA_CH2);
    dma_struct_para_init(&dma_init_struct);

    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)rxbuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 5;
    dma_init_struct.periph_addr = (uint32_t)&USART_DATA(USART2);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH2, &dma_init_struct);
  
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH2);
    dma_memory_to_memory_disable(DMA0, DMA_CH2);
    
    /* USART DMA0 enable for reception */
    usart_dma_receive_config(USART2, USART_DENR_ENABLE);
    /* enable DMA0 channel2 transfer complete interrupt */
    dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF);
    /* enable DMA0 channel2 */
    dma_channel_enable(DMA0, DMA_CH2); 
}

void DMA0_Channel1_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA0, DMA_CH1, DMA_INT_FLAG_FTF)){
        dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_G);
        dma_channel_disable(DMA0, DMA_CH1);
        dma_parameter_struct dma_init_struct;
        /* enable DMA0 clock */
        rcu_periph_clock_enable(RCU_DMA0);
    
        /* deinitialize DMA channel1(USART0 tx) */
        dma_deinit(DMA0, DMA_CH1);
        dma_struct_para_init(&dma_init_struct);
    
        dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
        dma_init_struct.memory_addr = (uint32_t)txbuffer;
        dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
        dma_init_struct.number = 7;
        dma_init_struct.periph_addr = (uint32_t)&USART_DATA(USART2);
        dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
        dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
        dma_init(DMA0, DMA_CH1, &dma_init_struct);
  
        /* configure DMA mode */
        dma_circulation_disable(DMA0, DMA_CH1);
        dma_memory_to_memory_disable(DMA0, DMA_CH1);

        /* USART DMA0 enable for transmission */
        usart_dma_transmit_config(USART2, USART_DENT_ENABLE);
        /* enable DMA0 channel1 transfer complete interrupt */
        dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF);
    }
}

void DMA0_Channel2_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA0, DMA_CH2, DMA_INT_FLAG_FTF)) {
        dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_G);
        dma_channel_disable(DMA0, DMA_CH2);

        dma_parameter_struct dma_init_struct;
        /* enable DMA0 clock */
        rcu_periph_clock_enable(RCU_DMA0);
        /* deinitialize DMA channel4 (USART0 rx) */
        dma_deinit(DMA0, DMA_CH2);
        dma_struct_para_init(&dma_init_struct);

        dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
        dma_init_struct.memory_addr = (uint32_t)rxbuffer;
        dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
        dma_init_struct.number = 5;
        dma_init_struct.periph_addr = (uint32_t)&USART_DATA(USART2);
        dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dma_init_struct.memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
        dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
        dma_init(DMA0, DMA_CH2, &dma_init_struct);
  
        /* configure DMA mode */
        dma_circulation_disable(DMA0, DMA_CH2);
        dma_memory_to_memory_disable(DMA0, DMA_CH2);
    
        /* USART DMA0 enable for reception */
        usart_dma_receive_config(USART2, USART_DENR_ENABLE);
        /* enable DMA0 channel4 transfer complete interrupt */
        dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF);
    }
}