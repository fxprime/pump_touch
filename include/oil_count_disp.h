SoftwareSerial odispSerial;


void oil_count_disp_init() {
    odispSerial.begin(57600, SWSERIAL_8N1, OILMETER_OUT_TX, OILMETER_OUT_TX, false, 256);
	// high speed half duplex, turn off interrupts during tx
	odispSerial.enableIntTx(false);
    odispSerial.enableTx(true);

    Serial.println("Oil count disp inited.");
}

static inline void oil_count_disp(const String& text) {
    odispSerial.write(text.c_str(), text.length());
}