time_ut last_connect = 0;

void clkset_in() {
    Serial.println("============clock setting========");
    setPage(PAGE_WAIT_WIFI);
    last_connect = 0;
    
}
void clkset_l() {

    cmd_t msg;
    if(get_lastest_cmd(msg)) {
        if(msg.msgid=='p') {
            uint8_t pageid = (uint8_t)atoi((char*)&msg.data[0]);
            Serial.printf("********* data %d *************\n", pageid);
            if(pageid == PAGE_HOME)   {
                wifi_stop(); 
                fsm.trigger(BACKWARD_1);
            }
            if(pageid == PAGE_WAIT_WIFI) nextion_write("selfcheck.en=0");


        }else if(msg.msgid=='b') {
            uint8_t pageid = (uint8_t)atoi((char*)&msg.data[0]);
            Serial.printf("********* data %d *************\n", pageid);
            if(pageid == PAGE_HOME)   { 
                nextion_write("page home");   
            } 


        }else if(msg.msgid=='j') {
            uint8_t con = (char)msg.data[0]-'0';
            if(con == 1)   {
                nextion_write("b0.txt=\"Connecting..\"");  
                delay(100);
                wifi_start();
                last_connect = getUTime();
            }

            
        }
    }

    if(last_connect!=0) {
        if(getUTime() - last_connect > 10000000) {
            Serial.println("WiFi not found");
            nextion_write("b0.txt=\"Connect\""); 
            last_connect = 0;
             
        }else{ 
            if(WiFi.status()!=WL_CONNECTED) {
                Serial.print(".");
            }else   {
                Serial.printf("WiFi connected! ip:%s\n", WiFi.localIP().toString().c_str());
                last_connect = 0;
                nextion_write("b0.txt=\"Connected\"");  
                udp_init_ntp();

            }
            
        }
    }


    
    if(WiFi.status()==WL_CONNECTED) {
        udp_receive_ntp();
        if(clock_ntp_synced()) {
            nextion_write("b0.txt=\"Synced!\""); 
            nextion_write("b0.bco=1024"); 
            clock_ntp_resetflag();
        }
    } 

    delay(10);

}