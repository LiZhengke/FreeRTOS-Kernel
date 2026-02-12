/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 * This is a simple main that will start the FreeRTOS-Kernel and run a periodic task
 * that only delays if compiled with the template port, this project will do nothing.
 * For more information on getting started please look here:
 * https://www.freertos.org/Documentation/01-FreeRTOS-quick-start/01-Beginners-guide/02-Quick-start-guide
 */

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

/* Standard includes. */
#include <stdio.h>
#include "utils.h"
/*-----------------------------------------------------------*/

static void TaskMain( void * parameters ) __attribute__( ( noreturn ) );

/*-----------------------------------------------------------*/

static void TaskMain( void * parameters )
{
    unsigned long esp;
    unsigned long stack_low, stack_high;
    void *stack_base;
    unsigned long stack_size;
    /* Unused parameters. */
    ( void ) parameters;


    for( ; ; )
    {
        register unsigned int test asm("ebx") = 0x12345678;

        TickType_t tickCount = xTaskGetTickCount();
        esp = get_esp();
        printf("TaskMain: tick=%lu esp=%p ebx=%x\n", tickCount, (void*)esp, test);
        vDebugGetCurrentStackInfo(&stack_base, &stack_size);

        stack_low  = (unsigned long)stack_base;
        stack_high = stack_low + stack_size;
        if (esp < stack_low || esp > stack_high) {
            printf("STACK VIOLATION TaskMain! esp=%p range=[%p-%p]\n",
                (void*)esp,
                (void*)stack_low,
                (void*)stack_high);
        }
        vTaskDelay( 100 ); /* delay 100 ticks */
    }
}
/*-----------------------------------------------------------*/

int main( void )
{
    static StaticTask_t exampleTaskTCB;
    static StackType_t exampleTaskStack[ configMINIMAL_STACK_SIZE ];

    ( void ) puts( "i486 flat Project\n" );

    ( void ) xTaskCreateStatic( TaskMain,
                                "TaskMain",
                                configMINIMAL_STACK_SIZE,
                                NULL,
                                configMAX_PRIORITIES - 1U,
                                &( exampleTaskStack[ 0 ] ),
                                &( exampleTaskTCB ) );

    /* Start the scheduler. */
    vTaskStartScheduler();

    for( ; ; )
    {
        /* Should not reach here. */
    }

    return 0;
}
/*-----------------------------------------------------------*/

#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )

    void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName )
    {
        /* Check pcTaskName for the name of the offending task,
         * or pxCurrentTCB if pcTaskName has itself been corrupted. */
        ( void ) xTask;
        ( void ) pcTaskName;
    }

#endif /* #if ( configCHECK_FOR_STACK_OVERFLOW > 0 ) */
/*-----------------------------------------------------------*/
#if  ( configUSE_TICK_HOOK != 0 )
    void vApplicationTickHook( void )
    {
        /* This function will be called by each tick interrupt if
         * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. */
        TickType_t tickCount = xTaskGetTickCount();
        if( tickCount % 10 == 0 )  /* Print every 10 ticks. */
        {
            printf( "Tick Hook: %lu esp=%p\n", ( unsigned long ) tickCount, ( void * ) get_esp() );
        }
    }
#endif/* ( configUSE_TICK_HOOK != 0 ) */
/*-----------------------------------------------------------*/

#if ( configUSE_IDLE_HOOK == 1 )
    void vApplicationIdleHook( void )
    {
        /* This function will be called by the idle task if
         * configUSE_IDLE_HOOK is set to 1 in FreeRTOSConfig.h. */
        static TickType_t last = 0;
        TickType_t now = xTaskGetTickCount();
        unsigned long esp;
        void *stack_base;
        unsigned long stack_size;
        unsigned long stack_low, stack_high;

        if (now != last) {
            esp = get_esp();
            printf("Idle: tick=%lu esp=%p\n", now, (void*)esp);

            vDebugGetCurrentStackInfo(&stack_base, &stack_size);
            stack_low  = (unsigned long)stack_base;
            stack_high = stack_low + stack_size;

            if (esp < stack_low || esp > stack_high) {
                printf("STACK VIOLATION Idle! esp=%p range=[%p-%p]\n",
                    (void*)esp,
                    (void*)stack_low,
                    (void*)stack_high);
            }
            last = now;
        }
    }
#endif /* configUSE_IDLE_HOOK == 1 */
/*-----------------------------------------------------------*/

