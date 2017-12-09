
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "ElectricManager.h"
#include "HourManager.h"
//#include "SparkfunManager.h"
//#include "ioManager.h"
#include "thingSpeakManager.h"
#include "WifiManager.h"
#include "SettingManager.h"

#include "DHTManager.h"

extern "C" {
#include "user_interface.h"
}
//Channel ID : 349755
//key : 80BASNDM7S5TMHIG

#define MODULE_NAME CURRENT_NAME
#define MODULE_MDNS CURRENT_MDNS
#define MODULE_MDNS_AP CURRENT_MDNS_AP
#define MODULE_IP   CURRENT_IP



#define LOG_LABEL     5 //"log"
//#define WATT_LABEL "watt"
#define CURRENT_LABEL 1 //"current"
#define KWH_LABEL     2 //"KWH"
#define HUM_LABEL     3 //"HUM"
#define TEMP_LABEL    4 //"TEMP"



const unsigned char  pinLed = D4;
const unsigned char  pinCurrent = A0;
const unsigned long timerFrequence = 6000;//ms
const unsigned long maxNbreMesure = 60000/timerFrequence; // send KPI every minute



ESP8266WebServer server ( 80 );
ESP8266HTTPUpdateServer httpUpdater;
SettingManager smManager(pinLed);
ElectricManager elecManager(pinCurrent,pinLed);
HourManager hrManager(2390,pinLed);
WifiManager wfManager(pinLed);
//ioManager sfManager(pinLed);
thingSpeakManager sfManager(pinLed);
DHTManager dhtManager(D2,pinLed);

os_timer_t myTimer;
boolean tickOccured;


void timerCallback(void *pArg) {
     tickOccured = true;
}

void restartESP() {
  ESP.restart();
}

void timerrestartESP(void *pArg) {
    restartESP();
}

String getJson()
{
  String tt("{\"module\":{");
    tt += "\"nom\":\"" + String(MODULE_NAME) +"\"," ;
    tt += "\"version\":\"" + String(CURRENT_VERSION) +"\"," ;
    tt += "\"uptime\":\"" + NTP.getUptimeString() +"\"," ;
    tt += "\"build_date\":\""+ String(__DATE__" " __TIME__)  +"\"},";
    tt += "\"LOG\":["+wfManager.log(JSON_TEXT)  + "," + dhtManager.log(JSON_TEXT)  + "," + hrManager.log(JSON_TEXT) + ","+ elecManager.log(JSON_TEXT) + "," + sfManager.log(JSON_TEXT)+"],";
    tt += "\"courant\":{" + elecManager.toString(JSON_TEXT) + "},";
    tt += "\"dht\":{"+ dhtManager.toString(JSON_TEXT) + "},";
    //tt += "\"datetime\":{\"date\":\""+NTP.getDateStr()+"\",\"time\":\""+NTP.getTimeStr()+"\"}}";
    tt += "\"datetime\":{" + hrManager.toDTString(JSON_TEXT) + "}}";
    return tt;
}

/*
{
  "module" : {
    "nom":"blabla",
    "version" : "1.0.2",
    "date":"10/08/2017 18:15"
    "Uptime": "32:55"
  },
"log" : [
    {
    "nom" : "class name",
    "code" : "10",
    "description" : "blabla"
    },
    {
    "nom" : "class name",
    "code" : "10",
    "description" : "blabla"
    }
  ],
  "CURRENT" : {
    "current" : "10250",
    "KWT"  : "40.25",
  },
  "DHT" : {
    "dhtHum" : "41.70",
    "dhtTemp" : "27.70"
  }
  "DATETIME" : {
    "date" : "25/10/2017",
    "time" : "18:08"
  }
}

*/


void dataSummaryJson() {
	digitalWrite ( pinLed, LOW );
  server.send ( 200, "text/json", getJson() );
  digitalWrite ( pinLed, HIGH );
}



void dataSummaryPage() {
	digitalWrite ( pinLed, LOW );
	char temp[400];
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

  String message =
  "<html>\
    <head>\
      <meta http-equiv='refresh' content='5'/>\
      <title>Summary page</title>\
      <style>\
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      </style>\
    </head>\
    <body>\
      <h1>Real time data!</h1>";
  message += "<p>" + wfManager.toString(STD_TEXT) + "</p>";
  message += "<p>Date Hour : " + hrManager.toDTString(STD_TEXT) + "</p>";
  message += "<p>Uptime: " + hrManager.toUTString() + "</p>";
  message += "<p>" + elecManager.toString(STD_TEXT) + "</p>";
  message += "<p>" + dhtManager.toString(STD_TEXT) + "</p>";
  message += "<h2>Log data</h2>\
  		<TABLE border=2 cellpadding=10 log>";
  message += "<TR><TD>"+smManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+elecManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+hrManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+sfManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+wfManager.log(STD_TEXT)+"</TD></TR>";
  message += "<TR><TD>"+dhtManager.log(STD_TEXT)+"</TD></TR>";
  message += "</TABLE>\
  		        <h2>Links</h2>";
  message += "<A HREF=\""+WiFi.localIP().toString()+ "\">cette page</A></br>";
  message += "<A HREF=\"https://thingspeak.com/channels/"+ String(smManager.m_privateKey) +"\">thingspeak</A></br>\
              <h2>Commands</h2>\
              <ul><li>/clear => erase credentials</li>\
                  <li>/credential => display credential</li>\
                  <li>/restart => restart module</li>\
                  <li>/status => Json details</li>\
                  <li>/whatever => display summary</li></ul>";
  message += "</body></html>";
  server.send ( 200, "text/html", message );

  digitalWrite ( pinLed, HIGH );

}

void displayCredentialCollection() {
	digitalWrite ( pinLed, LOW );

  char temp[400];

  String message =  "<html>\
    <head>\
      <title>Credentials page</title>\
      <style>\
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      </style>\
    </head>\
    <body>";
  message += "<p>";
  message +="<ul>";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      message += "<li>";
      message +=i + 1;
      message += ": ";
      message += WiFi.SSID(i);
      message += " (";
      message += WiFi.RSSI(i);
      message += ")";
      message += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      message += "</li>";
    }
  message += "</ul>";
  message += "<form method='get' action='set'>";
  message += "<label>SSID:</label><input name='ssid' test length=32 value=\""+String(smManager.m_ssid) +"\"><br>";
  message += "<label>Pass:</label><input name='pass' length=64 value=\""+String(HIDDEN_KEY) +"\"><br>";
  message += "<label>ChannelNumber:</label><input name='sparkPrivate' length=64 value=\""+String(HIDDEN_KEY) +"\"><br>";
  message += "<label>WriteAPIKey:</label><input name='sparkPublic' length=64 value=\""+String(smManager.m_publicKey) +"\"><br>";
  message += "<input type='submit'></form>";
  message += "</body></html>";

  server.send ( 200, "text/html", message );

  digitalWrite ( pinLed, HIGH );

}

void setCredential(){
  bool restartNeeded = false;
  String str = server.arg("ssid");
  if (str.length()>0) {
    strcpy(smManager.m_ssid, str.c_str());
    restartNeeded = true;
  }
  str = server.arg("pass");
  if (str.length()>0 && str != HIDDEN_KEY) {
    strcpy(smManager.m_password,str.c_str());
    restartNeeded = true;
  }
  str = server.arg("sparkPrivate");
  if (str.length()>0 && str != HIDDEN_KEY)
      strcpy(smManager.m_privateKey,str.c_str());
  str = server.arg("sparkPublic");
  if (str.length()>0)
      strcpy(smManager.m_publicKey,str.c_str());
  smManager.writeData();
  if (restartNeeded) {
    server.send ( 200, "text/html", "data recorded.restart board");
    restartESP();
  } else {
    server.send ( 200, "text/html", "data recorded");
  }

}

void clearMemory(){
  smManager.clearData();
  server.send ( 200, "text/html", "ok");
}

void startWiFiserver() {
  if (wfManager.connectSSID(smManager.m_ssid,smManager.m_password,IPAddress(MODULE_IP), MODULE_MDNS )==WL_CONNECTED) {
    os_timer_setfn(&myTimer, timerCallback, NULL);
    os_timer_arm(&myTimer, timerFrequence, true);
    server.on ( "/", dataSummaryPage );
    server.onNotFound ( dataSummaryPage );
    hrManager.begin("pool.ntp.org", 1, true);
  } else {
    os_timer_setfn(&myTimer, timerrestartESP, NULL);
    os_timer_arm(&myTimer, 5*60*1000, true);
    wfManager.connectAP(MODULE_MDNS_AP);
    server.on ( "/", displayCredentialCollection );
    server.onNotFound ( displayCredentialCollection );
  }
  server.on ( "/clear", clearMemory );
  server.on ( "/restart", restartESP );
  server.on ( "/set", setCredential );
  server.on ( "/credential", displayCredentialCollection );
  server.on ( "/status", dataSummaryJson );
  httpUpdater.setup(&server, ((const char *)"/firmware"), MODULE_UPDATE_LOGIN, MODULE_UPDATE_PASS);

  server.begin();

  Serial.println( "HTTP server started" );
  Serial.println(wfManager.toString(STD_TEXT));
}


void setup ( void ) {
  // Iniialise input

  Serial.begin ( 115200 );

  smManager.readData();
  DEBUGLOG("");DEBUGLOG(smManager.toString(STD_TEXT));
  startWiFiserver();
}

void loop ( void ) {
	server.handleClient();
  if (tickOccured) {

    //read courant
    elecManager.readCumulCurrent();


    //hrManager.getCurrentEpoch();

  DEBUGLOG("debug mode");
    /*DEBUGLOG (elecManager.toString(STD_TEXT));
    //DEBUGLOG (hrManager.toDTString(STD_TEXT));
    DEBUGLOG (dhtManager.toString(STD_TEXT));

    		DEBUGLOG(""); DEBUGLOG(" ");
    		DEBUGLOG(NTP.getTimeDateString()); DEBUGLOG(" ");
    		DEBUGLOG(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
    		DEBUGLOG("WiFi is ");
    		DEBUGLOG(WiFi.isConnected() ? "connected" : "not connected"); DEBUGLOG(". ");
    		DEBUGLOG("Uptime: ");
    		DEBUGLOG(NTP.getUptimeString()); DEBUGLOG(" since ");
    		DEBUGLOG(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

  #endif
    /*unsigned int averageCurrent = elecManager.getAverageCurrent();
    sfManager.addVariable("watt", String((220*averageCurrent)/1000));
    sfManager.addVariable("current", String(averageCurrent));*/

    /*DEBUGLOG ( smManager.log());
    DEBUGLOG ( elecManager.log());
    DEBUGLOG ( hrManager.log());
    DEBUGLOG ( sfManager.log());
    DEBUGLOG ( wfManager.log());

    sfManager.sendKPIsToIO( smManager.m_privateKey, smManager.m_publicKey);*/

    if (elecManager.m_globalNbreValue == maxNbreMesure) {
      unsigned int averageCurrent = elecManager.getAverageCurrent();
      //DEBUGLOG ( averageCurrent );

      sfManager.addVariable(CURRENT_LABEL, String(averageCurrent));
      //sfManager.addVariable(WATT_LABEL,  String((220*averageCurrent)/1000));
      // send KWattHour every 24 hours
      if (hrManager.isNextDay())
        sfManager.addVariable(KWH_LABEL,String(elecManager.getKWattHour()));

      sfManager.addVariable(HUM_LABEL, String(dhtManager.getHumidity()));
      sfManager.addVariable(TEMP_LABEL, String(dhtManager.getTemperature()));

    /*  if (smManager.newLog()){
          sfManager.addVariable(LOG_LABEL,smManager.log(STD_TEXT));
          smManager.clearLog();
      }
      if (elecManager.newLog()){
        sfManager.addVariable(LOG_LABEL,elecManager.log(STD_TEXT));
        elecManager.clearLog();
      }
      if (hrManager.newLog()) {
        sfManager.addVariable(LOG_LABEL,hrManager.log(STD_TEXT));
        hrManager.clearLog();
      }
      if (sfManager.newLog()) {
        sfManager.addVariable(LOG_LABEL,sfManager.log(STD_TEXT));
        sfManager.clearLog();
      }
      if (wfManager.newLog()) {
        sfManager.addVariable(LOG_LABEL,wfManager.log(STD_TEXT));
        wfManager.clearLog();
      }
      if (dhtManager.newLog()) {
        sfManager.addVariable(LOG_LABEL,dhtManager.log(STD_TEXT));
        dhtManager.clearLog();
      }*/
      //DEBUGLOG (elecManager.getKWattHour());
      //DEBUGLOG(sfManager.toString());
      DEBUGLOG(sfManager.toString(STD_TEXT));
      sfManager.sendIoT( smManager.m_privateKey, smManager.m_publicKey);

    }
    tickOccured = false;
  }
}
