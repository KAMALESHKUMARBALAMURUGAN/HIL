/* GPIO Example
 
   This example code is in the Public Domain (or CC0 licensed, at your option.)
 
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
 
 
#define Ignition 14
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
// #define ADC_WIDTH_BIT_DEFAULT (ADC_WIDTH_BIT_9 - 1)
 
static int Voltage_2 = 0;
static int Current_2 = 0;
 
static int SOC_3 = 0;
static int SOH_3 = 0;
 

static uint8_t state = 0;
static int M_CONT_TEMP = 0;
static int M_MOT_TEMP = 0;
static int M_THROTTLE = 0;
 
static int M_AC_CURRENT = 0;
static int M_AC_VOLTAGE = 0;
static int M_DC_CURRENT = 0;
static int M_DC_VOLATGE = 0;


int adc_value = 0;
int adc_value1 = 0;
int adc_value2 = 0;
 
uint32_t RPM;
 
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
char rpm[64];
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
//int val_of_temp = 0;
int new_val ;
 

static void Button_input(void *arg)
{

vTaskDelay(pdMS_TO_TICKS(50));

 
while(1)
{
vTaskDelay(pdMS_TO_TICKS(50));
 
 
int ingi = !gpio_get_level(Ignition);
int brake = !gpio_get_level(Break);
int modeL = !gpio_get_level(ModeL);
int modeR = !gpio_get_level(ModeR);
int reve = !gpio_get_level(Reverse);
int sidestand = !gpio_get_level(SideStand);

struct ControlBits {
    unsigned int b0 : 1;
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
static float batt_tmp = 0 ;
static void battery_temp(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0); //gpio 32
    while(1)
    {
        int val_of_temp = adc1_get_raw(ADC1_CHANNEL_4);
        batt_tmp = (float)val_of_temp * ((float)Vmax / (float)Dmax);
        printf("The temperature of the battery is : %.4f \n", batt_tmp);
        vTaskDelay(250/portTICK_PERIOD_MS);
    }
    vTaskDelay(NULL);
}
 
 

static float cnt_tmp = 0;
static void controller_temp(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0);
    while(1)
    {
        int val_temp_controller = adc1_get_raw(ADC1_CHANNEL_6);//gpio34
        cnt_tmp = (float)val_temp_controller * ((float)Vmax / (float)Dmax);
        printf("MCU temperature is : %.4f \n", cnt_tmp);
        vTaskDelay(250/portTICK_PERIOD_MS);
    }
    vTaskDelay(NULL);
}
 

static float pcb = 0 ;
static void pcb_temp(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_0);
    while(1)
    {
        int val_temp_pcb = adc1_get_raw(ADC1_CHANNEL_7); //GPIO -35
       
        pcb = (float)val_temp_pcb * ((float)Vmax / (float)Dmax);
        //printf("the value shown %d \n", val_temp_pcb);//val_of_temp);
        printf("PCB temperature is : %.4f \n", pcb);
        //ESP_LOGD(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        vTaskDelay(250/portTICK_PERIOD_MS);
            //new_val = val_of_temp ;
    }
    vTaskDelay(NULL);
}
 
static float soc = 0 ;
static void soc_battery(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_0);
    while(1)
    {
        int soc_of_batt = adc1_get_raw(ADC1_CHANNEL_3); // GPIO - 39
        soc = (float)soc_of_batt * ((float)Vmax / (float)Dmax);
        //printf("the value shown %d \n", soc_of_batt);//val_of_temp);
        printf("SoC of the battery is : %.4f  \n", soc);
        //ESP_LOGD(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        vTaskDelay(250/portTICK_PERIOD_MS);
            //new_val = val_of_temp ;
    }
    vTaskDelay(NULL);
}
 
int Dout ;
int V_motor_max = 150 ;
int D_motor_max = 4095 ;
static float V_motor_out = 0;
static void motor_temp(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_0);
    while(1)
    {
        int val_temp_motor = adc1_get_raw(ADC1_CHANNEL_5); // gpio33
        V_motor_out = (float)val_temp_motor * ((float)V_motor_max / (float)D_motor_max);
        //printf("the value shown %d \n", val_temp_motor);//val_of_temp);
        printf("Temperature of the motor is : %.4f \n", V_motor_out);
        //ESP_LOGD(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        vTaskDelay(250/portTICK_PERIOD_MS);
            //new_val = val_of_temp ;
    }
    vTaskDelay(NULL);
}
 

static float thr_per = 0 ;
static void throttle_percentage(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);// gpio 36
    while(1)
    {
        int throttle_per = adc1_get_raw(ADC1_CHANNEL_0);//,ADC_WIDTH_BIT_12, NULL);
        thr_per = (float)throttle_per * ((float)Vmax / (float)Dmax);
        vTaskDelay(250/portTICK_PERIOD_MS);
    }
    vTaskDelay(NULL);
}
 
static void twai_transmit_task(void *arg)
{
  printf("Entered transmit function------------------->\n");
  ESP_LOGI(EXAMPLE_TAG, "Transmit fucntion");
ESP_LOGI(EXAMPLE_TAG, "Transmitting to battery");
 
vTaskDelay(pdMS_TO_TICKS(50));
while (1)
{
 
 
 
//printf("this is my %d", new_val);
twai_message_t transmit_message_switch = {.identifier = (0x18530902), .data_length_code = 8, .extd = 1, .data = {0x00, 0x03, 0x00, state, 0x00, 0x00, 0x00, 0x00}};
if (twai_transmit(&transmit_message_switch, 1000) == ESP_OK)
{
ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
vTaskDelay(pdMS_TO_TICKS(50));
}
else
{
 
ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
}
vTaskDelay(pdMS_TO_TICKS(50));
 
twai_message_t transmit_message_batteryTemp = {.identifier = (0x000000A), .data_length_code = 8, .extd = 1, .data = {batt_tmp, batt_tmp, batt_tmp, batt_tmp, batt_tmp, batt_tmp, batt_tmp, batt_tmp}};
if (twai_transmit(&transmit_message_batteryTemp, 10000) == ESP_OK)
{
ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
vTaskDelay(pdMS_TO_TICKS(50));
}
else
{
 
ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
}
vTaskDelay(pdMS_TO_TICKS(50));
 
 
twai_message_t transmit_message_otherTemp = {.identifier = (0x18530903), .data_length_code = 8, .extd = 1, .data = {cnt_tmp, V_motor_out, V_motor_out, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 }};
if (twai_transmit(&transmit_message_otherTemp, 10000) == ESP_OK)
{
    ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
vTaskDelay(pdMS_TO_TICKS(50));
}
else
{
 
ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
}
vTaskDelay(pdMS_TO_TICKS(50));
 
 
 
 
twai_message_t transmit_message_SoC= {.identifier = ID_LX_BATTERY_SOC , .data_length_code = 8, .extd = 1, .data = {soc, 0x00, 0x00, 0x9A , 0xB0 , 0x63 , 0x1D , 0x01 }};
if (twai_transmit(&transmit_message_SoC, 10000) == ESP_OK)
{
ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");
vTaskDelay(pdMS_TO_TICKS(50));
}
else
{
 
ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");
}
vTaskDelay(pdMS_TO_TICKS(50));
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

 
  uint32_t MSG = 0;
 
      while (1)
        {
             twai_message_t message;
              ESP_LOGI(EXAMPLE_TAG, "Before receive call");
              if (twai_receive(&message, pdMS_TO_TICKS(250)) == ESP_OK)
              {
                  ESP_LOGI(EXAMPLE_TAG, "After receive call with identifier: %lx", message.identifier);
                  ESP_LOGI(EXAMPLE_TAG, "Received message with identifier: %lx, data: %x %x %x %x %x %x %x %x",
                          message.identifier,
                          message.data[0], message.data[1], message.data[2], message.data[3],
                          message.data[4], message.data[5], message.data[6], message.data[7]);

                  // if (MSG == message.identifier)
                  // {
                  //     twai_clear_receive_queue();  // Commented out to prevent clearing the queue
                  //     MSG = 0;
                  // }
              
              
                  // MSG = message.identifier;
                  ESP_LOGI(EXAMPLE_TAG, "message %lx",message.identifier);
                  printf("mtr decode =  %x , %x , %x, %x , %x, %x, %x, %x  \n", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]);

              if (message.identifier == ID_MOTOR_RPM)
                  { 
                    ESP_LOGI(EXAMPLE_TAG, "ID_MOTOR_RPM------------------------------------------------>");

                    if (!(message.rtr))

                        {
                  
                          // mcu_data = message.data;
                  
                          RPM = (message.data[1]) | (message.data[0] << 8);
                  
                              Motor_err = (message.data[7]) | (message.data[6]<<8) | (message.data[5]<<16)|(message.data[4]<<24) |(message.data[3]<<32)|(message.data[2]<<40) | (message.data[1]<<48)|(message.data[0]<<56);
                  
                              sprintf(motor_err, "%x,%x,%x,%x,%x,%x,%x,%x", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]);
                  
                  
                          union

                          {

                            uint32_t b;

                            int f;

                          } u; // crazy

                          u.b = RPM;
                  
                          // M_RPM = u.f;
                  
                          twai_message_t transmit_message = {.identifier = (0x14520902 ), .data_length_code = 8, .extd = 1, .data = {RPM, 0, 0, DC_VOLTAGE, DC_CURRENT}};

                          // twai_message_t transmit_message_SoC= {.identifier = ID_LX_BATTERY_SOC , .data_length_code = 8, .extd = 1, .data = {soc, 0x00, 0x00, 0x9A , 0xB0 , 0x63 , 0x1D , 0x01 }};


                        if (twai_transmit(&transmit_message, 1000) == ESP_OK)

                        {
                  
                          //          ESP_LOGI(EXAMPLE_TAG, "Message queued for transmission\n");

                          //          vTaskDelay(pdMS_TO_TICKS(250));

                        }

                        else

                        {
                  
                          ESP_LOGE(EXAMPLE_TAG, "Failed to queue message for transmission\n");

                        }
                  
                        vTaskDelay(pdMS_TO_TICKS(50));
                        }

                    }
          
              else if (message.identifier == ID_MOTOR_TEMP)

                  { 
                    ESP_LOGI(EXAMPLE_TAG, "ID_MOTOR_TEMP------------------------------------------------>");
                    if (!(message.rtr))

                    {

                      THROTTLE = (message.data[0]);
              
                      CONT_TEMP = (message.data[1]);

                      MOT_TEMP = (message.data[2]);

                      union

                      {

                        uint32_t b;

                        int f;

                      } u; // crazy

                      u.b = THROTTLE;

                      M_THROTTLE = u.f;

                      u.b = CONT_TEMP;

                      M_CONT_TEMP = u.f - 40;

                      u.b = MOT_TEMP;

                      M_MOT_TEMP = u.f - 40;

                      //  printf("Temp =  %d \n", M_CONT_TEMP);

                    }

                  }
          
              else if (message.identifier == ID_MOTOR_CURR_VOLT)

                    {
                      ESP_LOGI(EXAMPLE_TAG, "ID_MOTOR_CURR_VOLT------------------------------------------------>");
                
                      DC_CURRENT = (message.data[0]) | (message.data[1] << 8);

                      AC_CURRENT = (message.data[4]) | (message.data[5] << 8);
                
                      AC_VOLTAGE = (message.data[3]);
                
                      DC_VOLTAGE = (message.data[2]);
                
                      union

                      {

                        uint32_t b;

                        int f;

                      } u; // crazy

                      u.b = DC_CURRENT;
                
                      M_DC_CURRENT = u.f;
                
                      u.b = AC_CURRENT;
                
                      M_AC_CURRENT = u.f;
                
                      u.b = AC_VOLTAGE;
                
                      M_AC_VOLTAGE = u.f;
                
                      u.b = DC_VOLTAGE;
                
                      M_DC_VOLATGE = u.f;
                
                    } /////        motor temp
          
          
              else if (message.identifier == ID_LX_BATTERY_SOC)

                  {
                    ESP_LOGI(EXAMPLE_TAG, "ID_LX_BATTERY_SOC------------------------------------------------>");
                    if (!(message.rtr))

                    {

                      SOC3_hx = (message.data[1] << 8) | message.data[0];

                      SOH3_hx = message.data[5]  ;

                      union

                      {

                        uint32_t b;

                        int f;

                      } u; // crazy

                      u.b = SOC3_hx;

                      SOC_3 = u.f;

                      u.b = SOH3_hx;

                      SOH_3 = u.f;

                  //              printf("Battery 3 --> SOC[ %d ]   SOH[ %d ] \n", SOC_3,SOH_3);

                    }

                  }
          
              else if (message.identifier == ID_LX_BATTERY_VI)

                  {
                    ESP_LOGI(EXAMPLE_TAG, "ID_LX_BATTERY_VI------------------------------------------------>");

                    if (!(message.rtr))

                    {

                    current2_hx  =  (message.data[0] << 24) | (message.data[1] << 16) | (message.data[2] << 8) | message.data[3];

                      voltage2_hx =  (message.data[6] << 8) | (message.data[7] ) ;
              
                  

                    union

                      {

                        uint32_t b;

                        int f;

                      } u; // crazy

                      u.b = voltage2_hx;

                      Voltage_2 = u.f;

                      u.b = current2_hx;

                      Current_2 = u.f;

                  //      printf("Battery 2 --> Volatge[%d]   current[%f]   \n", ( Voltage_2),(Current_2*0.001));
              
                    }

                  }
          
          
              else if (message.identifier == ID_LX_BATTERY_PROT)

                  { ESP_LOGI(EXAMPLE_TAG, "ID_LX_BATTERY_PROT------------------------------------------------>");

                    if (!(message.rtr))

                    {
              
                      // mcu_data = message.data;
              
              
                      //   BATT_ERR = (message.data[7]) | (message.data[6]<<8) | (message.data[5]<<16)|(message.data[4]<<24) |(message.data[3]<<32)|(message.data[2]<<40) | (message.data[1]<<48)|(message.data[0]<<56);
              
                          sprintf(batt_err, "%x,%x,%x,%x,%x,%x,%x,%x", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]);
              
                    }

                  }
              
          
              else if (message.identifier == ID_LX_BATTERY_T)

                  { 
                    ESP_LOGI(EXAMPLE_TAG, "ID_LX_BATTERY_T------------------------------------------------>");
                    
                    if (!(message.rtr))

                    {
              
                      // mcu_data = message.data;
              
              
                      //   BATT_ERR = (message.data[7]) | (message.data[6]<<8) | (message.data[5]<<16)|(message.data[4]<<24) |(message.data[3]<<32)|(message.data[2]<<40) | (message.data[1]<<48)|(message.data[0]<<56);
              
                          sprintf(batt_temp, "%x,%x,%x,%x,%x,%x,%x,%x", message.data[0],message.data[1],message.data[2],message.data[3],message.data[4],message.data[5],message.data[6],message.data[7]);

                          printf("batt temp------------>%c",batt_temp[0]);
                          printf("batt temp------------>%c",batt_temp[1]);
                          printf("batt temp------------>%c",batt_temp[2]);
                          printf("batt temp------------>%c",batt_temp[3]);
                    }

                  }
              
              else
                    {

                      //   ESP_LOGE(EXAMPLE_TAG, " ID not match - %lx ",message.identifier );

                      //   vTaskDelay(pdMS_TO_TICKS(250));

                    }
            } 
        }
  // xSemaphoreGive(done_sem);
  vTaskDelete(NULL);
}

 
 
void app_main(void)
{
gpio_set_direction(Ignition,GPIO_MODE_INPUT);
gpio_set_pull_mode(Ignition,GPIO_PULLUP_ONLY);
 
gpio_set_direction(Reverse,GPIO_MODE_INPUT);
gpio_set_pull_mode(Reverse,GPIO_PULLUP_ONLY);
 
gpio_set_direction(ModeL,GPIO_MODE_INPUT);
gpio_set_pull_mode(ModeL,GPIO_PULLUP_ONLY);
 
gpio_set_direction(ModeR,GPIO_MODE_INPUT);
gpio_set_pull_mode(ModeR,GPIO_PULLUP_ONLY);
 
gpio_set_direction(Break,GPIO_MODE_INPUT);
gpio_set_pull_mode(Break,GPIO_PULLUP_ONLY);
 
gpio_set_direction(SideStand,GPIO_MODE_INPUT);
gpio_set_pull_mode(SideStand,GPIO_PULLUP_ONLY);
 
 
 
ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config)); /// driver install
ESP_LOGI(EXAMPLE_TAG, "Driver installed");
 
ESP_ERROR_CHECK(twai_start()); /// driver start
 
ESP_LOGI(EXAMPLE_TAG, "Driver started");
 
vTaskDelay(pdMS_TO_TICKS(100)) ;
 
xTaskCreate(throttle_percentage , "throttle", 4096, NULL, 8, NULL);
xTaskCreate(pcb_temp , "pcb temp", 4096, NULL, 8, NULL);
xTaskCreate(motor_temp , "motor temp", 4096, NULL, 8, NULL);
xTaskCreate(battery_temp , "battery temperature", 4096, NULL, 8, NULL);
xTaskCreate(controller_temp , "motor controller temperature", 4096, NULL, 8, NULL);
xTaskCreate(soc_battery, "battery SoC", 4096, NULL, 8, NULL);
xTaskCreate(Button_input, "Button_tsk", 4096, NULL, 8, NULL);
xTaskCreate(twai_transmit_task, "Transmit_Tsk", 4096, NULL, 8, NULL);
xTaskCreate(twai_receive_task, "receive_task", 4096, NULL, 8, NULL);
printf("Transmit done------------------>");
}