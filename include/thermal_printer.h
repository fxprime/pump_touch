#include <ThermalPrinter.h> 
#include <string>

#define PRINTER_MACROS              Macros_ThermPrn  // see ThermalPrinter.h

ThermalPrinter thp(PRINTER_RX_PIN, PRINTER_TX_PIN, PRINTER_BAUDRATE);
int thp_status = 0;



uint8_t convee(const string &schar)
{
  if(schar == " ")
    return 32;
  else if (schar == "ก")
    return 161;
  else if (schar == "ข")
    return 162;
  else if (schar == "ฃ")
    return 163;
  else if (schar == "ค")
    return 164;
  else if (schar == "ฅ")
    return 165;
  else if (schar == "ฆ")
    return 166;
  else if (schar == "ง")
    return 167;
  else if (schar == "จ")
    return 168;
  else if (schar == "ฉ")
    return 169;
  else if (schar == "ช")
    return 170;
  else if (schar == "ซ")
    return 171;
  else if (schar == "ซ")
    return 172;
  else if (schar == "ญ")
    return 173;
  else if (schar == "ฎ")
    return 174;
  else if (schar == "ฏ")
    return 175;
  else if (schar == "ฐ")
    return 176;
  else if (schar == "ฑ")
    return 177;
  else if (schar == "ฒ")
    return 178;
  else if (schar == "ณ")
    return 179;
  else if (schar == "ด")
    return 180;
  else if (schar == "ต")
    return 181;
  else if (schar == "ถ")
    return 182;
  else if (schar == "ท")
    return 183;
  else if (schar == "ธ")
    return 184;
  else if (schar == "น")
    return 185;
  else if (schar == "บ")
    return 186;
  else if (schar == "ป")
    return 187;
  else if (schar == "ผ")
    return 188;
  else if (schar == "ฝ")
    return 189;
  else if (schar == "พ")
    return 190;
  else if (schar == "ฟ")
    return 191;
  else if (schar == "ภ")
    return 192;
  else if (schar == "ม")
    return 193;
  else if (schar == "ย")
    return 194;
  else if (schar == "ร")
    return 195;
  else if (schar == "ฤ")
    return 196;
  else if (schar == "ล")
    return 197;
  else if (schar == "ฦ")
    return 198;
  else if (schar == "ว")
    return 199;
  else if (schar == "ศ")
    return 200;
  else if (schar == "ษ")
    return 201;
  else if (schar == "ส")
    return 202;
  else if (schar == "ห")
    return 203;
  else if (schar == "ฬ")
    return 204;
  else if (schar == "อ")
    return 205;
  else if (schar == "ฮ")
    return 206;
  else if (schar == "ฯ")
    return 207;
  else if (schar == "ะ")
    return 208;
  else if (schar == "ั")
    return 209;
  else if (schar == "า")
    return 210;
  else if (schar == "ำ")
    return 211;
  else if (schar == "ิ")
    return 212;
  else if (schar == "ี")
    return 213;
  else if (schar == "ึ")
    return 214;
  else if (schar == "ื")
    return 215;
  else if (schar == "ุ")
    return 216;
  else if (schar == "ู")
    return 217;
  else if (schar == "ฺ")
    return 218; 
  else if (schar == "฿")
    return 223;
  else if (schar == "เ")
    return 224;
  else if (schar == "แ")
    return 225;
  else if (schar == "โ")
    return 226;
  else if (schar == "ใ")
    return 227;
  else if (schar == "ไ")
    return 228;
  else if (schar == "ๅ")
    return 229;
  else if (schar == "ๆ")
    return 230;
  else if (schar == "็")
    return 231;
  else if (schar == "่")
    return 232;
  else if (schar == "้")
    return 233;
  else if (schar == "๊")
    return 234;
  else if (schar == "๋")
    return 235;
  else if (schar == "์")
    return 236;
  else if (schar == "๏")
    return 239;
  else if (schar == "๐")
    return 240;
  else if (schar == "๑")
    return 241;
  else if (schar == "๒")
    return 242;
  else if (schar == "๓")
    return 243;
  else if (schar == "๔")
    return 244;
  else if (schar == "๕")
    return 245;
  else if (schar == "๖")
    return 246;
  else if (schar == "๗")
    return 247;
  else if (schar == "๘")
    return 248;
  else if (schar == "๙")
    return 249;
  else if (schar == "๚")
    return 250;
  else if (schar == "๛")
    return 251; 
  return 0;
}



char* thai_to_bytes(const string& input) { 
    //Spare for maximum char size is equa to input length with each char size 1 (normal english)
    //and minimum size of output is input.length()/3+1 (thai char)
    char* output=(char*)malloc(input.length()+1);
    int chari=0;  //index of Special Char in byte (codepage encoded) 
    int idx=0;    //index of string which contain start of first byte in char because some char in string have 3 bytes

    //Loop until index is out of bound
    while(idx<input.length()) {
        char part[4]; 
        int csize;
        if((uint8_t)input.c_str()[idx]==32) {
            csize=1;
        }else{
            csize=3;
        }
        strncpy(part, &input.c_str()[idx],csize);  
        part[csize]='\0';
        idx+=csize;
        
         
        output[chari]=convee(part);
        chari++;
    } 
    output[chari]='\0';
    return output;
}




void serial_printer_init(){

    thp.begin();
    thp.loadMacros(&PRINTER_MACROS);   

    Serial.println("Thermal printer inited");

}

static inline void printThermal() {
    
    String ticket = "$init$a_center";
    ticket += "$fs_2$title1\n";
    ticket += "$fs_1$title2 $date $time\n";
    ticket += "$name\n\n\n";
    ticket += "$a_left$u_on$oil_type$u_off     : $type\n";
    ticket += "$a_left$u_on$oil_now$u_off      : $oil_count Liter\n";
    ticket += "$a_left$u_on$oil_overall$u_off : $oil_all Liter\n\n\n\n";
    ticket += "$a_center\n \n";
    ticket += "\n $cut_part\n";
 

    Serial.printf("Loading ticket to buffer\n");
    thp.load2Buffer(ticket.c_str());

    // Manually replace user defined macro 


    // add your own macro constants
    uint8_t m1[] = {27,45,1};
    thp.addMacro("$u_on",m1,3);
    uint8_t m2[] = {27,45,0};
    thp.addMacro("$u_off",m2,3); 
    thp.replaceMacros();


    char* m = new char[121]; 
    
    string input;
    
    input = "รายงานการเติมน้ำมัน";
    m = thai_to_bytes(input);
    thp.replaceMacro("$title1",(uint8_t*)m, strlen(m));
  
    input = "ประจำวันที่";
    m = thai_to_bytes(input);
    thp.replaceMacro("$title2",(uint8_t*)m, strlen(m));

    time_t raw = _cur_user.startpump_epoch;
    struct tm datetime = getDateTimeFromEpoch(raw);

    String date = String(datetime.tm_mday) + "/" + String(datetime.tm_mon+1) + "/" + String(datetime.tm_year+1900);
    strcpy(m, date.c_str());
    // sprintf(m, "%d/%d/%d", 31,10,2563);
    thp.replaceMacro("$date",(uint8_t*)m,strlen(m));

    
    String time = String(datetime.tm_hour) + ":" + String(datetime.tm_min) + ":" + String(datetime.tm_sec);
    strcpy(m, time.c_str());

    // sprintf(m, "%d:%d:%d", 0,44,30);
    thp.replaceMacro("$time",(uint8_t*)m,strlen(m)); 

    
    String name = "UID_" + String(_cur_user.uid);
    strcpy(m, name.c_str());
    // m = thai_to_bytes(input);
    thp.replaceMacro("$name",(uint8_t*)m, strlen(m));



    
    input = "ประเภทน้ำมัน";
    m = thai_to_bytes(input);
    thp.replaceMacro("$oil_type",(uint8_t*)m, strlen(m));


    String oiltype;
    switch(GetOilType()) {
        case 0: oiltype = "Benzene";break;
        case 1: oiltype = "Diesel"; break;
        case 2: oiltype = "Engine Oil";break;
        default: oiltype = "Unknow"; break;
    }    
    strcpy(m, oiltype.c_str());
    // input = "น้ำมันเครื่อง";
    // m = thai_to_bytes(input);
    thp.replaceMacro("$type",(uint8_t*)m, strlen(m));


    
    input = "จำนวนน้ำมัน";
    m = thai_to_bytes(input);
    thp.replaceMacro("$oil_now",(uint8_t*)m, strlen(m));


    snprintf(m,50,"%.2f", _cur_user.lastest_liter);
    thp.replaceMacro("$oil_count",(uint8_t*)m,strlen(m));

    
    input = "จำนวนน้ำมันทั้งหมด";
    m = thai_to_bytes(input);
    thp.replaceMacro("$oil_overall",(uint8_t*)m, strlen(m));

    snprintf(m,50,"%.2f", _cur_user.overall_liter[GetOilType()]);
    thp.replaceMacro("$oil_all",(uint8_t*)m,strlen(m)); 

    thp.writeBuffer(); 
    
    //Full cut command. It is placed here and no effect at all, but if have no this command 
    //cut_part will not work and i dont know why :( 
    thp.printNumStr("1b 69",16);

    thp_status=-1; 
    
    delete []m;

}