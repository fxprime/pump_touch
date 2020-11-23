void pump_control_init() {
    
    for (int i = 0; i < 3; i++)
    {
        pinMode(relay_pin[i], OUTPUT);
        delay(10);
        digitalWrite(relay_pin[i], LOW);
    }
    
    Serial.println("Pump control inited.");
}


static inline bool pump_running() {
    return _cur_user.pump_runing;
}


static inline void pump_start() {
    if(!_cur_user.pump_runing) {
        String dataout; 
        dataout = "_is_play.val=1";
        nextion_write(dataout);
        Serial.println("-----Pump start-----");
        _cur_user.pump_runing = 1;
        digitalWrite(relay_pin[GetOilType()], HIGH);
    } 
}

static inline void pump_stop() {
    if(_cur_user.pump_runing) {
        String dataout; 
        dataout = "_is_play.val=0";
        nextion_write(dataout);
        Serial.println("-----Pump stop-----");
        _cur_user.pump_runing = 0;
        digitalWrite(relay_pin[GetOilType()], LOW);
    } 
}