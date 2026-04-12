# WeatherCalculations

An Arduino library that provides commonly used meteorological calculations and unit conversions in a single, easy-to-use `Weather` class. It is designed for DIY weather stations, environmental monitoring projects, and any sketch that needs to turn raw sensor readings into meaningful values such as sea-level pressure, dew point, heat index, comfort level, AQI, or even a textual short-term weather forecast.

It is platform-agnostic and works on any Arduino-compatible board with a C++ STL `<map>` implementation (AVR, ESP32, ESP8266, RP2040, etc.).

## Features

- **Temperature unit conversion** between Celsius and Fahrenheit
- **Sea-level pressure** from a measured station pressure and altitude (and the inverse: estimated altitude from a known sea-level pressure)
- **Dew point** from temperature and relative humidity
- **Heat index** ("feels like" temperature in hot conditions) from temperature and humidity
- **Humidex** (Canadian "feels like" index) from temperature and dew point
- **Wind chill** from temperature and wind speed
- **Human comfort level** classification based on heat index, ranging from *comfortable* through to *dangerous*
- **Air Quality Index (AQI)** from PM2.5 and PM10 particulate concentrations
- **Zambretti-style 12-hour weather forecast** based on current pressure, month, wind direction, pressure trend, and hemisphere — returned either as a descriptive string (e.g. *"Fine weather"*, *"Stormy, much rain"*) or as a numeric severity index for your own UI

## Installation

### Manual installation

1. Download the repository as a ZIP from GitHub.
2. In the Arduino IDE, go to **Sketch → Include Library → Add .ZIP Library...** and select the downloaded file.
3. The library will appear in **Sketch → Include Library → WeatherCalculations**.

Alternatively, clone or extract the folder into your Arduino `libraries/` directory and restart the IDE.

## Quick Start

```cpp
#include <WeatherCalculations.h>

Weather weather;

void setup() {
  Serial.begin(115200);

  double temperature = 28.5;   // °C
  double humidity    = 65.0;   // %
  double pressure    = 1005.2; // hPa, measured at the station
  double altitude    = 230.0;  // meters above sea level

  double seaLevel = weather.getSeaLevelPressure(pressure, altitude);
  double dewPoint = weather.getDewPoint(temperature, humidity);
  double heatIdx  = weather.getHeatIndex(temperature, humidity);

  Serial.print("Sea-level pressure: "); Serial.println(seaLevel);
  Serial.print("Dew point: ");          Serial.println(dewPoint);
  Serial.print("Heat index: ");         Serial.println(heatIdx);

  // 12-hour forecast for the northern hemisphere in June, no wind, steady pressure
  char* forecast = weather.getForecast(seaLevel, 6, Weather::NOW, Weather::steady);
  Serial.print("Forecast: "); Serial.println(forecast);
}

void loop() {}
```

## API Reference

All functions are members of the `Weather` class. Temperatures are in **°C**, pressures in **hPa**, humidity in **%**, wind speed in **km/h**, altitude in **meters**, and PM concentrations in **µg/m³**, unless otherwise noted.

### Unit conversions

| Function | Description |
|---|---|
| `double tempCtoF(double Temperature)` | Convert Celsius to Fahrenheit. |
| `double tempFtoC(double Temperature)` | Convert Fahrenheit to Celsius. |

### Pressure and altitude

| Function | Description |
|---|---|
| `double getSeaLevelPressure(double AirPressure, double Altitude)` | Reduce a measured station pressure to its equivalent at sea level using the given altitude. |
| `double getAltitude(double AirPressure, double SeaLevelPressure)` | Estimate altitude from a measured pressure and a known sea-level pressure. |

### Comfort and "feels like" indices

| Function | Description |
|---|---|
| `double getDewPoint(double Temperature, double Humidity)` | Dew point temperature. |
| `double getHeatIndex(double Temperature, double Humidity)` | Apparent temperature in hot, humid conditions. |
| `double getHumidex(double Temperature, double DewPoint)` | Canadian humidex index. |
| `double getWindChill(double Temperature, double WindSpeed)` | Apparent temperature in cold, windy conditions. |
| `Comfort getComfort(double heatIndex)` | Returns a `Weather::Comfort` enum value classifying human comfort. |

The `Comfort` enum has the following values: `uncomfortable`, `comfortable`, `small_discomfort`, `medium_discomfort`, `great_discomfort`, `dangerous`.

### Air quality

| Function | Description |
|---|---|
| `uint16_t getAQI(uint16_t PM25, uint16_t PM10)` | Computes a US-EPA-style AQI from PM2.5 and PM10 readings. |

### Forecasting

The library implements a Zambretti-style forecaster, which estimates the next ~12 hours of weather from current pressure, the season, the wind direction, and the pressure trend over the previous few hours.

```cpp
char* getForecast(
  double currentPressure,
  const uint8_t month,
  WindDirection windDirection,
  PressureTrend pressureTrend,
  const boolean hemisphere = true,             // true = northern, false = southern
  const double highestPressureEverRecorded = 1050,
  const double lowestPressureEverRecorded  = 950
);

uint8_t getForecastSeverity( /* same parameters */ );
```

- `getForecast(...)` returns a human-readable C string such as `"Fine weather"`, `"Showery, becoming less settled"`, or `"Stormy, much rain"`.
- `getForecastSeverity(...)` returns a numeric index into the internal forecast table (0 = best, 25 = worst), useful for driving icons, gauges, or color coding.
- `month` is **1–12**.
- `hemisphere` defaults to `true` (northern). Pass `false` for the southern hemisphere — the seasonal correction is flipped automatically.
- `highestPressureEverRecorded` / `lowestPressureEverRecorded` let you calibrate the algorithm to your local climate. The defaults (1050 / 950 hPa) are reasonable for most temperate locations.
- If you don't have a wind sensor, pass `Weather::NOW` (short for *NO Wind*) as the wind direction; the wind correction term will be zero.

#### `WindDirection` enum

`N`, `NNE`, `NE`, `ENE`, `E`, `ESE`, `SE`, `SSE`, `S`, `SSW`, `SW`, `WSW`, `W`, `WNW`, `NW`, `NNW`, and `NOW` (no wind / unknown). Each direction has a built-in correction factor that nudges the forecast slightly toward better or worse weather, since wind direction is a meaningful predictor in mid-latitudes.

#### `PressureTrend` enum

`rising`, `falling`, `steady`. You are responsible for tracking pressure history in your sketch and deciding which trend applies — typically by comparing the current reading to one taken about 3 hours ago.

## Example: full weather station

This library is used in the [ESP32-ThingSpeak-Weather-Station](https://github.com/MihajloPi/ESP32-ThingSpeak-Weather-Station) project, which combines a BME680, VEML6075, BH1750, and PMS7003 to produce a complete environmental dashboard with cloud logging. See that repository for a real-world integration example, including how to maintain a 24-hour pressure history buffer for trend detection.

## Notes and Limitations

- Forecast strings are returned as `char*` pointing to library-internal storage; copy them if you need to keep them across calls.
- The Zambretti algorithm is a *short-term, single-station* forecaster. It works surprisingly well for general "what's the weather doing today?" use, but it cannot replace a real numerical weather model.
- Calibrating `highestPressureEverRecorded` and `lowestPressureEverRecorded` to your local climate records noticeably improves forecast accuracy.
- The library uses `std::map`, so it requires a C++ STL implementation. This is fine on ESP32, ESP8266, RP2040, SAMD, etc., but may not compile on the smallest AVR targets without extra effort.

## License

Released under the **GNU General Public License v3.0**. See the [LICENSE](LICENSE) file for the full text.
