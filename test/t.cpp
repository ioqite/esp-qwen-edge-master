#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#if __has_include("user_config.h")
#include "user_config.h"
#else
#include "user_config-template.h"
#endif

// --- 配置部分 ---
const char* ssid = ssids[1];
const char* password = passwords[1];
const char* apiEndpoint = "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation";

// --- 对话管理 ---
const int MAX_MESSAGES = 20; // 最大保存的对话轮数
// 使用一个数组来存储每一条JSON格式的消息
String chatHistory[MAX_MESSAGES * 2 + 1]; 
int messageCount = 0; // 当前历史消息数量

// 系统提示词
const char* systemPrompt = "你是一个乐于助人且语言简练的AI助手。请始终用中文简短地回答。";

/**
 * @brief 向对话历史中添加消息
 * @param role 消息角色 ("system", "user", "assistant")
 * @param content 消息内容
 */
void addMessageToHistory(const char* role, const String& content) {
    // 如果历史记录已满，则移除最早的一条用户和助手消息
    if (messageCount >= (MAX_MESSAGES * 2 + 1)) {
        for (int i = 1; i < messageCount - 2; i++) {
        chatHistory[i] = chatHistory[i + 2];
        }
        messageCount -= 2;
    }
    // 将消息以JSON字符串的形式存入数组
    chatHistory[messageCount++] = String("{\"role\":\"") + role + "\",\"content\":\"" + content + "\"}";
}

/**
 * @brief 构建并发送HTTP请求到AI API
 * @param userPrompt 用户的当前问题
 * @return AI的回复内容字符串
 */
String sendChatRequest(const String& userPrompt) {
    if (WiFi.status() != WL_CONNECTED) {
    return "Error: 未连接到Wi-Fi";
    }

    WiFiClientSecure client;
    client.setInsecure(); // 跳过SSL证书验证

    HTTPClient http;
    http.begin(client, apiEndpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " apiKey);

	// --- 核心：使用 ArduinoJson 构建请求体 ---
	DynamicJsonDocument doc(4096);

	// 【修改点 3】将模型名称更改为 "qwen-turbo"
	doc["model"] = "qwen-turbo"; 

	// 创建 "input" 对象，这是通义千问API要求的格式
	JsonObject input = doc.createNestedObject("input");
	JsonArray messages = input.createNestedArray("messages");

    // 如果是第一次对话，先加入系统提示词
    if (messageCount == 0) {
        addMessageToHistory("system", systemPrompt);
    }
    
    // 加入用户当前的问题
    addMessageToHistory("user", userPrompt);

    // 将历史记录中的每一条消息解析并添加到JSON数组中
    for (int i = 0; i < messageCount; i++) {
        DynamicJsonDocument msgDoc(512);
        DeserializationError error = deserializeJson(msgDoc, chatHistory[i]);
        if (!error) {
        messages.add(msgDoc.as<JsonObject>());
        }
    }

    // 将构建好的JSON文档序列化为字符串
    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // 发送POST请求
    int httpResponseCode = http.POST(jsonPayload);
    String response = "";

    if (httpResponseCode > 0) {
        response = http.getString();
        // 解析AI的响应
        DynamicJsonDocument responseDoc(2048);
        DeserializationError error = deserializeJson(responseDoc, response);

        if (!error) {
                // 【修改点 4】通义千问的回复内容在 output.text 中
                const char* aiContent = responseDoc["output"]["text"];
                response = String(aiContent);
                // 将AI的回复也加入历史记录
                addMessageToHistory("assistant", response);
        } else {
                response = "JSON解析失败";
        }
    } else {
        response = "HTTP请求失败，错误码: " + String(httpResponseCode);
    }

    http.end();
    return response;
}

void setup() {
    Serial.begin(115200);
    
    // 连接Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("正在连接Wi-Fi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(700 / portTICK_PERIOD_MS);
    }
    Serial.println(" OK");
    Serial.println("请输入您的问题：");
}

void loop() {
    // 检查串口是否有用户输入
    if (Serial.available() > 0) {
        String userQuestion = Serial.readStringUntil('\n');
        userQuestion.trim();

        if (userQuestion.length() > 0) {
        Serial.print("你: ");
        Serial.println(userQuestion);
        Serial.print("AI: ");
        
        String aiAnswer = sendChatRequest(userQuestion);
        
        Serial.println(aiAnswer);
        Serial.println("请输入下一个问题：");
        }
    }
}
















// #include <Arduino.h>
// #include <string>
// #include <Arduino_GFX_Library.h>

// uint16_t colr[] = {
//     BLACK,
//     NAVY,
//     DARKGREEN,
//     DARKCYAN,
//     MAROON,
//     PURPLE,
//     OLIVE,
//     LIGHTGREY,
//     DARKGREY,
    
//     BLUE,
//     GREEN,
//     CYAN,
//     RED,
//     MAGENTA,
//     YELLOW,
//     WHITE,
//     ORANGE,
//     GREENYELLOW,
//     PALERED
// };

// #define EXAMPLE_PIN_NUM_LCD_SCLK 39
// #define EXAMPLE_PIN_NUM_LCD_MOSI 38
// #define EXAMPLE_PIN_NUM_LCD_MISO 40
// #define EXAMPLE_PIN_NUM_LCD_DC 42
// #define EXAMPLE_PIN_NUM_LCD_RST -1
// #define EXAMPLE_PIN_NUM_LCD_CS 45
// #define EXAMPLE_PIN_NUM_LCD_BL 1

// #define EXAMPLE_LCD_ROTATION 3
// #define EXAMPLE_LCD_H_RES 240
// #define EXAMPLE_LCD_V_RES 320

// /* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
// Arduino_DataBus *bus = new Arduino_ESP32SPI(
//   EXAMPLE_PIN_NUM_LCD_DC /* DC */, EXAMPLE_PIN_NUM_LCD_CS /* CS */,
//   EXAMPLE_PIN_NUM_LCD_SCLK /* SCK */, EXAMPLE_PIN_NUM_LCD_MOSI /* MOSI */, EXAMPLE_PIN_NUM_LCD_MISO /* MISO */);

// /* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
// Arduino_GFX *gfx = new Arduino_ST7789(
//   bus, EXAMPLE_PIN_NUM_LCD_RST /* RST */, EXAMPLE_LCD_ROTATION /* rotation */, true /* IPS */,
//   EXAMPLE_LCD_H_RES /* width */, EXAMPLE_LCD_V_RES /* height */);

// void setup(){
//     Serial.begin(115200);

//     // Init Display
// #ifdef GFX_EXTRA_PRE_INIT
//     GFX_EXTRA_PRE_INIT();
// #endif
//     if (!gfx->begin()) Serial.println("gfx->begin() failed!");
//     gfx->fillScreen(BLACK);
// #ifdef EXAMPLE_PIN_NUM_LCD_BL
//     pinMode(EXAMPLE_PIN_NUM_LCD_BL, OUTPUT);
//     digitalWrite(EXAMPLE_PIN_NUM_LCD_BL, HIGH);
// #endif
//     gfx->fillScreen(BLACK);
//     for (int i=0;i<19;i++) {
//         gfx->setCursor(0, i*10);
//         gfx->setTextColor(colr[i]);
//         gfx->print(colr[i]);
//     }
// }

// void loop(){}