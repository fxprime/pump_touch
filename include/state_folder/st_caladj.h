float temp_cal = 1.00;

void caladj_in() {
  setPage(PAGE_CAL_ADJUST);      
  Serial.println("=========cal adjust.========");
  temp_cal = _cal_l_ratio_comp[GetOilType()];

}

void caladj_l()
{
    cmd_t msg;
    if (get_lastest_cmd(msg))
    {
        if (msg.msgid == 'p')
        {
            uint8_t pageid = (char)msg.data[0] - '0';
            if (pageid == PAGE_HOME)
            {
                flowmeter_result();
                fsm.trigger(FORWARD_1);
            }
            if(pageid == PAGE_CAL_ADJUST)           nextion_write("selfcheck.en=0");
        }
        if (msg.msgid == 'b')
        {
            uint8_t buttonid = (char)msg.data[0] - '0';
            if( buttonid == 1) {
                Serial.println("Calcel caladj. Back home.");
                nextion_write("page home");
            }
            if( buttonid == 2) {
                _cal_l_ratio_comp[GetOilType()] = temp_cal;
                SaveParameters();
                nextion_write("page home");
            }

        }
        if (msg.msgid == 'g')
        {
            uint8_t buttonid = (char)msg.data[0] - '0';

            /* --------------------------------- Big add -------------------------------- */
            if( buttonid == 1) {
                temp_cal+=0.01; 

            }

            /* -------------------------------- Big minus ------------------------------- */
            if( buttonid == 2) {
                temp_cal-=0.01;

            }

            /* --------------------------------- Small add ------------------------------ */
            if( buttonid == 3) {
                temp_cal+=0.001;

            }

            /* -------------------------------- Small minus ----------------------------- */
            if( buttonid == 4) {
                temp_cal-=0.001;

            }
        } 
    }

    time_ut cur_time = getUTime();
    static time_ut last_send = 0;
    if( cur_time - last_send > 300000 ) {
        last_send = cur_time;
        nextion_write( "t1.txt=\"" + String(temp_cal,3) + "\"" );

        double temp_val = (double)_cur_user.amout_tick[GetOilType()]*L_PER_PULSE*temp_cal;
        nextion_write( "x0.val=" + String( (long)( temp_val*100 ) ) );
    }

    
}