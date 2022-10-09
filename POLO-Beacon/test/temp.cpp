#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
// Insert your network credentials
#define WIFI_SSID "BMDT"
#define WIFI_PASSWORD "PTN@203B3%"
// Insert Firebase project API Key
#define API_KEY "AIzaSyCZV-SoLDJbf5kU82kZV7JbocvtNcYpjWY"
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://polocaserver-ea3d5-default-rtdb.firebaseio.com"
// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
void setup()
{
	Serial.begin(115200);
	Serial.println("hello");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	Serial.print("Con");
	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print('.');
		delay(300);
	}
	Serial.println();
	Serial.print("IP:");
	Serial.println(WiFi.localIP());
	Serial.println();
	/* Assign the api key (required) */
	config.api_key = API_KEY;
	/* Assign the RTDB URL (required) */
	config.database_url = DATABASE_URL;
	/* Sign up */
	if (Firebase.signUp(&config, &auth, "", ""))
	{
		Serial.println("ok");
		signupOK = true;
	}
	else
	{
		Serial.printf("%s\n", config.signer.signupError.message.c_str());
	}
	/* Assign the callback function for the long running token generation task */
	config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
	Firebase.begin(&config, &auth);
	Firebase.reconnectWiFi(true);
}
bool state = false;
void loop()
{
	if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
	{
		sendDataPrevMillis = millis();
		Firebase.setBool(fbdo, "Beacon1/Patient/1234", state);
		state = !state;
	}
	// put your main code here, to run repeatedly:
	delay(1000);
}