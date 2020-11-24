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

TaskHandle_t TaskSerial_hd;
TaskHandle_t Taskwiegan_hd;
TaskHandle_t Taskfsm_hd;

void taskFSM( void * parameter );

void setup()
{
  pump_control_init();
  nextion.begin(BAUD_RATE, SWSERIAL_8N1, 33, 32, false, 93, 11);
  nextion.flush();
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


  // xTaskCreatePinnedToCore(
  //                   taskSerial,     /* Task function. */
  //                   "TaskSerial",   /* name of task. */
  //                   10000,       /* Stack size of task */
  //                   NULL,        /* parameter of the task */
  //                   0,           /* priority of the task */
  //                   &TaskSerial_hd,      /* Task handle to keep track of created task */
  //                   1);          /* pin task to core 0 */
 
    
  // xTaskCreatePinnedToCore(
  //                   taskWiegan,     /* Task function. */
  //                   "TaskWiegan",   /* name of task. */
  //                   10000,       /* Stack size of task */
  //                   NULL,        /* parameter of the task */
  //                   1,           /* priority of the task */
  //                   &Taskwiegan_hd,      /* Task handle to keep track of created task */
  //                   1);          /* pin task to core 0 */


                    
  xTaskCreatePinnedToCore(
                    taskFSM,     /* Task function. */
                    "TaskFSM",   /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &Taskfsm_hd,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */
 
}
 

void taskFSM( void * parameter )
{
    while(1) {
      fsm.run_machine();

      nextion_write_check();
      taskYIELD();
      delay(1); 
    }
    Serial.println("Ending taskFSM");
    vTaskDelete( NULL );
}


  



void loop()
{

  static uint8_t st     =0;
  static uint8_t lenp   =0;
  static uint8_t leni   =0;
  static char    msgid;
  static uint8_t datalen=0;
  static char    payl[64];


  esp_task_wdt_reset();
  card_reader_runing(); 

  
  uint32_t len = nextion.available();
      
      
  for (int i = 0; i < len; i++)
  {
    char data=nextion.read(); 

    switch(st) {
      case 0: {
        if(data=='#') {
          st++;
          // Serial.println("Found header");
          Serial.println("Core id = " + String(xPortGetCoreID()));
        }else if(data=='\x1A') {
          st=4;
          Serial.println("Found fail char");
        }
        continue;
      }
      case 1: {
        lenp      = data - '0';
        datalen   = lenp-1;
        Serial.printf("len = %d datalen = %d\n", lenp, datalen);
        if(datalen <6 && datalen >0)              st++;
        else {
          nextion.flush();
          st=0;
          Serial.println("---!----!----!---Parse Error---!---!---!---!");
        }
        continue;
      }
      case 2: {
        msgid     = data;
        // Serial.printf("msgid = %c\n", msgid);
        st++;
        continue;
      }
      case 3: {
        
        // Serial.printf("Collect pl %d\n", leni);
        payl[leni++] = data;

        if(leni>=datalen) {

          switch(datalen) {
            case 1: { 
              Serial.printf("received msg %c %d\n", msgid, payl[0]-'0');
              cmd_t msg;
              msg.msgid=msgid;
              msg.lenp=datalen;
              memcpy(&msg.data[0], (uint8_t*)&payl[0], datalen);
              msg.data[1] = (uint8_t)'\0';
              if(msg.msgid == '@') {
                esp_restart();
              }
              _cmd_q.push(msg);
              break;
            } 
            case 2: {
              Serial.printf("received msg %c %d\n", msgid, atoi(payl));
              cmd_t msg;
              msg.msgid=msgid;
              msg.lenp=datalen;
              memcpy(&msg.data[0], (uint8_t*)&payl[0], datalen);
              msg.data[2] = (uint8_t)'\0';
              _cmd_q.push(msg);
              break;
            }
            case 4: {
              Serial.printf("received msg %c %d\n", msgid, toUint32(payl));
              cmd_t msg;
              msg.msgid=msgid;
              msg.lenp=datalen;
              memcpy(&msg.data[0], (uint8_t*)&payl[0], datalen);
              msg.data[4] = (uint8_t)'\0';
              _cmd_q.push(msg);
              break;
            }
          }
          Serial.printf("Cur q = %d\n", _cmd_q.size());
          
          st=0;
          leni=0;
        }
        continue;
      } 
      case 4 : {
        if(data=='\xFF') {
          st++;
          Serial.println(" - fail confirm1");
        }else st=0;
      }
      case 5 : {
        if(data=='\xFF') {
          st++;
          Serial.println(" - fail confirm2");
        }else st=0;
      }
      case 6 : {
        if(data=='\xFF') {
          st++;
          Serial.println(" - fail confirm2");
        }else st=0;
      }
      case 7 : {
        Serial.println(" - fail sure");
        _dataOut_fail_ts = getUTime();
        st=0;
      }
    } 
  } 
}