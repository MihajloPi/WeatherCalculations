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

double Weather::getDewPoint(double Temperature, double Humidity) {  //Temperature is in Celsius so make sure to convert Fahrenheit to Celsius!
  // (1) Saturation Vapor Pressure = ESGG(T)
  double RATIO = 373.15 / (273.15 + Temperature);
  double RHS = -7.90298 * (RATIO - 1.0);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10.0, (11.344 * (1.0 - 1.0 / RATIO))) - 1.0);
  RHS += 8.1328e-3 * (pow(10.0, (-3.49149 * (RATIO - 1.0))) - 1.0);
  RHS += log10(1013.246);

  // factor -3 is to adjust units - Vapor Pressure SVP * humidity
  double VP = pow(10.0, RHS - 3.0) * Humidity;

  // (2) DEWPOINT = F(Vapor Pressure)
  double T = log(VP / 0.61078);  // temp var
  return (241.88 * T) / (17.558 - T);
};

double Weather::getHeatIndex(double Temperature, double Humidity) {
  Temperature = tempCtoF(Temperature);
  double Adjustment = 0;

  double HeatIndex = -42.379 + (2.04901523 * Temperature) + (10.14333127 * Humidity) + (-0.22475541 * Temperature * Humidity) + (-0.00683783 * Temperature * Temperature) + (-0.05481717 * Humidity * Humidity) + (0.00122874 * Temperature * Temperature * Humidity) + (0.00085282 * Temperature * Humidity * Humidity) + (-0.00000199 * Temperature * Temperature * Humidity * Humidity);

  if (Humidity < 13.0 && Temperature > 80.0 && Temperature < 112.0) {
    Adjustment = ((13.0 - Humidity) * 0.25) * sqrt((17.0 - fabs(Temperature - 95.0)) / 17.0);
    HeatIndex -= Adjustment;
  } else if (Humidity > 85.0 && Temperature > 80.0 && Temperature < 87.0) {
    Adjustment = ((Humidity - 85.0) * 0.1) * ((87.0 - Temperature) * 0.2);
    HeatIndex += Adjustment;
  } else if (HeatIndex < 80.0) {
    HeatIndex = 0.5 * (Temperature + 61.0 + ((Temperature - 68.0) * 1.2) + (Humidity * 0.094));
  }

  return tempFtoC(HeatIndex);
};

double Weather::getHumidex(double Temperature, double DewPoint) {
  return Temperature + (-0.5555 * ((6.11 * pow(EULER, 5417.753 * ((1.0 / 273.15) - (1.0 / (273.15 + DewPoint))))) - 10.0));
};

double Weather::getWindChill(double Temperature, double WindSpeed) {
  Temperature = tempCtoF(Temperature);
  WindSpeed *= 0.621371; // Converts wind speed from km/h to mph
  double WindChill = 0;

  if (Temperature < 50.0 && WindSpeed > 3.0) {
    WindChill = tempFtoC(35.74 + (0.6215 * Temperature) - (35.75 * pow(WindSpeed, 0.16)) + (0.4275 * Temperature * pow(WindSpeed, 0.16)));
  } else {
    WindChill = tempFtoC(Temperature);
  }
//  WindChill = (13.12 + 0.6215 * Temperature - 11.37 * pow(WindSpeed, 0.16) + 0.3965 * Temperature * pow(WindSpeed, 0.16));
  return WindChill;
};

uint8_t Weather::getComfort(double heatIndex) {
  if (heatIndex <= 23.0) return 1;       //Uncomfortable
  else if (heatIndex <= 26.0) return 2;  //Comfortable
  else if (heatIndex <= 29.0) return 3;  //Some discomfort
  else if (heatIndex <= 39.0) return 4;  //Hot feeling
  else if (heatIndex <= 45.0) return 5;  //Great discomfort; avoid exertion
  else return 6;                         //Dangerous; probable heat stroke
}

/**
 * Calculates the Air Quality Index (AQI) based on the PM2.5 and PM10 values.
 * @param PM25 The PM2.5 value.
 * @param PM10 The PM10 value.
 * @return The calculated AQI value.
 */
uint16_t Weather::getAQI(uint16_t PM25, uint16_t PM10) {
  uint16_t AQI_25 = 0, AQI_10 = 0;

  //Calculate AQI for PM2.5
  if (PM25 >= 0 && PM25 <= 12) AQI_25 = map(PM25, 0, 12, 0, 50);
  else if (PM25 >= 13 && PM25 <= 35) AQI_25 = map(PM25, 13, 35, 51, 100);
  else if (PM25 >= 36 && PM25 <= 55) AQI_25 = map(PM25, 36, 55, 101, 150);
  else if (PM25 >= 56 && PM25 <= 150) AQI_25 = map(PM25, 56, 150, 151, 200);
  else if (PM25 >= 151 && PM25 <= 250) AQI_25 = map(PM25, 151, 250, 201, 300);
  else if (PM25 >= 251 && PM25 <= 500) AQI_25 = map(PM25, 251, 500, 301, 500);

  //Calculate AQI for PM10
  if (PM10 >= 0 && PM10 <= 54) AQI_10 = map(PM10, 0, 54, 0, 50);
  else if (PM10 >= 55 && PM10 <= 154) AQI_10 = map(PM10, 55, 154, 51, 100);
  else if (PM10 >= 155 && PM10 <= 254) AQI_10 = map(PM10, 155, 254, 101, 150);
  else if (PM10 >= 255 && PM10 <= 354) AQI_10 = map(PM10, 255, 354, 151, 200);
  else if (PM10 >= 355 && PM10 <= 424) AQI_10 = map(PM10, 355, 424, 201, 300);
  else if (PM10 >= 425 && PM10 <= 604) AQI_10 = map(PM10, 425, 604, 301, 500);

  //Return the highest AQI value
  return max(AQI_25, AQI_10);
}

uint8_t Weather::getForecastSeverity(double currentPressure, const uint8_t month, WindDirection windDirection, const uint8_t pressureTrend, const boolean hemisphere, const double highestPressureEverRecorded, const double lowestPressureEverRecorded) {
  double pressureRange = highestPressureEverRecorded - lowestPressureEverRecorded;
  double constant = (pressureRange / 22.0);
  boolean summer = false;
  if (hemisphere == true && month >= 4 && month <= 9) summer = true;  // true if 'Summer'
  if (hemisphere == false && month < 4 && month > 9)  summer = true;  // true if 'Summer'

  if (hemisphere == false) {                     //South hemisphere
    windDirection = static_cast<WindDirection>((windDirection + 8) % 16);  // Adjust wind direction for Southern Hemisphere, basically creates a circle of enums and rotates it by 180 degrees and removes the period of 16 (since there are 16 enums, not considering calm (NOW) wind situation)
  }

  if (correctionFactorsNorthHemisphere.find(windDirection) != correctionFactorsNorthHemisphere.end()) {
    currentPressure += correctionFactorsNorthHemisphere[windDirection] * pressureRange;
  }

  if (summer == 1) {                             // if Summer
    if (pressureTrend == 1) {                    // rising
      currentPressure += 0.07 * pressureRange;
    } else if (pressureTrend == 2) {             //  falling
      currentPressure -= 0.07 * pressureRange;
    }
  }

  if (currentPressure >= highestPressureEverRecorded) currentPressure = highestPressureEverRecorded - 1;
  uint8_t forecastOption = floor((currentPressure - lowestPressureEverRecorded) / constant);
  forecastOption = constrain(forecastOption, 0, 21);

  uint8_t outputForecast;

  if (pressureTrend == 1) {                        // rising
    outputForecast = rise_options[forecastOption];
  } else if (pressureTrend == 2) {                 // falling
    outputForecast = fall_options[forecastOption];
  } else {                                         // must be 'steady'
    outputForecast = steady_options[forecastOption];
  }

  return outputForecast;
}

char* Weather::getForecast(double currentPressure, const uint8_t month, WindDirection windDirection, const uint8_t pressureTrend, const boolean hemisphere, const double highestPressureEverRecorded, const double lowestPressureEverRecorded) {

  uint8_t forecastOption = getForecastSeverity(currentPressure, month, windDirection, pressureTrend, hemisphere, highestPressureEverRecorded, lowestPressureEverRecorded);

  static char outputForecast[57];
  strcpy(outputForecast, "");                      // Initialise an empty char array

  if (forecastOption == 0 || forecastOption == 21) {
    strcpy(outputForecast, "Exceptional Weather, ");
  }

  if (pressureTrend == 1) {                        // rising
    strcat(outputForecast, forecast[rise_options[forecastOption]]);
  } else if (pressureTrend == 2) {                 // falling
    strcat(outputForecast, forecast[fall_options[forecastOption]]);
  } else {                                         // must be 'steady'
    strcat(outputForecast, forecast[steady_options[forecastOption]]);
  }

  return outputForecast;
}
