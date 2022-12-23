#ifndef WeatherCalculations_h
#define WeatherCalculations_h

#include "Arduino.h"

char forecast[27][36] = {"Settled fine", "Fine weather", "Becoming fine", "Fine, becoming less settled", "Fine, possible showers", "Fairly fine, improving", "Fairly fine, possible showers early", "Fairly fine, showery later", "Showery early, improving", "Changeable, mending", "Fairly fine, showers likely", "Rather unsettled clearing later", "Unsettled, probably improving", "Showery, bright intervals", "Showery, becoming less settled", "Changeable, some rain", "Unsettled, short fine intervals", "Unsettled, rain later", "Unsettled, some rain", "Mostly very unsettled", "Occasional rain, worsening", "Rain at times, very unsettled", "Rain at frequent intervals", "Rain, very unsettled", "Stormy, may improve", "Stormy, much rain"};

// equivalents of Zambretti 'dial window' letters A - Z
int rise_options[27] = {25, 25, 25, 24, 24, 19, 16, 12, 11, 9, 8, 6, 5, 2, 1, 1, 0, 0, 0, 0, 0, 0};
int steady_options[27] = {25, 25, 25, 25, 25, 25, 23, 23, 22, 18, 15, 13, 10, 4, 1, 1, 0, 0, 0, 0, 0, 0};
int fall_options[27] = {25, 25, 25, 25, 25, 25, 25, 25, 23, 23, 21, 20, 17, 14, 7, 3, 1, 1, 1, 0, 0, 0};

class Weather {
  public:
    Weather();
    double tempFtoC(double Temperature);
    double tempCtoF(double Temperature);
    double getSeaLevelPressure(double AirPressure, double Altitude);
    double getAltitude(double AirPressure, double SeaLevelPressure);
    double getDewPoint(double Temperature, double Humidity);
    double getHeatIndex(double Temperature, double Humidity);
    double getHumidex(double Temperature, double DewPoint);
    double getWindChill(double Temperature, double WindSpeed);
    byte getComfort(double heatIndex);
    const char* getForecast (double currentPressure, const int month, const char windDirection[4], const int pressureTrend, const boolean hemisphere = true, const double highestPressureEverRecorded = 1050, const double lowestPressureEverRecorded = 950);
    int getForecastSeverity (double currentPressure, const int month, const char windDirection[4], const int pressureTrend, const boolean hemisphere = true, const double highestPressureEverRecorded = 1050, const double lowestPressureEverRecorded = 950);

  private:
};

#endif
