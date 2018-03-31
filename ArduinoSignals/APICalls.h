/*
Copyright 2017 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>
#include <time.h>
#include "Credentials.h"

#define SERVO_PIN 14

// Servo Positions and Intent specific Numbers

#define CENTER_POSITION 90

// Crypto Currency
#define CURRENCY_UP 180
#define CURRENCY_DOWN 0
#define CURRENCY_NO_CHANGE 90

// Umbrella
#define UMBRELLA_OPEN 60
#define UMBRELLA_CLOSED 180

// Shorts or Pants
#define SHORTSORPANTS_PANTS 0
#define SHORTSORPANTS_SHORTS 65
#define SHORTSORPANTS_THRESHOLD 70.0

// Timer
#define TIMER_START 0
#define TIMER_END 180
#define TIMER_WIGGLE_TOP 0
#define TIMER_WIGGLE_BOTTOM 180

// RocketLaunch
#define ROCKET_ON_TIME 43200000 // Rocket shows fire for 12 hours
#define ROCKET_LAUNCH 0
#define ROCKET_NOT_LAUNCHED 65

// Celebrate
#define STRETCHBREAK_TIME_REACHED 0
#define STRETCHBREAK_NOT_YET 90

// Test Signal
#define TEST_FIRST_POSITION 10
#define TEST_SECOND_POSITION 170
#define TEST_NUM_SWINGS 5

class PaperSignals
{
public:
	PaperSignals() {};
	~PaperSignals() {};

	void StartUp();
	String getJson(String host, String url);
	String getJsonHTTP(String host, String url);
	void ParseIntentName(String intentName, String JSONData);
	String getSignalByID(String signalID);
	void RunPaperSignals();

	// Intent Types
	String CryptoCurrencyType = "CryptoCurrency";
	String ShortsOrPantsType = "ShortsOrPants";
	String UmbrellaType = "Umbrella";
	String TimerType = "Timer";
	String StretchBreakType = "StretchBreak";
	String RocketLaunchType = "RocketLaunch";
	String CountdownType = "Countdown";
	String TestSignalType = "TestSignal";
	String StockType = "Stock";
	String CustomIntentType = "YOUR_CUSTOM_INTENT_NAME";

	String currentIntent = "";
	String currentIntentTimeStamp = "";
	bool updatedIntentTimeStamp = true;

	unsigned long lastWeatherCall = 0;
	unsigned long timeBetweenWeatherCalls = 120000;
	bool throttleWeatherAPI();


	unsigned long lastStockCall = 0;
	unsigned long timeBetweenStockCalls = 120000;
	bool throttleStockAPI();

public:

	// Use WiFiClientSecure class to create TLS connection
	WiFiClientSecure client;
	const int httpsPort = 443;

	// Signal Functions
	void DefaultExecution(String JSONData);
	void CountdownExecution(String JSONData);
	void TimerExecution(String JSONData);
	void CryptoCurrencyExecution(String JSONData);
	void ShortsOrPantsExecution(String JSONData);
	void UmbrellaExecution(String JSONData);
	void StretchBreakExecution(String JSONData);
	void RocketExecution(String JSONData);
	void TestSignalExecution(String JSONData);
	void StockExecution(String JSONData);
	void CustomExecution(String JSONData);
	String GetWeather(String JSONData);
	String GetLatLong(String JSONData);

	double GetBitcoin();
	double GetEthereum();

	void MoveServoToPosition(int position, int speed);

	double initialBitcoin;
	double initialEthereum;

	int lastTimerTime = 0;

	unsigned long breakTimeLength = 60000; // 1 Minute Default
	unsigned long breakTimeInterval = 900000; // 15 Minute Default
	unsigned long lastBreakTime = 0;

	unsigned long rocketStartedTime = 0;
	boolean rocketLaunched = false;

	int numTestServoSwings = 0;

	String mostRecentDateString = "";
	String NextRocketLaunchTime = "";

	Servo myservo;
	int currentServoPosition = 0;
};
