#include <Arduino.h>
#include <Wire.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <BLEPeripheral.h>

// define pins (varies per shield/board)
#define BLE_REQ 10
#define BLE_RDY 2
#define BLE_RST 9

// create peripheral instance, see pinouts above
BLEPeripheral POLOBAND(BLE_REQ, BLE_RDY, BLE_RST);
BLEService InfoService("abc0");
// BLECharacteristic IDCharacteristic("abc1", BLERead | BLEWrite, 2);
// BLECharacteristic StateCharacteristic("abc2", BLERead | BLENotify, 1);

MAX30105 sensor;
long lastBeat = 0;
float BPM;
byte preState = 0;
unsigned char state[] = {0x0, 0x0};
unsigned short preMillis = 0;
bool ledToggle = false;
char ID[] = {0x12, 0x34, 0x0};

void setup()
{
	Serial.begin(9600);
	Serial.println(F("[POLOBAND - start setup]"));
	/* Set pin mode */
	pinMode(PIN_LED1, OUTPUT);
	pinMode(PIN_LED2, OUTPUT);
	pinMode(PIN_LED3, OUTPUT);
	pinMode(PIN_LED4, OUTPUT);
	pinMode(PIN_LED5, OUTPUT);
	pinMode(PIN_BUTTON1, INPUT_PULLUP);
	/* Confirgure BLE device */
	POLOBAND.setLocalName("POLOBAND");
	POLOBAND.setDeviceName("POLOBAND");
	POLOBAND.setAppearance(BLE_APPEARANCE_GENERIC_WATCH);
	POLOBAND.setConnectable(true);
	POLOBAND.setTxPower(-10);
	POLOBAND.setAdvertisedServiceUuid(InfoService.uuid());
	// POLOBAND.addAttribute(InfoService);
	// POLOBAND.addAttribute(IDCharacteristic);
	// POLOBAND.addAttribute(StateCharacteristic);
	POLOBAND.begin();
	// IDCharacteristic.setValue((const char *)ID);
	/* Initialize sensor */
	while (!sensor.begin(Wire, I2C_SPEED_FAST))
	{
		Serial.println(F("MAX30105 was not found. Please check wiring/power."));
		delay(500);
	}
	sensor.setup(
		0x01,
		1,
		2,
		50,
		69,
		2048);
	Serial.println(F("[POLOBAND - finish setup]"));
}

void loop()
{
	BLECentral central = POLOBAND.central();
	long irVal = sensor.getIR();
	if (irVal < 1700)
	{
		digitalWrite(PIN_LED3, LOW);
		state[0] = 0x34;
	}
	else
	{
		digitalWrite(PIN_LED3, HIGH);
		if (checkForBeat(irVal))
		{
			long delta = millis() - lastBeat;
			lastBeat = millis();
			BPM = 60 / (delta / 1000.0f);
			if (BPM > 60)
				state[0] = 0x31;
			else if (BPM >= 20)
				state[0] = 0x32;
			else
				state[0] = 0x33;
		}
	}
	if (state[0] != preState || preState == 0)
	{
		preState = state[0];
		// StateCharacteristic.setValue((const char *)state);
		POLOBAND.setManufacturerData((const unsigned char *)state, 1);
		POLOBAND.begin();
	}
	digitalWrite(PIN_LED1, !digitalRead(PIN_BUTTON1));
	if (millis() - preMillis > 2000)
	{
		preMillis = millis();
		digitalWrite(PIN_LED4, ledToggle);
		ledToggle = !ledToggle;
	}
}