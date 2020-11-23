void identify_in() {  setPage(PAGE_IDENTIFY);       Serial.println("========identify.==========");} 



void identify_l()
{ 

  // /* --------------------------- Fake login for now --------------------------- */
  // static time_ut fake_login_st = 0;
  // time_ut cur_time = getUTime();
  // if(cur_time-fake_login_st>3000000) {
  //   fake_login_st=cur_time;
  //   fakeLoginAdmin(); 
  // } 
  /* --------------------------- get lastest command -------------------------- */
  cmd_t msg;
  if(get_lastest_cmd(msg)) {
    if(msg.msgid=='p') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_HOME) {
        SendParameters();
        fsm.trigger(FORWARD_1);
      }
      if(pageid == PAGE_IDENTIFY)           nextion_write("selfcheck.en=0");
    } 
  }

  checkLogin();

  

  delay(10);
}


void identify_out() {
  updateToSDCard("Login");
}

