
#include <SoftwareSerial.h>
#define BAUD_RATE 9600 
SoftwareSerial nextion;

/* -------------------------------------------------------------------------- */
/*                    send data to nextion with end symbol                    */
/* -------------------------------------------------------------------------- */
inline void nextion_write(const String& msgout)
{
    nextion.write(msgout.c_str());
    nextion.write(0xff);
    nextion.write(0xff);
    nextion.write(0xff);
}

/* -------------------------------------------------------------------------- */
/*                       Task for read data from nextion                      */
/* -------------------------------------------------------------------------- */
void taskOne( void * parameter )
{
    uint8_t st=0;
    uint8_t lenp=0;
    uint8_t leni=0;
    char    msgid;
    uint8_t datalen=0;
    char    payl[64];

    while(1) {
      char buff[64];
      uint32_t len = nextion.available();
      
      
      for (int i = 0; i < len; i++)
      {
        char data=nextion.read(); 

        switch(st) {
          case 0: {
            if(data=='#') {
              st++;
              // Serial.println("Found header");
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
        } 
      }
      
      



      // Serial.write((uint8_t*)buff, len);
 
      // while (Serial.available() > 0) {
      //   nextion.write(Serial.read());
      //   yield();
      // }
      delay(1);
    }
 
 
    Serial.println("Ending task 1");
    vTaskDelete( NULL );
 
}