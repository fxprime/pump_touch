void userst_in() {    setPage(PAGE_USER_SEL_TYPE);  Serial.println("=======user sel type.======");}

void userst_l()
{ 
  cmd_t msg;
  if(get_lastest_cmd(msg)) {
    if(msg.msgid=='p') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_USER_KEY_OIL) fsm.trigger(FORWARD_1);
      if(pageid == PAGE_HOME)         fsm.trigger(BACKWARD_1);
      if(pageid == PAGE_USER_SEL_TYPE)           nextion_write("selfcheck.en=0");
    }else if(msg.msgid=='b') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_USER_KEY_OIL) nextion_write("page user_key_oil");
      if(pageid == PAGE_HOME)         nextion_write("page home"); 
    } 
  }
  delay(10);
}