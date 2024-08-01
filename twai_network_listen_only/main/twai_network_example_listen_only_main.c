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
#include "driver/uart.h" 
#include "sdkconfig.h"
#include "esp_log.h"
#include "cJSON.h"
#include "driver/twai.h"
#include "esp_timer.h"
 
#define BLINK_GPIO 5
#define ECHO_TEST_TXD  1
#define ECHO_TEST_RXD  3
#define ECHO_UART_PORT_NUM      UART_NUM_1
#define ECHO_UART_BAUD_RATE     115200
#define ECHO_TASK_STACK_SIZE    2048
  
#define temp5 23 // adc
#define temp6 25 // adc
#define temp7 26 // adc
#define temp8 27 // adc
 
int ingi =0;
int modeL = 0;
int modeR = 0;
 
int brake = 0;
int reve = 0;
// int sidestand = 0 ;
static int received_value_ignition;
static int received_value_brake;
static int received_value_reverse;
static int received_value_modeR;
static int received_value_modeL;
// static int received_value_sidestand;
static int received_value_soc;
static int received_value_batt_tmp;
static int received_value_throttle;
static int received_value_motorTemp;
static int received_value_pcbTemp;
static int received_value_controllerTemp;
static int received_value_rpm;
static int MotorWarn;
static int sidestand_pulse;
static int controllerWarn;
static int BattWarn ;
static int BattLowWarn;
static int PackCurrent;
static uint8_t rpm1_hex;
static uint8_t rpm2_hex;
static uint8_t packCurr1 ;
static uint8_t packCurr2 ;
static uint8_t packCurr3 ;
static uint8_t packCurr4 ;


static int thr_per;
static int batt_tmp;
static int V_motor_out;
static int pcb;
static float cnt_tmp;
static int rpm;
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
#define ID_Battery_ProtectionsAndWarnings 0x9
 
 
#define ID_MOTOR_RPM 0x230
#define VCU_msg 0x18f20309
#define ID_MOTOR_TEMP 0x233
#define ID_MOTOR_CURR_VOLT 0x32
static uint8_t state = 0;
int adc_value = 0;
int adc_value1 = 0;
int adc_value2 = 0;
int rpm_rx = 1234; // Example RPM value
static int Motor_status;
static int DC_current_limit;
 
 
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
// sidestand= received_value_sidestand;
soc= received_value_soc;
batt_tmp= received_value_batt_tmp;
thr_per= received_value_throttle;
V_motor_out= received_value_motorTemp;
pcb= received_value_pcbTemp;
cnt_tmp= received_value_controllerTemp;
rpm= received_value_rpm;

     // Convert RPM to hexadecimal bytes
    rpm1_hex = rpm >> 8; // Most significant byte
    rpm2_hex = rpm & 0xFF; // Least significant byte


 // Convert to unsigned 32-bit for two's complement representation
    uint32_t PackCurrent_twos_complement_value = (uint32_t)PackCurrent;

    // Print the 8-digit hexadecimal representation
    // printf("Hex signed 2's complement (8 digits): %08X\n", PackCurrent_twos_complement_value);

       // Extract each byte and store them in variables
     packCurr1 = (PackCurrent_twos_complement_value >> 24) & 0xFF;
     packCurr2 = (PackCurrent_twos_complement_value >> 16) & 0xFF;
     packCurr3 = (PackCurrent_twos_complement_value >> 8) & 0xFF;
     packCurr4 = PackCurrent_twos_complement_value & 0xFF;



struct ControlBits {
    unsigned int b0 : 1;                //single bit is allocated for each ('1' represents that bit)- this is to pack several boolean flags inside a single byte(for space efficiency)
    unsigned int modeR : 1;
    unsigned int modeL : 1;
    unsigned int brake : 1;
    unsigned int ingi : 1;
    unsigned int reve : 1;
    // unsigned int sidestand : 1;
    unsigned int b8 : 1;
};
 
// Define a union to share the same memory space for the bit field and an 8-bit integer
union ControlUnion {
    struct ControlBits bits;
    uint8_t combinedValue;
};
 
 
// not needed
union ControlUnion control;
//"these following is the code snippet for controlling the Controls like brake,modeR,modeL etc "
    // Assign values to the bit field members
    control.bits.b0 = 0;
    control.bits.modeR = modeR;  // Replace with your actual modeR value
    control.bits.modeL = modeL;  // Replace with your actual modeL value
    control.bits.brake = brake;  // Replace with your actual brake value
    control.bits.ingi = ingi;   // Replace with your actual ingi value
    control.bits.reve = reve;   // Replace with your actual reve value
    // control.bits.sidestand = sidestand;
    control.bits.b8 = 0;

union
{
int b;
uint32_t f;
} u; 
u.b = control.combinedValue;
 
state = u.f;  
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



void uart_send_task(void *arg) {
    const uart_port_t uart_num = ECHO_UART_PORT_NUM;
    char motor_buffer[32];
    char DC_current_limit_buffer[25];

    while (1) {
        snprintf(motor_buffer, sizeof(motor_buffer), "Motor_status:%d\n", Motor_status);
        uart_write_bytes(uart_num, motor_buffer, strlen(motor_buffer));
        vTaskDelay(pdMS_TO_TICKS(1000)); // Send every 1 second

        snprintf(DC_current_limit_buffer, sizeof(DC_current_limit_buffer), "DC_current_limit:%d\n", DC_current_limit);
        uart_write_bytes(uart_num, DC_current_limit_buffer, strlen(DC_current_limit_buffer));
        vTaskDelay(pdMS_TO_TICKS(1000)); // Send every 1 second
    }
}

 
static void twai_transmit_task(void *arg)
{
    ESP_LOGI(EXAMPLE_TAG, "Transmitting to battery");
    vTaskDelay(pdMS_TO_TICKS(100));

    
    while (1)
    {
        twai_message_t transmit_message_switch = {.identifier = (0x18530902), .data_length_code = 8, .extd = 1, .data = {thr_per, 0x00, MotorWarn, sidestand_pulse, controllerWarn, 0x00, 0x00, 0x00}};
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

///////////////////////////////////
        twai_message_t transmit_message_PackCurrAndPackVol= {.identifier = ID_LX_BATTERY_VI , .data_length_code = 8, .extd = 1, .data = {packCurr1,packCurr2,packCurr3, packCurr4, 0x00 , 0x00 , 0x00 , 0x00 }};    //First 4 bytes for PackCurrent and last 4 bytes for PackVoltage
        if (twai_transmit(&transmit_message_PackCurrAndPackVol, 10000) == ESP_OK)
        {
        ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
        vTaskDelay(pdMS_TO_TICKS(100));
        }
        else
        {       
        
        ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
///////////////////////////////////


        twai_message_t transmit_message_rpm = {.identifier = (0x14520902), .data_length_code = 8, .extd = 1, .data = {rpm2_hex, rpm1_hex, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 }};
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


        
        twai_message_t transmit_message_warning = {.identifier = ID_Battery_ProtectionsAndWarnings , .data_length_code = 8, .extd = 1, .data = {0x00, BattLowWarn, BattWarn , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 }};
        if (twai_transmit(&transmit_message_warning, 10000) == ESP_OK)
        {
        ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
        vTaskDelay(pdMS_TO_TICKS(100));
        }
        else
        {
        
        ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}


static void twai_receive_task(void *arg)
{
  printf("Entered receive function------------------->\n");
  ESP_LOGI(EXAMPLE_TAG, "receive fucntion");
 
  // xSemaphoreTake(ctrl_task_receive, portMAX_DELAY); // Wait for completion

  // xSemaphoreGive(ctrl_task_send);                   // Start control task
 
  ESP_LOGI(EXAMPLE_TAG, "Receiving1");

 
  // uint32_t MSG = 0;
 
      while (1)
        {
             twai_message_t message;
              // ESP_LOGI(EXAMPLE_TAG, "Before receive call");
              if (twai_receive(&message, pdMS_TO_TICKS(250)) == ESP_OK)
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
                  ESP_LOGI(EXAMPLE_TAG, "message id------> %lx",message.identifier);
                  printf("mtr decode =  %x , %x , %x, %x , %x, %x, %x, %x  \n", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]);

                if (message.identifier == VCU_msg)
                {
                  if(message.data[0] == 0x15)
                  {
                    printf("<--------------------------------------------------MOTOR ON------------------------------------------------------------------->\n");
                    Motor_status = 1;
                  }

                  else if (message.data[0]== 0x14 || message.data[0]== 0x4 )
                  {
                    printf("<--------------------------------------------------MOTOR OFF------------------------------------------------------------------->\n");
                    Motor_status = 0;
                  }

                  DC_current_limit = message.data[3];
                }

                 

/////////////////////////////////////////////////////
            //   if (message.identifier == ID_MOTOR_RPM)
            //       { 
            //         ESP_LOGI(EXAMPLE_TAG, "ID_MOTOR_RPM------------------------------------------------>");

            //         if (!(message.rtr))

            //             {
                  
            //               // mcu_data = message.data;
                  
            //               RPM = (message.data[1]) | (message.data[0] << 8);
                  
            //                   Motor_err = (message.data[7]) | (message.data[6]<<8) | (message.data[5]<<16)|(message.data[4]<<24) |(message.data[3]<<32)|(message.data[2]<<40) | (message.data[1]<<48)|(message.data[0]<<56);
                  
            //                   sprintf(motor_err, "%x,%x,%x,%x,%x,%x,%x,%x", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]);
                  
                  
            //               union

            //               {

            //                 uint32_t b;

            //                 int f;

            //               } u; // crazy

            //               u.b = RPM;
                  
            //               // M_RPM = u.f;
                  
            //               twai_message_t transmit_message = {.identifier = (0x14520902 ), .data_length_code = 8, .extd = 1, .data = {RPM, 0, 0, DC_VOLTAGE, DC_CURRENT}};

            //               // twai_message_t transmit_message_SoC= {.identifier = ID_LX_BATTERY_SOC , .data_length_code = 8, .extd = 1, .data = {soc, 0x00, 0x00, 0x9A , 0xB0 , 0x63 , 0x1D , 0x01 }};


            //             if (twai_transmit(&transmit_message, 1000) == ESP_OK)

            //             {
                  
            //               //          ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");

            //               //          vTaskDelay(pdMS_TO_TICKS(250));

            //             }

            //             else

            //             {
                  
            //               ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");

            //             }
                  
            //             vTaskDelay(pdMS_TO_TICKS(50));
            //             }

            //         }
        //   //////////////////////////
        //       else if (message.identifier == ID_MOTOR_TEMP)

        //           { 
        //             ESP_LOGI(EXAMPLE_TAG, "ID_MOTOR_TEMP------------------------------------------------>");
        //             if (!(message.rtr))

        //             {

        //               THROTTLE = (message.data[0]);
              
        //               CONT_TEMP = (message.data[1]);

        //               MOT_TEMP = (message.data[2]);

        //               union

        //               {

        //                 uint32_t b;

        //                 int f;

        //               } u; // crazy

        //               u.b = THROTTLE;

        //               M_THROTTLE = u.f;

        //               u.b = CONT_TEMP;

        //               M_CONT_TEMP = u.f - 40;

        //               u.b = MOT_TEMP;

        //               M_MOT_TEMP = u.f - 40;

        //               //  printf("Temp =  %d \n", M_CONT_TEMP);

        //             }

        //           }
          
        //       else if (message.identifier == ID_MOTOR_CURR_VOLT)

        //             {
        //               ESP_LOGI(EXAMPLE_TAG, "ID_MOTOR_CURR_VOLT------------------------------------------------>");
                
        //               DC_CURRENT = (message.data[0]) | (message.data[1] << 8);

        //               AC_CURRENT = (message.data[4]) | (message.data[5] << 8);
                
        //               AC_VOLTAGE = (message.data[3]);
                
        //               DC_VOLTAGE = (message.data[2]);
                
        //               union

        //               {

        //                 uint32_t b;

        //                 int f;

        //               } u; // crazy

        //               u.b = DC_CURRENT;
                
        //               M_DC_CURRENT = u.f;
                
        //               u.b = AC_CURRENT;
                
        //               M_AC_CURRENT = u.f;
                
        //               u.b = AC_VOLTAGE;
                
        //               M_AC_VOLTAGE = u.f;
                
        //               u.b = DC_VOLTAGE;
                
        //               M_DC_VOLATGE = u.f;
                
        //             } /////        motor temp
          
          
        //       else if (message.identifier == ID_LX_BATTERY_SOC)

        //           {
        //             ESP_LOGI(EXAMPLE_TAG, "ID_LX_BATTERY_SOC------------------------------------------------>");
        //             if (!(message.rtr))

        //             {

        //               SOC3_hx = (message.data[1] << 8) | message.data[0];

        //               SOH3_hx = message.data[5]  ;

        //               union

        //               {

        //                 uint32_t b;

        //                 int f;

        //               } u; // crazy

        //               u.b = SOC3_hx;

        //               SOC_3 = u.f;

        //               u.b = SOH3_hx;

        //               SOH_3 = u.f;

        //           //              printf("Battery 3 --> SOC[ %d ]   SOH[ %d ] \n", SOC_3,SOH_3);

        //             }

        //           }
          
        //       else if (message.identifier == ID_LX_BATTERY_VI)

        //           {
        //             ESP_LOGI(EXAMPLE_TAG, "ID_LX_BATTERY_VI------------------------------------------------>");

        //             if (!(message.rtr))

        //             {

        //             current2_hx  =  (message.data[0] << 24) | (message.data[1] << 16) | (message.data[2] << 8) | message.data[3];

        //               voltage2_hx =  (message.data[6] << 8) | (message.data[7] ) ;
              
                  

        //             union

        //               {

        //                 uint32_t b;

        //                 int f;

        //               } u; // crazy

        //               u.b = voltage2_hx;

        //               Voltage_2 = u.f;

        //               u.b = current2_hx;

        //               Current_2 = u.f;

        //           //      printf("Battery 2 --> Volatge[%d]   current[%f]   \n", ( Voltage_2),(Current_2*0.001));
              
        //             }

        //           }
          
          
        //       else if (message.identifier == ID_LX_BATTERY_PROT)

        //           { ESP_LOGI(EXAMPLE_TAG, "ID_LX_BATTERY_PROT------------------------------------------------>");

        //             if (!(message.rtr))

        //             {
              
        //               // mcu_data = message.data;
              
              
        //               //   BATT_ERR = (message.data[7]) | (message.data[6]<<8) | (message.data[5]<<16)|(message.data[4]<<24) |(message.data[3]<<32)|(message.data[2]<<40) | (message.data[1]<<48)|(message.data[0]<<56);
              
        //                   sprintf(batt_err, "%x,%x,%x,%x,%x,%x,%x,%x", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]);
              
        //             }

        //           }
              
          
        //       else if (message.identifier == ID_LX_BATTERY_T)

        //           { 
        //             ESP_LOGI(EXAMPLE_TAG, "ID_LX_BATTERY_T------------------------------------------------>");
                    
        //             if (!(message.rtr))

        //             {
              
        //               // mcu_data = message.data;
              
              
        //               //   BATT_ERR = (message.data[7]) | (message.data[6]<<8) | (message.data[5]<<16)|(message.data[4]<<24) |(message.data[3]<<32)|(message.data[2]<<40) | (message.data[1]<<48)|(message.data[0]<<56);
              
        //                   sprintf(batt_temp, "%x,%x,%x,%x,%x,%x,%x,%x", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]);

        //                   printf("batt temp------------>%c",batt_temp[0]);
        //                   printf("batt temp------------>%c",batt_temp[1]);
        //                   printf("batt temp------------>%c",batt_temp[2]);
        //                   printf("batt temp------------>%c",batt_temp[3]);
        //             }

        //           }
              
        //       else
        //             {

        //               //   ESP_LOGE(EXAMPLE_TAG, " ID not match - %lx ",message.identifier );

        //               //   vTaskDelay(pdMS_TO_TICKS(250));

        //             }
            } 
        }

        /////////////////////
  // xSemaphoreGive(done_sem);
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
    xTaskCreate(uart_send_task, "uart_send_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
 
 
void process_uart_data(uint8_t *data, int len) {
    int switch_number = data[0]; // No need to convert to integer
    int switch_state;
    float pack_current;
    int value;

    if (switch_number == 'x') {
        // Handle pack current separately
        // Create a buffer to hold the float string
        char buffer[16];
        strncpy(buffer, (char*)data + 1, len - 1);
        buffer[len - 1] = '\0';

        // Convert string to float
        pack_current = atof(buffer);

        // Transform the float value
        if (pack_current == (int)pack_current) {
            value = (int)pack_current; // If no decimal part, keep as is
        } else {
            value = (int)(pack_current * 1000); // Convert float to int by scaling
        }

        // Assign to PackCurrent (which is of type float)
        PackCurrent = value;

    } else {
        // Existing logic for other switch numbers
        if (len == 5) {
            switch_state = (data[1] - '0') * 1000 + (data[2] - '0') * 100 + (data[3] - '0') * 10 + (data[4] - '0');
        } else if (len == 4) {
            switch_state = (data[1] - '0') * 100 + (data[2] - '0') * 10 + (data[3] - '0');
        } else if (len == 3) {
            switch_state = (data[1] - '0') * 10 + (data[2] - '0');
        } else if (len == 2) {
            switch_state = data[1] - '0'; // Convert ASCII to integer
        } else {
            // Invalid length, return or handle error
            return;
        }

        switch (switch_number) {
            case '1':
                received_value_brake = switch_state;
                break;
            case '2':
                received_value_reverse = switch_state;
                break;
            case '3':
                received_value_modeR = switch_state;
                break;
            case '4':
                received_value_modeL = switch_state;
                break;
            case '5':
                // received_value_sidestand = switch_state;
                break;
            case '6':
                received_value_ignition = switch_state;
                break;
            case '7':
                received_value_soc = switch_state;
                break;
            case '8':
                received_value_throttle = switch_state;
                break;
            case '9':
                received_value_batt_tmp = switch_state;
                break;
            case 'a':
                received_value_motorTemp = switch_state;
                break;
            case 'b':
                received_value_controllerTemp = switch_state;
                break;
            case 'c':
                received_value_pcbTemp = switch_state;
                break;
            case 'd':
                received_value_rpm = switch_state;
                break;
            case 'e':  // Motor Over Temperature warning
                MotorWarn = switch_state == 1 ? 16 : 0;
                break;
            case 'f':  // Throttle error warning
                MotorWarn = switch_state == 1 ? 32 : 0;
                break;
            case 'v':  // Controller Over Temperature Warning
                MotorWarn = switch_state == 1 ? 8 : 0;
                break;
            case 'g':  // Controller Over Voltage warning
                controllerWarn = switch_state == 1 ? 1 : 0;
                break;
            case 'h':  // Controller Under Voltage warning
                controllerWarn = switch_state == 1 ? 2 : 0;
                break;
            case 'i':  // Overcurrent Fault
                controllerWarn = switch_state == 1 ? 4 : 0;
                break;
            case 'j':  // Motor Hall Input Abnormal
                MotorWarn = switch_state == 1 ? 1 : 0;
                break;
            case 'k':  // Motor Stalling
                MotorWarn = switch_state == 1 ? 2 : 0;
                break;
            case 'l':  // Motor Phase Loss
                MotorWarn = switch_state == 1 ? 4 : 0;
                break;
            case 'm':  // BattLowSocWarn
                BattLowWarn = switch_state == 1 ? 2 : 0;
                break;
            case 'n':  // CellUnderVolWarn
                BattLowWarn = switch_state == 1 ? 16 : 0;
                break;
            case 'o':  // CellOverVolWarn
                BattLowWarn = switch_state == 1 ? 32 : 0;
                break;
            case 'p':  // PackUnderVolWarn
                BattWarn = switch_state == 1 ? 4 : 0;
                break;
            case 'q':  // PackOverVolWarn
                BattWarn = switch_state == 1 ? 8 : 0;
                break;
            case 'r':  // ChgUnderTempWarn
                BattWarn = switch_state == 1 ? 16 : 0;
                break;
            case 's':  // ChgOverTempWarn
                BattWarn = switch_state == 1 ? 32 : 0;
                break;
            case 't':  // DchgUnderTempWarn
                BattWarn = switch_state == 1 ? 64 : 0;
                break;
            case 'u':  // DchgOverTempWarn
                BattWarn = switch_state == 1 ? 128 : 0;
                break;
            case 'y':  // TempSensorFault
                BattWarn = switch_state == 1 ? 2 : 0;
                break;
            case 'z':
                sidestand_pulse = switch_state = 1 ? 64 : 0;
                

            default:
                // Handle invalid switch number
                break;
        }
    }
}


void app_main(void) {
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(EXAMPLE_TAG, "Driver installed");

    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(EXAMPLE_TAG, "Driver started");

    vTaskDelay(pdMS_TO_TICKS(500));

    xTaskCreate(switch_ip, "Swicth_Tsk", 4096, NULL, 8, NULL);
    xTaskCreate(twai_transmit_task, "Transmit_Tsk", 4096, NULL, 8, NULL);
    xTaskCreate(twai_receive_task, "receive_task", 4096, NULL, 8, NULL);

    configure_uart();

    while (1) {
        uint8_t data[10]; 
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, sizeof(data), 20 / portTICK_PERIOD_MS);

        if (len > 0) {
            process_uart_data(data, len);
        } else {
            // Handle UART read error
        }
    }
}
