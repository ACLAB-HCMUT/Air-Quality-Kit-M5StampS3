#ifdef IS_ROOT
#include "TaskLed.h"

Adafruit_NeoPixel led_rgb(NUM_PIXELS, MY_LED, NEO_GRB + NEO_KHZ800);

TaskHandle_t TaskLedHandle = NULL;
TaskHandle_t TaskLedACPHandle = NULL;

void TaskLed(void *pvParameters)
{
    led_rgb.begin();
    led_rgb.show();
    bool ledState = false;

    while (true)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            if (tb.connected())
            {
                if (ledState)
                {
                    led_rgb.setPixelColor(0, led_rgb.Color(0, 255, 0));
                }
                else
                {
                    led_rgb.setPixelColor(0, led_rgb.Color(0, 0, 0));
                }
            }
            else if (!tb.connected())
            {
                if (ledState)
                {
                    led_rgb.setPixelColor(0, led_rgb.Color(255, 165, 0));
                }
                else
                {
                    led_rgb.setPixelColor(0, led_rgb.Color(0, 0, 0));
                }
            }

            led_rgb.setBrightness(Brightness);
            led_rgb.show();
            ledState = !ledState;
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            if (ledState)
            {
                led_rgb.setPixelColor(0, led_rgb.Color(255, 0, 0));
            }
            else
            {
                led_rgb.setPixelColor(0, led_rgb.Color(0, 0, 0));
            }
            led_rgb.show();
            ledState = !ledState;
        }
        vTaskDelay(delay_led / portTICK_PERIOD_MS);
    }
}

void TaskLed_ACP(void *pvParameters)
{
    led_rgb.begin();
    led_rgb.show();
    bool ledState = false;

    while (true)
    {
        if (ledState)
        {
            led_rgb.setPixelColor(0, led_rgb.Color(255, 255, 255));
        }
        else
        {
            led_rgb.setPixelColor(0, led_rgb.Color(0, 0, 0));
        }

        led_rgb.setBrightness(Brightness);
        led_rgb.show();
        ledState = !ledState;
        vTaskDelay(delay_connect / portTICK_PERIOD_MS);
    }
}

void LED_init()
{
    if (TaskLedACPHandle != NULL)
    {
        vTaskDelete(TaskLedACPHandle);
        TaskLedACPHandle = NULL;
    }
    xTaskCreate(TaskLed, "TaskLed", 4096, NULL, 2, &TaskLedHandle);
}

void LED_ACP()
{
    if (TaskLedHandle != NULL)
    {
        vTaskDelete(TaskLedHandle);
        TaskLedHandle = NULL;
    }
    xTaskCreate(TaskLed_ACP, "TaskLed_ACP", 4096, NULL, 1, &TaskLedACPHandle);
}
#endif