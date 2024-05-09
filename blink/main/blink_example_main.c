#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/uart.h"
 
#define BLINK_GPIO 5
#define ECHO_TEST_TXD  1
#define ECHO_TEST_RXD  3
#define ECHO_UART_PORT_NUM      UART_NUM_1
#define ECHO_UART_BAUD_RATE     115200
#define ECHO_TASK_STACK_SIZE    2048
 
static const char *TAG = "example";
 
void configure_uart(void) {
    const uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(ECHO_UART_PORT_NUM, &uart_config);
    uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(ECHO_UART_PORT_NUM, 256 * 2, 0, 0, NULL, 0);
}
 
static void blink_led(int state) {
    gpio_set_level(BLINK_GPIO, state);
}
 
void app_main(void) {
    configure_uart();
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
 
    uint8_t data[1];
    while (1) {
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, sizeof(data), 20 / portTICK_PERIOD_MS);
        if (len > 0) {
            ESP_LOGI(TAG, "Received: %d", data[0]);
            if (data[0] == '1') {
                blink_led(1); // Turn ON the LED
            } else if (data[0] == '0') {
                blink_led(0); // Turn OFF the LED
            }
        }
    }
}
 