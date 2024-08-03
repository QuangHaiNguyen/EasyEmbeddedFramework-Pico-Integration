#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"
#include "ez_easy_embedded.h"

#define DEBUG_LVL   LVL_TRACE      /**< logging level */
#define MOD_NAME    "MAIN"  /**< module name */
#include "ez_logging.h"

const uint LED_PIN = 25;

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1


int main()
{
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    ezEasyEmbedded_Initialize();

    while (1) {
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
        gpio_put(LED_PIN, 1);
        EZDEBUG("Hello world\n");
        sleep_ms(1000);
    }
}