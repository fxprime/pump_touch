#include <Arduino.h>
#include "config.h"
#include "rtc.h"
#include "sdcard.h"
#include "thermal_printer.h"
#include "nextion_com.h"
#include "Fsm.h"
 
#include "oil_count_disp.h"
#include "flowmeter.h"
#include "pump_cnt.h"
#include "user_check.h"
#include "wifi_module.h"
#include "states.h"



/* ---------- https://github.com/espressif/arduino-esp32/issues/595 --------- */
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

 
void setup()
{
  pump_control_init();
  nextion.begin(BAUD_RATE, SWSERIAL_8N1, 33, 32, false, 93, 11);
  Serial.begin(115200);
  Serial.println("Start system"); 
  Parameters_init();
  wifi_init();
  rtc_init();
  states_init();
  flowmeter_init();
  card_reader_init();
  sdcard_init();
  serial_printer_init();
  oil_count_disp_init();


  xTaskCreate(
                  taskOne,          /* Task function. */
                  "TaskOne",        /* String with name of task. */
                  10000,            /* Stack size in bytes. */
                  NULL,             /* Parameter passed as input of the task */
                  0,                /* Priority of the task. */
                  NULL);            /* Task handle. */

  xTaskCreate(
                  taskTwo,          /* Task function. */
                  "TaskTwo",        /* String with name of task. */
                  10000,            /* Stack size in bytes. */
                  NULL,             /* Parameter passed as input of the task */
                  1,                /* Priority of the task. */
                  NULL);            /* Task handle. */

}
 
void loop()
{
  
  fsm.run_machine();
  

  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;


  delay(10);
}