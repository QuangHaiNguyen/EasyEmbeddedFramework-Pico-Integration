#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"

#define DEBUG_LVL   LVL_TRACE      /**< logging level */
#define MOD_NAME    "MAIN"  /**< module name */
#include "ez_logging.h"

#include "ez_easy_embedded.h"
#include "ez_task_worker.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ez_freertos_port.h"

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define BUFF_SIZE   256
#define PRIORITY    10
#define STACK_SIZE  512

#define SUM_RESULT_EVENT    1

/******************************************************************************
* Module Typedefs
*******************************************************************************/
/* None */

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/

typedef struct
{
    int a;
    int b;
}Worker1_SumContext;

static struct ezTaskWorkerThreadInterfaces *freertos_interfaces = NULL;
static INIT_WORKER(worker1, 10, PRIORITY, STACK_SIZE);
static INIT_WORKER(worker2, 2000, PRIORITY, STACK_SIZE);
static uint8_t buff2[BUFF_SIZE] = {0};
static uint8_t buff1[BUFF_SIZE] = {0};

const uint LED_PIN = 25;

/******************************************************************************
* Function Definitions
*******************************************************************************/
INIT_THREAD_FUNCTIONS(worker1);
INIT_THREAD_FUNCTIONS(worker2);

static bool worker1_SumService(int a, int b,  ezTaskWorkerCallbackFunc callback);
static bool worker1_HandleSumService(void *context, ezTaskWorkerCallbackFunc callback);
static void worker2_Callback(uint8_t event, void *ret_data);



int main()
{
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    bool ret = false;

    ezEasyEmbedded_Initialize();
    ezFreeRTOSPort_Init();
    freertos_interfaces = ezFreeRTOSPort_GetInterface();
    ret = (freertos_interfaces != NULL);
    if(ret == true)
    {
        ret = ezTaskWorker_SetRtosInterface(freertos_interfaces);
        if(ret == false)
        {
            EZERROR("Set interface failed");
        }
    }

    if(ret == true)
    {
        ret = ezTaskWorker_CreateWorker(&worker1,
                                        buff1,
                                        BUFF_SIZE,
                                        GET_THREAD_FUNC(worker1));

        ret &= ezTaskWorker_CreateWorker(&worker2,
                                         buff2,
                                         BUFF_SIZE,
                                         GET_THREAD_FUNC(worker2));
    }
    vTaskStartScheduler();
}


/******************************************************************************
* Internal functions
*******************************************************************************/

THREAD_FUNC(worker1)
{
    ezTaskWorker_ExecuteTask(&worker1, EZ_THREAD_WAIT_FOREVER);
}

THREAD_FUNC(worker2)
{
    int a = rand() % 255;
    int b = rand() % 255;

    EZINFO("Request worker1 to calculate the sum of %d and %d", a, b);
    bool ret = worker1_SumService(a, b, worker2_Callback);
    if(ret == true)
    {
        EZINFO("Call sum service success");
    }
}


static bool worker1_SumService(int a, int b, ezTaskWorkerCallbackFunc callback)
{
    bool ret = false;
    Worker1_SumContext context;
    context.a = a;
    context.b = b;

    ret = ezTaskWorker_EnqueueTask(&worker1,
                                   worker1_HandleSumService,
                                   callback,
                                   (void*)&context,
                                   sizeof(context),
                                   EZ_THREAD_WAIT_FOREVER);
    return ret;
}

static bool worker1_HandleSumService(void *context, ezTaskWorkerCallbackFunc callback)
{
    bool ret = false;
    int sum = 0;
    Worker1_SumContext *sum_context = (Worker1_SumContext *)context;

    if(sum_context != NULL && callback != NULL)
    {
        EZINFO("Process the request. Calculate sum of %d and %d",
               sum_context->a, sum_context->b);

        sum = sum_context->a + sum_context->b;
        callback(SUM_RESULT_EVENT, &sum);
        ret = true;
    }

    return ret;
}

static void worker2_Callback(uint8_t event, void *ret_data)
{
    switch (event)
    {
    case SUM_RESULT_EVENT:
        if(ret_data != NULL)
        {
            EZINFO("sum = %d", *(int*)ret_data);
        }
        break;
    
    default:
        break;
    }
}
