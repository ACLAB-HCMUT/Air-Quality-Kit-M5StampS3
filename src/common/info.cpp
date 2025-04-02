#include "globals.h"

AsyncWebServer server_1(80);

String NAME_DEVICE;
String WIFI_SSID;
String WIFI_PASS;
String TOKEN;
String HTTP_SELECT;
String webhook;
String auth_token;
String device_id;
// String EMAIL;

void loadInfoFromFile()
{
    File file = LittleFS.open("/info.dat", "r");
    if (!file)
    {
        Serial.println("Không thể mở file để đọc");
        return;
    }
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
    }
    else
    {
        NAME_DEVICE = strdup(doc["NAME_DEVICE"]);
        WIFI_SSID = strdup(doc["WIFI_SSID"]);
        WIFI_PASS = strdup(doc["WIFI_PASS"]);
        TOKEN = strdup(doc["TOKEN"]);
        HTTP_SELECT = strdup(doc["HTTP_SELECT"]);
        if (HTTP_SELECT == "yes")
        {
            webhook = strdup(doc["webhook"]);
            auth_token = strdup(doc["auth_token"]);
            device_id = strdup(doc["device_id"]);
        }
        // EMAIL = strdup(doc["EMAIL"]);
    }
    file.close();
}

void deleteInfoFile()
{
    if (LittleFS.exists("/info.dat"))
    {
        LittleFS.remove("/info.dat");
    }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AIR QUALITY Configuration</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f2f2f2;
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }

        .container {
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            width: 300px;
        }

        h2 {
            text-align: center;
            color: #333;
        }

        label {
            margin-top: 10px;
            display: block;
            color: #666;
        }

        input[type="text"],
        input[type="password"],
        input[type="email"],
        select {
            width: 100%;
            padding: 10px;
            margin-top: 5px;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }

        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 10px;
            border-radius: 4px;
            cursor: pointer;
            width: 100%;
            margin-top: 15px;
        }

        input[type="submit"]:hover {
            background-color: #45a049;
        }

        .ota-title {
            margin-top: 20px;
            font-size: 18px;
            text-align: center;
            color: #333;
        }

        .additional-inputs {
            display: none;
            margin-top: 10px;
        }
    </style>
    <script>
        function toggleHTTPFields() {
            var httpSelect = document.getElementById("http_select");
            var additionalInputs = document.getElementById("additional_inputs");
            if (httpSelect.value === "yes") {
                additionalInputs.style.display = "block";
            } else {
                additionalInputs.style.display = "none";
            }
        }
    </script>
</head>

<body>
    <div class="container">
        <h2>ESP32 Configuration</h2>
        <form action="/save" method="post">
            <label for="name_device">Name Device :</label>
            <input type="text" name="name_device" id="name_device" required>

            <label for="ssid">WiFi SSID:</label>
            <input type="text" name="ssid" id="ssid" required>

            <label for="pass">WiFi Password:</label>
            <input type="password" name="pass" id="pass">

            <label for="token">MQTT Token:</label>
            <input type="text" name="token" id="token" required>

            <label for="http_select">HTTP :</label>
            <select name="http_select" id="http_select" onchange="toggleHTTPFields()" required>
                <option value="default">Default</option>
                <option value="yes">Yes</option>
            </select>


            <div id="additional_inputs" class="additional-inputs">
                <label for="webhook">Webhook Endpoint:</label>
                <input type="text" name="webhook" id="webhook" >

                <label for="auth_token">Authorization Token:</label>
                <input type="text" name="auth_token" id="auth_token" >

                <label for="device_id">ID Device:</label>
                <input type="text" name="device_id" id="device_id" >
            </div>

            <input type="submit" value="Save">
        </form>
    </div>
</body>

</html>
)rawliteral";

void startAccessPoint()
{
#ifdef IS_ROOT
    LED_ACP();
#endif
    unsigned long previousMillis = 0;
    const long interval = 1000;
    int lastStationCount = -1;
    int textWidth = 0;
    int xPosition = 0;
    WiFi.softAP(SSID_AP);
    Serial.println("Access Point Started");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    server_1.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", index_html); });

    server_1.on("/save", HTTP_POST, [](AsyncWebServerRequest *request)
                {
NAME_DEVICE = request->getParam("name_device", true)->value();
WIFI_SSID = request->getParam("ssid", true)->value();
WIFI_PASS = request->getParam("pass", true)->value();
TOKEN = request->getParam("token", true)->value();
HTTP_SELECT = request->getParam("http_select", true)->value();
if (HTTP_SELECT == "yes")
{
    webhook = request->getParam("webhook", true)->value();
    auth_token = request->getParam("auth_token", true)->value();
    device_id = request->getParam("device_id", true)->value();
}

// EMAIL = request->getParam("email", true)->value();

DynamicJsonDocument doc(2048);
doc["NAME_DEVICE"] = NAME_DEVICE;
doc["WIFI_SSID"] = WIFI_SSID;
doc["WIFI_PASS"] = WIFI_PASS;
doc["TOKEN"] = TOKEN;
doc["HTTP_SELECT"]=HTTP_SELECT;
if(HTTP_SELECT == "yes")
{
    doc["webhook"] = webhook;
    doc["auth_token"] = auth_token;
    doc["device_id"] = device_id;
}

File configFile = LittleFS.open("/info.dat", "w");
if (configFile) {
serializeJson(doc, configFile);
configFile.close();
request->send(200, "text/html", "Configuration has been saved. ESP32 will restart...");
delay(1000);
ESP.restart();
} else {
request->send(500, "text/html", "Unable to save the configuration.");
} });

    server_1.begin();

#ifdef M5_CORE2
    while (true)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            int currentStationCount = WiFi.softAPgetStationNum();
            if (currentStationCount != lastStationCount)
            {
                lastStationCount = currentStationCount;

                lcd.clear(TFT_BLACK);
                lcd.waitDisplay();
                lcd.clear(TFT_WHITE);
                lcd.waitDisplay();

                if (currentStationCount == 0)
                {
                    String apQrcode = "WIFI:T:nopass;S:" + String(SSID_AP) + ";P:;H:false;;";
                    lcd.drawString("Scan me", (lcd.width() - lcd.textWidth("Scan me", &fonts::FreeSansBold9pt7b)) / 2, 15, &fonts::FreeSansBold9pt7b);
                    lcd.qrcode(apQrcode, 35, 35, 130);
                    textWidth = lcd.textWidth(String(SSID_AP), &fonts::FreeSansBold9pt7b);
                    xPosition = (lcd.width() - textWidth) / 2;
                    lcd.drawString(String(SSID_AP), xPosition, 175, &fonts::FreeSansBold9pt7b);
                }
                else
                {
                    String ipAddress = "http://" + WiFi.softAPIP().toString();
                    lcd.drawString("Scan me", (lcd.width() - lcd.textWidth("Scan me", &fonts::FreeSansBold9pt7b)) / 2, 15, &fonts::FreeSansBold9pt7b);
                    lcd.qrcode(ipAddress, 35, 35, 130);
                    textWidth = lcd.textWidth(String(ipAddress), &fonts::FreeSansBold9pt7b);
                    xPosition = (lcd.width() - textWidth) / 2;
                    lcd.drawString(String(ipAddress), xPosition, 175, &fonts::FreeSansBold9pt7b);
                }
                lcd.waitDisplay();
            }
        }
    }
#endif
}

void TaskResetDevice(void *pvParameters)
{
    unsigned long buttonPressStartTime = 0;
    while (true)
    {
        if (digitalRead(BOOT) == LOW)
        {
            if (buttonPressStartTime == 0)
            {
                buttonPressStartTime = millis();
            }
            else if (millis() - buttonPressStartTime > 5000)
            {
                deleteInfoFile();
                ESP.restart();
                vTaskDelete(NULL);
            }
        }
#ifdef M5_CORE2
        else if (M5.BtnB.wasPressed())
        {
            if (buttonPressStartTime == 0)
            {
                buttonPressStartTime = millis();
            }
            else if (millis() - buttonPressStartTime > 5000)
            {
                deleteInfoFile();
                ESP.restart();
                vTaskDelete(NULL);
            }
        }
#endif
        else
        {
            buttonPressStartTime = 0;
        }
        vTaskDelay(delay_connect / portTICK_PERIOD_MS);
    }
}

void reset_device()
{
    xTaskCreate(TaskResetDevice, "TaskResetDevice", 4096, NULL, 1, NULL);
}

bool check_info(bool check)
{

#ifdef IS_LEAF
    pinMode(BOOT, INPUT);
#endif
    if (!check)
    {
        loadInfoFromFile();
        reset_device();
    }
    if (NAME_DEVICE.isEmpty() || WIFI_SSID.isEmpty() || TOKEN.isEmpty())
    {
        if (!check)
        {
            startAccessPoint();
        }

        // if (HTTP_SELECT == "yes" && (webhook.isEmpty() || auth_token.isEmpty() || device_id.isEmpty()))
        // {
        //     startAccessPoint();
        // }
        return false;
    }
    return true;
}
