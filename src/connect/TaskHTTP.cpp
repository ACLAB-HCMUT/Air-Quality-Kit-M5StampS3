#include "TaskHTTP.h"

void sendHTTP(String json_send)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(webhook);
        http.addHeader("Authorization", auth_token);

        int httpCode = http.POST(json_send);

        if (httpCode > 0)
        {
            String payload = http.getString();
            Serial.println("POST Response code: " + String(httpCode));
            Serial.println("Response: " + payload);
        }
        else
        {
            Serial.println("Error on HTTP POST");
        }
        http.end();
    }
}
