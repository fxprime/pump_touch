
const       int64_t DEBOUNCE_TIME   = 1500;
volatile    int     _edge_count     = 0;
volatile    int64_t _last_fall      = 0; 
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR handleInterrupt() {
  
  int64_t cur_time = esp_timer_get_time();
  if(cur_time-_last_fall>DEBOUNCE_TIME) {
    _last_fall = cur_time;
    portENTER_CRITICAL_ISR(&mux);
    _edge_count++;
    inter_t in;
    in.id = _edge_count;
    in.st = cur_time;
    _f_q.push(in); 
    portEXIT_CRITICAL_ISR(&mux);
  }
  
}
 

static inline void flowmeter_init() {
    pinMode(interruptPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE);
    Serial.println("Flowmeter inited.");
}

static inline void flowmeter_result() {
    if(_f_q.size()>0) {
        Serial.printf("!!!! Flow queue is not empty.. %d left clearing  !!!!", _f_q.size());
        while(!_f_q.empty()) _f_q.pop();
    }
    portENTER_CRITICAL_ISR(&mux);
    _edge_count=0;
    portEXIT_CRITICAL_ISR(&mux);
    _cur_user.trip_tick[GetOilType()]+=_cur_user.amout_tick[GetOilType()];
    _cur_user.overall_liter[GetOilType()]+=TickToLitter(_cur_user.amout_tick, GetOilType());
    _cur_user.lastest_liter = TickToLitter(_cur_user.amout_tick, GetOilType());
    
}

static inline void flowmeter_run() {
   if(_f_q.size()>0){

      inter_t out = _f_q.front();
      _f_q.pop();
       
      if( (out.id-1)%2==0 ) {
          _cur_user.amout_tick[GetOilType()]=(out.id-1)/2;
      }
  }
}

static inline void sendCurLiter() { 
    nextion_write("n0.val=" + String( (long)(Litter_now(GetOilType())*100) ));
    Serial.printf("SP %.2f cur %.2f\n", _oil_sp, Litter_now(GetOilType()));
}
static inline void sendCurLiterLimited() {
    long limited_litter = Litter_now_limited(GetOilType())*100 ;
    nextion_write("n0.val=" + String( limited_litter ));
    oil_count_disp("#"+String( limited_litter )+",");
    Serial.printf("SP %.3f cur %.3f\n", _oil_sp, Litter_now_limited(GetOilType()));
} 
static inline void sendCurLiterSetpoint() { 
    nextion_write("n0.val=" + String( long( ceilf(_oil_sp*100) ) ));
    Serial.printf("Send oil setpoint = %.4f.\n", ceilf(_oil_sp*100));
} 