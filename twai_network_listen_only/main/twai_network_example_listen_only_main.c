 /*This example code is in the Public Domain (or CC0 licensed, at your option.)
 
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/uart.h" 
#include "sdkconfig.h"
#include "esp_log.h"
#include "cJSON.h"
#include "esp_adc_cal.h"
#include "driver/twai.h"
#include "esp_timer.h"
 
#define BLINK_GPIO 5
#define ECHO_TEST_TXD  1
#define ECHO_TEST_RXD  3
#define ECHO_UART_PORT_NUM      UART_NUM_1
#define ECHO_UART_BAUD_RATE     115200
#define ECHO_TASK_STACK_SIZE    2048
 
static const char *TAG = "example";
 
 
// #define Ignition 14
#define Reverse 12
#define Break 15
#define ModeL 18
#define ModeR 13
#define SideStand 16
 
#define temp5 23 // adc
#define temp6 25 // adc
#define temp7 26 // adc
#define temp8 27 // adc
 
int ingi =0;
int modeL = 0;
int modeR = 0;
 
int brake = 0;
int reve = 0;
int sidestand = 0 ;
static int received_value_ignition;
static int received_value_brake;
static int received_value_reverse;
static int received_value_modeR;
static int received_value_modeL;
static int received_value_sidestand;
static int received_value_soc;
static int received_value_batt_tmp;
static int received_value_throttle;
static int received_value_motorTemp;
static int received_value_pcbTemp;
static int received_value_controllerTemp;
static int received_value_rpm;
static int rpm1, rpm2;
static char rpm1_hex[10], rpm2_hex[10];
static char rpm_hex[10];

static int thr_per;
static int batt_tmp;
static int V_motor_out;
static int pcb;
static float cnt_tmp;
static int rpm;
// static int 


static int soc;


/* --------------------- Definitions and static variables ------------------ */
// Example Configuration
#define PING_PERIOD_MS 250
#define NO_OF_DATA_MSGS 50
#define NO_OF_ITERS 3
#define ITER_DELAY_MS 500
#define RX_TASK_PRIO 9
#define TX_TASK_PRIO 11
#define CTRL_TSK_PRIO 8
#define TX_GPIO_NUM CONFIG_EXAMPLE_TX_GPIO_NUM
#define RX_GPIO_NUM CONFIG_EXAMPLE_RX_GPIO_NUM
#define EXAMPLE_TAG "TWAI Master"
 
#define ID_MASTER_STOP_CMD 0x0A0
#define ID_MASTER_START_CMD 0x0A1
#define ID_MASTER_PING 0x0A2
#define ID_SLAVE_STOP_RESP 0x0B0
#define ID_SLAVE_DATA 0x0B1
#define ID_SLAVE_PING_RESP 0x0B2
 
#define ID_BATTERY_3_VI 0x1725
#define ID_BATTERY_2_VI 0x1bc5
#define ID_BATTERY_1_VI 0x1f05
 
#define ID_BATTERY_3_SOC 0x1727
#define ID_BATTERY_2_SOC 0x1bc7
#define ID_BATTERY_1_SOC 0x1f07
 
#define ID_LX_BATTERY_VI 0x6
#define ID_LX_BATTERY_T 0xa
#define ID_LX_BATTERY_SOC 0x8
#define ID_LX_BATTERY_PROT 0x9
 
 
 
#define ID_MOTOR_RPM 0x230
#define ID_MOTOR_TEMP 0x233
#define ID_MOTOR_CURR_VOLT 0x32
#define ADC_WIDTH_BIT_DEFAULT (ADC_WIDTH_BIT_9 - 1)
 
 
static float Voltage_1 = 0;
static float Current_1 = 0;
static float SOC_1 = 0;
static float SOH_1 = 0;
 
static int Voltage_2 = 0;
static int Current_2 = 0;
static float SOC_2 = 0;
static float SOH_2 = 0;
 
static float Voltage_3 = 0;
static float Current_3 = 0;
static int SOC_3 = 0;
static int SOH_3 = 0;
 
static uint16_t iDs[ 2 ] = {0x1f00,0x1bc0,0x1720};
 
// static uint16_t iDs[2] = {0x1f00, 0x1bc0};
 
static uint8_t state = 0;
static int M_CONT_TEMP = 0;
static int M_MOT_TEMP = 0;
static int M_THROTTLE = 0;
 
static int M_AC_CURRENT = 0;
static int M_AC_VOLTAGE = 0;
static int M_DC_CURRENT = 0;
static int M_DC_VOLATGE = 0;
 
static int S_DC_CURRENT = 0;
static int S_AC_CURRENT1 = 0;
static int S_AC_CURRENT2 = 0;
 
static float TRIP_1 = 0;
static float TRIP1 = 0;
 
static int t_stamp = 0;
int adc_value = 0;
int adc_value1 = 0;
int adc_value2 = 0;
 
 
char motor_err[32];
 
char batt_err[32];
char batt_temp[32];
 
 
uint32_t THROTTLE;
uint32_t CONT_TEMP;
uint32_t MOT_TEMP;
 
uint32_t DC_CURRENT;
uint32_t AC_CURRENT;
uint32_t AC_VOLTAGE;
uint32_t DC_VOLTAGE;
 
uint32_t voltage1_hx;
uint32_t current1_hx;
 
uint32_t voltage2_hx;
uint32_t current2_hx;
 
uint32_t voltage3_hx;
uint32_t current3_hx;
 
uint32_t SOC1_hx;
uint32_t SOH1_hx;
 
uint32_t SOC2_hx;
uint32_t SOH2_hx;
 
uint32_t SOC3_hx;
uint32_t SOH3_hx;
 
uint32_t Motor_err;
 
char bat1[32];
char bat2[32];
char bat3[32];
char vol_avg[32];
char sOC_avg[32];
char cur_tot[32];
char CVoltage_1[32];
char CVoltage_3[32];
char CVoltage_2[32];
char CCurrent_1[32];
char CCurrent_2[32];
char CCurrent_3[32];
char CSOC_1[32];
char CSOC_2[32];
char CSOC_3[32];
char CSOH_1[32];
char CSOH_2[32];
char CSOH_3[32];
char T_stamp[32];
char sensor[32];
char speed[64];
char throttle[64];
 
char motorTemp[64];
char contTmep[64];
char acCurrent[64];
char acVoltage[64];
char dcCurrent[64];
char dcVoltage[64];
char s_dcCurrent[64];
char s_acCurrent1[64];
char s_acCurrent2[64];
char trip1[64];
 
/////////////////////////////////////////////////////////////////////////////////
 
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
 
static const twai_general_config_t g_config = {.mode = TWAI_MODE_NORMAL,
.tx_io = GPIO_NUM_21,
.rx_io = GPIO_NUM_22,
.clkout_io = TWAI_IO_UNUSED,
.bus_off_io = TWAI_IO_UNUSED,
.tx_queue_len = 10,
.rx_queue_len = 20,
.alerts_enabled = TWAI_ALERT_ALL,
.clkout_divider = 0};
 
static QueueHandle_t tx_task_queue;
static QueueHandle_t rx_task_queue;
static SemaphoreHandle_t stop_ping_sem;
static SemaphoreHandle_t cnt_Switch_start;
static SemaphoreHandle_t done_sem;
static SemaphoreHandle_t ctrl_task_Transmit;
static SemaphoreHandle_t ctrl_task_receive;
static SemaphoreHandle_t ctrl_task_send;
 

uint8_t arr[8];
uint8_t combinedValue = 0;
int new_val ;
 

static void switch_ip(void *arg)
{
vTaskDelay(pdMS_TO_TICKS(50));
while(1)
{
 
vTaskDelay(pdMS_TO_TICKS(50));
 
 
ingi = received_value_ignition;
brake=received_value_brake;
modeL=received_value_modeL;
modeR=received_value_modeR;
reve=received_value_reverse;
sidestand= received_value_sidestand;
soc= received_value_soc;
batt_tmp= received_value_batt_tmp;
thr_per= received_value_throttle;
V_motor_out= received_value_motorTemp;
pcb= received_value_pcbTemp;
cnt_tmp= received_value_controllerTemp;
rpm= received_value_rpm;

    
    // Convert rpm to hexadecimal string
    sprintf(rpm_hex, "%x", rpm);

    // Determine the length of the hexadecimal string
    int len = strlen(rpm_hex);

    // If the hexadecimal string is less than 4 characters, pad it with zeros at the beginning
    if (len < 4) {
        for (int i = 3; i >= 0; i--) {
            if (len > 0) {
                rpm_hex[i] = rpm_hex[len - 1];
                len--;
            } else {
                rpm_hex[i] = '0';
            }
        }
        rpm_hex[4] = '\0';  // Null-terminate the string
    }

    // Split the hexadecimal string into two parts
    strncpy(rpm1_hex, rpm_hex, 2);
    rpm1_hex[2] = '\0'; // Null-terminate the string
    strncpy(rpm2_hex, rpm_hex + 2, 2);
    rpm2_hex[2] = '\0'; // Null-terminate the string

    // Print the hexadecimal strings
    printf("rpm (hexadecimal): %s\n", rpm_hex);
    printf("rpm1 (hexadecimal): %s\n", rpm1_hex);
    printf("rpm2 (hexadecimal): %s\n", rpm2_hex);



struct ControlBits {
    unsigned int b0 : 1;                //single bit is allocated for each ('1' represents that bit)- this is to pack several boolean flags inside a single byte(for space efficiency)
    unsigned int modeR : 1;
    unsigned int modeL : 1;
    unsigned int brake : 1;
    unsigned int ingi : 1;
    unsigned int reve : 1;
    unsigned int sidestand : 1;
    unsigned int b8 : 1;
};
 
// Define a union to share the same memory space for the bit field and an 8-bit integer
union ControlUnion {
    struct ControlBits bits;
    uint8_t combinedValue;
};
 
 
// not needed
union ControlUnion control;
    // Assign values to the bit field members
    control.bits.b0 = 0;
    control.bits.modeR = modeR;  // Replace with your actual modeR value
    control.bits.modeL = modeL;  // Replace with your actual modeL value
    control.bits.brake = brake;  // Replace with your actual brake value
    control.bits.ingi = ingi;   // Replace with your actual ingi value
    control.bits.reve = reve;   // Replace with your actual reve value
    control.bits.sidestand = sidestand;
    control.bits.b8 = 0;
 
    // Print the combined value
    // printf("Combined value: %d\n", control.combinedValue);
   
union
{
int b;
uint32_t f;
} u; // crazy
u.b = control.combinedValue;
 
state = u.f;  // converted to hexa
    printf("\n");
}
vTaskDelete(NULL);
 
}
 
 
int Dout ;
int Vmax = 100 ;
int Dmax = 4095 ;


int Dout ;
int V_motor_max = 150 ;
int D_motor_max = 4095 ;


 
static void twai_transmit_task(void *arg)

{
ESP_LOGI(EXAMPLE_TAG, "Transmitting to battery");
vTaskDelay(pdMS_TO_TICKS(100));
int i,x,pos,n=8 ;
 
while (1)
{
twai_message_t transmit_message_switch = {.identifier = (0x18530902), .data_length_code = 8, .extd = 1, .data = {thr_per, 0x03, 0x00, state, 0x00, 0x00, 0x00, 0x00}};
if (twai_transmit(&transmit_message_switch, 1000) == ESP_OK)
{
ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
vTaskDelay(pdMS_TO_TICKS(100));
}
else
{
 
ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
}
vTaskDelay(pdMS_TO_TICKS(100));
 
twai_message_t transmit_message_batteryTemp = {.identifier = (0x000000A), .data_length_code = 8, .extd = 1, .data = {batt_tmp, batt_tmp, batt_tmp, batt_tmp, batt_tmp, batt_tmp, batt_tmp, batt_tmp}};
if (twai_transmit(&transmit_message_batteryTemp, 10000) == ESP_OK)
{
ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
vTaskDelay(pdMS_TO_TICKS(100));
}
else
{
 
ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
}
vTaskDelay(pdMS_TO_TICKS(100));
 
 
twai_message_t transmit_message_otherTemp = {.identifier = (0x18530903), .data_length_code = 8, .extd = 1, .data = {cnt_tmp, V_motor_out, V_motor_out, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 }};
if (twai_transmit(&transmit_message_otherTemp, 10000) == ESP_OK)
{
    ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
vTaskDelay(pdMS_TO_TICKS(100));
}
else
{
 
ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
}
vTaskDelay(pdMS_TO_TICKS(100));
 
 
 
 
twai_message_t transmit_message_SoC= {.identifier = ID_LX_BATTERY_SOC , .data_length_code = 8, .extd = 1, .data = {soc, 0x00, 0x00, 0x9A , 0xB0 , 0x63 , 0x1D , 0x01 }};
if (twai_transmit(&transmit_message_SoC, 10000) == ESP_OK)
{
ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
vTaskDelay(pdMS_TO_TICKS(100));
}
else
{
 
ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
}
vTaskDelay(pdMS_TO_TICKS(100));

////////////////////////////
twai_message_t transmit_message_rpm = {.identifier = (0x14520902), .data_length_code = 8, .extd = 1, .data = {rpm1_hex, rpm2_hex, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 }};
if (twai_transmit(&transmit_message_rpm, 10000) == ESP_OK)
{
    ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
vTaskDelay(pdMS_TO_TICKS(100));
}
else
{
 
ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
}
vTaskDelay(pdMS_TO_TICKS(100));


////////////////////////////
 
}
 
 
 
vTaskDelete(NULL);
}
 

void configure_uart(void)
{
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
 
 
void app_main(void)
{ 
ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config)); /// driver install
ESP_LOGI(EXAMPLE_TAG, "Driver installed");
 
ESP_ERROR_CHECK(twai_start()); /// driver start
 
ESP_LOGI(EXAMPLE_TAG, "Driver started");
 
vTaskDelay(pdMS_TO_TICKS(500)) ;
 
// xTaskCreate(throttle_percentage , "throttle", 4096, NULL, 8, NULL);
// xTaskCreate(pcb_temp , "pcb temp", 4096, NULL, 8, NULL);
// xTaskCreate(motor_temp , "motor temp", 4096, NULL, 8, NULL);
// // xTaskCreate(battery_temp , "battery temperature", 4096, NULL, 8, NULL);
// xTaskCreate(controller_temp , "motor controller temperature", 4096, NULL, 8, NULL);
// xTaskCreate(soc_battery, "battery SoC", 4096, NULL, 8, NULL);
xTaskCreate(switch_ip, "Swicth_Tsk", 4096, NULL, 8, NULL);
//xTaskCreate(twai_transmit_task, "transmit_Tsk", 4096, NULL, 8, NULL);
xTaskCreate(twai_transmit_task, "Transmit_Tsk", 4096, NULL, 8, NULL);

    configure_uart();
    while (1) 
    {
        uint8_t data[4]; 
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, sizeof(data), 20 / portTICK_PERIOD_MS);

        if (len==5)
        {
            int switch_number = data[0] - '0'; // Convert ASCII to integer

            // Concatenate the characters and convert to integer
            int switch_state = (data[1] - '0') * 1000 + (data[2] - '0')* 100 + (data[3] - '0')*10 +(data[4]);

            switch (switch_number)
            {
                case 52:
                    received_value_rpm = switch_state;
            }
        }


         if (len ==4)
        {
            int switch_number = data[0] - '0'; // Convert ASCII to integer
            // Concatenate the characters and convert to integer
            int switch_state = (data[1] - '0') * 100 + (data[2] - '0')* 10 + (data[3] - '0');

            switch (switch_number)
            {
                case 7: // Handle slider value (received_value_slider)
                    received_value_soc = switch_state;
                break;

                case 8: // Handle slider value (received_value_slider)
                    received_value_throttle = switch_state;
                break;

                case 9: // Handle slider value (received_value_slider)
                    received_value_batt_tmp = switch_state;
                break;

                case 49:
                    received_value_motorTemp= switch_state;
                break;

                case 50:
                    received_value_controllerTemp= switch_state;
                break;

                case 51:
                    received_value_pcbTemp= switch_state;
                break;

                case 52:
                    received_value_rpm = switch_state;
                break;

                default:    
                    // Handle invalid switch number
                    break;
            }
        }

       
        if (len ==3)
        {
            int switch_number = data[0] - '0'; // Convert ASCII to integer

            // Concatenate the characters and convert to integer
            int switch_state = (data[1] - '0') * 10 + (data[2] - '0');

            switch (switch_number)
            {
                case 7: // Handle slider value (received_value_slider)
                    received_value_soc = switch_state;
                break;

                case 8: // Handle slider value (received_value_slider)
                    received_value_throttle = switch_state;
                break;

                case 9: // Handle slider value (received_value_slider)
                    received_value_batt_tmp = switch_state;
                break;

                case 49:
                    received_value_motorTemp= switch_state;
                break;

                case 50:
                    received_value_controllerTemp= switch_state;
                break;

                case 51:
                    received_value_pcbTemp= switch_state;
                break;

                case 52:
                    received_value_rpm = switch_state;
                break;

                default:    
                    // Handle invalid switch number
                    break;
            }
        }

        if (len == 2) 
        {
            // Process received command
            int switch_number = data[0] - '0'; // Convert ASCII to integer
            int switch_state = data[1] - '0'; // Convert ASCII to integer
            switch (switch_number) 
            {
                case 1:
                    received_value_brake = switch_state; // Assuming 1 represents ON and 0 represents OFF
                    break;
                
                case 2: //for reverse ,Press Brake then press Reverse- ON and then OFF
                    received_value_reverse = switch_state;
                    break;
                case 3: //for modeR, press modeR- ON and then OFF
                    received_value_modeR = switch_state;
                    break;
                case 4: //for modeL, press modeL- ON and then OFF
                    received_value_modeL = switch_state;
                    break;
                case 5: //for sidestand,press sidestand- ON and then OFF                //for now, sidestand is not integrated with the TWAI code
                    received_value_sidestand = switch_state;
                    break;
                case 6: //for Motor ON ,Press Brake then press Ignition- ON and then OFF
                    received_value_ignition = switch_state;
                    break;
                
                case 7: // Handle slider value (received_value_slider)
                    received_value_soc = switch_state;
                break;

                case 8: // Handle slider value (received_value_slider)
                    received_value_throttle = switch_state;
                    break;
                case 9: // Handle slider value (received_value_slider)
                    received_value_batt_tmp = switch_state;
                break; 

                case 49:
                    received_value_motorTemp= switch_state;
                break;

                case 50:
                    received_value_controllerTemp= switch_state;
                break;

                case 51:
                    received_value_pcbTemp= switch_state;
                break;

                case 52:
                    received_value_rpm = switch_state;
                break;

                default:    
                    // Handle invalid switch number
                    break;
            }
        }
    }
}