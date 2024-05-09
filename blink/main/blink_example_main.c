#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/uart.h"
 
#define LED1_GPIO 5
#define LED2_GPIO 18
#define LED3_GPIO 19
#define ECHO_TEST_TXD  1
#define ECHO_TEST_RXD  3
#define ECHO_UART_PORT_NUM      UART_NUM_1
#define ECHO_UART_BAUD_RATE     115200
#define ECHO_TASK_STACK_SIZE    2048
 
static const char *TAG = "LED_Control";
 
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
 
void blink_led(int gpio_pin, int state) {
    gpio_reset_pin(gpio_pin);
    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_pin, state);
    ESP_LOGI(TAG, "LED on GPIO %d set to %d", gpio_pin, state);
}
 
void app_main(void) {
    configure_uart();
    uint8_t data[2];  // Assuming two byte commands
    while (1) {
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, sizeof(data), 20 / portTICK_PERIOD_MS);
        if (len == 2) {
            ESP_LOGI(TAG, "Received Command: %d%d", data[0] - '0', data[1] - '0');
            int led_num = data[0] - '0';
            int led_state = data[1] - '0';
            switch(led_num) {
                case 1:
                    blink_led(LED1_GPIO, led_state);
                    break;
                case 2:
                    blink_led(LED2_GPIO, led_state);
                    break;
                case 3:
                    blink_led(LED3_GPIO, led_state);
                    break;
            }
        }
    }
}
