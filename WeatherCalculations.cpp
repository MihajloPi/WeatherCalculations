#include "Arduino.h"
#include "WeatherCalculations.h"

Weather::Weather() {}

double Weather::tempCtoF(double Temperature) {
  return (Temperature * 1.8) + 32.0;
};

double Weather::tempFtoC(double Temperature) {
  return (Temperature - 32.0) / 1.8;
};

double Weather::getSeaLevelPressure(double AirPressure, double Altitude) {
  return AirPressure / pow((1.0 - (Altitude / 44330.0)), 5.225);
};

double Weather::getAltitude(double AirPressure, double SeaLevelPressure) {
  return 44330.0 * (1.0 - pow((AirPressure / SeaLevelPressure), (1.0 / 5.225)));
};

double Weather::getDewPoint(double Temperature, double Humidity) { //Temperature is in celsius so make sure to convert fahrenheit to celsius!
  // (1) Saturation Vapor Pressure = ESGG(T)
  double RATIO = 373.15 / (273.15 + Temperature);
  double RHS = -7.90298 * (RATIO - 1.0);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10.0, (11.344 * (1.0 - 1.0 / RATIO))) - 1.0) ;
  RHS += 8.1328e-3 * (pow(10.0, (-3.49149 * (RATIO - 1.0))) - 1.0) ;
  RHS += log10(1013.246);

  // factor -3 is to adjust units - Vapor Pressure SVP * humidity
  double VP = pow(10.0, RHS - 3.0) * Humidity;

  // (2) DEWPOINT = F(Vapor Pressure)
  double T = log(VP / 0.61078); // temp var
  return (241.88 * T) / (17.558 - T);
};

double Weather::getHeatIndex(double Temperature, double Humidity) {
  Temperature = tempCtoF(Temperature);
  double Adjustment;

  double HeatIndex = -42.379 + (2.04901523 * Temperature) + (10.14333127 * Humidity) + (-0.22475541 * Temperature * Humidity) + (-0.00683783 * Temperature * Temperature) + (-0.05481717 * Humidity * Humidity) + (0.00122874 * Temperature * Temperature * Humidity) + (0.00085282 * Temperature * Humidity * Humidity) + (-0.00000199 * Temperature * Temperature * Humidity * Humidity);

  if (Humidity < 13.0 && Temperature > 80.0 && Temperature < 112.0) {
    double i = Temperature - 95.0;

    Adjustment = ((13.0 - Humidity) / 4.0) * sqrt((17.0 - fabs(i)) / 17.0);
    HeatIndex -= Adjustment;
  }
  else if (Humidity > 85.0 && Temperature > 80.0 && Temperature < 87.0) {
    Adjustment = ((Humidity - 85.0) / 10.0) * ((87.0 - Temperature) / 5.0);
    HeatIndex += Adjustment;
  }

  else if (HeatIndex < 80.0) {
    HeatIndex = 0.5 * (Temperature + 61.0 + ((Temperature - 68.0) * 1.2) + (Humidity * 0.094));
  }
  return tempFtoC(HeatIndex);
};

double Weather::getHumidex(double Temperature, double DewPoint) {
  return Temperature + (-0.5555 * ((6.11 * pow(EULER, 5417.753 * ((1.0 / 273.15) - (1.0 / (273.15 + DewPoint))))) - 10.0));
};

double Weather::getWindChill(double Temperature, double WindSpeed) {
  double WindChill;
  WindSpeed *= 0.621371;

  WindChill = (13.12 + 0.6215 * Temperature - 11.37 * pow(WindSpeed, 0.16) + 0.3965 * Temperature * pow(WindSpeed, 0.16));
  return WindChill;
};

uint8_t Weather::getComfort(double heatIndex) {
  if (heatIndex >= 20.0 && heatIndex <= 23.0) return 1; //Uncomfortable
  else if (heatIndex > 23.0 && heatIndex <= 26.0) return 2; //Comfortable
  else if (heatIndex > 26.0 && heatIndex <= 29.0) return 3; //Some discomfort
  else if (heatIndex > 29.0 && heatIndex <= 39.0) return 4; //Hot feeling
  else if (heatIndex > 39.0 && heatIndex <= 45.0) return 5; //Great discomfort; avoid exertion
  else if (heatIndex > 45.0) return 6; //Dangerous; probable heat stroke
}

uint16_t Weather::getAQI(uint16_t PM25, uint16_t PM10) {
  uint16_t AQI_25, AQI_10;

  if (PM25 >= 0 && PM25 <= 12) AQI_25 = map(PM25, 0, 12, 0, 50);
  else if (PM25 >= 13 && PM25 <= 35) AQI_25 = map(PM25, 13, 35, 51, 100);
  else if (PM25 >= 36 && PM25 <= 55) AQI_25 = map(PM25, 36, 55, 101, 150);
  else if (PM25 >= 56 && PM25 <= 150) AQI_25 = map(PM25, 56, 150, 151, 200);
  else if (PM25 >= 151 && PM25 <= 250) AQI_25 = map(PM25, 151, 250, 201, 300);
  else if (PM25 >= 251 && PM25 <= 500) AQI_25 = map(PM25, 251, 500, 301, 500);

  if (PM10 >= 0 && PM10 <= 54) AQI_10 = map(PM10, 0, 54, 0, 50);
  else if (PM10 >= 55 && PM10 <= 154) AQI_10 = map(PM10, 55, 154, 51, 100);
  else if (PM10 >= 155 && PM10 <= 254) AQI_10 = map(PM10, 155, 254, 101, 150);
  else if (PM10 >= 255 && PM10 <= 354) AQI_10 = map(PM10, 255, 354, 151, 200);
  else if (PM10 >= 355 && PM10 <= 424) AQI_10 = map(PM10, 355, 424, 201, 300);
  else if (PM10 >= 425 && PM10 <= 604) AQI_10 = map(PM10, 425, 604, 301, 500);

  return max(AQI_25, AQI_10);
}

char* Weather::getForecast (double currentPressure, const int month, const char windDirection[4], const int pressureTrend, const boolean hemisphere, const double highestPressureEverRecorded, const double lowestPressureEverRecorded) {
  double pressureRange = highestPressureEverRecorded - lowestPressureEverRecorded;
  double constant = (pressureRange / 22.0);
  boolean z_season = false;
  if (month >= 4 && month <= 9) z_season = true;              // true if 'Summer'

  if (hemisphere == 1) {                                      // North hemisphere
    if (!strcmp(windDirection, "N")) {
      currentPressure += 0.06 * pressureRange;
    } else if (!strcmp(windDirection, "NNE")) {
      currentPressure += 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "NE")) {
      currentPressure += 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "ENE")) {
      currentPressure += 0.02 * pressureRange;
    } else if (!strcmp(windDirection, "E")) {
      currentPressure -= 0.005 * pressureRange;
    } else if (!strcmp(windDirection, "ESE")) {
      currentPressure -= 0.02 * pressureRange;
    } else if (!strcmp(windDirection, "SE")) {
      currentPressure -= 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "SSE")) {
      currentPressure -= 0.085 * pressureRange;
    } else if (!strcmp(windDirection, "S")) {
      currentPressure -= 0.12 * pressureRange;
    } else if (!strcmp(windDirection, "SSW")) {
      currentPressure -= 0.1 * pressureRange;
    } else if (!strcmp(windDirection, "SW")) {
      currentPressure -= 0.06 * pressureRange;
    } else if (!strcmp(windDirection, "WSW")) {
      currentPressure -= 0.045 * pressureRange;
    } else if (!strcmp(windDirection, "W")) {
      currentPressure -= 0.03 * pressureRange;
    } else if (!strcmp(windDirection, "WNW")) {
      currentPressure -= 0.005 * pressureRange;
    } else if (!strcmp(windDirection, "NW")) {
      currentPressure += 0.015 * pressureRange;
    } else if (!strcmp(windDirection, "NNW")) {
      currentPressure += 0.03 * pressureRange;
    }

    if (z_season == 1) {    // if Summer
      if (pressureTrend == 1) {   // rising
        currentPressure += 0.07 * pressureRange;
      } else if (pressureTrend == 2) {  //  falling
        currentPressure -= 0.07 * pressureRange;
      }
    }
  }

  else {                                                      // must be South hemisphere
    if (!strcmp(windDirection, "S")) {
      currentPressure += 0.06 * pressureRange;
    } else if (!strcmp(windDirection, "SSW")) {
      currentPressure += 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "SW")) {

      currentPressure += 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "WSW")) {
      currentPressure += 0.02 * pressureRange;
    } else if (!strcmp(windDirection, "W")) {
      currentPressure -= 0.005 * pressureRange;
    } else if (!strcmp(windDirection, "WNW")) {
      currentPressure -= 0.02 * pressureRange;
    } else if (!strcmp(windDirection, "NW")) {
      currentPressure -= 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "NNW")) {
      currentPressure -= 0.085 * pressureRange;
    } else if (!strcmp(windDirection, "N")) {
      currentPressure -= 0.12 * pressureRange;
    } else if (!strcmp(windDirection, "NNE")) {
      currentPressure -= 0.1 * pressureRange;
    } else if (!strcmp(windDirection, "NE")) {
      currentPressure -= 0.06 * pressureRange;
    } else if (!strcmp(windDirection, "ENE")) {
      currentPressure -= 0.045 * pressureRange;
    } else if (!strcmp(windDirection, "E")) {
      currentPressure -= 0.03 * pressureRange;
    } else if (!strcmp(windDirection, "ESE")) {
      currentPressure -= 0.005 * pressureRange;
    } else if (!strcmp(windDirection, "SE")) {
      currentPressure += 0.015 * pressureRange;
    } else if (!strcmp(windDirection, "SSE")) {
      currentPressure += 0.03 * pressureRange;
    }
    if (z_season == 0) {                                // if Winter
      if (pressureTrend == 1) {                         // rising
        currentPressure += 0.07 * pressureRange;
      } else if (pressureTrend == 2) {                  // falling
        currentPressure -= 0.07 * pressureRange;
      }
    }
  }   // END North / South

  if (currentPressure == highestPressureEverRecorded) currentPressure = highestPressureEverRecorded - 1;
  int forecastOption = floor((currentPressure - lowestPressureEverRecorded) / constant);

  static char outputForecast[57];
  strcpy(outputForecast, "");

  if (forecastOption < 0) {
    forecastOption = 0;
    strcpy(outputForecast, "Exceptional Weather, ");
  }
  if (forecastOption > 21) {
    forecastOption = 21;
    strcpy(outputForecast, "Exceptional Weather, ");
  }

  if (pressureTrend == 1) {                                                           // rising
    strcat(outputForecast, forecast[rise_options[forecastOption]]);
  }
  else if (pressureTrend == 2) {                                                      // falling
    strcat(outputForecast, forecast[fall_options[forecastOption]]);
  }
  else {                                                                              // must be 'steady'
    strcat(outputForecast, forecast[steady_options[forecastOption]]);
  }

  return outputForecast;
}

int Weather::getForecastSeverity (double currentPressure, const int month, const char windDirection[4], const int pressureTrend, const boolean hemisphere, const double highestPressureEverRecorded, const double lowestPressureEverRecorded) {
  double pressureRange = highestPressureEverRecorded - lowestPressureEverRecorded;
  double constant = (pressureRange / 22.0);
  boolean z_season = false;
  if (month >= 4 && month <= 9) z_season = true;              // true if 'Summer'

  if (hemisphere == 1) {                                      // North hemisphere
    if (!strcmp(windDirection, "N")) {
      currentPressure += 0.06 * pressureRange;
    } else if (!strcmp(windDirection, "NNE")) {
      currentPressure += 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "NE")) {
      currentPressure += 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "ENE")) {
      currentPressure += 0.02 * pressureRange;
    } else if (!strcmp(windDirection, "E")) {
      currentPressure -= 0.005 * pressureRange;
    } else if (!strcmp(windDirection, "ESE")) {
      currentPressure -= 0.02 * pressureRange;
    } else if (!strcmp(windDirection, "SE")) {
      currentPressure -= 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "SSE")) {
      currentPressure -= 0.085 * pressureRange;
    } else if (!strcmp(windDirection, "S")) {
      currentPressure -= 0.12 * pressureRange;
    } else if (!strcmp(windDirection, "SSW")) {
      currentPressure -= 0.1 * pressureRange;
    } else if (!strcmp(windDirection, "SW")) {
      currentPressure -= 0.06 * pressureRange;
    } else if (!strcmp(windDirection, "WSW")) {
      currentPressure -= 0.045 * pressureRange;
    } else if (!strcmp(windDirection, "W")) {
      currentPressure -= 0.03 * pressureRange;
    } else if (!strcmp(windDirection, "WNW")) {
      currentPressure -= 0.005 * pressureRange;
    } else if (!strcmp(windDirection, "NW")) {
      currentPressure += 0.015 * pressureRange;
    } else if (!strcmp(windDirection, "NNW")) {
      currentPressure += 0.03 * pressureRange;
    }

    if (z_season == 1) {    // if Summer
      if (pressureTrend == 1) {   // rising
        currentPressure += 0.07 * pressureRange;
      } else if (pressureTrend == 2) {  //  falling
        currentPressure -= 0.07 * pressureRange;
      }
    }
  }

  else {                                                      // must be South hemisphere
    if (!strcmp(windDirection, "S")) {
      currentPressure += 0.06 * pressureRange;
    } else if (!strcmp(windDirection, "SSW")) {
      currentPressure += 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "SW")) {
      currentPressure += 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "WSW")) {
      currentPressure += 0.02 * pressureRange;
    } else if (!strcmp(windDirection, "W")) {
      currentPressure -= 0.005 * pressureRange;
    } else if (!strcmp(windDirection, "WNW")) {
      currentPressure -= 0.02 * pressureRange;
    } else if (!strcmp(windDirection, "NW")) {
      currentPressure -= 0.05 * pressureRange;
    } else if (!strcmp(windDirection, "NNW")) {
      currentPressure -= 0.085 * pressureRange;
    } else if (!strcmp(windDirection, "N")) {
      currentPressure -= 0.12 * pressureRange;
    } else if (!strcmp(windDirection, "NNE")) {
      currentPressure -= 0.1 * pressureRange;
    } else if (!strcmp(windDirection, "NE")) {
      currentPressure -= 0.06 * pressureRange;
    } else if (!strcmp(windDirection, "ENE")) {
      currentPressure -= 0.045 * pressureRange;
    } else if (!strcmp(windDirection, "E")) {
      currentPressure -= 0.03 * pressureRange;
    } else if (!strcmp(windDirection, "ESE")) {
      currentPressure -= 0.005 * pressureRange;
    } else if (!strcmp(windDirection, "SE")) {
      currentPressure += 0.015 * pressureRange;
    } else if (!strcmp(windDirection, "SSE")) {
      currentPressure += 0.03 * pressureRange;
    }
    if (z_season == 0) {                                // if Winter
      if (pressureTrend == 1) {                         // rising
        currentPressure += 0.07 * pressureRange;
      } else if (pressureTrend == 2) {                  // falling
        currentPressure -= 0.07 * pressureRange;
      }
    }
  }   // END North / South

  if (currentPressure == highestPressureEverRecorded) currentPressure = highestPressureEverRecorded - 1;
  int forecastOption = floor((currentPressure - lowestPressureEverRecorded) / constant);
  int outputForecast;
  if (forecastOption < 0) {
    forecastOption = 0;
  }
  if (forecastOption > 21) {
    forecastOption = 21;
  }

  if (pressureTrend == 1) {                                                           // rising
    outputForecast = rise_options[forecastOption];
  }
  else if (pressureTrend == 2) {                                                      // falling
    outputForecast = fall_options[forecastOption];
  }
  else {                                                                              // must be 'steady'
    outputForecast = steady_options[forecastOption];
  }

  return outputForecast;
}
