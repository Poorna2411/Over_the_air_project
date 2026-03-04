#include <WiFi.h>
#include <HTTPClient.h>

#define STM32_RX 16
#define STM32_TX 17

const char* ssid = "poorna";
const char* password = "poorna2411";

String firmwareURL = "http://10.39.169.237:8000/firmware.bin";

HardwareSerial stm32(2);

/* ---------- Robust UART parser ---------- */
bool waitFor(const char* target, uint32_t timeout = 30000)
{
    int matchIndex = 0;
    unsigned long start = millis();

    while (millis() - start < timeout)
    {
        while (stm32.available())
        {
            char c = stm32.read();
            Serial.write(c);   // debug visibility

            if (c == target[matchIndex])
            {
                matchIndex++;
                if (target[matchIndex] == '\0')
                    return true;
            }
            else
            {
                matchIndex = 0;
            }
        }
    }
    return false;
}

/* ---------- Flush UART ---------- */
void flushUART()
{
    while (stm32.available())
        stm32.read();
}

void setup()
{
    Serial.begin(115200);
    stm32.begin(115200, SERIAL_8N1, STM32_RX, STM32_TX);

    Serial.println("\n=== ESP32 OTA Bridge Starting ===");

    /* ===== WiFi Connect ===== */
    WiFi.begin(ssid, password);
    Serial.print("Connecting WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected");
    Serial.println(WiFi.localIP());

    /* ===== Download firmware ===== */
    HTTPClient http;
    http.begin(firmwareURL);

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("HTTP Error: %d\n", httpCode);
        return;
    }

    int totalSize = http.getSize();
    WiFiClient *stream = http.getStreamPtr();

    Serial.printf("Firmware Size: %d bytes\n", totalSize);

    /* ===== Trigger STM32 ===== */
    stm32.write('U');
    flushUART();

    Serial.println("Waiting STM32 READY...");

    if (!waitFor("READY"))
    {
        Serial.println("\nSTM32 not ready!");
        return;
    }

    Serial.println("\nSTM32 READY");

    /* ----- Critical stabilization ----- */
    delay(500);
    flushUART();

    /* ===== Send size slowly ===== */
    uint8_t sizeBytes[4] = {
        (uint8_t)(totalSize >> 24),
        (uint8_t)(totalSize >> 16),
        (uint8_t)(totalSize >> 8),
        (uint8_t)(totalSize)
    };

    Serial.printf("Sending size bytes: %02X %02X %02X %02X\n",
                  sizeBytes[0], sizeBytes[1],
                  sizeBytes[2], sizeBytes[3]);

    for (int i = 0; i < 4; i++)
    {
        stm32.write(sizeBytes[i]);
        delay(20);   // IMPORTANT pacing fix
    }

    if (!waitFor("OK"))
    {
        Serial.println("\nSize rejected!");
        return;
    }

    Serial.println("Size acknowledged");

    /* ===== Send firmware ===== */
    uint8_t buffer[256];
    int sent = 0;

    Serial.println("Sending firmware...");

    while (sent < totalSize)
    {
        int chunk = stream->readBytes(buffer, 256);
        if (chunk <= 0) break;

        stm32.write(buffer, chunk);

        if (!waitFor("ACK"))
        {
            Serial.println("\nACK timeout!");
            return;
        }

        sent += chunk;
        Serial.printf("Progress: %d/%d\r", sent, totalSize);
    }

    if (waitFor("DONE"))
        Serial.println("\nOTA SUCCESS!");
    else
        Serial.println("\nOTA FAILED!");

    http.end();
}

void loop()
{
}