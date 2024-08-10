#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"

#define DEBUG_LVL   LVL_TRACE   /**< logging level */
#define MOD_NAME    "MAIN"      /**< module name */
#include "ez_logging.h"

#include "ez_easy_embedded.h"
#if (EZ_TASK_WORKER_ENABLE == 1)
#include "ez_task_worker.h"
#include "ez_freertos_port.h"
#else
#include "FreeRTOS.h"
#include "task.h"
#endif


#define BUFF_SIZE           256
#define PRIORITY            10
#define STACK_SIZE          512
#define WORKER1_DELAY_MS    10
#define WORKER2_DELAY_MS    2000

#define SUM_RESULT_EVENT    1

/******************************************************************************
* Module Typedefs
*******************************************************************************/
/* None */

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/

#if (EZ_TASK_WORKER_ENABLE == 1)
typedef struct
{
    int a;
    int b;
}Worker1_SumContext;

static struct ezTaskWorkerThreadInterfaces *freertos_interfaces = NULL;
static INIT_WORKER(worker1, WORKER1_DELAY_MS, PRIORITY, STACK_SIZE);
static INIT_WORKER(worker2, WORKER2_DELAY_MS, PRIORITY, STACK_SIZE);
static uint8_t buff2[BUFF_SIZE] = {0};
static uint8_t buff1[BUFF_SIZE] = {0};
#endif /* EZ_TASK_WORKER_ENABLE */

const uint LED_PIN = 25;

/******************************************************************************
* Function Definitions
*******************************************************************************/
#if (EZ_TASK_WORKER_ENABLE == 1)
INIT_THREAD_FUNCTIONS(worker1);
INIT_THREAD_FUNCTIONS(worker2);

static bool worker1_SumService(int a, int b,  ezTaskWorkerCallbackFunc callback);
static bool worker1_HandleSumService(void *context, ezTaskWorkerCallbackFunc callback);
static void worker2_Callback(uint8_t event, void *ret_data);
#else
void vTask1()
{
    while(1)
    {
        EZDEBUG("Task1");
        vTaskDelay(500);
    }
}

void vTask2()
{
    while(1)
    {
        EZDEBUG("Task2");
        vTaskDelay(1000);
    }
}
#endif /* EZ_TASK_WORKER_ENABLE */

int main()
{
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    ezEasyEmbedded_Initialize();

#if (EZ_TASK_WORKER_ENABLE == 1)
    bool ret = false;
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
#else
    xTaskCreate(vTask1, "vTask1", 128, NULL, 1, NULL);
    xTaskCreate(vTask2, "vTask2", 128, NULL, 1, NULL);
    vTaskStartScheduler();
#endif
}


/******************************************************************************
* Internal functions
*******************************************************************************/
#if (EZ_TASK_WORKER_ENABLE == 1)
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
        EZINFO("Unknown event");
        break;
    }
}
#endif

/* End of file */
