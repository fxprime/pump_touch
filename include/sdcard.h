// Libraries for SD card
#include "FS.h"
#include "SD.h"
#include <SPI.h>
 

#include <Arduino.h>
 
/* -------------------------------------------------------------------------- */
/*                               High level dev                               */
/* -------------------------------------------------------------------------- */
static inline void fetchDataSDCard();
static inline void updateToSDCard(const String& action);
void appendLog(fs::FS &fs, const event_t& msg);
void writeLog(fs::FS& fs, const event_t& msg);
bool readLog (fs::FS& fs, std::vector<event_t>& msg_v);
bool getUserStatus(fs::FS &fs);


/* -------------------------------------------------------------------------- */
/*                                Low level dev                               */
/* -------------------------------------------------------------------------- */

void writeFile(fs::FS &fs, const char * path, const uint8_t * message, int size);
void appendFile(fs::FS &fs, const char * path, const uint8_t * message, int size);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void readFile(fs::FS &fs, const char * path);

void sdcard_init() {
  
  Serial.println("SDCARD initing.");
  // Initialize SD card
  SD.begin(SD_CS);  
  if(!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    _sdcard_detected = false;
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    _sdcard_detected = false;
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    _sdcard_detected = false;
    return;    // init failed
  }
  
  fs::FS &fs = SD;
  if(!fs.exists( logbinName )) {
    event_t msg;
    msg.user_uid = 8000;
    sprintf(msg.action, "Init");
    sprintf(msg.last_logout, "0:0:0 0/0/0");    
    writeLog(SD, msg);
  }

  _sdcard_detected = true;
  Serial.println("SDCARD inited.");
}

void sdcard_recheck() {
  
  Serial.println("SDCARD rechecking.");
  SD.end();
  sdcard_init();
}


static inline void fetchDataSDCard() {
  /* ------------------------- Keep try to init sdcard ------------------------ */
  time_ut cur_time = getUTime();
  if(!_sdcard_detected) {
    static time_ut last_try = 0;
    if( cur_time - last_try > 2000000 ) {
      sdcard_init();
      if(_sdcard_detected) nextion_write("vis sdcard,0");
      else                 nextion_write("vis sdcard,1");
      last_try = cur_time;
    } 
    _cur_user.data_fetched = false;

  }
  
  if(_sdcard_detected) { 
    if(!getUserStatus(SD)) { 
      Serial.println("User use first time?!!!??!"); 
    }else{
      Serial.println("Data fetched.");
    }
    _cur_user.data_fetched = true;
  }
}

static inline void updateToSDCard(const String& action) {
  event_t msg;
  msg.user_uid = _cur_user.uid;
  struct tm ts;

  /* --------------------------- Update active epoch -------------------------- */
  _cur_user.active_epoch = (int32_t)getEpochTime();


  ts = getDateTimeFromEpoch(_cur_user.active_epoch);
  strftime(msg.last_active, sizeof(msg.last_active), "%a %Y/%m/%d %H:%M:%S", &ts);

  
  if(_cur_user.logout_epoch!=0) {
    ts = getDateTimeFromEpoch(_cur_user.logout_epoch);
    strftime(msg.last_logout, sizeof(msg.last_logout), "%a %Y/%m/%d %H:%M:%S", &ts);
  }else{
    sprintf(msg.last_logout, "");
  }
  
 
  for(int i=0;i<3;i++) {
    msg.litter[i]         = Litter_now(i)*100;
    msg.litter_today[i]   = TickToLitter(_cur_user.trip_tick, i)*100;
    msg.litter_overall[i] = (_cur_user.overall_liter[i])*100;
  }

  strcpy(msg.action, action.c_str());

  appendLog(SD, msg);
  
}


/* -------------------------------------------------------------------------- */
/*                               High level dev                               */
/* -------------------------------------------------------------------------- */


void appendLog(fs::FS &fs, const event_t& msg) {
  uint8_t* data = new uint8_t[sizeof(event_t)];
  memcpy(&data[0], &msg, sizeof(event_t));
  appendFile(SD, logbinName, data, sizeof(event_t));

  String individual_file= "/uid_" +String(msg.user_uid) + ".csv";

  
  String txt="";
  txt += String(msg.user_uid) + ", ";
  txt += String(msg.action) + ", ";
  txt += String(msg.last_active) + ", ";
  txt += String(msg.last_logout);

  for(int i=0;i<3;i++) {
    txt += ", " + String(msg.litter[i]/100.0, 2) + ", ";
    txt += String(msg.litter_today[i]/100.0, 2) + ", ";
    txt += String(msg.litter_overall[i]/100.0, 2);
  }
  
  txt += "\r";
  appendFile(SD, logFileName, txt.c_str());

  if(!fs.exists( individual_file.c_str())) {
    String header="User ID, Action, Active Time, Logout time, Benzene, Benzene today, Benzene all, Diesel, Diesel today, Diesel all, Engine Oil, Engine Oil today, Engine Oil all\r";
    writeFile(SD, individual_file.c_str(), header.c_str());
  } 

  appendFile(SD, individual_file.c_str(), txt.c_str());
  
  String individual_log= "/uid_" +String(msg.user_uid) + ".bin";
  if(!fs.exists( individual_log.c_str())) {
    writeFile(SD, individual_log.c_str(), data, sizeof(event_t));
  }else{
    appendFile(SD, individual_log.c_str(), data, sizeof(event_t));
  }
  delete[] data; 
}

void writeLog(fs::FS &fs, const event_t& msg) { 
  uint8_t* data = new uint8_t[sizeof(event_t)];
  memcpy(&data[0], &msg, sizeof(event_t));
  writeFile(SD, logbinName, data, sizeof(event_t));
  String header="User ID, Action, Active Time, Logout time, Benzene, Benzene today, Benzene all, Diesel, Diesel today, Diesel all, Engine Oil, Engine Oil today, Engine Oil all\r";
  writeFile(SD, logFileName, header.c_str());


  delete[] data; 
}


bool getUserStatus(fs::FS &fs) {

  event_t msgout;
 
  String individual_log= "/uid_" +String(_cur_user.uid) + ".bin";
 
  File file = fs.open(individual_log);
  if(!file){ Serial.println("Failed to open file for reading"); return false; }
  int rows = file.available()/sizeof(event_t);
  Serial.printf("Data rows = %d\n\n", rows);
  if(rows==0) {
    Serial.println("Cannot find any old information. Oil overall reset then.");
    for(int i=0;i<3;i++) {
      _cur_user.overall_liter[i] = 0.0; 
    }

    return false;
  }
  if(file.seek( (rows-1)*sizeof(event_t) )) {
      
    uint8_t* raw = new uint8_t[sizeof(event_t)];
    file.read(raw, sizeof(event_t)); 

    memcpy(&msgout, &raw[0], sizeof(event_t)); 
    delete[] raw;

    for(int i=0;i<3;i++) {
      _cur_user.overall_liter[i] = msgout.litter_overall[i]/100.0;
      Serial.printf("Oil overall %d = %.2f\n", i, _cur_user.overall_liter[i]);
    }


    return true;
  }else{
    Serial.println("Cannot seek");
    return false;
  } 

}


bool readLog(fs::FS &fs, std::vector<event_t>& msg_v) {
  Serial.printf("Reading file: %s\n", logbinName);

  File file = fs.open(logbinName);
  if(!file){ Serial.println("Failed to open file for reading"); return false; }

  Serial.printf("Data rows = %d\n\n", file.available()/sizeof(event_t));

  while(file.available()){
      uint8_t* raw = new uint8_t[sizeof(event_t)];

      if(file.available() >= sizeof(event_t)) {
        file.read(raw, sizeof(event_t)); 
        event_t msg;

        memcpy(&msg, &raw[0], sizeof(event_t)); 
        msg_v.push_back(msg);
      }
      delete[] raw;
      delay(10);
  }
  file.close();
  return true;
}





















/* -------------------------------------------------------------------------- */
/*                                Lowlevel dev                                */
/* -------------------------------------------------------------------------- */

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.printf("Data size %d Read from file: %d\n\n", sizeof(event_t), file.available());
    while(file.available()){
        uint8_t* raw = new uint8_t[sizeof(event_t)];

        if(file.available() >= sizeof(event_t)) {
          file.read(raw, sizeof(event_t)); 
          event_t msg;

          memcpy(&msg, &raw[0], sizeof(event_t)); 
        }
        delete[] raw;
        delay(10);

    }
    file.close();
}

 

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const uint8_t * message, int size) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.write(message, size)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const uint8_t * message, int size) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.write(message, size)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}
// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
