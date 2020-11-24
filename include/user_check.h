
#include <Wiegand.h>
#include <stdlib.h> 
// The object that handles the wiegand protocol
Wiegand wiegand;

void stateChanged(bool plugged, const char* message);
void receivedData(uint8_t* data, uint8_t bits, const char* message);
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message);

#include <esp_task_wdt.h>

void card_reader_init() {
    //Install listeners and initialize Wiegand reader
  wiegand.onReceive(receivedData, "Card readed: ");
  wiegand.onReceiveError(receivedDataError, "Card read error: ");
  wiegand.onStateChange(stateChanged, "State changed: ");
  wiegand.begin(Wiegand::LENGTH_ANY, true);

  //initialize pins as INPUT
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);


  esp_task_wdt_init(WDTO_1S, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch


  Serial.println("CardReader inited");
}

static inline void card_reader_runing() {
  // Checks for pending messages
  wiegand.flush();

  // Check for changes on the the wiegand input pins
  wiegand.setPin0State(digitalRead(PIN_D0));
  wiegand.setPin1State(digitalRead(PIN_D1));
}


// void taskWiegan( void * parameter )
// {
//     while(1) {
//         esp_task_wdt_reset();
//         card_reader_runing(); 
//         // taskYIELD();
//     }

//     Serial.println("Ending taskWiegan");
//     vTaskDelete( NULL );
    
// }
// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char* message) {
    Serial.print(message);
    Serial.println(plugged ? "CONNECTED" : "DISCONNECTED");
    _raw_card_reader.is_connected = plugged;
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
    Serial.print(message);
    Serial.print(bits);
    Serial.print("bits / ");
    //Print value in HEX
    uint8_t bytes = (bits+7)/8;

    String name="";
    for (int i=0; i<bytes; i++) {
        Serial.print(data[i] >> 4, 16);
        Serial.print(data[i] & 0xF, 16);
        name+=String(data[i]>> 4, 16);
        name+=String(data[i] & 0xF, 16);
    }
    

    _raw_card_reader.user_id            = strtoul(name.c_str(), nullptr, 16);
    _raw_card_reader.last_login_time    = getTime();
    _raw_card_reader.last_login_epoch   = getEpochTime();

    Serial.printf("\nUser id %d\nLoin data: %s\nLogin time : %s\nEpoch : %ld\n" 
                                                                , _raw_card_reader.user_id
                                                                , getDate().c_str()
                                                                , _raw_card_reader.last_login_time.c_str()
                                                                , (int32_t)_raw_card_reader.last_login_epoch);

 
    Serial.println();
}

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
    Serial.print(message);
    Serial.print(Wiegand::DataErrorStr(error));
    Serial.print(" - Raw data: ");
    Serial.print(rawBits);
    Serial.print("bits / ");

    //Print value in HEX
    uint8_t bytes = (rawBits+7)/8;
    for (int i=0; i<bytes; i++) {
        Serial.print(rawData[i] >> 4, 16);
        Serial.print(rawData[i] & 0xF, 16);
    }
    Serial.println();
}

static inline bool isLoggedin() {
    return (bool)_cur_user.logged_in;
}

static inline bool isAdmin() {
    return (bool)_cur_user.admin;
}

static inline void Login(bool flag) {
    if(_cur_user.logged_in == flag) return;

    if(flag) {
        _cur_user.admin         = _raw_card_reader.user_id>=8000;
        _cur_user.active_epoch   = (int32_t)getEpochTime();
        _cur_user.logout_epoch  = 0;
        _cur_user.logged_in     = 1;
        _cur_user.name          = "User id : " + String(_raw_card_reader.user_id);
        _cur_user.uid           = _raw_card_reader.user_id;

        String dataout;
        dataout = "tm0.en=1";
        nextion_write(dataout);
        dataout = "p0.pic=9";
        nextion_write(dataout);
        dataout = "identify._username.txt=\"" + _cur_user.name  + "\"";
        nextion_write(dataout);
        dataout = "identify._admin_mode.val=" + String(_cur_user.admin);
        nextion_write(dataout); 
    }else{
        _cur_user.logout_epoch  = (int32_t)getEpochTime();
        
        updateToSDCard("Logout");

        /* ------------------------------- Reset state ------------------------------ */
        memset(&_cur_user, 0, sizeof(_cur_user));
        _cur_user.logout_epoch  = (int32_t)getEpochTime();

        String dataout;
        dataout = "identify._username.txt=\"\"";
        nextion_write(dataout);
        dataout = "identify._login_status.val=0";
        nextion_write(dataout);
        dataout = "page identify";
        nextion_write(dataout);
    } 
}

static inline void checkLogin() {
    bool is_not_logged_out  = _cur_user.logout_epoch    < (int32_t)_raw_card_reader.last_login_epoch;
    bool detect_card        = _cur_user.active_epoch     < (int32_t)_raw_card_reader.last_login_epoch;
    bool new_login          =  is_not_logged_out && detect_card && _raw_card_reader.is_connected;

    // Serial.printf("%d %d %d\n", is_not_logged_out, detect_card, new_login);
    if(new_login) {
        _cur_user.uid = _raw_card_reader.user_id;
        sdcard_recheck();
        fetchDataSDCard();
        if(_cur_user.data_fetched)
            Login(true);
    }
    
}

static inline void fakeLoginAdmin() {
    _cur_user.admin=1;
    _cur_user.logged_in=1;
    _cur_user.name="Thanabadee Bulunseechart";
    _cur_user.uid=1;

    String dataout;
    dataout = "tm0.en=1";
    nextion_write(dataout);
    dataout = "p0.pic=9";
    nextion_write(dataout);
    dataout = "identify._username.txt=\"" + _cur_user.name  + "\"";
    nextion_write(dataout);
    dataout = "identify._admin_mode.val=" + String(_cur_user.admin);
    nextion_write(dataout); 
}
 