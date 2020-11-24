void userko_in() {    setPage(PAGE_USER_KEY_OIL);   Serial.println("=======user key oil.=======");}



void userko_l()
{ 
  cmd_t msg;
  if(get_lastest_cmd(msg)) {
    if(msg.msgid=='p') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_OIL_METER)      fsm.trigger(FORWARD_1);
      if(pageid == PAGE_USER_SEL_TYPE)  fsm.trigger(BACKWARD_1);
      if(pageid == PAGE_USER_KEY_OIL)   nextion_write("selfcheck.en=0");
    }else if(msg.msgid=='b') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_OIL_METER)      nextion_write("page oil_meter"); 
      if(pageid == PAGE_USER_SEL_TYPE)  nextion_write("page user_sel_type");
    }else if(msg.msgid=='t') {
      uint8_t tid = (char)msg.data[0]-'0';
      setOilType(tid);
    } 
  }
  delay(10);
}