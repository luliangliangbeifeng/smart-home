/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* Kernel includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "queue.h"    /* RTOS queue related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */
#include "task.h"     /* RTOS task related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */

/* TODO Add any manufacture supplied header files can be included
here. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "nuclei_sdk_soc.h"
#include "nuclei_sdk_hal.h"

#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainEVENT_SEMAPHORE_TASK_PRIORITY (configMAX_PRIORITIES - 1)

#define NBIT_DEFAULT 3
#define REG_LEN 8

uint8_t txbuffer[7];
uint8_t rxbuffer[5];

uint8_t RH_result,T_result,L_result,led_result,motor_result,Ledstate,Motorstate;

__IO uint8_t txcount = 0; 

/* The period of the example software timer, specified in milliseconds, and
converted to ticks using the pdMS_TO_TICKS() macro. */
#define mainSOFTWARE_TIMER_PERIOD_MS pdMS_TO_TICKS(1000)

#define mainQUEUE_LENGTH (1)
static void prvSetupHardware(void);
extern void idle_task(void);
static void vExampleTimerCallback(TimerHandle_t xTimer);

/* The queue used by the queue send and queue receive tasks. */
static QueueHandle_t xQueue = NULL;

void prvSetupHardware(void)
{
}

TaskHandle_t LightTask_Handler;
TaskHandle_t UartTask1_Handler;
TaskHandle_t UartTask2_Handler;
TaskHandle_t Sht20Task_Handler;
TaskHandle_t LedTask_Handler;
TaskHandle_t MotorTask_Handler;

void light_task(void *pvParameters);
void uart_task1(void *pvParameters);
void uart_task2(void *pvParameters);
void sht20_task(void *pvParameters);
void led_task(void *pvParameters);
void motor_task(void *pvParameters);


void UART2_init(void);

int main(void)
{

    int32_t returnCode;

    LIGHT_init();

    SHT20_init();

    motor_init();

    UART2_init();

    gd_rvstar_led_init(LED1);

    gd_rvstar_led_init(LED2);

    gd_rvstar_led_init(LED3);



    returnCode = ECLIC_Register_IRQ(DMA0_Channel1_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
                    ECLIC_LEVEL_TRIGGER, 2, 0, NULL);
    returnCode = ECLIC_Register_IRQ(DMA0_Channel2_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
                    ECLIC_LEVEL_TRIGGER, 1, 0, NULL);


    __enable_irq();

    TimerHandle_t xExampleSoftwareTimer = NULL;

    /* Configure the system ready to run the demo.  The clock configuration
    can be done here if it was not done before main() was called. */
    prvSetupHardware();

    xQueue = xQueueCreate(/* The number of items the queue can hold. */
                          mainQUEUE_LENGTH,
                          /* The size of each item the queue holds. */
                          sizeof(uint32_t));

    if (xQueue == NULL) {
        for (;;) ;
    }
    xTaskCreate((TaskFunction_t)light_task, (const char *)"light_task",
                (uint16_t)256, (void *)NULL, (UBaseType_t)3,
                (TaskHandle_t *)&LightTask_Handler);

    xTaskCreate((TaskFunction_t)uart_task1, (const char *)"uart_task1",
                (uint16_t)256, (void *)NULL, (UBaseType_t)2,
                (TaskHandle_t *)&UartTask1_Handler);

    xTaskCreate((TaskFunction_t)uart_task2, (const char *)"uart_task2",
                (uint16_t)256, (void *)NULL, (UBaseType_t)4,
                (TaskHandle_t *)&UartTask2_Handler);

    xTaskCreate((TaskFunction_t)sht20_task, (const char *)"sht20_task",
                (uint16_t)256, (void *)NULL, (UBaseType_t)1,
                (TaskHandle_t *)&Sht20Task_Handler);
    xTaskCreate((TaskFunction_t)led_task, (const char *)"led_task",
                (uint16_t)256, (void *)NULL, (UBaseType_t)5,
                (TaskHandle_t *)&LedTask_Handler);
    xTaskCreate((TaskFunction_t)motor_task, (const char *)"motor_task",
                (uint16_t)256, (void *)NULL, (UBaseType_t)6,
                (TaskHandle_t *)&MotorTask_Handler);

    // xExampleSoftwareTimer =
    //     xTimerCreate((const char *)"LEDTimer", mainSOFTWARE_TIMER_PERIOD_MS,
    //                  pdTRUE, (void *)0, vExampleTimerCallback);

    //xTimerStart(xExampleSoftwareTimer, 0);
    printf("Before StartScheduler\r\n"); // Bob: I added it to here to easy
                                         // analysis

    vTaskStartScheduler();

    printf("post   ok \r\n");

    for (;;) {
        ;
    };
}

void light_task(void *pvParameters)
{
    TickType_t xNextWakeTime;
    printf("light_task\r\n");
    while (1) {
        while(SET != adc_flag_get(ADC0, ADC_FLAG_EOC));
        adc_flag_clear(ADC0, ADC_FLAG_EOC);

        L_result = ADC_RDATA(ADC0);
        printf("light: %d\r\n", L_result);
        printf("light_task_running..... \r\n");

        vTaskDelay(500);
    }
}

void sht20_task(void *pvParameters)
{
    printf("sht20_task\r\n");
    while(1) {
        RH_result = SHT20_Measure('H');
        T_result = SHT20_Measure('T');

        printf("Humidity: %d\r\n", RH_result);
        printf("Temperature: %d\r\n", T_result);
        printf("sht20_task_running..... \r\n");

        vTaskDelay(500);
    }
}

void led_task(void *pvParameters)
{
    printf("led_task\r\n");
    while(1) {
        led_result = !gpio_input_bit_get(GPIOA, GPIO_PIN_1);

        printf("led: %d\r\n", led_result);
        printf("led_task_running..... \r\n");

        vTaskDelay(500);
    }
}

void motor_task(void *pvParameters)
{
    printf("motor_task\r\n");
    while(1) {
    //    motor_result = !gpio_input_bit_get(GPIOA, GPIO_PIN_2);
        if (Motorstate == 1 ) motor_result = 1;
        else motor_result = 0;

        printf("motor: %d\r\n", motor_result);
        printf("motor_task_running..... \r\n");

        vTaskDelay(500);
    }
}

void uart_task1(void *pvParameters)
{
    uint32_t ulReceivedValue;
    printf("uart1_task\r\n");
    /* Initialise xNextWakeTime - this only needs to be done once. */

    while (1) {
        printf("uart_task1_running..... \r\n");

        txbuffer[0] = 0xaa;
        txbuffer[1] = 0xff;
        txbuffer[2] = RH_result;
        txbuffer[3] = T_result;
//        txbuffer[3] = 27;
        txbuffer[4] = L_result;
        txbuffer[5] = led_result;
        txbuffer[6] = motor_result;
        for(int m = 0; m < 7; m++) {
            printf("%d\n", txbuffer[m]); 
        }

        dma_channel_enable(DMA0, DMA_CH1);

    	vTaskDelay(500);
    }
}

void uart_task2(void *pvParameters)
{
    uint32_t ulReceivedValue;
    uint8_t i, head, find_head;

    printf("uart2_task\r\n");
    /* Initialise xNextWakeTime - this only needs to be done once. */

    while (1) {
        printf("uart_task2_running..... \r\n");

        for(i = 0; i < 5; i++) {
            if( rxbuffer[i % 5] == 0xaa && rxbuffer[( i + 1 ) % 5] == 0xbb && rxbuffer[( i + 2 ) % 5] == 0xcc) {
                printf("%d\r\n",rxbuffer[i % 5]);
                printf("%d\r\n",rxbuffer[( i + 1 ) % 5]);
                printf("%d\r\n",rxbuffer[( i + 2 ) % 5]);
                printf("%d\r\n",rxbuffer[( i + 3 ) % 5]);
                printf("%d\r\n",rxbuffer[( i + 4 ) % 5]);

                Ledstate = rxbuffer[(i + 3) % 5];
                Motorstate = rxbuffer[(i + 4) % 5];
                printf("Ledstate: %d\r\n", Ledstate);
                printf("Motorstate: %d\r\n", Motorstate);
            } 
        }
        if(Ledstate == 1) {
            gd_rvstar_led_on(LED1); // green
        } else {
            gd_rvstar_led_off(LED1); // green
        }
        if(Motorstate == 1) {
        //    gd_rvstar_led_on(LED3); //red
            pwmout(100);
        } else {
        //    gd_rvstar_led_off(LED3);
            pwmout(0);
        }
        dma_channel_enable(DMA0, DMA_CH2);
        vTaskDelay(500);
    }
}

// static void vExampleTimerCallback(TimerHandle_t xTimer)
// {
//      The timer has expired.  Count the number of times this happens.  The
//     timer that calls this function is an auto re-load timer, so it will
//     execute periodically. 

//     printf("timers Callback\r\n");
// }

void vApplicationTickHook(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    static uint32_t ulCount = 0;

    /* The RTOS tick hook function is enabled by setting configUSE_TICK_HOOK to
    1 in FreeRTOSConfig.h.

    "Give" the semaphore on every 500th tick interrupt. */


    /* If xHigherPriorityTaskWoken is pdTRUE then a context switch should
    normally be performed before leaving the interrupt (because during the
    execution of the interrupt a task of equal or higher priority than the
    running task was unblocked).  The syntax required to context switch from
    an interrupt is port dependent, so check the documentation of the port you
    are using.

    In this case, the function is running in the context of the tick interrupt,
    which will automatically check for the higher priority task to run anyway,
    so no further action is required. */
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
    /* The malloc failed hook is enabled by setting
    configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

    Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    printf("malloc failed\n");
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
    //  ( void ) pcTaskName;
    // ( void ) xTask;

    /* Run time stack overflow checking is performed if
    configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected.  pxCurrentTCB can be
    inspected in the debugger if the task name passed into this function is
    corrupt. */
    printf("Stack Overflow\n");
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

extern UBaseType_t uxCriticalNesting;
void vApplicationIdleHook(void)
{
    volatile size_t xFreeStackSpace;
    /* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
    FreeRTOSConfig.h.

    This function is called on each cycle of the idle task.  In this case it
    does nothing useful, other than report the amount of FreeRTOS heap that
    remains unallocated. */
    /* By now, the kernel has allocated everything it is going to, so
    if there is a lot of heap remaining unallocated then
    the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
    reduced accordingly. */
}
