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

#include <iostream>
#include <string>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <curl/curl.h>
#include "APICalls.h"

// #define PRINT_PAYLOAD

inline std::string trim(const std::string s){
  auto wsfront = std::find_if_not(s.begin(), s.end(), [] (int c){return std::isspace(c);});
  auto wsback = std::find_if_not(s.rbegin(), s.rend(), [] (int c){return std::isspace(c);}).base();
  return (wsback <= wsfront ? std::string() : std::string(wsfront,wsback));
}


std::string urlencode(std::string str)
{
    std::string encodedString="";
    char c;
    char code0;
    char code1;
    // char code2;
    for (size_t i =0; i < str.length(); i++){
      c=str[i];
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
        // code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      // yield();
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

std::string makeLessPrettyJSON(std::string JSONData)
{
  std::string notPretty = "";
  for(size_t i = 0; i < JSONData.length(); i++)
  {
    if(JSONData[i] != '\n' && JSONData[i] != '\r' &&
      JSONData[i] != ' ' && JSONData[i+1] != ' ' &&
      JSONData[i] != '[' && JSONData[i] != ']')
    {
      notPretty += JSONData[i];
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

void PaperSignals::DefaultExecution(std::string JSONData)
{
    std::cout << "Default" << std::endl;
}

void PaperSignals::TimerExecution(std::string JSONData)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(JSONData);
  int seconds = root["parameters"]["duration"]["amount"];

  std::cout << "Timer: " << seconds << std::endl;

  if(lastTimerTime != seconds)
  {
    int totalDistance = abs(TIMER_END - TIMER_START);
    int speed = (seconds*1000)/totalDistance;
    MoveServoToPosition(TIMER_START, 0);
    MoveServoToPosition(TIMER_END, speed);

    MoveServoToPosition(TIMER_WIGGLE_TOP, 10);
    MoveServoToPosition(TIMER_WIGGLE_BOTTOM, 10);
    MoveServoToPosition(TIMER_WIGGLE_TOP, 10);
    MoveServoToPosition(TIMER_WIGGLE_BOTTOM, 10);

    lastTimerTime = seconds;
  }
}

double PaperSignals::GetBitcoin()
{
    std::string host = "api.coinmarketcap.com";
    std::string url = "/v1/ticker/bitcoin/";

    std::string payload = getJson(host, url);

    std::string unPretty = makeLessPrettyJSON(payload);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(unPretty);

    // Test if parsing succeeds.
    if (!root.success()) {
      std::cout << "parseObject() failed" << std::endl;
    }

    double change = root[std::string("percent_change_24h")];

    return change;
}

double PaperSignals::GetEthereum()
{
    std::string host = "api.coinmarketcap.com";
    std::string url = "/v1/ticker/ethereum/";

    std::string payload = getJson(host, url);

    std::string unPretty = makeLessPrettyJSON(payload);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(unPretty);

    // Test if parsing succeeds.
    if (!root.success()) {
      std::cout << "parseObject() failed" << std::endl;
    }

    double change = root[std::string("percent_change_24h")];

    return change;
}

void PaperSignals::CryptoCurrencyExecution(std::string JSONData)
{
    std::string Bitcoin = "Bitcoin";
    std::string Ethereum = "Ethereum";

    DynamicJsonBuffer cryptoBuffer;
    JsonObject& cryptoRoot = cryptoBuffer.parseObject(JSONData);
    std::string CurrencyType = cryptoRoot["parameters"]["Crypto"];

    if(CurrencyType == Bitcoin)
    {
      double curChange = GetBitcoin();
      std::cout << "Tracking Bitcoin " << curChange << std::endl;

      if(curChange < 0)
      {
         std::cout << "Bitcoin is Down" << std::endl;
         MoveServoToPosition(CURRENCY_DOWN, 10);
      }
      else if(curChange > 0)
      {
        std::cout << "Bitcoin is Up" << std::endl;
        MoveServoToPosition(CURRENCY_UP, 10);
      }
      else
      {
        std::cout << "Bitcoin No Change" << std::endl;
        MoveServoToPosition(CURRENCY_NO_CHANGE, 10);
      }
    }
    else if(CurrencyType == Ethereum)
    {
      double curChange = GetEthereum();
      std::cout << "Tracking Ethereum " << curChange << std::endl;

      if(curChange < 0)
      {
        std::cout << "Ethereum is Down" << std::endl;
        MoveServoToPosition(CURRENCY_DOWN, 10);
      }
      else if(curChange > 0)
      {
        std::cout << "Ethereum is Up" << std::endl;
        MoveServoToPosition(CURRENCY_UP, 10);
      }
      else
      {
        std::cout << "Ethereum No Change" << std::endl;
        MoveServoToPosition(CURRENCY_NO_CHANGE, 10);
      }
    }
    else
    {
      std::cout << "Currency not supported" << std::endl;
      return;
    }


}

void PaperSignals::CelebrateExecution(std::string JSONData)
{
  DynamicJsonBuffer celebrateBuffer;
  JsonObject& celebrateRoot = celebrateBuffer.parseObject(JSONData);
  breakTimeInterval = celebrateRoot["parameters"]["duration"]["amount"]; // In Minutes
  breakTimeInterval = breakTimeInterval*60*1000;

  if(lastBreakTime > millis()) // Reset every 49 days
  {
    lastBreakTime = millis();
  }

  if(millis() - lastBreakTime > breakTimeInterval)
  {
    lastBreakTime = millis();
  }

  std::cout << "Break in " << (breakTimeInterval - (millis() - lastBreakTime)) << std::endl;

  if(millis() - lastBreakTime < breakTimeLength)
  {
    // Hands Up
    std::cout << "Break Time" << std::endl;
    MoveServoToPosition(CELEBRATE_TIME_REACHED, 10);
  }
  else
  {
    // Hands Down
    std::cout << "No Breaks Yet" << std::endl;
    MoveServoToPosition(CELEBRATE_NOT_YET, 10);
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

void PaperSignals::ShortsOrPantsExecution(std::string JSONData)
{
  //Only poll the weather API if a new location has been requested or it has been more than 2 minutes
  if(throttleWeatherAPI()) return;

  std::cout << "CUR DATA  " << JSONData << std::endl;
  DynamicJsonBuffer weatherLocationBuffer;
  JsonObject& locationRoot = weatherLocationBuffer.parseObject(JSONData);
  std::string City = locationRoot["parameters"]["location"]["city"];
  std::string State = locationRoot["parameters"]["location"]["admin-area"];
  std::string Address = locationRoot["parameters"]["location"]["street-address"];
  std::string MaybeAlsoACity = locationRoot["parameters"]["location"]["subadmin-area"];

  std::string Location = MaybeAlsoACity + " " + City + " " + State + " " + Address;

  std::string WeatherJSON = GetWeather(Location);

  DynamicJsonBuffer jsonBufferWeather;
  JsonObject& weatherRoot = jsonBufferWeather.parseObject(WeatherJSON);

  double temperature = weatherRoot["daily"]["data"][0]["temperatureLow"];
  std::cout << "temperature " << temperature << std::endl;

  if(temperature > SHORTSORPANTS_THRESHOLD)
  {
    MoveServoToPosition(SHORTSORPANTS_SHORTS, 10);
  }
  else
  {
    MoveServoToPosition(SHORTSORPANTS_PANTS, 10);
  }
}

void PaperSignals::UmbrellaExecution(std::string JSONData)
{
  //Only poll the weather API if a new location has been requested or it has been more than 2 minutes
  if(throttleWeatherAPI()) return;

  DynamicJsonBuffer weatherLocationBuffer;
  JsonObject& locationRoot = weatherLocationBuffer.parseObject(JSONData);
  std::string City = locationRoot["parameters"]["location"]["city"];
  std::string State = locationRoot["parameters"]["location"]["admin-area"];
  std::string Address = locationRoot["parameters"]["location"]["street-address"];
  std::string MaybeAlsoACity = locationRoot["parameters"]["location"]["subadmin-area"];

  std::string Location = MaybeAlsoACity + " " + City + " " + State + " " + Address;

  std::string WeatherJSON = GetWeather(Location);

  DynamicJsonBuffer jsonBufferWeather;
  JsonObject& weatherRoot = jsonBufferWeather.parseObject(WeatherJSON);

  std::string iconWeather = weatherRoot["daily"]["data"][0]["icon"];
  std::cout << "iconWeather " << iconWeather << std::endl;

  std::string rainIcon = "rain";
  std::string sleetIcon = "sleet";

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
std::string PaperSignals::GetWeather(std::string Location)
{
  std::string latLong = GetLatLong(Location);

  std::string curTimeHost = "currentmillis.com";
  std::string curTimeURL = "/time/minutes-since-unix-epoch.php";
  std::string unixTime = getJsonHTTP(curTimeHost, curTimeURL);

  unsigned long unixTimeLong = atol(unixTime.c_str())*60;
  std::string finalUnixTime = std::to_string(unixTimeLong);

  std::string weatherHost = "api.darksky.net";
  std::string weatherURL = "/forecast/" + DarkSkyAPIKey + "/" + latLong + "," + finalUnixTime + "?exclude=minutely,flags,hourly,currently,alerts";

  // Get Weather
  std::string weatherPayload = getJson(weatherHost, weatherURL);
  std::cout << "Weather payload " << weatherPayload << std::endl;

  return weatherPayload;
}

// This function handles the Google Geocoding API Pretty Print Output
std::string PaperSignals::GetLatLong(std::string Location)
{
  Location = urlencode(Location);
  std::string  geoHost = "maps.googleapis.com";
  std::string geoURL = "/maps/api/geocode/json?address=" + Location + "&key=" + GeoCodingAPiKey;
  std::cout << "geoURL " << geoURL << std::endl;

  // Get Lat Long
  std::string LatLongPayload = getJson(geoHost, geoURL);
  std::cout << "LatLongPayload " << LatLongPayload << std::endl;

  int index = LatLongPayload.find("lat");

  std::cout << "index " << index << std::endl;
  bool foundColon = false;
  std::string latString = "";
  for(int i = index; i < index+20; i++)
  {
    if(LatLongPayload[i] == ':')
    {
      foundColon = true;
    }
    else if(foundColon)
    {
      if(LatLongPayload[i] == ',') break;
      latString += LatLongPayload[i];
    }
  }

  latString = trim(latString);
  std::cout << "Lat String " << latString << std::endl;

  index = LatLongPayload.find("lng");

  std::cout << "index " << index << std::endl;
  foundColon = false;
  std::string lngString = "";
  for(int i = index; i < index+20; i++)
  {
    if(LatLongPayload[i] == ':')
    {
      foundColon = true;
    }
    else if(foundColon)
    {
      if(LatLongPayload[i] == '\n'|| LatLongPayload[i] == '\r') break;
      lngString += LatLongPayload[i];
    }
  }

  lngString = trim(lngString);
  std::cout << "Lng String: " << lngString << std::endl;

  return latString + "," + lngString;

}

void PaperSignals::RocketExecution(std::string JSONData)
{
  std::cout << "Rockets" << std::endl;

  DynamicJsonBuffer agencyBuffer;
  JsonObject& agencyRoot = agencyBuffer.parseObject(JSONData);
  std::string curSpaceAgency = agencyRoot["parameters"]["SpaceAgency"];
  std::string curCountry = agencyRoot["parameters"]["RocketCountryCodes"];

  std::string  rocketHost = "launchlibrary.net";
  std::string rocketURL = "/1.2/launch/next/1";
  std::string rocketPayload = getJson(rocketHost, rocketURL);
  std::cout << rocketPayload << std::endl;

  DynamicJsonBuffer jsonBufferRockets;
  JsonObject& rocketRoot = jsonBufferRockets.parseObject(rocketPayload);

  std::string output = rocketRoot["launches"][0]["isonet"];

  if(curSpaceAgency != "" || curCountry != "")
  {
    std::cout << "Looking for " << curSpaceAgency << " Rockets" << std::endl;
    bool foundAgency = false;
    std::string agency = "";
    int spaceAgencyNum = 0;
    do
    {
      std::string countryCode = rocketRoot["launches"][0]["location"]["pads"][0]["agencies"][spaceAgencyNum]["countryCode"];
      std::string agencyNew = rocketRoot["launches"][0]["location"]["pads"][0]["agencies"][spaceAgencyNum]["abbrev"];
      agency = agencyNew;

      std::cout << countryCode << " " << agency << std::endl;

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
    std::cout << "Initalized Launch" << std::endl;

    NextRocketLaunchTime = output;
    rocketLaunched = false;

    MoveServoToPosition(ROCKET_NOT_LAUNCHED, 10);
  }
  else if(NextRocketLaunchTime != output)
  {
    std::cout << "LAUNCH" << std::endl;
    NextRocketLaunchTime = output;
    rocketStartedTime = millis();
    rocketLaunched = true;

    MoveServoToPosition(ROCKET_LAUNCH, 10);
  }
  else
  {
    std::cout << "No launch yet" << std::endl;

    rocketLaunched = false;
    MoveServoToPosition(ROCKET_NOT_LAUNCHED, 10);
  }
}

void PaperSignals::CountdownExecution(std::string JSONData)
{
  DynamicJsonBuffer countDownBuffer;
  JsonObject& countDownRoot = countDownBuffer.parseObject(JSONData);
  std::string timeStamp = countDownRoot["timestamp"];

  // GET START DATE
  int startYear = (timeStamp[0]-'0')*1000 + (timeStamp[1]-'0')*100 + (timeStamp[2]-'0')*10 + (timeStamp[3]-'0');
  int startMonth = (timeStamp[5]-'0')*10 + (timeStamp[6]-'0');
  int startDay = (timeStamp[8]-'0')*10 + (timeStamp[9]-'0');

  std::cout << "TimeStamp Year: " << startYear << "; Month: " << startMonth << "; Day: " << startDay << std::endl;

  // GET CURRENT DATE
  int curYear = 0;
  int curMonth = 0;
  int curDay = 0;
  const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  for(int i = 0; i < 12; i++)
  {
    std::size_t monthIndex = mostRecentDateString.find(months[i]);
    if(monthIndex != std::string::npos)
    {
      curMonth = i+1;

      curDay = ((mostRecentDateString[monthIndex - 3] - '0')*10) + (mostRecentDateString[monthIndex - 2] - '0');
      curYear = ((mostRecentDateString[monthIndex + 4] - '0')*1000) + ((mostRecentDateString[monthIndex + 5] - '0')*100) + ((mostRecentDateString[monthIndex + 6] - '0')*10) + (mostRecentDateString[monthIndex + 7] - '0');
    }
  }
  std::cout << "Current Year: " << curYear << "; Month: " << curMonth << "; Day: " << curDay << std::endl;

  // GET DESTINATION DATE
  std::string destTime = countDownRoot["parameters"]["date"];

  int destYear = (destTime[0]-'0')*1000 + (destTime[1]-'0')*100 + (destTime[2]-'0')*10 + (destTime[3]-'0');
  int destMonth = (destTime[5]-'0')*10 + (destTime[6]-'0');
  int destDay = (destTime[8]-'0')*10 + (destTime[9]-'0');
  std::cout << "Dest Year: " << destYear << "; Month: " << destMonth << "; Day: " << destDay << std::endl;

  int destDays = number_of_days(destYear, destDay, destMonth);
  int curDays = number_of_days(curYear, curDay, curMonth);
  int startDays = number_of_days(startYear, startDay, startMonth);

  int totalDays = destDays - startDays;
  int daysLeft = destDays - curDays;

  std::cout << "Days Left: " << daysLeft << std::endl;

  if(daysLeft <= 0) // Countdown done
  {
    std:: cout << "Countdown Complete" << std::endl;
  }
  else
  {
    double percentComplete = 1.0 - ((double)daysLeft / (double)totalDays);
    std::cout << "Percent Complete: " << percentComplete << std::endl;
  }
}

void PaperSignals::TestSignalExecution(std::string JSONData)
{
  if(numTestServoSwings < TEST_NUM_SWINGS)
  {
    MoveServoToPosition(TEST_FIRST_POSITION, 10);
    MoveServoToPosition(TEST_SECOND_POSITION, 10);
    numTestServoSwings++;
  }
}

void PaperSignals::StockExecution(std::string JSONData)
{
  //Only poll the barchart API if a new stock has been requested or it has been more than 2 minutes
  if(throttleStockAPI()) return;

  DynamicJsonBuffer stockBuffer;
  JsonObject& stockRoot = stockBuffer.parseObject(JSONData);
  std::string StockSymbol = stockRoot["parameters"]["StockSymbol"];

  std::cout << "Stock Symbol: " << StockSymbol << std::endl;

  std::string host = "marketdata.websol.barchart.com";
  std::string url = "/getQuote.json?apikey=" + BarChartAPIKey + "&symbols=" + StockSymbol;

  std::string barChartStockJSON = getJson(host, url);
  JsonObject& barChartStockRoot = stockBuffer.parseObject(barChartStockJSON);
  double stockNumber = barChartStockRoot["results"][0]["netChange"];

  if(stockNumber < 0)
  {
     std::cout <<  StockSymbol <<  " is down" << std::endl;
     MoveServoToPosition(CURRENCY_DOWN, 10);
  }
  else if(stockNumber > 0)
  {
    std::cout <<  StockSymbol <<  " is up" << std::endl;
    MoveServoToPosition(CURRENCY_UP, 10);
  }
  else
  {
    std::cout << StockSymbol << " no change" << std::endl;
    MoveServoToPosition(CURRENCY_NO_CHANGE, 10);
  }
}

void PaperSignals::CustomExecution(std::string JSONData)
{
  DynamicJsonBuffer customIntentBuffer;
  JsonObject& customIntentRoot = customIntentBuffer.parseObject(JSONData);
  std::string customIntentData = customIntentRoot["parameters"]["customParameter"];

  std::cout << "Current Custom Parameter Data: " << customIntentData << std::endl;
}

std::string str_toupper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), 
                   [](unsigned char c){ return std::toupper(c); } // correct
                  );
    return s;
}

void PaperSignals::ParseIntentName(std::string intentName, std::string JSONData)
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
    else if(intentName == CelebrateType)
    {
      CelebrateExecution(JSONData);
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
    else if(str_toupper(intentName) == str_toupper(CustomIntentType))
    {
      CustomExecution(JSONData);
    }
    else
    {
      DefaultExecution(JSONData);
    }
}

std::string PaperSignals::getSignalByID(std::string signalID){
    std::string host = "gweb-paper-signals.firebaseio.com";
    std::string url = "/signals/" + signalID + ".json";

    std::string payload = getJson(host, url);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);

    // Test if parsing succeeds.
    if (!root.success()) {
      std::cout << "parseObject() failed" << std::endl;
      return "bad";
    }
    else
    {
      std::string signalInfo = root["result"];
      std::string intentName = root["result"]["metadata"]["intentName"];
      std::string intentTimeStamp = root["result"]["timestamp"];

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

static size_t WriteResultToString(void *contents, size_t size, size_t nmemb, void *userp){
  size_t realsize = size * nmemb;
  std::string *str = (std::string*)userp;
  *str += (char*) contents;
  return realsize;
}

static size_t HeaderReceived(char *buffer, size_t size, size_t nitems, void *userp){
  size_t realsize = size * nitems;
  std::string *str = (std::string*)userp;
  std::string line = std::string(buffer);

  if(line.find("Date") != std::string::npos)
  {
    *str += line;
    std::cout << "Date Header: " << line << std::endl;
  }else{
    std::cout << "Header: " << line << std::endl;
  }


  return realsize;
}

std::string PaperSignals::getJson(std::string host, std::string url, bool http){
    std::ostringstream urls;
    urls << "http" << (http ? "" : "s") << "://" << host << url;
    // urls << "https://" << host << url;
    std::cout << "getJson to " << urls.str() << std::endl;

    CURL *easyhandle = curl_easy_init();
    curl_easy_setopt(easyhandle,CURLOPT_URL, urls.str().c_str());

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, (std::string("Host: ") + host).c_str());
    headers = curl_slist_append(headers, "User-Agent: BuildFailureDetectorESP8266");
    headers = curl_slist_append(headers, "connection: close");
    curl_easy_setopt(easyhandle,CURLOPT_HTTPHEADER, headers);
    std::string resultString = "";
    
    
    curl_easy_setopt(easyhandle,CURLOPT_WRITEFUNCTION, WriteResultToString);
    curl_easy_setopt(easyhandle,CURLOPT_WRITEDATA, (void*)&resultString);

    curl_easy_setopt(easyhandle,CURLOPT_HEADERFUNCTION, HeaderReceived);
    curl_easy_setopt(easyhandle,CURLOPT_HEADERDATA, (void*)&mostRecentDateString);

    CURLcode result = curl_easy_perform(easyhandle);

    if (result != CURLE_OK) {
      std::cout << "connection failed" << std::endl;
      return "bad";
    }

    // std::cout << "requesting URL " << url << std::endl;

    // client.print(std::string("GET ") + url + " HTTP/1.1\r\n" +
    //              "Host: " + host + "\r\n" +
    //              "User-Agent: BuildFailureDetectorESP8266\r\n" +
    //              "Connection: close\r\n\r\n");

    curl_slist_free_all(headers);
    curl_easy_cleanup(easyhandle);


    std::cout << "request done" << std::endl << resultString << std::endl;
    // std::istringstream lines (resultString);
    // std::string line;
    // std::ostringstream payload;
    // bool inHeaders = true;
    // while (std::getline(lines,line)) {
    //   if(inHeaders){
    //     if(line.find("Date") != std::string::npos)
    //     {
    //       mostRecentDateString = line;
    //     }
    //     std::cout << "Header: " << line << std::endl;
    //     if (line == "\r") {
    //       std::cout << "headers received" << std::endl;
    //       inHeaders = false;
    //     }
    //   }else{
    //     payload << line << std::endl;
    //     std::cout << "Payload: " << line << std::endl;
    //   }
    // }

    #ifdef PRINT_PAYLOAD
        std::cout << "reply was:" << std::endl;
        std::cout << "==========" << std::endl;
        std::cout << resultString << std::endl;
        std::cout << "==========" << std::endl;
    #endif
    return resultString;
}

std::string PaperSignals::getJsonHTTP(std::string host, std::string url){
  return getJson(host,url,true);
    // HTTPClient http;
    // std::string payload;

    // std::cout << "[HTTP] begin..." << std::endl;

    // http.begin("http://"+host+url); //HTTP

    // std::cout << "[HTTP] GET..." << std::endl;
    // // start connection and send HTTP header
    // int httpCode = http.GET();

    // // httpCode will be negative on error
    // if(httpCode > 0) {
    //     // HTTP header has been send and Server response header has been handled
    //     std::cout << "[HTTP] GET... code: " << httpCode << std::endl;

    //     // file found at server
    //     if(httpCode == HTTP_CODE_OK) {
    //         payload = http.getString();
    //     }
    // } else {
    //     std::cout << "[HTTP] GET... failed, error: " << http.errorToString(httpCode) << std::endl;
    // }

    // http.end();


    // #ifdef PRINT_PAYLOAD
    //     std::cout << "reply was:" << std::endl;
    //     std::cout << "==========" << std::endl;
    //     std::cout << payload << std::endl;
    //     std::cout << "==========" << std::endl;
    // #endif
    // return payload;
}
void PaperSignals::RunPaperSignals()
{
  std::string JSONData = getSignalByID(SignalID.c_str());
  ParseIntentName(currentIntent, JSONData);
}
