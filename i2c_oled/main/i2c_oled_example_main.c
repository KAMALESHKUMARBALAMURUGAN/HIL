#include <stdio.h>                           // Standard I/O library
#include "freertos/FreeRTOS.h"               // FreeRTOS main header
#include "freertos/task.h"                   // FreeRTOS task management
#include "esp_timer.h"                       // ESP32 timer API
#include "esp_lcd_panel_io.h"                // LCD panel I/O operations
#include "esp_lcd_panel_ops.h"               // LCD panel operations
#include "driver/i2c.h"                      // I2C driver
#include "esp_err.h"                         // ESP32 error codes
#include "esp_log.h"                         // ESP32 logging
#include "lvgl.h"                            // LVGL graphics library
#include "esp_lvgl_port.h"                   // ESP32 LVGL port
#include "driver/gpio.h"                     // GPIO driver
#include "driver/twai.h"                     // CAN bus driver

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#include "esp_lcd_sh1107.h"                  // SH1107 LCD driver
#else
#include "esp_lcd_panel_vendor.h"            // Other LCD vendor drivers
#endif

static const char *TAG = "example";          // Log tag for this module

#define I2C_HOST  0                          // I2C host number
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ    (400 * 1000) // LCD pixel clock speed
#define EXAMPLE_PIN_NUM_SDA           GPIO_NUM_21  // I2C SDA pin
#define EXAMPLE_PIN_NUM_SCL           GPIO_NUM_22  // I2C SCL pin
#define EXAMPLE_PIN_NUM_RST           -1           // LCD reset pin (-1 if not used)
#define EXAMPLE_I2C_HW_ADDR           0x3C         // I2C address of the LCD

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
#define EXAMPLE_LCD_H_RES              128        // Horizontal resolution for SSD1306
#define EXAMPLE_LCD_V_RES              64         // Vertical resolution for SSD1306
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#define EXAMPLE_LCD_H_RES              64         // Horizontal resolution for SH1107
#define EXAMPLE_LCD_V_RES              128        // Vertical resolution for SH1107
#endif

#define EXAMPLE_LCD_CMD_BITS           8          // Number of bits for LCD command
#define EXAMPLE_LCD_PARAM_BITS         8          // Number of bits for LCD parameters

#define ID_LX_BATTERY_VI 0x6 // Define your CAN message ID here
#define ID_LX_BATTERY_SOC 0x8 // Define your CAN message ID here
#define MotorRpm 0x14520902  //For motor rpm

#define CAN_TX_GPIO_NUM GPIO_NUM_5      // CAN bus TX pin
#define CAN_RX_GPIO_NUM GPIO_NUM_4      // CAN bus RX pin
#define CAN_BUS_BITRATE 500000          // CAN bus bitrate

extern void example_lvgl_demo_ui(lv_disp_t *disp); // External function declaration

// Callback function to notify LVGL that flush is ready
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_t * disp = (lv_disp_t *)user_ctx;  // Cast user context to LVGL display
    lvgl_port_flush_ready(disp);               // Notify LVGL that flush is ready
    return false;                              // Return false to indicate no further action needed
}

// Function to configure and start the CAN bus
void can_bus_config()
{
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO_NUM, CAN_RX_GPIO_NUM, TWAI_MODE_NORMAL); // General CAN config
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); // Timing config for 500kbps
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL(); // Accept all filter config

    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config)); // Install CAN driver
    ESP_ERROR_CHECK(twai_start()); // Start CAN driver
    ESP_LOGI(TAG, "CAN bus started"); // Log that CAN bus has started
}

// Function to setup LVGL UI and display CAN data
void example_lvgl_demo_ui(lv_disp_t *disp)
{
    lv_obj_t *scr = lv_disp_get_scr_act(disp);    // Get active screen
    lv_obj_t *label = lv_label_create(scr);       // Create a label
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR); // Set label scroll mode to circular
    lv_label_set_text(label, "RPM");     // Set label text
    lv_obj_set_width(label, disp->driver->hor_res); // Set label width
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0); // Align label to top middle

    lv_obj_t *can_value_label = lv_label_create(scr); // Create a label for CAN value
    lv_obj_align(can_value_label, LV_ALIGN_TOP_RIGHT, 0, 0); // Center align CAN value label


    lv_obj_t *speed_label = lv_label_create(scr);
    lv_label_set_text(speed_label, "Speed:"); // Initial text
    lv_obj_align(speed_label, LV_ALIGN_BOTTOM_MID, 0, 0); // Position below the RPM label


    while (1) { // Infinite loop to read CAN messages
        twai_message_t message; // Define CAN message structure
        if (twai_receive(&message, pdMS_TO_TICKS(500)) == ESP_OK) // Receive CAN message with 1 second timeout
        {
            printf("mtr decode =  %x , %x , %x, %x , %x, %x, %x, %x  \n", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]); // Print received data

            // if (message.identifier == ID_LX_BATTERY_VI) // Check if message ID matches
            // {
            //     if (!(message.rtr)) // Check if not a remote transmission request
            //     {
            //         int32_t current2_hx = (int32_t)((message.data[0] << 24) | (message.data[1] << 16) | (message.data[2] << 8) | message.data[3]); // Combine bytes into 32-bit integer
            //         printf("current2_hx: %" PRId32 "\n", current2_hx); // Print integer value

            //         float value = (float)current2_hx / 1000.0; // Convert to float with 3 decimal places
            //         printf("Original value: %" PRId32 "\n", current2_hx); // Debug message
            //         printf("Value with decimal point: %.3f\n", value); // Print float value

            //         char can_value_str[15]; // Buffer for formatted string
            //         snprintf(can_value_str, sizeof(can_value_str), "%.3f", value); // Format float value as string

            //         lv_label_set_text(can_value_label, can_value_str); // Update label with CAN value
            //         printf("Value with decimal point: %.3f\n", value); // Debug message
            //     }
            // }

            // if (message.identifier == ID_LX_BATTERY_SOC) // Check if message ID matches
            // {
            //     if (!(message.rtr)) // Check if not a remote transmission request
            //     {
            //         uint8_t hex_value = message.data[0]; // Extract the value from message.data[0]
            //         int soc_value = (int)hex_value; // Convert hexadecimal to decimal
            //         printf("SOC Value (Hex): 0x%02X\n", hex_value); // Debug message for hex value
            //         printf("SOC Value (Decimal): %d\n", soc_value); // Print decimal value

            //         // You can now use soc_value as needed
            //         // For example, update an LVGL label or perform other operations
            //         char soc_value_str[10]; // Buffer for formatted string
            //         snprintf(soc_value_str, sizeof(soc_value_str), "%d", soc_value); // Format decimal value as string

            //         lv_label_set_text(can_value_label, soc_value_str); // Update label with SOC value
            //     }
            // }

            if (message.identifier == MotorRpm) // Check if message ID matches
            {
             if (!(message.rtr)) // Check if not a remote transmission request
                {
                                    // Extract the hexadecimal RPM bytes from the received message
                    uint8_t rpm1_hex = message.data[1]; // Most significant byte
                    printf("1---------->%u\n", rpm1_hex);
                    uint8_t rpm2_hex = message.data[0]; // Least significant byte
                    printf("2---------->%u\n", rpm2_hex);

                    // Combine the bytes to form the original RPM value in hexadecimal
                    uint16_t rpm_hex = (rpm1_hex << 8) | rpm2_hex;
                    printf("Combined Hex RPM---------->%04X\n", rpm_hex); // Print as hexadecimal

                    // Convert the combined hexadecimal RPM value to a decimal value
                    uint16_t rpm_dec = (uint16_t)rpm_hex;
                    printf("Decimal RPM---------->%u\n", rpm_dec);

                    // Buffer for formatted RPM string
                    char rpm_value_str[10];

                    // Format decimal RPM value as a string
                    snprintf(rpm_value_str, sizeof(rpm_value_str), "%u", rpm_dec);

                    // Update the LVGL label with the RPM value
                    lv_label_set_text(can_value_label, rpm_value_str);

                    // ESP_LOGI(EXAMPLE_TAG, "Received RPM: %u", rpm_dec);
                    // Calculate speed
                    float speed = rpm_dec * 0.0875;
                    char speed_value_str[10];

                    // Format the speed value as a string with 2 decimal places
                    snprintf(speed_value_str, sizeof(speed_value_str), "%.2f", speed);

                    // Update the LVGL label with the speed value
                    lv_label_set_text(speed_label, speed_value_str);

                }   
            }


        }

       // vTaskDelay(pdMS_TO_TICKS(200)); // Delay for 200ms before next read
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initialize I2C bus"); // Log message
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER, // Set I2C mode to master
        .sda_io_num = EXAMPLE_PIN_NUM_SDA, // Set SDA pin
        .scl_io_num = EXAMPLE_PIN_NUM_SCL, // Set SCL pin
        .sda_pullup_en = GPIO_PULLUP_ENABLE, // Enable pull-up for SDA
        .scl_pullup_en = GPIO_PULLUP_ENABLE, // Enable pull-up for SCL
        .master.clk_speed = EXAMPLE_LCD_PIXEL_CLOCK_HZ, // Set clock speed
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_HOST, &i2c_conf)); // Configure I2C parameters
    ESP_ERROR_CHECK(i2c_driver_install(I2C_HOST, I2C_MODE_MASTER, 0, 0, 0)); // Install I2C driver

    ESP_LOGI(TAG, "Install panel IO"); // Log message
    esp_lcd_panel_io_handle_t io_handle = NULL; // Declare IO handle
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = EXAMPLE_I2C_HW_ADDR, // Set device address
        .control_phase_bytes = 1, // Set control phase bytes
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS, // Set command bits
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS, // Set parameter bits
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
        .dc_bit_offset = 6, // Set DC bit offset for SSD1306
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
        .dc_bit_offset = 0, // Set DC bit offset for SH1107
        .flags =
        {
            .disable_control_phase = 1, // Disable control phase
        }
#endif
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_HOST, &io_config, &io_handle)); // Create new panel IO

    ESP_LOGI(TAG, "Install SSD1306 panel driver"); // Log message
    esp_lcd_panel_handle_t panel_handle = NULL; // Declare panel handle
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1, // Set bits per pixel
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST, // Set reset GPIO number
    };
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle)); // Create new SSD1306 panel
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1107(io_handle, &panel_config, &panel_handle)); // Create new SH1107 panel
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle)); // Reset panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle)); // Initialize panel
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true)); // Turn display on

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true)); // Invert colors for SH1107
#endif

    ESP_LOGI(TAG, "Initialize LVGL"); // Log message
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG(); // LVGL port config
    lvgl_port_init(&lvgl_cfg); // Initialize LVGL port

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle, // Set IO handle
        .panel_handle = panel_handle, // Set panel handle
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES, // Set buffer size
        .double_buffer = true, // Enable double buffering
        .hres = EXAMPLE_LCD_H_RES, // Set horizontal resolution
        .vres = EXAMPLE_LCD_V_RES, // Set vertical resolution
        .monochrome = true, // Set to monochrome
        .rotation = {
            .swap_xy = false, // Do not swap X and Y
            .mirror_x = false, // Do not mirror X
            .mirror_y = false, // Do not mirror Y
        }
    };
    lv_disp_t * disp = lvgl_port_add_disp(&disp_cfg); // Add LVGL display
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready, // Register flush ready callback
    };
    esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, disp); // Register event callbacks

    lv_disp_set_rotation(disp, LV_DISP_ROT_NONE); // Set display rotation

    ESP_LOGI(TAG, "Display LVGL UI"); // Log message
    can_bus_config(); // Initialize the CAN bus
    example_lvgl_demo_ui(disp); // Call UI function to display CAN data
}
