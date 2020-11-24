
#include <SoftwareSerial.h>
#define BAUD_RATE 9600 
SoftwareSerial nextion;

/* -------------------------------------------------------------------------- */
/*                    send data to nextion with end symbol                    */
/* -------------------------------------------------------------------------- */
static inline void nextion_write(const String& msgout)
{
    dataOut_t msg;
    msg.msg       = msgout;
    msg.was_send  = false;
    msg.timestamp = getUTime();
    _dataOut_q.push(msg);
    Serial.println("MsgOut q size = " + String(_dataOut_q.size()));
    // nextion.write(msgout.c_str());
    // nextion.write(0xff);
    // nextion.write(0xff);
    // nextion.write(0xff);
}
static inline void nextion_write_check()
{
    time_ut cur_time = getUTime();
    static uint8_t try_count = 0;
    

    if( _dataOut_q.size()>0 ) {

      dataOut_t &curData = _dataOut_q.front();

      if(curData.was_send==false) {
        curData.was_send = true;
        curData.timestamp = cur_time;
        nextion.write(curData.msg.c_str());
        nextion.write(0xff);
        nextion.write(0xff);
        nextion.write(0xff);
        Serial.println(" - send command "+curData.msg);
      }else{
 
        if(cur_time - curData.timestamp > 100000 ) {
          _dataOut_q.pop();
          try_count = 0;
        }else{
          time_ut check_fail = (_dataOut_fail_ts - curData.timestamp);
          if(check_fail < 100000 && check_fail > 0) {
            curData.timestamp = cur_time;
            curData.was_send = false;
            Serial.println(" - update time and resend");
            _dataOut_fail_ts = 0;
            try_count++;
            if(try_count>20)  {
              _dataOut_q.pop();
              try_count=0;
              Serial.println("!!!!!!!!!!!!!!Give up sending!!!!!!!!!!!!");
            }
          } 
        }
 
      }
 
    }
     
}


// /* -------------------------------------------------------------------------- */
// /*                       Task for read data from nextion                      */
// /* -------------------------------------------------------------------------- */
// void taskSerial( void * parameter )
// {
//     uint8_t st=0;
//     uint8_t lenp=0;
//     uint8_t leni=0;
//     char    msgid;
//     uint8_t datalen=0;
//     char    payl[64];

//     while(1) { 
//       uint32_t len = nextion.available();
      
      
//       for (int i = 0; i < len; i++)
//       {
//         char data=nextion.read(); 

//         switch(st) {
//           case 0: {
//             if(data=='#') {
//               st++;
//               // Serial.println("Found header");
//               Serial.println("Core id = " + String(xPortGetCoreID()));
//             }else if(data=='\x1A') {
//               st=4;
//               Serial.println("Found fail char");
//             }
//             continue;
//           }
//           case 1: {
//             lenp      = data - '0';
//             datalen   = lenp-1;
//             Serial.printf("len = %d datalen = %d\n", lenp, datalen);
//             if(datalen <6 && datalen >0)              st++;
//             else {
//               nextion.flush();
//               st=0;
//               Serial.println("---!----!----!---Parse Error---!---!---!---!");
//             }
//             continue;
//           }
//           case 2: {
//             msgid     = data;
//             // Serial.printf("msgid = %c\n", msgid);
//             st++;
//             continue;
//           }
//           case 3: {
            
//             // Serial.printf("Collect pl %d\n", leni);
//             payl[leni++] = data;

//             if(leni>=datalen) {

//               switch(datalen) {
//                 case 1: { 
//                   Serial.printf("received msg %c %d\n", msgid, payl[0]-'0');
//                   cmd_t msg;
//                   msg.msgid=msgid;
//                   msg.lenp=datalen;
//                   memcpy(&msg.data[0], (uint8_t*)&payl[0], datalen);
//                   msg.data[1] = (uint8_t)'\0';
//                   if(msg.msgid == '@') {
//                     esp_restart();
//                   }
//                   _cmd_q.push(msg);
//                   break;
//                 } 
//                 case 2: {
//                   Serial.printf("received msg %c %d\n", msgid, atoi(payl));
//                   cmd_t msg;
//                   msg.msgid=msgid;
//                   msg.lenp=datalen;
//                   memcpy(&msg.data[0], (uint8_t*)&payl[0], datalen);
//                   msg.data[2] = (uint8_t)'\0';
//                   _cmd_q.push(msg);
//                   break;
//                 }
//                 case 4: {
//                   Serial.printf("received msg %c %d\n", msgid, toUint32(payl));
//                   cmd_t msg;
//                   msg.msgid=msgid;
//                   msg.lenp=datalen;
//                   memcpy(&msg.data[0], (uint8_t*)&payl[0], datalen);
//                   msg.data[4] = (uint8_t)'\0';
//                   _cmd_q.push(msg);
//                   break;
//                 }
//               }
//               Serial.printf("Cur q = %d\n", _cmd_q.size());
              
//               st=0;
//               leni=0;
//             }
//             continue;
//           } 
//           case 4 : {
//             if(data=='\xFF') {
//               st++;
//               Serial.println(" - fail confirm1");
//             }else st=0;
//           }
//           case 5 : {
//             if(data=='\xFF') {
//               st++;
//               Serial.println(" - fail confirm2");
//             }else st=0;
//           }
//           case 6 : {
//             if(data=='\xFF') {
//               st++;
//               Serial.println(" - fail confirm2");
//             }else st=0;
//           }
//           case 7 : {
//             Serial.println(" - fail sure");
//             _dataOut_fail_ts = getUTime();
//             st=0;
//           }
//         } 
//       }
      
       
//       delay(1);
//     }
 
 
//     Serial.println("Ending taskSerial");
//     vTaskDelete( NULL );
 
// }