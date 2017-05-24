/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <Arduino.h>
#include <ESP8266WebServer.h>

#include "ElectricManager.h"
#include "HourManager.h"
//#include "SparkfunManager.h"
#include "WifiManager.h"
#include "SettingManager.h"
#include "ioManager.h"

//#include <DateTime.h>
//#include "EmonLib.h"                   // Include Emon Library
#define MQTT_DEBUG
#define MQTT_ERROR
#include <Adafruit_MQTT.h>                                 // Adafruit MQTT library
#include <Adafruit_MQTT_Client.h>                           // Adafruit MQTT library


extern "C" {
#include "user_interface.h"
}


const int  pinLed = D4;
const int  pinCurrent = A0;

ESP8266WebServer server ( 80 );
SettingManager smManager(pinLed);
ElectricManager elecManager(pinCurrent,pinLed);
HourManager hrManager(2390,pinLed);
//SparkfunManager sfManager(pinLed); //"P4z49d76BYSgw4Y4wXnd","DJKJmvynz2Tp65j56yQX",
WifiManager wfManager(pinLed);
ioManager sfManager(pinLed);


os_timer_t myTimer;
boolean tickOccured;
//EnergyMonitor emon1;

//unsigned long  m_cumulCurrent = 0;
//unsigned char  currentDay = 0;

const unsigned long timerFrequence = 6000;//ms
const unsigned long maxNbreMesure = 60000/timerFrequence; // send KPI every minute

//unsigned long m_timeReference=0;




/************ Global State (you don't need to change this!) ******************/


// start of timerCallback
void timerCallback(void *pArg) {
     tickOccured = true;
} // End of timerCallback



void restartESP() {
  ESP.restart();
}

void timerrestartESP(void *pArg) {
    restartESP();
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
  message += "<p>" + wfManager.toString() + "</p>";
  message += "<p>Date Hour : " + hrManager.toDTString() + "</p>";
  message += "<p>Uptime: " + hrManager.toUTString() + "</p>";
  message += "<p>" + elecManager.toString() + "</p>";
  message += "<h2>Log data</h2>\
  		<TABLE border=2 cellpadding=10 log>";
  message += "<TR><TD>"+smManager.log()+"</TD></TR>";
  message += "<TR><TD>"+elecManager.log()+"</TD></TR>";
  message += "<TR><TD>"+hrManager.log()+"</TD></TR>";
  message += "<TR><TD>"+sfManager.log()+"</TD></TR>";
  message += "<TR><TD>"+wfManager.log()+"</TD></TR>";
  message += "</TABLE>\
  		        <h2>Links</h2>";
  message += "<A HREF=\""+WiFi.localIP().toString()+ "\">cette page</A></br>";
  message += "<A HREF=\"https://io.adafruit.com/"+ String(smManager.m_publicKey) +">io.adafruit.com</A></br>\
              <h2>Commands</h2>\
              <ul><li>/clear => erase credentials</li>\
                  <li>/credential => display credential</li>\
                  <li>/restart => restart module</li>\
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
  message += "<label>Pass:</label><input name='pass' length=64 value=\""+String(smManager.m_password) +"\"><br>";
  message += "<label>PrivateKey:</label><input name='sparkPrivate' length=64 value=\""+String(smManager.m_privateKey) +"\"><br>";
  message += "<label>PublicKey:</label><input name='sparkPublic' length=64 value=\""+String(smManager.m_publicKey) +"\"><br>";
  message += "<input type='submit'></form>";
  message += "</body></html>";

  server.send ( 200, "text/html", message );

  digitalWrite ( pinLed, HIGH );

}

void setCredential(){
  String str = server.arg("ssid");
  if (str.length()>0)
    strcpy(smManager.m_ssid, str.c_str());
  str = server.arg("pass");
  if (str.length()>0)
    strcpy(smManager.m_password,str.c_str());
  str = server.arg("sparkPrivate");
  if (str.length()>0)
      strcpy(smManager.m_privateKey,str.c_str());
  str = server.arg("sparkPublic");
  if (str.length()>0)
      strcpy(smManager.m_publicKey,str.c_str());
  smManager.writeData();
  server.send ( 200, "text/html", "data recorded.restart board");
  /*String message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
  message += "data recorded ";
  message += "<p> restart board</html>\r\n\r\n";
  server.send ( 200, "text/html", message );*/

}

void clearMemory(){
  smManager.clearData();
  server.send ( 200, "text/html", "ok");
}

void MQTT_connect(Adafruit_MQTT_Client *mqtt) {

  Serial.println("start Connecting to MQTT... ");
  if (mqtt->connected()) { return; }                     // Stop and return to Main Loop if already connected to Adafruit IO
  Serial.print("Connecting to MQTT... ");
  /*  Serial.println(publicKey.c_str());
      Serial.println(privateKey.c_str());*/


  uint8_t ret = mqtt->connect(/*publicKey.c_str(),privateKey.c_str()*/);//publicKey.c_str(),privateKey.c_str());
  if ( ret != 0 ) {
    Serial.println(mqtt->connectErrorString(ret));
    mqtt->disconnect();
  } else {
    Serial.println("MQTT Connected!");
    delay(500);
  }
}


void setup ( void ) {
  // Iniialise input
  pinMode ( pinCurrent, INPUT );
  pinMode ( pinLed, OUTPUT );


	Serial.begin ( 115200 );

  smManager.readData();
  Serial.println(smManager.toString());
  if (wfManager.connectSSID(smManager.m_ssid,smManager.m_password )==WL_CONNECTED) {
    os_timer_setfn(&myTimer, timerCallback, NULL);
    os_timer_arm(&myTimer, timerFrequence, true);
    server.on ( "/", dataSummaryPage );
    server.onNotFound ( dataSummaryPage );
  } else {
    os_timer_setfn(&myTimer, timerrestartESP, NULL);
    os_timer_arm(&myTimer, 5*60*1000, true);
    wfManager.connectAP();
    server.on ( "/", displayCredentialCollection );
    server.onNotFound ( displayCredentialCollection );
  }
  server.on ( "/clear", clearMemory );
  server.on ( "/restart", restartESP );
  server.on ( "/set", setCredential );
  server.on ( "/credential", displayCredentialCollection );
  server.begin();
  Serial.println ( "HTTP server started" );
  Serial.println(wfManager.toString());

}


void loop ( void ) {
	server.handleClient();
  if (tickOccured) {
    //read courant
    elecManager.readCumulCurrent();
    Serial.println (elecManager.toString());

    hrManager.getCurrentEpoch();
    Serial.println (hrManager.toDTString() );


    /*unsigned int averageCurrent = elecManager.getAverageCurrent();
    sfManager.addVariable("watt", String((220*averageCurrent)/1000));
    sfManager.addVariable("current", String(averageCurrent));*/

    /*Serial.println ( smManager.log());
    Serial.println ( elecManager.log());
    Serial.println ( hrManager.log());
    Serial.println ( sfManager.log());
    Serial.println ( wfManager.log());

    sfManager.sendKPIsToIO( smManager.m_privateKey, smManager.m_publicKey);*/

    if (elecManager.m_globalNbreValue == maxNbreMesure) {
      unsigned int averageCurrent = elecManager.getAverageCurrent();
      Serial.println ( averageCurrent );
      sfManager.addVariable("current", String(averageCurrent));
      sfManager.addVariable("watt",  String((220*averageCurrent)/1000));
      // send KWattHour every 24 hours
      if (hrManager.isNextDay())
        sfManager.addVariable("KWH",String(elecManager.getKWattHour()));

      if (smManager.newLog())   sfManager.addVariable("log",smManager.log());
      if (elecManager.newLog()) sfManager.addVariable("log",elecManager.log());
      if (hrManager.newLog())   sfManager.addVariable("log",hrManager.log());
      if (sfManager.newLog())   sfManager.addVariable("log",sfManager.log());
      if (wfManager.newLog())   sfManager.addVariable("log",wfManager.log());

      //Serial.println (elecManager.getKWattHour());
      //Serial.println(sfManager.toString());
      sfManager.sendKPIsToIO( smManager.m_privateKey, smManager.m_publicKey);

    }
    tickOccured = false;
  }
}
