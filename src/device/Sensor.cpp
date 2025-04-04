#include "Sensor.h"

Sensor sensor;

void Sensor::setSCD40MeasurementResult(uint16_t co2Value, float temperatureValue, float humidityValue)
{
    scd40.co2 = co2Value;
    scd40.temperature = temperatureValue;
    scd40.humidity = humidityValue;
}

void Sensor::setSEN55MeasurementResult(
    float pm1p0,
    float pm2p5,
    float pm4p0,
    float pm10p0,
    float temperature,
    float humidity,
    float voc,
    float nox)
{
    sen55.massConcentrationPm1p0 = pm1p0;
    sen55.massConcentrationPm2p5 = pm2p5;
    sen55.massConcentrationPm4p0 = pm4p0;
    sen55.massConcentrationPm10p0 = pm10p0;
    sen55.Temperature = temperature;
    sen55.Humidity = humidity;
    sen55.vocIndex = voc;
    sen55.noxIndex = nox;
}

void Sensor::setSENSOR_5_MeasurementResult(
    float Temperature,
    float Humidity,
    float sound,
    float pressure,
    float light,
    float pm2p5,
    float pm10)
{
    sensor5.Temperature = Temperature;
    sensor5.Humidity = Humidity;
    sensor5.sound = sound;
    sensor5.pressure = pressure;
    sensor5.light = light;
    sensor5.pm2p5 = pm2p5;
    sensor5.pm10 = pm10;
}

void Sensor::setMOREMeasurementResult(float sound, float vibration, float battery)
{
    more_sensor.sound = sound;
    more_sensor.vibration = vibration;
    more_sensor.battery = battery;
}

void Sensor::sensor_init()
{
    /// MORE
    more_sensor.sound = 0;
    more_sensor.vibration = 0;
    more_sensor.battery = 0;
    /// SCD40
    scd40.co2 = 0;
    scd40.temperature = 0.0f;
    scd40.humidity = 0.0f;
    /// SEN55
    sen55.massConcentrationPm1p0 = 0.0f;
    sen55.massConcentrationPm2p5 = 0.0f;
    sen55.massConcentrationPm4p0 = 0.0f;
    sen55.massConcentrationPm10p0 = 0.0f;
    sen55.Humidity = 0.0f;
    sen55.Temperature = 0.0f;
    sen55.vocIndex = 0.0f;
    sen55.noxIndex = 0.0f;
    /// SENSOR 5
    sensor5.Temperature = 0.0;
    sensor5.Humidity = 0.0;
    sensor5.sound = 0.0;
    sensor5.pressure = 0.0;
    sensor5.light = 0.0;
    sensor5.pm2p5 = 0.0;
    sensor5.pm10 = 0.0;

    /// TIME
    memset(timeData.time, 0, sizeof(timeData.time));
    memset(timeData.date, 0, sizeof(timeData.date));
    memset(_errorMessage, 0, sizeof(_errorMessage));
    log_i("Sensor initialized.");
}

void Sensor::setTime(const char *timeStr, const char *dateStr)
{
    strncpy(timeData.time, timeStr, sizeof(timeData.time) - 1);
    timeData.time[sizeof(timeData.time) - 1] = '\0';

    strncpy(timeData.date, dateStr, sizeof(timeData.date) - 1);
    timeData.date[sizeof(timeData.date) - 1] = '\0';
}

void updateView()
{
#ifdef M5_CORE2
    statusView.updateSCD40(
        sensor.scd40.co2,
        sensor.scd40.temperature,
        sensor.scd40.humidity);
    statusView.updateSEN55(
        sensor.sen55.massConcentrationPm1p0,
        sensor.sen55.massConcentrationPm2p5,
        sensor.sen55.massConcentrationPm4p0,
        sensor.sen55.massConcentrationPm10p0,
        sensor.sen55.Temperature,
        sensor.sen55.Humidity,
        sensor.sen55.vocIndex,
        sensor.sen55.noxIndex);
    if (WiFi.status() == WL_CONNECTED)
    {
        if (tb.connected())
        {
            statusView.updateNetworkStatus("WIFI", WiFi.localIP().toString().c_str());
        }
        else
        {
            statusView.updateNetworkStatus("ERROR", "Connect MQTT");
        }
    }
    else
    {
        statusView.updateNetworkStatus("ERROR", "No Internet");
    }
    statusView.updateTime(sensor.timeData.time, sensor.timeData.date);
    statusView.updateMORE(sensor.more_sensor.sound, sensor.more_sensor.vibration, sensor.more_sensor.battery);
    statusView.load();
#endif
    if (HTTP_SELECT == "yes")
    {
        StaticJsonDocument<4096> doc;
#ifdef IS_ROOT
        doc["temperature"] = sensor.sensor5.Temperature;
        doc["humidity"] = sensor.sensor5.Humidity;
        doc["noise"] = sensor.sensor5.sound = 0.0;
        doc["pressure"] = sensor.sensor5.pressure;
        doc["light"] = sensor.sensor5.light;
        doc["pm2.5"] = sensor.sensor5.pm2p5;
        doc["pm10"] = sensor.sensor5.pm10;
        doc["vibration"] = sensor.more_sensor.vibration;
#else
        doc["temperature"] = sensor.sen55.Temperature;
        doc["humidity"] = sensor.sen55.Humidity;
        doc["co2"] = sensor.scd40.co2;
        doc["noise"] = sensor.more_sensor.sound;
        doc["vibration"] = sensor.more_sensor.vibration;
        doc["battery"] = sensor.more_sensor.battery;
        doc["voc"] = sensor.sen55.vocIndex;
        doc["voc"] = sensor.sen55.noxIndex;
        doc["pm1"] = sensor.sen55.massConcentrationPm1p0;
        doc["pm2.5"] = sensor.sen55.massConcentrationPm2p5;
        doc["pm4"] = sensor.sen55.massConcentrationPm4p0;
        doc["pm10"] = sensor.sen55.massConcentrationPm10p0;
#endif
        doc["id"] = device_id;
        String output;
        serializeJson(doc, output);
        sendHTTP(output);
    }
}

void TaskSENSOR(void *pvParameters)
{
    sensor.sensor_init();
    while (true)
    {
        updateView();
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
    }
}

void SENSOR_init()
{
#ifdef M5_CORE2
    SEN55_init();
    SCD40_init();
#endif

#ifdef IS_ROOT
    tasksensor_init();
    LED_init();
#endif

    More_init();
    xTaskCreate(TaskSENSOR, "TaskSENSOR", 8192, NULL, 1, NULL);
}