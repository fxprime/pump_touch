#include <WiFi.h>
 

unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.windows.com";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
const int DST_OFFSET = 7;

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

const int STATE_NTP_SEND = 1;
const int STATE_NTP_RECV = 2;
const int STATE_NTP_WAIT = 3;
int _state_udp = STATE_NTP_SEND;
bool _clock_is_sync		 = false;
long recvStart;



void wifi_init() {
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi inited.");
}
static inline void wifi_stop();

static inline void wifi_start() {
    
    wifi_stop();
    WiFi.mode(WIFI_MODE_STA);
    /* ------------------- WiFi username password define here ------------------- */
    const char* ssid        = "abcd";
    const char* password    = "1234567890";
    WiFi.begin(ssid, password);
    Serial.printf("Wifi connecting to %s : %s\n", ssid, password);
 
}

static inline void wifi_stop() {
    if(WiFi.status()==WL_CONNECTED) {
        WiFi.disconnect();
        Serial.println("WiFi disconnected.");
        WiFi.mode(WIFI_OFF);
    }
}



// send an NTP request to the time server at the given address
static inline unsigned long sendNTPpacket(IPAddress& address) {
	Serial.println("sending NTP packet...");
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}


static inline void udp_init_ntp() {
    udp.stop();
    Serial.println("Starting UDP");
	udp.begin(localPort);
	Serial.print("Local port: ");
	Serial.println(udp.remoteIP());
    _state_udp = STATE_NTP_SEND;
	_clock_is_sync = false;
}

static inline void udp_receive_ntp() {
    if (_state_udp == STATE_NTP_SEND) {
		//get a random server from the pool
		WiFi.hostByName(ntpServerName, timeServerIP);
		sendNTPpacket(timeServerIP); // send an NTP packet to a time server
		_state_udp = STATE_NTP_RECV;
		recvStart = millis();
	} else if (_state_udp == STATE_NTP_RECV) {
		int cb = udp.parsePacket();
		long elapsed = millis() - recvStart;
		if (!cb) {
			if (elapsed>=1000) {
				Serial.println("no packet yet");
				_state_udp = STATE_NTP_WAIT;
			}
		} else {
			// We've received a packet, read the data from it
			udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

			//the timestamp starts at byte 40 of the received packet and is four bytes,
			// or two words, long. First, esxtract the two words:

			// dt = clock.getDateTime();

			unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
			unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
			// combine the four bytes (two words) into a long integer
			// this is NTP time (seconds since Jan 1 1900):
			unsigned long secsSince1900 = highWord << 16 | lowWord;

			// now convert NTP time into everyday time:
			// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
			const unsigned long seventyYears = 2208988800UL;
			// subtract seventy years:
			unsigned long epoch = secsSince1900 - seventyYears  + DST_OFFSET *3600;

			// clock.setDateTime(epoch);

			Serial.print("packet received, length=");
			Serial.print(cb);
			Serial.print("   elapsed=");
			Serial.print(elapsed);
			Serial.println();

			time_t raw = epoch;
			struct tm dt = getDateTimeFromEpoch(raw);
			Clock.setYear	(dt.tm_year+1900-2000);
    		Clock.setMonth	(dt.tm_mon + 1);
    		Clock.setDate	(dt.tm_mday);
    		Clock.setDoW	(dt.tm_wday);
    		Clock.setHour	(dt.tm_hour);
    		Clock.setMinute	(dt.tm_min);
    		Clock.setSecond	(dt.tm_sec);
			_clock_is_sync = true;

			Serial.printf("%d/%d/%d  %d:%d:%d\n", dt.tm_mday, dt.tm_mon, dt.tm_year, dt.tm_hour, dt.tm_min, dt.tm_sec);
  
			// print Unix time:
			Serial.print(" unix ntp time: ");
			Serial.println(epoch);


			// print the hour, minute and second:
			Serial.print("The UTC time is "); // UTC is the time at Greenwich Meridian (GMT)
			Serial.print((epoch % 86400L) / 3600); // print the hour (86400 equals secs per day)
			Serial.print(':');
			if (((epoch % 3600) / 60) < 10) {
				// In the first 10 minutes of each hour, we'll want a leading '0'
				Serial.print('0');
			}
			Serial.print((epoch % 3600) / 60); // print the minute (3600 equals secs per minute)
			Serial.print(':');
			if ((epoch % 60) < 10) {
				// In the first 10 seconds of each minute, we'll want a leading '0'
				Serial.print('0');
			}
			Serial.println(epoch % 60); // print the second
			Serial.println();
			_state_udp = STATE_NTP_WAIT;
		}
	} else if (_state_udp == STATE_NTP_WAIT) {
		// // wait ten seconds before asking for the time again
		// delay(10000);
		// _state_udp = STATE_NTP_SEND;
	}
}

static inline bool clock_ntp_synced() {
	return _clock_is_sync;
}
static inline void clock_ntp_resetflag() {
	_clock_is_sync = false;
}