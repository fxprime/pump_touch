void setting_in() {   setPage(PAGE_SETTING);        Serial.println("=========setting.==========");}

void setting_l()
{
  cmd_t msg;
  if(get_lastest_cmd(msg)) {
    if(msg.msgid=='p') {
      uint8_t pageid = (uint8_t)atoi((char*)&msg.data[0]);
      Serial.printf("Chars %c %c page = %d\n", (char)msg.data[0], (char)msg.data[1], pageid);
      if(pageid == PAGE_HOME) fsm.trigger(BACKWARD_1);
      if(pageid == PAGE_SETTING)           nextion_write("selfcheck.en=0");
      if(pageid == PAGE_WAIT_WIFI)  {
        nextion_write("page wait_wifi");
        fsm.trigger(FORWARD_1);
      }
    }else if(msg.msgid=='L') {
      _l_max = (float)toLong(msg.data)/100.0; 
    }else if(msg.msgid=='l') {
      _l_min = (float)toLong(msg.data)/100.0; 
    }else if(msg.msgid=='d') {
      _delay = (float)toLong(msg.data)*1000.0; 
    }else if(msg.msgid=='s') {
      _oil_sp = (float)toLong(msg.data)/100.0; 
    }else if(msg.msgid=='z') {
      SaveParameters();
    }else if(msg.msgid=='r') {
      SendParameters();
    } 
  }
  delay(10);
}
 