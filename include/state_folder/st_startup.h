void startup_in() {   setPage(PAGE_STARTUP);        Serial.println("=========startup.==========");}

void startup_l()
{

  delay(10); 
 
  cmd_t msg;
  if(get_lastest_cmd(msg)) {
    if(msg.msgid=='p') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_IDENTIFY) fsm.trigger(FORWARD_1);
      if(pageid == PAGE_STARTUP)           nextion_write("selfcheck.en=0");
    } 
  }

  static time_ut last_send = 0;
  if( getUTime() - last_send > 333333 ) {
    last_send = getUTime();
    nextion_write("page identify");
  }
  
}
 