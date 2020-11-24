
void print_in() {   
  setPage(PAGE_PRINT);     
  Serial.println("========Print========"); 
  printThermal();
}


void print_l()
{ 
  cmd_t msg;
  if(get_lastest_cmd(msg)) {
    if(msg.msgid=='p') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_USER_KEY_OIL)    fsm.trigger(BACKWARD_1);  
      if(pageid == PAGE_PRINT)           nextion_write("selfcheck.en=0");
    }
    if(msg.msgid=='b') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_USER_KEY_OIL)    nextion_write("page user_key_oil");   
    }
    if(msg.msgid=='r') {
      uint8_t val = (char)msg.data[0]-'0';
      if(val==1) { 
        printThermal();
      }
    } 
  } 

  time_ut cur_time = getUTime();
  static time_ut lastUpdate = 0;
  if (cur_time - lastUpdate > 2000000){
      thp.updatePrinterStatus();
      // Serial.printf("Printer status =%x\n",thp_status);
      lastUpdate = cur_time;
  }
  if (thp.status() != thp_status){
      thp_status = thp.status();
      Serial.printf("=== Printer status changed ====%x\n",thp_status);
      String msg="";
      if(thp_status==0) {
        msg="Printer OK";
      }
      if(thp_status&PRN_STAT_BIT_OFFLINE) {
        msg="Printer offline";
      }
      if(thp_status&PRN_STAT_BIT_PAPER_OUT) {
        msg="Paper out";
      }
      if(thp_status&PRN_STAT_BIT_PAPER_ENDING) {
        msg="Paper ending";
      }
      if(thp_status&PRN_STAT_BIT_SHAFT_UNSET) {
        msg="Shaft unset";
      }
      if(thp_status&PRN_STAT_BIT_COVER_OPEN) {
        msg="Cover open";
      }
      if(thp_status&PRN_STAT_BIT_CUTTER_ERROR) {
        msg="Cutter error";
      }
      if(thp_status&PRN_STAT_OTHER_ERROR) {
        msg="Other error";
      }
      if(thp_status&PRN_STAT_UNRECOVERABLE_ERROR) {
        msg="Fatal error";
      }

      Serial.println(msg);
      nextion_write("pstat.txt=\"" + msg + "\"");
 
  }
  delay(10);
}
 
 