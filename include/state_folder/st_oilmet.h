
void oilmet_in() {    
  _cur_user.autoplay_timer = 1;
  _oil_meter_st = getUTime();
  
  Liter_tick_reset();
  flowmeter_result();
  setPage(PAGE_OIL_METER);      
  Serial.println("=========oil meter.========");
}


void oilmet_l()
{ 

  bool oil_meet_setpoint = ( Litter_now(GetOilType()) >= _oil_sp ) ;
  cmd_t msg;
  if(get_lastest_cmd(msg)) {
    if(msg.msgid=='p') {
      uint8_t pageid = (char)msg.data[0]-'0';
      if(pageid == PAGE_USER_KEY_OIL)   {
        pump_stop(); 
        flowmeter_result();
        if(_cur_user.amout_tick[GetOilType()]==0) updateToSDCard("CancelFill");
        else                                      updateToSDCard("FillWOPrint");
        Liter_tick_reset();
        nextion_write("page user_key_oil"); 
        fsm.trigger(BACKWARD_1);
      }
      if(pageid == PAGE_PRINT){ 
        pump_stop();
        flowmeter_result();
        updateToSDCard("FillandPrint");
        Liter_tick_reset();
        nextion_write("page print");
        fsm.trigger(FORWARD_1);
      }
      if(pageid == PAGE_CAL_ADJUST){
        pump_stop();
        
        updateToSDCard("Calibrate");
        nextion_write("page cal_adjust");
        fsm.trigger(FORWARD_2);
      }
      if(pageid == PAGE_OIL_METER)           nextion_write("selfcheck.en=0");
    }else if(msg.msgid=='n') {
      float sp = (float)toLong(msg.data)/100.0;
      Serial.printf("Setpoint = %.2f\n", sp);
      Liter_SP_set(sp);
      nextion_write("oil_sp_recheck.val="+String(toLong(msg.data)));
    }else if(msg.msgid=='b') {
      uint8_t play = (char)msg.data[0]-'0';
      if(play && !oil_meet_setpoint)  pump_start();
      else                            pump_stop();
    } 
  }

  time_ut cur_time = getUTime();
  static time_ut last_send=0;
  /* ------------------ Check if litter now is meet setpoint ------------------ */
  if(oil_meet_setpoint) {
    pump_stop(); 
    if(cur_time - last_send > 500000) 
    {
      last_send = cur_time;
      sendCurLiterSetpoint(); 
    }
  }else{

    /* -------------- Countdown autoplay only when page is show up -------------- */
    if( cur_time - _oil_meter_st > _delay * 1000 && _cur_user.autoplay_timer ) {
      pump_start();
      _cur_user.autoplay_timer  = 0;
      _cur_user.startpump_epoch = getEpochTime();
    }
  }
  
  /* --------------- Keep send litter if it changed at rate 10hz -------------- */
  
  static double last_val = 0;
  if(cur_time - last_send > 100000  
      && fabs(last_val - Litter_now_limited(GetOilType())) > __DBL_EPSILON__) 
  {
    last_val  = Litter_now_limited(GetOilType());
    last_send = cur_time;
    sendCurLiterLimited();
    Serial.printf("%.3f %.3f\n", last_val, _oil_sp);
  }
  
  flowmeter_run();
  delay(10);
}
 