#include <Arduino.h>
// BLE connection
#include <BLEDevice.h>
#include <Firebase.h>
BLEUUID infoUUID("abc0");
BLEUUID IDUUID("abc1");
BLEUUID isWearingUUID("abc2");
BLEAddress *pBandAddress;
BLERemoteCharacteristic *IDCharacteristic;
BLERemoteCharacteristic *IsWearingCharacteristic;
BLEScan *pBLEScan;
bool doConnect = false;
bool connected = false;
bool IDchange = false;
bool isWearChange = false;
byte *isWearing;
String status;
String ID;
String tmp;
FirebaseJson json;
class MyADCB : public BLEAdvertisedDeviceCallbacks
{
	void onResult(BLEAdvertisedDevice advertisedDevice)
	{
		if (advertisedDevice.getServiceUUID().equals(infoUUID))
		{
			tmp = advertisedDevice.getAddress().toString().c_str();
			if (tmp == "fc:b5:41:25:a1:97")
			{
				ID = "1123";
				status = advertisedDevice.getManufacturerData().c_str();
			}
			else if (tmp == "c9:5c:d3:93:df:e1")
			{
				ID = "1124";
				status = "1";
			}
			else
			{
				ID = advertisedDevice.getAddress().toString().c_str();
				status = advertisedDevice.getManufacturerData().c_str();
			}
			json.set("/Patient/" + ID, status);
			Serial.println(ID);
			Serial.println(status);
		}
	}
};
// Wifi & database connection
#include <WiFi.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define WIFI_SSID "DUI-LAPTOP"
#define WIFI_PASSWORD "12345678"
#define DATABASE_URL "https://polocaserver-ea3d5-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyCZV-SoLDJbf5kU82kZV7JbocvtNcYpjWY"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned short preSendMillis = 0;
unsigned short sendDelay = 3000;
bool singUpOK = false;
byte waitCycle = 6;
String beaconName("Beacon1");
String room("204B3");
void setup()
{
	Serial.begin(9600);
	// Setup BLE
	Serial.print("[Setup BLE]");
	BLEDevice::init("POLOBeacon");
	pBLEScan = BLEDevice::getScan();
	pBLEScan->setActiveScan(true);
	pBLEScan->setInterval(100);
	pBLEScan->setWindow(100);
	pBLEScan->setAdvertisedDeviceCallbacks(new MyADCB());
	Serial.println("->Done!");
	// Setup Wifi
	Serial.print("[Setup Wifi]");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	Serial.print("Connecting");
	while (waitCycle-- && WiFi.status() != WL_CONNECTED)
	{
		Serial.print('.');
		delay(500);
	}
	Serial.print(F("\nIP: "));
	Serial.println(WiFi.localIP());
	config.api_key = API_KEY;
	config.database_url = DATABASE_URL;
	if (Firebase.signUp(&config, &auth, "", ""))
	{
		Serial.println("OKE!");
		singUpOK = true;
	}
	else
		Serial.printf("%s\n", config.signer.signupError.message.c_str());
	config.token_status_callback = tokenStatusCallback;
	Firebase.begin(&config, &auth);
	Firebase.reconnectWiFi(true);
}

void loop()
{
	// Scan for POLOBAND
	Serial.println("[Scan result]");
	pBLEScan->start(2);
	Serial.println("->Scan done!");
	// Send data to firebase
	if (Firebase.ready() && singUpOK && (millis() - preSendMillis > sendDelay))
	{
		json.set("/Room", room);
		preSendMillis = millis();
		Serial.printf("Set...%s\n",
					  Firebase.setJSON(fbdo, beaconName, json) ? "oke" : fbdo.errorReason().c_str());
		json.clear();
	}
	pBLEScan->clearResults();
}