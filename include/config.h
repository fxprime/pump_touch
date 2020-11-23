
#include <queue>
// #include <EEPROMex.h>
#include <EEPROM.h>
#include <math.h>

using namespace std;



/* -------------------------------------------------------------------------- */
/*                              Constant variable                             */
/* -------------------------------------------------------------------------- */

#define L_PER_PULSE         10.02/397.0 //Liter per a single pulse define by sensor
#define L_MAX_DEFAULT       50          //Liter max
#define L_MIN_DEFAULT       1           //Liter min
#define DELAY_DEFAULT       3000        //Millisecond
#define L_COMP_DEFAULT      1.0         //Compensate ratio default
#define SP_L_DEFAULT        5.0         //Setpoint for leter default


/* -------------------------------------------------------------------------- */
/*                                   Pinout                                   */
/* -------------------------------------------------------------------------- */

// Flowmeter input pin
const byte interruptPin = 25;

// Card reader wiegand port
#define PIN_D0 16
#define PIN_D1 17

// SD card select pin (Hardware spi)
#define SD_CS 5

// Thermal printer
#define PRINTER_RX_PIN              27
#define PRINTER_TX_PIN              26
#define PRINTER_BAUDRATE            19200

// Relay 3ch 
const int relay_pin[3] = {15, 2, 4};

// Oil meter output
#define OILMETER_OUT_TX             13

/* -------------------------------------------------------------------------- */
/*                              Useful data type                              */
/* -------------------------------------------------------------------------- */

#define toUint32(X)     *(uint32_t*)(&X)
#define toUint8(X)      *(uint8_t*) (&X)
#define toSint8(X)      *(int8_t*)  (&X)
#define toUint(X)       *(uint16_t*)(&X)
#define toSint(X)       *(int16_t*) (&X)
#define toFlt(X)        *(float*)   (&X)
#define toLong(X)       *(int32_t*) (&X)


#define getUTime()   esp_timer_get_time()

typedef int64_t time_ut;

/* ------------------------- Page number define here ------------------------ */
typedef enum{
    PAGE_STARTUP=0,
    PAGE_IDENTIFY,
    PAGE_HOME,
    PAGE_SETTING,
    PAGE_USER_SEL_TYPE,
    PAGE_USER_KEY_OIL,
    PAGE_OIL_METER,
    PAGE_PRINT,
    PAGE_CAL_ADJUST,
    PAGE_REPORT,
    PAGE_WAIT_WIFI
}page_t;

/* -------------------------------- oil type -------------------------------- */
typedef enum{ 
    BENZENE=0,
    DIESEL,
    ENGINE_OIL
}oil_t;

/* ---------------------------- command data type --------------------------- */
typedef struct {
  char msgid;
  uint8_t lenp;
  uint8_t data[64];
}cmd_t;

/* ------------------------------- User struct ------------------------------ */
typedef struct {
    uint8_t     logged_in           =0;
    uint8_t     admin               =0;
    String      name                ="";
    uint32_t    uid                 =0;
    uint8_t     data_fetched        =0;
    uint8_t     oil_type            =0;
    
    uint32_t    amout_tick[3]       ={0,0,0};
    uint32_t    trip_tick[3]        ={0,0,0};
    double      lastest_liter       =0;
    double      overall_liter[3]    ={0,0,0};
    uint8_t     cal_mode            =0;
    uint8_t     pump_runing         =0;
    uint8_t     autoplay_timer      =0;
    int32_t     active_epoch        =0;
    int32_t     logout_epoch        =0; 
    int32_t     startpump_epoch     =0; 
}user_t;

/* --------------------------- Flowmter tick queue -------------------------- */
typedef struct {
  int64_t st;
  int     id;
}inter_t;
/* --------------------------- Card reader struct --------------------------- */
typedef struct {
    uint8_t is_connected;
    uint32_t user_id;
    String  last_login_time;
    time_t  last_login_epoch;
}card_reader_t;


/* -------------------------- SD card memory format ------------------------- */

const char* logbinName = "/log.bin";
const char* logFileName = "/log.csv";


#pragma pack(push, 1)

typedef struct {
  uint32_t user_uid;
  uint32_t litter[3];
  uint32_t litter_today[3];
  uint32_t litter_overall[3];
  char  last_active[30];
  char  last_logout[30];
  char  action[30];
}event_t;

#pragma pack(pop)


/* -------------------------------------------------------------------------- */
/*                                  Variable                                  */
/* -------------------------------------------------------------------------- */
float               _l_max      =L_MAX_DEFAULT;
float               _l_min      =L_MIN_DEFAULT;
float               _delay      =DELAY_DEFAULT;
std::queue<cmd_t>   _cmd_q;
user_t              _cur_user;
page_t              _cur_page;
time_ut             _oil_meter_st=0;

/* ---------------------------- Setpoint variable --------------------------- */
double      _oil_sp              =SP_L_DEFAULT;

/* ------------------------------ sdcard online ----------------------------- */

bool _sdcard_detected=false;

/* --------------------------- Flowmeter variable --------------------------- */
std::queue<inter_t> _f_q;

card_reader_t _raw_card_reader;
  

/* ---------------------- Calibration cutoff compensate --------------------- */

double          _cal_l_ratio_comp[3] = {L_COMP_DEFAULT,L_COMP_DEFAULT,L_COMP_DEFAULT}; 
/* ----------------------------- eeprome address ---------------------------- */

const int eeprom_signature          = 0xb4;
const int maxAllowedWrites          = 80;
const int memBase                   = 350;
const int _signature_address        = 0;
const int _cal_l_ratio_address[3]   = {150,160,170}; 
const int _sp_liter_address         = 180;
const int _l_min_address            = 190;
const int _l_max_address            = 200;
const int _delay_address            = 210;



/* -------------------------------------------------------------------------- */
/*                               Useful function                              */
/* -------------------------------------------------------------------------- */

static inline void setPage(page_t page) {
    _cur_page = page;
}
static inline page_t curPage() {
    return _cur_page;
}

inline bool get_lastest_cmd(cmd_t& msgout){
    if(_cmd_q.size()>0) {
        msgout=_cmd_q.front();
        _cmd_q.pop();
        return true;
    }else{
        return false;
    }
}

static inline double TickToLitter(const uint32_t tick[], const int& id) {
    return (double)tick[id]*L_PER_PULSE*_cal_l_ratio_comp[id];
}
static inline double Litter_now(const int& id) {
    return TickToLitter(_cur_user.amout_tick, id);
}
static inline double Litter_now_limited(const int& id) {
    float val =(float)TickToLitter(_cur_user.amout_tick, id);
    float constrained = ceilf(min( val*100, (float)_oil_sp*100))/100.0;
    return constrained;
}
static inline double Litter_trip_now(const int& id) {
    return TickToLitter(_cur_user.trip_tick, id);
}   
static inline int GetOilType() {
    return _cur_user.oil_type;
}
static inline void Delay_set(const float& delay)
{ _delay = delay;  } 
static inline void Liter_max_set(const float& l_max)
{ _l_max = l_max;  } 
static inline void Liter_min_set(const float& l_min)
{ _l_min = l_min;  } 
static inline void Liter_SP_set(const float& setpoint)
{
    _oil_sp = setpoint;
    _oil_sp = constrain(_oil_sp, _l_min, _l_max);
    Serial.printf("Set liter sp to %.2f lim(%.2f, %.2f)\n", _oil_sp, _l_min, _l_max);
} 
static inline bool Liter_SP_is_enough() { 
    return _oil_sp > _l_min;
}
static inline bool Water_is_full(const int& id) { 
    return ( Litter_now(id) > _oil_sp );
}
static inline void Liter_SP_reset() {
    _oil_sp = 0;
}
static inline void Liter_tick_reset() {
    for(int id=0;id<3;id++)
        _cur_user.amout_tick[id]=0;
    Serial.println("Litter tick reset.");
}
static inline void Cal_cmp_set(const int& id, const float& value) {
    _cal_l_ratio_comp[id] = value;
} 
static inline void Cal_cmp_DEFAULT() {
    for(int i=0;i<3;i++)
        _cal_l_ratio_comp[i] = L_COMP_DEFAULT;
}


static inline bool UpdateDoubleParam(const int& adds, const double& val) {
    double value_test;
    
    value_test = EEPROM.readDouble(adds);
    if( fabs(val - value_test) > __DBL_EPSILON__ ) {
        Serial.println("Write OK");
        EEPROM.writeDouble(adds, val);   
    }else {
        Serial.println("Same data!!");
    }
    delay(20);
}
static inline void SaveParameters() { 
    for(int i=0;i<3;i++)
        UpdateDoubleParam(_cal_l_ratio_address[i], _cal_l_ratio_comp[i]);
    UpdateDoubleParam(_sp_liter_address, _oil_sp);
    UpdateDoubleParam(_l_min_address, _l_min);
    UpdateDoubleParam(_l_max_address, _l_max);
    UpdateDoubleParam(_delay_address, _delay);
    EEPROM.commit();
}

void Parameters_init() {
    Serial.println("Parameter initing.");
    if (!EEPROM.begin(1000)) {
        Serial.println("Failed to initialise EEPROM");
        Serial.println("Restarting...");
        delay(1000);
        ESP.restart();
    }
    delay(100);
 

    int test_signature;
    test_signature              = EEPROM.readUInt(_signature_address);
    Serial.printf("Signature = %d\n", test_signature);




    if(test_signature == eeprom_signature) {


        /* ----------------------------- Read cal ratio ----------------------------- */
        for(int i=0;i<3;i++) {
            _cal_l_ratio_comp[i]   = EEPROM.readDouble(_cal_l_ratio_address[i]);
            if(isnan(_cal_l_ratio_comp[i]))    { _cal_l_ratio_comp[i]     = L_COMP_DEFAULT;};
            _cal_l_ratio_comp[i]   = constrain(_cal_l_ratio_comp[i]   , 0.0, 100000.0);
        }

        /* ----------------------------- Read max liter ----------------------------- */
        _l_max           = EEPROM.readDouble(_l_max_address); 
        if(isnan(_l_max))            { _l_max             = L_MAX_DEFAULT;}; 

        
        /* ----------------------------- Read min liter ----------------------------- */
        _l_min           = EEPROM.readDouble(_l_min_address); 
        if(isnan(_l_min))            { _l_min             = L_MIN_DEFAULT;}; 

        /* ------------------------------- Read delay ------------------------------- */
        _delay           = EEPROM.readDouble(_delay_address); 
        if(isnan(_delay))            { _delay             = DELAY_DEFAULT;}; 


        /* ------------------------------ Read sp liter ----------------------------- */
        _oil_sp           = EEPROM.readDouble(_sp_liter_address); 
        if(isnan(_oil_sp))            { _oil_sp             = SP_L_DEFAULT;};
        _oil_sp           = constrain(_oil_sp           , _l_min, _l_max);

        String params = "Found old parameter :\n";
        params += "cal[0]=" + String(_cal_l_ratio_comp[0],5) + "\n";
        params += "cal[1]=" + String(_cal_l_ratio_comp[1],5) + "\n";
        params += "cal[2]=" + String(_cal_l_ratio_comp[2],5) + "\n";
        params += "sp    =" + String(_oil_sp,2) + "\n";
        params += "l_min =" + String(_l_min,2) + "\n";
        params += "l_max =" + String(_l_max,2) + "\n";
        params += "delay =" + String(_delay) + "\n";
        Serial.println(params);
    }else{
        EEPROM.writeUInt     (_signature_address     , eeprom_signature);
        EEPROM.writeDouble  (_cal_l_ratio_address[0]   , L_COMP_DEFAULT);
        EEPROM.writeDouble  (_cal_l_ratio_address[1]   , L_COMP_DEFAULT);
        EEPROM.writeDouble  (_cal_l_ratio_address[2]   , L_COMP_DEFAULT);
        EEPROM.writeDouble  (_sp_liter_address      , SP_L_DEFAULT);
        EEPROM.writeDouble  (_l_max_address         , L_MAX_DEFAULT);
        EEPROM.writeDouble  (_l_min_address         , L_MIN_DEFAULT);
        EEPROM.writeDouble  (_delay_address         , DELAY_DEFAULT);
        EEPROM.commit();
        Serial.println("Not found old parameter. Using default.");
    }

    Serial.println("Parameter inited.");
}

/* -------------------------------- Oil type -------------------------------- */
static inline void setOilType(uint8_t type) {
    switch(type) {
        case BENZENE    : _cur_user.oil_type=0; Serial.println("Oil type = BENZENE."); break;
        case DIESEL     : _cur_user.oil_type=1; Serial.println("Oil type = DIESEL."); break;
        case ENGINE_OIL : _cur_user.oil_type=2; Serial.println("Oil type = ENGINE_OIL."); break;
        default: Serial.printf("Oil type : Invalid. %d\n", type);break;
    }
}


inline void nextion_write(const String& msgout);

static inline void SendParameters() {
    String dataout; 
    dataout = "setting._l_max.val="+ String(long(_l_max*100));
    nextion_write(dataout);
    dataout = "setting._l_min.val="+ String(long(_l_min*100));
    nextion_write(dataout); 
    dataout = "setting._delay.val="+ String(long(_delay/1000));
    nextion_write(dataout); 
    dataout = "setting._sp.val="+ String(long(_oil_sp*100));
    nextion_write(dataout); 
    Serial.println("Send out data");
}