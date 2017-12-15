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

/*
Copyright © 2017 All market data provided by Barchart Solutions.

BATS market data is at least 15-minutes delayed. Forex market data is at least 10-minutes delayed.
AMEX, NASDAQ, NYSE and futures market data (CBOT, CME, COMEX and NYMEX) is end-of-day.
Information is provided 'as is' and solely for informational purposes, not for trading purposes or advice,
and is delayed. To see all exchange delays and terms of use, please see our disclaimer.
*/

#include "APICalls.h"

// #define PRINT_PAYLOAD

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}

int days_in_month[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

int leap_year(int year) {
    if(year%400==0) return 1;

    if(year%4==0 && year%100!=0) return 1;

    return 0;
}

int number_of_days(int year, int day, int month) {
    int result = 0;
    int i;

    for(i=1; i < year; i++) {
        if(leap_year(i))
            result += 366;
        else
            result += 365;
    }

    for(i=1; i < month; i++) {
        result += days_in_month[i];

        if(leap_year(year) && i == 2) result++;
    }

    result += day;
    return result;
}

String makeLessPrettyJSON(String JSONData)
{
  String notPretty = "";
  for(int i = 0; i < JSONData.length(); i++)
  {
    if(JSONData.charAt(i) != '\n' && JSONData.charAt(i) != '\r' &&
      JSONData.charAt(i) != ' ' && JSONData.charAt(i) != '  ' &&
      JSONData.charAt(i) != '[' && JSONData.charAt(i) != ']')
    {
      notPretty += JSONData.charAt(i);
    }
  }

  return notPretty;
}

void PaperSignals::StartUp()
{
  myservo.attach(SERVO_PIN);
  MoveServoToPosition(CENTER_POSITION, 10); // Initialize
}

void PaperSignals::MoveServoToPosition(int position, int speed)
{
  if(position < currentServoPosition)
  {
    for(int i = currentServoPosition; i > position; i--)
    {
      myservo.write(i);
      delay(speed);
    }
  }
  else if(position > currentServoPosition)
  {
    for(int i = currentServoPosition; i < position; i++)
    {
      myservo.write(i);
      delay(speed);
    }
  }

  currentServoPosition = position;
}

void PaperSignals::DefaultExecution(String JSONData)
{
    Serial.println("Default");
}

void PaperSignals::TimerExecution(String JSONData)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(JSONData);
  int seconds = root["parameters"]["duration"]["amount"];

  Serial.print("Timer: ");
  Serial.println(seconds);

  if(updatedIntentTimeStamp)
  {
    int totalDistance = abs(TIMER_END - TIMER_START);
    int speed = (seconds*1000)/totalDistance;
    MoveServoToPosition(TIMER_START, 0);
    MoveServoToPosition(TIMER_END, speed);

    MoveServoToPosition(TIMER_WIGGLE_TOP, 10);
    MoveServoToPosition(TIMER_WIGGLE_BOTTOM, 10);
    MoveServoToPosition(TIMER_WIGGLE_TOP, 10);
    MoveServoToPosition(TIMER_WIGGLE_BOTTOM, 10);
  }
}

double PaperSignals::GetBitcoin()
{
    char* host = "api.coinmarketcap.com";
    String url = "/v1/ticker/bitcoin/";

    String payload = getJson(host, url);

    String unPretty = makeLessPrettyJSON(payload);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(unPretty);

    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
    }

    double change = root[String("percent_change_24h")];

    return change;
}

double PaperSignals::GetEthereum()
{
    char* host = "api.coinmarketcap.com";
    String url = "/v1/ticker/ethereum/";

    String payload = getJson(host, url);

    String unPretty = makeLessPrettyJSON(payload);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(unPretty);

    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
    }

    double change = root[String("percent_change_24h")];

    return change;
}

void PaperSignals::CryptoCurrencyExecution(String JSONData)
{
    String Bitcoin = "Bitcoin";
    String Ethereum = "Ethereum";

    DynamicJsonBuffer cryptoBuffer;
    JsonObject& cryptoRoot = cryptoBuffer.parseObject(JSONData);
    String CurrencyType = cryptoRoot["parameters"]["Crypto"];

    if(CurrencyType == Bitcoin)
    {
      Serial.println("Tracking Bitcoin");
      double curChange = GetBitcoin();
      Serial.println(curChange);

      if(curChange < 0)
      {
         Serial.println("Bitcoin is Down");
         MoveServoToPosition(CURRENCY_DOWN, 10);
      }
      else if(curChange > 0)
      {
        Serial.println("Bitcoin is Up");
        MoveServoToPosition(CURRENCY_UP, 10);
      }
      else
      {
        Serial.println("Bitcoin No Change");
        MoveServoToPosition(CURRENCY_NO_CHANGE, 10);
      }
    }
    else if(CurrencyType == Ethereum)
    {
      Serial.println("Tracking Ethereum");
      double curChange = GetEthereum();
      Serial.println(curChange);

      if(curChange < 0)
      {
        Serial.println("Ethereum is Down");
        MoveServoToPosition(CURRENCY_DOWN, 10);
      }
      else if(curChange > 0)
      {
        Serial.println("Ethereum is Up");
        MoveServoToPosition(CURRENCY_UP, 10);
      }
      else
      {
        Serial.println("Ethereum No Change");
        MoveServoToPosition(CURRENCY_NO_CHANGE, 10);
      }
    }
    else
    {
      Serial.println("Currency not supported");
      return;
    }


}

void PaperSignals::StretchBreakExecution(String JSONData)
{
  DynamicJsonBuffer stretchBreakBuffer;
  JsonObject& stretchBreakRoot = stretchBreakBuffer.parseObject(JSONData);
  breakTimeInterval = stretchBreakRoot["parameters"]["duration"]["amount"]; // In Minutes
  breakTimeInterval = breakTimeInterval*60*1000;

  if(lastBreakTime > millis()) // Reset every 49 days
  {
    lastBreakTime = millis();
  }

  if(millis() - lastBreakTime > breakTimeInterval)
  {
    lastBreakTime = millis();
  }

  Serial.print("Break in "); Serial.println(breakTimeInterval - (millis() - lastBreakTime));

  if(millis() - lastBreakTime < breakTimeLength)
  {
    // Hands Up
    Serial.println("Break Time");
    MoveServoToPosition(STRETCHBREAK_TIME_REACHED, 10);
  }
  else
  {
    // Hands Down
    Serial.println("No Breaks Yet");
    MoveServoToPosition(STRETCHBREAK_NOT_YET, 10);
  }

}
bool PaperSignals::throttleWeatherAPI()
{
  if(millis() < lastWeatherCall) lastWeatherCall = 0; // Reset every 49 days

  if(!updatedIntentTimeStamp && millis() - lastWeatherCall < timeBetweenWeatherCalls) return true;
  lastWeatherCall = millis();

  return false;
}

bool PaperSignals::throttleStockAPI()
{
  if(millis() < lastStockCall) lastStockCall = 0; // Reset every 49 days

  if(!updatedIntentTimeStamp && millis() - lastStockCall < timeBetweenStockCalls) return true;
  lastStockCall = millis();
  return false;
}

void PaperSignals::ShortsOrPantsExecution(String JSONData)
{
  //Only poll the weather API if a new location has been requested or it has been more than 2 minutes
  if(throttleWeatherAPI()) return;

  Serial.print("CUR DATA  ");
  Serial.println(JSONData);
  DynamicJsonBuffer weatherLocationBuffer;
  JsonObject& locationRoot = weatherLocationBuffer.parseObject(JSONData);
  String City = locationRoot["parameters"]["location"]["city"];
  String State = locationRoot["parameters"]["location"]["admin-area"];
  String Address = locationRoot["parameters"]["location"]["street-address"];
  String MaybeAlsoACity = locationRoot["parameters"]["location"]["subadmin-area"];

  String Location = MaybeAlsoACity + " " + City + " " + State + " " + Address;

  String WeatherJSON = GetWeather(Location);

  DynamicJsonBuffer jsonBufferWeather;
  JsonObject& weatherRoot = jsonBufferWeather.parseObject(WeatherJSON);

  double temperature = weatherRoot["daily"]["data"][0]["temperatureLow"];
  Serial.println(temperature);

  if(temperature > SHORTSORPANTS_THRESHOLD)
  {
    MoveServoToPosition(SHORTSORPANTS_SHORTS, 10);
  }
  else
  {
    MoveServoToPosition(SHORTSORPANTS_PANTS, 10);
  }
}

void PaperSignals::UmbrellaExecution(String JSONData)
{
  //Only poll the weather API if a new location has been requested or it has been more than 2 minutes
  if(throttleWeatherAPI()) return;

  DynamicJsonBuffer weatherLocationBuffer;
  JsonObject& locationRoot = weatherLocationBuffer.parseObject(JSONData);
  String City = locationRoot["parameters"]["location"]["city"];
  String State = locationRoot["parameters"]["location"]["admin-area"];
  String Address = locationRoot["parameters"]["location"]["street-address"];
  String MaybeAlsoACity = locationRoot["parameters"]["location"]["subadmin-area"];

  String Location = MaybeAlsoACity + " " + City + " " + State + " " + Address;

  String WeatherJSON = GetWeather(Location);

  DynamicJsonBuffer jsonBufferWeather;
  JsonObject& weatherRoot = jsonBufferWeather.parseObject(WeatherJSON);

  String iconWeather = weatherRoot["daily"]["data"][0]["icon"];
  Serial.println(iconWeather);

  String rainIcon = "rain";
  String sleetIcon = "sleet";

  if(iconWeather == rainIcon || iconWeather == sleetIcon)
  {
    MoveServoToPosition(UMBRELLA_OPEN, 10);
  }
  else
  {
    MoveServoToPosition(UMBRELLA_CLOSED, 10);
  }
}

// Performs Geocoding and a dark sky call to
String PaperSignals::GetWeather(String Location)
{
  String latLong = GetLatLong(Location);

  String curTimeHost = "currentmillis.com";
  String curTimeURL = "/time/minutes-since-unix-epoch.php";
  String unixTime = getJsonHTTP(curTimeHost, curTimeURL);

  unsigned long unixTimeLong = atol(unixTime.c_str())*60;
  String finalUnixTime = String(unixTimeLong);

  String weatherHost = "api.darksky.net";
  String weatherURL = "/forecast/" + DarkSkyAPIKey + "/" + latLong + "," + finalUnixTime + "?exclude=minutely,flags,hourly,currently,alerts";

  // Get Weather
  String weatherPayload = getJson(weatherHost, weatherURL);
  Serial.println(weatherPayload);

  return weatherPayload;
}

// This function handles the Google Geocoding API Pretty Print Output
String PaperSignals::GetLatLong(String Location)
{
  Location = urlencode(Location);
  char*  geoHost = "maps.googleapis.com";
  String geoURL = "/maps/api/geocode/json?address=" + Location + "&key=" + GeoCodingAPiKey;
  Serial.println(geoURL);

  // Get Lat Long
  String LatLongPayload = getJson(geoHost, geoURL);
  Serial.println(LatLongPayload);

  int index = LatLongPayload.indexOf("lat");

  Serial.println(index);
  boolean foundColon = false;
  String latString = "";
  for(int i = index; i < index+20; i++)
  {
    if(LatLongPayload.charAt(i) == ':')
    {
      foundColon = true;
    }
    else if(foundColon)
    {
      if(LatLongPayload.charAt(i) == ',') break;
      latString += LatLongPayload.charAt(i);
    }
  }

  latString.trim();
  Serial.print("Lat String: ");
  Serial.println(latString);

  index = LatLongPayload.indexOf("lng");

  Serial.println(index);
  foundColon = false;
  String lngString = "";
  for(int i = index; i < index+20; i++)
  {
    if(LatLongPayload.charAt(i) == ':')
    {
      foundColon = true;
    }
    else if(foundColon)
    {
      if(LatLongPayload.charAt(i) == '\n'| LatLongPayload.charAt(i) == '\r') break;
      lngString += LatLongPayload.charAt(i);
    }
  }

  lngString.trim();
  Serial.print("Lng String: ");
  Serial.println(lngString);

  return latString + "," + lngString;

}

void PaperSignals::RocketExecution(String JSONData)
{
  Serial.println("Rockets");

  DynamicJsonBuffer agencyBuffer;
  JsonObject& agencyRoot = agencyBuffer.parseObject(JSONData);
  String curSpaceAgency = agencyRoot["parameters"]["SpaceAgency"];
  String curCountry = agencyRoot["parameters"]["RocketCountryCodes"];

  char*  rocketHost = "launchlibrary.net";
  String rocketURL = "/1.2/launch/next/1";
  String rocketPayload = getJson(rocketHost, rocketURL);
  Serial.println(rocketPayload);

  DynamicJsonBuffer jsonBufferRockets;
  JsonObject& rocketRoot = jsonBufferRockets.parseObject(rocketPayload);

  String output = rocketRoot["launches"][0]["isonet"];

  if(curSpaceAgency != "" || curCountry != "")
  {
    Serial.print("Looking for "); Serial.print(curSpaceAgency); Serial.println(" Rockets");
    boolean foundAgency = false;
    String agency = "";
    int spaceAgencyNum = 0;
    do
    {
      String countryCode = rocketRoot["launches"][0]["location"]["pads"][0]["agencies"][spaceAgencyNum]["countryCode"];
      String agencyNew = rocketRoot["launches"][0]["location"]["pads"][0]["agencies"][spaceAgencyNum]["abbrev"];
      agency = agencyNew;

      Serial.print(countryCode); Serial.print(" ");
      Serial.println(agency);

      if(agency == curSpaceAgency) foundAgency = true; // Find your agency if defined

      if(countryCode == curCountry) foundAgency = true;

      spaceAgencyNum++;
    }while(agency.length() > 0);

    // If wrong agency found, then we'll just ignore launches
    if(!foundAgency)
      NextRocketLaunchTime = "";
  }

  if(rocketLaunched && (millis() - rocketStartedTime) < ROCKET_ON_TIME)
  {
    MoveServoToPosition(ROCKET_LAUNCH, 10);
  }
  else if(NextRocketLaunchTime == "")
  {
    Serial.println("Initalized Launch");

    NextRocketLaunchTime = output;
    rocketLaunched = false;

    MoveServoToPosition(ROCKET_NOT_LAUNCHED, 10);
  }
  else if(NextRocketLaunchTime != output)
  {
    Serial.println ("LAUNCH");
    NextRocketLaunchTime = output;
    rocketStartedTime = millis();
    rocketLaunched = true;

    MoveServoToPosition(ROCKET_LAUNCH, 10);
  }
  else
  {
    Serial.println("No launch yet");

    rocketLaunched = false;
    MoveServoToPosition(ROCKET_NOT_LAUNCHED, 10);
  }
}

void PaperSignals::CountdownExecution(String JSONData)
{
  DynamicJsonBuffer countDownBuffer;
  JsonObject& countDownRoot = countDownBuffer.parseObject(JSONData);
  String timeStamp = countDownRoot["timestamp"];

  // GET START DATE
  int startYear = (timeStamp.charAt(0)-'0')*1000 + (timeStamp.charAt(1)-'0')*100 + (timeStamp.charAt(2)-'0')*10 + (timeStamp.charAt(3)-'0');
  int startMonth = (timeStamp.charAt(5)-'0')*10 + (timeStamp.charAt(6)-'0');
  int startDay = (timeStamp.charAt(8)-'0')*10 + (timeStamp.charAt(9)-'0');

  Serial.print("TimeStamp Year: "); Serial.print(startYear); Serial.print("; Month: "); Serial.print(startMonth); Serial.print("; Day: "); Serial.println(startDay);

  // GET CURRENT DATE
  int curYear = 0;
  int curMonth = 0;
  int curDay = 0;
  const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  for(int i = 0; i < 12; i++)
  {
    int monthIndex = mostRecentDateString.indexOf(months[i]);
    if(monthIndex != -1)
    {
      curMonth = i+1;

      curDay = ((mostRecentDateString.charAt(monthIndex - 3) - '0')*10) + (mostRecentDateString.charAt(monthIndex - 2) - '0');
      curYear = ((mostRecentDateString.charAt(monthIndex + 4) - '0')*1000) + ((mostRecentDateString.charAt(monthIndex + 5) - '0')*100) + ((mostRecentDateString.charAt(monthIndex + 6) - '0')*10) + (mostRecentDateString.charAt(monthIndex + 7) - '0');
    }
  }
  Serial.print("Current Year: "); Serial.print(curYear); Serial.print("; Month: "); Serial.print(curMonth); Serial.print("; Day: "); Serial.println(curDay);

  // GET DESTINATION DATE
  String destTime = countDownRoot["parameters"]["date"];

  int destYear = (destTime.charAt(0)-'0')*1000 + (destTime.charAt(1)-'0')*100 + (destTime.charAt(2)-'0')*10 + (destTime.charAt(3)-'0');
  int destMonth = (destTime.charAt(5)-'0')*10 + (destTime.charAt(6)-'0');
  int destDay = (destTime.charAt(8)-'0')*10 + (destTime.charAt(9)-'0');
  Serial.print("Dest Year: "); Serial.print(destYear); Serial.print("; Month: "); Serial.print(destMonth); Serial.print("; Day: "); Serial.println(destDay);

  int destDays = number_of_days(destYear, destDay, destMonth);
  int curDays = number_of_days(curYear, curDay, curMonth);
  int startDays = number_of_days(startYear, startDay, startMonth);

  int totalDays = destDays - startDays;
  int daysLeft = destDays - curDays;

  Serial.print("Days Left: "); Serial.println(daysLeft);

  if(daysLeft <= 0) // Countdown done
  {
    Serial.println("Countdown Complete");
  }
  else
  {
    double percentComplete = 1.0 - ((double)daysLeft / (double)totalDays);
    Serial.print("Percent Complete: "); Serial.println(percentComplete);
  }
}

void PaperSignals::TestSignalExecution(String JSONData)
{
  if(numTestServoSwings < TEST_NUM_SWINGS)
  {
    MoveServoToPosition(TEST_FIRST_POSITION, 10);
    MoveServoToPosition(TEST_SECOND_POSITION, 10);
    numTestServoSwings++;
  }
}

void PaperSignals::StockExecution(String JSONData)
{
  //Only poll the barchart API if a new stock has been requested or it has been more than 2 minutes
  if(throttleStockAPI()) return;

  DynamicJsonBuffer stockBuffer;
  JsonObject& stockRoot = stockBuffer.parseObject(JSONData);
  String StockSymbol = stockRoot["parameters"]["StockSymbol"];

  Serial.print("Stock Symbol: "); Serial.println(StockSymbol);

  char* host = "marketdata.websol.barchart.com";
  String url = "/getQuote.json?apikey=" + BarChartAPIKey + "&symbols=" + StockSymbol;

  String barChartStockJSON = getJson(host, url);
  JsonObject& barChartStockRoot = stockBuffer.parseObject(barChartStockJSON);
  double stockNumber = barChartStockRoot["results"][0]["netChange"];

  if(stockNumber < 0)
  {
     String debugMsg = StockSymbol + " is down";
     Serial.println(debugMsg);
     MoveServoToPosition(CURRENCY_DOWN, 10);
  }
  else if(stockNumber > 0)
  {
    String debugMsg = StockSymbol + " is up";
     Serial.println(debugMsg);
    MoveServoToPosition(CURRENCY_UP, 10);
  }
  else
  {
    String debugMsg = StockSymbol + " no change";
     Serial.println(debugMsg);
    MoveServoToPosition(CURRENCY_NO_CHANGE, 10);
  }
}

void PaperSignals::CustomExecution(String JSONData)
{
  DynamicJsonBuffer customIntentBuffer;
  JsonObject& customIntentRoot = customIntentBuffer.parseObject(JSONData);
  String customIntentData = customIntentRoot["parameters"]["customParameter"];

  Serial.print("Current Custom Parameter Data: "); Serial.println(customIntentData);
}

void PaperSignals::ParseIntentName(String intentName, String JSONData)
{
    if(intentName == CryptoCurrencyType)
    {
      CryptoCurrencyExecution(JSONData);
    }
    else if(intentName == ShortsOrPantsType)
    {
      ShortsOrPantsExecution(JSONData);
    }
    else if(intentName == UmbrellaType)
    {
      UmbrellaExecution(JSONData);
    }
    else if(intentName == TimerType)
    {
      TimerExecution(JSONData);
    }
    else if(intentName == StretchBreakType)
    {
      StretchBreakExecution(JSONData);
    }
    else if(intentName == RocketLaunchType)
    {
      RocketExecution(JSONData);
    }
    else if(intentName == CountdownType)
    {
      CountdownExecution(JSONData);
    }
    else if(intentName == TestSignalType)
    {
      TestSignalExecution(JSONData);
    }
    else if(intentName == StockType)
    {
      StockExecution(JSONData);
    }
    else if(intentName.equalsIgnoreCase(CustomIntentType))
    {
      CustomExecution(JSONData);
    }
    else
    {
      DefaultExecution(JSONData);
    }
}

String PaperSignals::getSignalByID(String signalID){
    char* host = "gweb-paper-signals.firebaseio.com";
    String url = "/signals/" + signalID + ".json";

    String payload = getJson(host, url);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);

    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return "bad";
    }
    else
    {
      String signalInfo = root["result"];
      String intentName = root["result"]["metadata"]["intentName"];
      String intentTimeStamp = root["result"]["timestamp"];

      if(intentTimeStamp != currentIntentTimeStamp)
      {
        updatedIntentTimeStamp = true;
        numTestServoSwings = 0;
      }
      else {
        updatedIntentTimeStamp = false;
      }
      currentIntent = intentName;
      currentIntentTimeStamp = intentTimeStamp;

      return signalInfo;
    }
}

String PaperSignals::getJson(String host, String url){

    Serial.print("connecting to ");
    Serial.println(host);

    if (!client.connect(host.c_str(), httpsPort)) {
      Serial.println("connection failed");
      return "bad";
    }

    Serial.print("requesting URL ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("request sent");

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if(line.indexOf("Date") != -1)
      {
        mostRecentDateString = line;
      }
      Serial.println(line);
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }

    String payload = client.readString();

    #ifdef PRINT_PAYLOAD
        Serial.println("reply was:");
        Serial.println("==========");
        Serial.println(payload);
        Serial.println("==========");
    #endif
    return payload;
}

String PaperSignals::getJsonHTTP(String host, String url){

    HTTPClient http;
    String payload;

    Serial.print("[HTTP] begin...\n");

    http.begin("http://"+host+url); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            payload = http.getString();
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();


    #ifdef PRINT_PAYLOAD
        Serial.println("reply was:");
        Serial.println("==========");
        Serial.println(payload);
        Serial.println("==========");
    #endif
    return payload;
}
void PaperSignals::RunPaperSignals()
{
  String JSONData = getSignalByID(SignalID.c_str());
  ParseIntentName(currentIntent, JSONData);
}
