
void home_in() {      
  _cur_user.cal_mode=0;
  setPage(PAGE_HOME);           Serial.println("==========home.============");
}


void home_l()
{ 

  /* --------------------------- get lastest command -------------------------- */
  cmd_t msg;
  if(get_lastest_cmd(msg)) {
    if(msg.msgid=='p') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_SETTING)        fsm.trigger(FORWARD_1);
      if(pageid == PAGE_REPORT)         fsm.trigger(FORWARD_2);
      if(pageid == PAGE_USER_SEL_TYPE)  fsm.trigger(FORWARD_3);
      if(pageid == PAGE_IDENTIFY)       fsm.trigger(BACKWARD_1); 
      if(pageid == PAGE_HOME)           nextion_write("selfcheck.en=0");
    }else if(msg.msgid=='b') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_SETTING)        nextion_write("page setting");
      if(pageid == PAGE_REPORT)         nextion_write("page report");
      if(pageid == PAGE_USER_SEL_TYPE)  nextion_write("page user_sel_type"); 
    }else if(msg.msgid=='o') {
      Login(false);
    }else if(msg.msgid=='m') {
      uint8_t val = (char)msg.data[0]-'0';
      if(val==1)  _cur_user.cal_mode = 1;
      nextion_write("page user_sel_type");
    } 
  }


  /* -------------------------- Date time send period ------------------------- */
  time_ut cur_time = getUTime();
  static time_ut last_p_time = 0;
  static time_ut last_p_date = 0;
  if( cur_time - last_p_time > 1000000) {
    last_p_time = cur_time; 
    nextion_write("time.txt=\"" + getTime() + "\"");
  }
  if( cur_time - last_p_date > 10000000) {
    last_p_date = cur_time; 
    nextion_write("date.txt=\"" + getDate() + "\"");
  }
  delay(10);
  
}