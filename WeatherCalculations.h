#ifndef WeatherCalculations_h
#define WeatherCalculations_h

#include "Arduino.h"
#include <map>

class Weather {
public:


  enum WindDirection {
      N,    // North
      NNE,  // North-Northeast
      NE,   // Northeast
      ENE,  // East-Northeast
      E,    // East
      ESE,  // East-Southeast
      SE,   // Southeast
      SSE,  // South-Southeast
      S,    // South
      SSW,  // South-Southwest
      SW,   // Southwest
      WSW,  // West-Southwest
      W,    // West
      WNW,  // West-Northwest
      NW,   // Northwest
      NNW,  // North-Northwest
      NOW   // No Wind (or an invalid direction)
  };

  enum PressureTrend {
    rising,
    falling,
    steady
  };

  enum Comfort {
    uncomfortable,
    comfortable,
    small_discomfort,
    medium_discomfort,
    great_discomfort,
    dangerous
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
  Comfort getComfort(double heatIndex);
  uint16_t getAQI(uint16_t PM25, uint16_t PM10);
  //Months are 1-12
  //Wind direction is descriptive (N, SSE etc.); if there's no wind, you should send nonsense string like NOW (NO Wind)
  //Hemisphere true for the northern, false for the southern

  //getForecastSeverity does the same but returns the index in the forecast array
  //getForecast function returns the descriptive explanation of the weather (e.g. "Fine weather")
  uint8_t getForecastSeverity(double currentPressure, const uint8_t month, WindDirection windDirection, PressureTrend pressureTrend, const boolean hemisphere = true, const double highestPressureEverRecorded = 1050, const double lowestPressureEverRecorded = 950);
  char* getForecast(double currentPressure, const uint8_t month, WindDirection windDirection, PressureTrend pressureTrend, const boolean hemisphere = true, const double highestPressureEverRecorded = 1050, const double lowestPressureEverRecorded = 950);

private:
  const char forecast[27][36] = { "Settled fine", "Fine weather", "Becoming fine", "Fine, becoming less settled", "Fine, possible showers", "Fairly fine, improving", "Fairly fine, possible showers early", "Fairly fine, showery later", "Showery early, improving", "Changeable, mending", "Fairly fine, showers likely", "Rather unsettled clearing later", "Unsettled, probably improving", "Showery, bright intervals", "Showery, becoming less settled", "Changeable, some rain", "Unsettled, short fine intervals", "Unsettled, rain later", "Unsettled, some rain", "Mostly very unsettled", "Occasional rain, worsening", "Rain at times, very unsettled", "Rain at frequent intervals", "Rain, very unsettled", "Stormy, may improve", "Stormy, much rain" };
  // equivalents of Zambretti 'dial window' letters A - Z
  const uint8_t rise_options[22] = {25, 25, 25, 24, 24, 19, 16, 12, 11, 9, 8, 6, 5, 2, 1, 1, 0, 0, 0, 0, 0, 0};
  const uint8_t steady_options[22] = {25, 25, 25, 25, 25, 25, 23, 23, 22, 18, 15, 13, 10, 4, 1, 1, 0, 0, 0, 0, 0, 0};
  const uint8_t fall_options[22] = {25, 25, 25, 25, 25, 25, 25, 25, 23, 23, 21, 20, 17, 14, 7, 3, 1, 1, 1, 0, 0, 0};

  // Correction factors for wind direction
  std::map<WindDirection, double> WindCorrectionFactors = {
    {N, 0.06},
    {NNE, 0.05},
    {NE, 0.05},
    {ENE, 0.02},
    {E, -0.005},
    {ESE, -0.02},
    {SE, -0.05},
    {SSE, -0.085},
    {S, -0.12},
    {SSW, -0.1},
    {SW, -0.06},
    {WSW, -0.045},
    {W, -0.03},
    {WNW, -0.005},
    {NW, 0.015},
    {NNW, 0.03},
    {NOW, 0.0}
  };
};

#endif