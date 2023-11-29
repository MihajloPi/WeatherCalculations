#ifndef WeatherCalculations_h
#define WeatherCalculations_h

#include "Arduino.h"
#include <map>

class Weather {
public:
  const char forecast[27][36] = { "Settled fine", "Fine weather", "Becoming fine", "Fine, becoming less settled", "Fine, possible showers", "Fairly fine, improving", "Fairly fine, possible showers early", "Fairly fine, showery later", "Showery early, improving", "Changeable, mending", "Fairly fine, showers likely", "Rather unsettled clearing later", "Unsettled, probably improving", "Showery, bright intervals", "Showery, becoming less settled", "Changeable, some rain", "Unsettled, short fine intervals", "Unsettled, rain later", "Unsettled, some rain", "Mostly very unsettled", "Occasional rain, worsening", "Rain at times, very unsettled", "Rain at frequent intervals", "Rain, very unsettled", "Stormy, may improve", "Stormy, much rain" };
  // equivalents of Zambretti 'dial window' letters A - Z
  const uint8_t rise_options[22] = { 25, 25, 25, 24, 24, 19, 16, 12, 11, 9, 8, 6, 5, 2, 1, 1, 0, 0, 0, 0, 0, 0 };
  const uint8_t steady_options[22] = { 25, 25, 25, 25, 25, 25, 23, 23, 22, 18, 15, 13, 10, 4, 1, 1, 0, 0, 0, 0, 0, 0 };
  const uint8_t fall_options[22] = { 25, 25, 25, 25, 25, 25, 25, 25, 23, 23, 21, 20, 17, 14, 7, 3, 1, 1, 1, 0, 0, 0 };

  enum WindDirection { N,
                       NNE,
                       NE,
                       ENE,
                       E,
                       ESE,
                       SE,
                       SSE,
                       S,
                       SSW,
                       SW,
                       WSW,
                       W,
                       WNW,
                       NW,
                       NNW,
                       NOW};


  // Correction factors for wind direction in the northern hemisphere
  std::map<WindDirection, double> correctionFactorsNorthHemisphere = {
    { N, 0.06 },
    { NNE, 0.05 },
    { NE, 0.05 },
    { ENE, 0.02 },
    { E, -0.005 },
    { ESE, -0.02 },
    { SE, -0.05 },
    { SSE, -0.085 },
    { S, -0.12 },
    { SSW, -0.1 },
    { SW, -0.06 },
    { WSW, -0.045 },
    { W, -0.03 },
    { WNW, -0.005 },
    { NW, 0.015 },
    { NNW, 0.03 },
    { NOW, 0.0 }
  };

  // Correction factors for wind direction in the southern hemisphere
  std::map<WindDirection, double> correctionFactorsSouthHemisphere = {
    { S, 0.06 },
    { SSW, 0.06 },
    { SW, 0.05 },
    { WSW, 0.02 },
    { W, -0.005 },
    { WNW, -0.02 },
    { NW, -0.05 },
    { NNW, -0.085 },
    { N, -0.12 },
    { NNE, -0.1 },
    { NE, -0.1 },
    { ENE, -0.06 },
    { E, -0.045 },
    { ESE, -0.03 },
    { SE, -0.005 },
    { SSE, 0.015 },
    { NOW, 0.0 }
  };


  Weather();
  double tempFtoC(double Temperature);
  double tempCtoF(double Temperature);
  double getSeaLevelPressure(double AirPressure, double Altitude);
  double getAltitude(double AirPressure, double SeaLevelPressure);
  double getDewPoint(double Temperature, double Humidity);
  double getHeatIndex(double Temperature, double Humidity);
  double getHumidex(double Temperature, double DewPoint);
  double getWindChill(double Temperature, double WindSpeed);
  uint8_t getComfort(double heatIndex);
  uint16_t getAQI(uint16_t PM25, uint16_t PM10);
  //Pressure trend variable: 1 for rising, 2 for falling, everything else is by default steady
  //Months are 1-12
  //Wind direction is descriptive ("N", "SSE" etc.); if there's no wind, you should send nonsense string like "NOW" (NO Wind)
  //Hemisphere true for the northern, false for the southern

  //getForecastSeverity does the same but returns the index in the forecast array
  //getForecast function returns the descriptive explanation of the weather (e.g. "Fine weather")
  uint8_t getForecastSeverity(double currentPressure, const uint8_t month, WindDirection windDirection, const uint8_t pressureTrend, const boolean hemisphere = true, const double highestPressureEverRecorded = 1050, const double lowestPressureEverRecorded = 950);
  char* getForecast(double currentPressure, const uint8_t month, WindDirection windDirection, const uint8_t pressureTrend, const boolean hemisphere = true, const double highestPressureEverRecorded = 1050, const double lowestPressureEverRecorded = 950);

private:
};

#endif
