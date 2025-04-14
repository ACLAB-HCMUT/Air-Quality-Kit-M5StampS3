#include "TaskSendMail.h"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL ""
#define AUTHOR_PASSWORD ""

SMTPSession smtp;

void smtpCallback(SMTP_Status status);

void sendMail(String content)
{

    if (content.isEmpty())
    {
        return;
    }
    Serial.println(content);

    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, content);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    float temperature_value = doc["temperature"]["value"];
    int temperature_compare = doc["temperature"]["compare"];

    float humidity_value = doc["humidity"]["value"];
    int humidity_compare = doc["humidity"]["compare"];

    float noise_value = doc["noise"]["value"];
    int noise_compare = doc["noise"]["compare"];

    float pressure_value = doc["pressure"]["value"];
    int pressure_compare = doc["pressure"]["compare"];

    float light_value = doc["light"]["value"];
    int light_compare = doc["light"]["compare"];

    float pm2p5_value = doc["pm2.5"]["value"];
    int pm2p5_compare = doc["pm2.5"]["compare"];

    float pm10_value = doc["pm10"]["value"];
    int pm10_compare = doc["pm10"]["compare"];

    float vibration_value = doc["vibration"]["value"];
    int vibration_compare = doc["vibration"]["compare"];

    MailClient.networkReconnect(true);
    smtp.debug(1);
    smtp.callback(smtpCallback);
    Session_Config config;

    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;
    config.login.user_domain = "";
    config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    config.time.gmt_offset = 3;
    config.time.day_light_offset = 0;
    SMTP_Message message;
    message.sender.name = F("Alert !!!");
    message.sender.email = AUTHOR_EMAIL;
    message.subject = String("Warning: Exceeds threshold at ") + NAME_DEVICE;
    message.addRecipient(NAME_DEVICE, EMAIL);

    String htmlMsg = "<html><head><style>"
                     "body { font-family: Arial, sans-serif; background-color: #f4f4f9; padding: 20px; margin: 0; }"
                     "h1 { color: #333; text-align: center; font-size: 28px; font-weight: bold; margin-bottom: 20px; }"
                     "table { width: 80%; margin-left: auto; margin-right: auto; border-collapse: collapse; }"
                     "th, td { padding: 15px; text-align: left; font-size: 18px; border-bottom: 1px solid #ddd; }"
                     "th { background-color: #f2f2f2; font-weight: bold; }"
                     "tr:nth-child(even) { background-color: #f9f9f9; }"
                     ".highlight { background-color: yellow; font-weight: bold; }"
                     "</style></head><body>";

    htmlMsg += "<h1>Sensor Value Warning for " + NAME_DEVICE + "</h1>";
    htmlMsg += "<table>";

    htmlMsg += "<tr><th>Temperature</th><td";
    if (temperature_compare == 1)
    {
        htmlMsg += " class='highlight'";
    }
    htmlMsg += ">" + String(temperature_value) + "°C</td></tr>";

    htmlMsg += "<tr><th>Humidity</th><td";
    if (humidity_compare == 1)
    {
        htmlMsg += " class='highlight'";
    }
    htmlMsg += ">" + String(humidity_value) + "%</td></tr>";

    htmlMsg += "<tr><th>Noise</th><td";
    if (noise_compare == 1)
    {
        htmlMsg += " class='highlight'";
    }
    htmlMsg += ">" + String(noise_value) + " dB</td></tr>";

    htmlMsg += "<tr><th>Pressure</th><td";
    if (pressure_compare == 1)
    {
        htmlMsg += " class='highlight'";
    }
    htmlMsg += ">" + String(pressure_value) + " hPa</td></tr>";

    htmlMsg += "<tr><th>Light</th><td";
    if (light_compare == 1)
    {
        htmlMsg += " class='highlight'";
    }
    htmlMsg += ">" + String(light_value) + " lux</td></tr>";

    htmlMsg += "<tr><th>PM2.5</th><td";
    if (pm2p5_compare == 1)
    {
        htmlMsg += " class='highlight'";
    }
    htmlMsg += ">" + String(pm2p5_value) + " µg/m³</td></tr>";

    htmlMsg += "<tr><th>PM10</th><td";
    if (pm10_compare == 1)
    {
        htmlMsg += " class='highlight'";
    }
    htmlMsg += ">" + String(pm10_value) + " µg/m³</td></tr>";

    htmlMsg += "<tr><th>Vibration</th><td";
    if (vibration_compare == 1)
    {
        htmlMsg += " class='highlight'";
    }
    htmlMsg += ">" + String(vibration_value) + "</td></tr>";

    htmlMsg += "</table></body></html>";

    // Gán nội dung HTML vào email
    message.html.content = htmlMsg.c_str();
    message.text.charSet = "us-ascii";
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    /* Connect to the server */
    if (!smtp.connect(&config))
    {
        ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
        return;
    }

    if (!smtp.isLoggedIn())
    {
        Serial.println("\nNot yet logged in.");
    }
    else
    {
        if (smtp.isAuthenticated())
            Serial.println("\nSuccessfully logged in.");
        else
            Serial.println("\nConnected with no Auth.");
    }

    /* Start sending Email and close the session */
    if (!MailClient.sendMail(&smtp, &message))
        ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
    /* Print the current status */
    Serial.println(status.info());

    /* Print the sending result */
    if (status.success())
    {
        // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
        // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
        // In ESP8266 and ESP32, you can use Serial.printf directly.

        Serial.println("----------------");
        ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
        ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
        Serial.println("----------------\n");

        for (size_t i = 0; i < smtp.sendingResult.size(); i++)
        {
            /* Get the result item */
            SMTP_Result result = smtp.sendingResult.getItem(i);

            // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
            // your device time was synched with NTP server.
            // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
            // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

            ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
            ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
            ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
            ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
            ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
        }
        Serial.println("----------------\n");

        // You need to clear sending result as the memory usage will grow up.
        smtp.sendingResult.clear();
    }
}