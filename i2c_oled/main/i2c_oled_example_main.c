#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "driver/gpio.h"
#include "driver/twai.h" // CAN bus library
 
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#include "esp_lcd_sh1107.h"
#else
#include "esp_lcd_panel_vendor.h"
#endif
 
static const char *TAG = "example";
 
#define I2C_HOST  0
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define EXAMPLE_PIN_NUM_SDA           GPIO_NUM_21
#define EXAMPLE_PIN_NUM_SCL           GPIO_NUM_22
#define EXAMPLE_PIN_NUM_RST           -1
#define EXAMPLE_I2C_HW_ADDR           0x3C
 
// The pixel number in horizontal and vertical
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
#define EXAMPLE_LCD_H_RES              128
#define EXAMPLE_LCD_V_RES              64
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#define EXAMPLE_LCD_H_RES              64
#define EXAMPLE_LCD_V_RES              128
#endif
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8
 
#define ID_LX_BATTERY_VI 0x6 // Define your CAN message ID here
 
// CAN bus configuration
#define CAN_TX_GPIO_NUM GPIO_NUM_5
#define CAN_RX_GPIO_NUM GPIO_NUM_4
#define CAN_BUS_BITRATE 500000
 
extern void example_lvgl_demo_ui(lv_disp_t *disp);
 
/* The LVGL port component calls esp_lcd_panel_draw_bitmap API for send data to the screen. There must be called
lvgl_port_flush_ready(disp) after each transaction to display. The best way is to use on_color_trans_done
callback from esp_lcd IO config structure. In IDF 5.1 and higher, it is solved inside LVGL port component. */
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_t * disp = (lv_disp_t *)user_ctx;
    lvgl_port_flush_ready(disp);
    return false;
}
 
// Function to configure and start the CAN bus
void can_bus_config()
{
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO_NUM, CAN_RX_GPIO_NUM, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
 
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(TAG, "CAN bus started");
}
 
void example_lvgl_demo_ui(lv_disp_t *disp)
{
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
    lv_label_set_text(label, "PackCurrent:");
    /* Size of the screen (if you use rotation 90 or 270, please set disp->driver->ver_res) */
    lv_obj_set_width(label, disp->driver->hor_res);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
 
    // Create a label for displaying the CAN value
    lv_obj_t *can_value_label = lv_label_create(scr);
    lv_obj_align(can_value_label, LV_ALIGN_CENTER, 0, 0);  // Center align
 
    // Task to update the CAN value on the display
    while (1) {
        twai_message_t message;
 if (twai_receive(&message, pdMS_TO_TICKS(1000)) == ESP_OK)
              {
                  // ESP_LOGI(EXAMPLE_TAG, "After receive call with identifier: %lx", message.identifier);
                  // ESP_LOGI(EXAMPLE_TAG, "Received message with identifier: %lx, data: %x %x %x %x %x %x %x %x",
                  //         message.identifier,
                  //         message.data[0], message.data[1], message.data[2], message.data[3],
                  //         message.data[4], message.data[5], message.data[6], message.data[7]);
 
                  // if (MSG == message.identifier)
                  // {
                  //     twai_clear_receive_queue();  // Commented out to prevent clearing the queue
                  //     MSG = 0;
                  // }
 
                  // MSG = message.identifier;
                  //ESP_LOGI(EXAMPLE_TAG, "message id------> %lx",message.identifier);
                  printf("mtr decode =  %x , %x , %x, %x , %x, %x, %x, %x  \n", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]);
 
                //   if(message.data[0] == 0x15)
                //   {
                //     printf("<--------------------------------------------------MOTOR ON------------------------------------------------------------------->\n");
                //     Motor_status = 1;
                //   }
 
                //   else if (message.data[0]== 0x14 || message.data[0]== 0x4 )
                //   {
                //     printf("<--------------------------------------------------MOTOR OFF------------------------------------------------------------------->\n");
                //     Motor_status = 0;
                //   }
 
                   if (message.identifier == ID_LX_BATTERY_VI)
 
                  {
                   
 
                    if (!(message.rtr))
 
                    {
                      printf("Entered!\n");
                    //   current2_hx  =  (message.data[0] << 24) | (message.data[1] << 16) | (message.data[2] << 8) | message.data[3];
                    //   printf("current2_hx: %" PRIu32 "\n", current2_hx);
 
                       // Combining the bytes into a 32-bit integer
                        int32_t current2_hx = (int32_t)((message.data[0] << 24) | (message.data[1] << 16) | (message.data[2] << 8) | message.data[3]);
 
                        // Print the value
                        printf("current2_hx: %" PRId32 "\n", current2_hx);
                       
                      //Convert to float with the decimal point after the second digit
                        float value = (float)current2_hx / 1000.0;
 
                        // Print the value
                        printf("Original value: %" PRId32 "\n", current2_hx);
                        printf("Value with decimal point: %.3f\n", value);
                               
                //     union
 
                //       {
 
                //         uint32_t b;
 
                //         int f;
 
                //       } u; // crazy
 
                //       u.b = voltage2_hx;
 
                //       Voltage_2 = u.f;
 
                //       u.b = current2_hx;
 
                //       Current_2 = u.f;
 
                //   //      printf("Battery 2 --> Volatge[%d]   current[%f]   \n", ( Voltage_2),(Current_2*0.001));
             char can_value_str[15]; // Ensure the buffer is large enough to hold the formatted string
snprintf(can_value_str, sizeof(can_value_str), "%.3f", value); // Format the float with 3 decimal places
 
// Update label with CAN value
lv_label_set_text(can_value_label, can_value_str);
 
// Optionally print the value to the console for debugging
printf("Value with decimal point: %.3f\n", value);
               
                    }
               
                  }
               
            }
        }
 
        // Delay for a short time before reading CAN bus again (adjust as needed)
        vTaskDelay(pdMS_TO_TICKS(200));  // 200ms delay
    }
 
 
void app_main(void)
{
    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = EXAMPLE_PIN_NUM_SDA,
        .scl_io_num = EXAMPLE_PIN_NUM_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_HOST, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_HOST, I2C_MODE_MASTER, 0, 0, 0));
 
    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = EXAMPLE_I2C_HW_ADDR,
        .control_phase_bytes = 1,               // According to SSD1306 datasheet
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,   // According to SSD1306 datasheet
        .lcd_param_bits = EXAMPLE_LCD_CMD_BITS, // According to SSD1306 datasheet
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
        .dc_bit_offset = 6,                     // According to SSD1306 datasheet
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
        .dc_bit_offset = 0,                     // According to SH1107 datasheet
        .flags =
        {
            .disable_control_phase = 1,
        }
#endif
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_HOST, &io_config, &io_handle));
 
    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
    };
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1107(io_handle, &panel_config, &panel_handle));
#endif
 
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
 
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif
 
    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);
 
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
        .double_buffer = true,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        }
    };
    lv_disp_t * disp = lvgl_port_add_disp(&disp_cfg);
    /* Register done callback for IO */
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, disp);
 
    /* Rotation of the screen */
    lv_disp_set_rotation(disp, LV_DISP_ROT_NONE);
 
    ESP_LOGI(TAG, "Display LVGL UI");
    can_bus_config(); // Initialize the CAN bus
    example_lvgl_demo_ui(disp);
}
 