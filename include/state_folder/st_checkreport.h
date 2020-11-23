
void report_in() {    
  setPage(PAGE_REPORT);         
  Serial.println("==========report.==========");
  for(int i=0;i<3;i++) { 
    nextion_write( "td" +String(i) + ".txt=\"" + String(TickToLitter(_cur_user.trip_tick, i), 2) + "\"" ) ;
    nextion_write("ova" +String(i) + ".txt=\"" + String(_cur_user.overall_liter[i], 2) + "\"" );
  }
}


void report_l()
{ 
  cmd_t msg;
  if(get_lastest_cmd(msg)) {
    if(msg.msgid=='p') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_HOME) fsm.trigger(BACKWARD_1);
      if(pageid == PAGE_REPORT)           nextion_write("selfcheck.en=0");
    }
  } 
  
  time_ut cur_time = getUTime();
  static time_ut last_send = 0;
  if( cur_time - last_send > 200000) {
    last_send = cur_time;
    for(int i=0;i<3;i++) { 
      nextion_write( "td" +String(i) + ".txt=\"" + String(TickToLitter(_cur_user.trip_tick, i), 2) + "\"" ) ;
      nextion_write("ova" +String(i) + ".txt=\"" + String(_cur_user.overall_liter[i], 2) + "\"" );
    }
  }
  
  delay(10);
}