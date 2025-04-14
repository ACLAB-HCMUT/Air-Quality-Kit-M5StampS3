#include "Sensor.h"

Sensor sensor;

bool check_value_warning(String type, float value)
{
    String comparison_operator;
    float compare_value = 0;
    String compare_string;

    if (type.equals("temp"))
        compare_string = compare_temp;
    else if (type.equals("humi"))
        compare_string = compare_humi;
    else if (type.equals("sound"))
        compare_string = compare_sound;
    else if (type.equals("pressure"))
        compare_string = compare_pressure;
    else if (type.equals("light"))
        compare_string = compare_light;
    else if (type.equals("pm2p5"))
        compare_string = compare_pm2p5;
    else if (type.equals("pm10"))
        compare_string = compare_pm10;
    else
        return false;

    if (!compare_string.isEmpty())
    {
        if (compare_string.startsWith("<=") || compare_string.startsWith(">="))
        {
            comparison_operator = compare_string.substring(0, 2);
            compare_value = compare_string.substring(2).toFloat();
        }
        else
        {
            comparison_operator = compare_string.substring(0, 1);
            compare_value = compare_string.substring(1).toFloat();
        }
    }
    else
    {
        return false;
    }

    if (comparison_operator.equals("<"))
    {
        return value < compare_value;
    }
    else if (comparison_operator.equals(">"))
    {
        return value > compare_value;
    }
    else if (comparison_operator.equals("="))
    {
        return value == compare_value;
    }
    else if (comparison_operator.equals("<="))
    {
        return value <= compare_value;
    }
    else if (comparison_operator.equals(">="))
    {
        return value >= compare_value;
    }

    return false;
}

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
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }

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

    if (WARNING_VALUE == "yes")
    {
        StaticJsonDocument<4096> doc;
        bool check_send_mail = false;
#ifdef IS_ROOT
        // Kiểm tra và gán cho các cảm biến
        if (check_value_warning("temp", sensor.sensor5.Temperature))
        {
            doc["temperature"]["value"] = sensor.sensor5.Temperature;
            doc["temperature"]["compare"] = 1;
            check_send_mail = true;
        }
        else
        {
            doc["temperature"]["value"] = sensor.sensor5.Temperature;
            doc["temperature"]["compare"] = 0;
        }

        if (check_value_warning("humi", sensor.sensor5.Humidity))
        {
            doc["humidity"]["value"] = sensor.sensor5.Humidity;
            doc["humidity"]["compare"] = 1;
            check_send_mail = true;
        }
        else
        {
            doc["humidity"]["value"] = sensor.sensor5.Humidity;
            doc["humidity"]["compare"] = 0;
        }

        if (check_value_warning("sound", sensor.sensor5.sound))
        {
            doc["noise"]["value"] = sensor.sensor5.sound;
            doc["noise"]["compare"] = 1;
            check_send_mail = true;
        }
        else
        {
            doc["noise"]["value"] = sensor.sensor5.sound;
            doc["noise"]["compare"] = 0;
        }

        if (check_value_warning("pressure", sensor.sensor5.pressure))
        {
            doc["pressure"]["value"] = sensor.sensor5.pressure;
            doc["pressure"]["compare"] = 1;
            check_send_mail = true;
        }
        else
        {
            doc["pressure"]["value"] = sensor.sensor5.pressure;
            doc["pressure"]["compare"] = 0;
        }

        if (check_value_warning("light", sensor.sensor5.light))
        {
            doc["light"]["value"] = sensor.sensor5.light;
            doc["light"]["compare"] = 1;
            check_send_mail = true;
        }
        else
        {
            doc["light"]["value"] = sensor.sensor5.light;
            doc["light"]["compare"] = 0;
        }

        if (check_value_warning("pm2p5", sensor.sensor5.pm2p5))
        {
            doc["pm2.5"]["value"] = sensor.sensor5.pm2p5;
            doc["pm2.5"]["compare"] = 1;
            check_send_mail = true;
        }
        else
        {
            doc["pm2.5"]["value"] = sensor.sensor5.pm2p5;
            doc["pm2.5"]["compare"] = 0;
        }

        if (check_value_warning("pm10", sensor.sensor5.pm10))
        {
            doc["pm10"]["value"] = sensor.sensor5.pm10;
            doc["pm10"]["compare"] = 1;
            check_send_mail = true;
        }
        else
        {
            doc["pm10"]["value"] = sensor.sensor5.pm10;
            doc["pm10"]["compare"] = 0;
        }

        if (check_value_warning("vibration", sensor.more_sensor.vibration))
        {
            doc["vibration"]["value"] = sensor.more_sensor.vibration;
            doc["vibration"]["compare"] = 1;
            check_send_mail = true;
        }
        else
        {
            doc["vibration"]["value"] = sensor.more_sensor.vibration;
            doc["vibration"]["compare"] = 0;
        }
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
        String output;
        serializeJson(doc, output);
        if (check_send_mail)
            sendMail(output);
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
    xTaskCreate(TaskSENSOR, "TaskSENSOR", 12288, NULL, 1, NULL);
}