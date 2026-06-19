// // ======== esp32s3-lcd-2 ========
// #pragma once

// #include <Arduino.h>
// #include <WiFi.h>
// #include <esp_now.h>
// #include <string>
// #include <HTTPClient.h>
// #include <ArduinoJson.h>
// #include "esp_task_wdt.h"
// #include <HTTPClient.h>
// #include <ArduinoJson.h>
// #include "FastIMU.h"
// #include <Wire.h>
// #include <Arduino_GFX_Library.h>
// #include <lvgl.h>
// #include <pthread.h>

// // new_add
// #include "time.h"
// #include <base64.h>
// #include <driver/i2s.h>
// #include <mbedtls/md.h>
// #include <ArduinoWebsockets.h>
// #if __has_include("user_config.h")
// #include "user_config.h"
// #else
// #include "user_config-template.h"
// #endif
// // new_add end

// // Use LV_USE_LABEL && LV_USE_TEXTAREA  && LV_USE_IME_PINYIN 

// // #include <examples/lv_examples.h>
// // #include <demos/lv_demos.h>


// // new_add
// #define RECORD_BTN 11  // 录音按钮引脚

// #define I2S_WS 9   // WS 引脚
// #define I2S_SD 12   // SD 引脚
// #define I2S_SCK 14  // SCK 引脚
// #define I2S_PORT I2S_NUM_0 // I2S 编号
// #define SAMPLE_RATE 16000  // 采样率
// #define MAX_RECORD_TIME_SECONDS 30 // 最大录音时间 (秒)

// #define BUFFER_SIZE (SAMPLE_RATE * MAX_RECORD_TIME_SECONDS)
// #define CHUNK_SIZE 2048
// // new_add end

// #define EXAMPLE_PIN_NUM_LCD_SCLK 39
// #define EXAMPLE_PIN_NUM_LCD_MOSI 38
// #define EXAMPLE_PIN_NUM_LCD_MISO 40
// #define EXAMPLE_PIN_NUM_LCD_DC 42
// #define EXAMPLE_PIN_NUM_LCD_RST -1
// #define EXAMPLE_PIN_NUM_LCD_CS 45
// #define EXAMPLE_PIN_NUM_LCD_BL 1
// #define IMU_PIN_SDA 48
// #define IMU_PIN_SCL 47

// #define EXAMPLE_LCD_ROTATION 3
// #define EXAMPLE_LCD_H_RES 240
// #define EXAMPLE_LCD_V_RES 320

// #define IMU_ADDRESS 0x6B

// // 刷屏回调
// void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

// void setup();

// // 初始化 WiFi 并 启动按键接收循环 (Core 0)
// void my_loop(void *param);

// // 根据情况发送不同请求
// void getAnswer(String _user_prompt);

// // 主循环 (Core 1)
// void loop();

// // 获取 剩余RAM
// void print_heap_free();

// // 修改 各个Label 的文本
// void main_label_print(String print_text);
// void main_label_clear_print(String print_text);

// // 鼠标回调
// void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data);

// // 读取 IMU, 并 更新鼠标位置
// void my_read_imu();

// static void ta_event_cb(lv_event_t * e);

// // 物理按键输入接口
// void physical_key_input(uint32_t key);

// // 全部初始化
// void my_init();

// /**
//  * @brief 向对话历史中添加消息
//  * @param role 消息角色 ("system", "user", "assistant")
//  * @param content 消息内容
//  */
// void addMessageToHistory(const char* role, const String content);


// /**
//  * @brief 构建并发送 HTTP 请求到 API
//  * @param _systemPrompt 系统提示词
//  * @param _userPrompt 用户的当前问题
//  * @param _model_name 模型名称
//  * @param _response 存储回复内容的字符串指针
//  * @param useHistory 是否使用轮次对话
//  * @return AI的回复内容字符串
//  */
// int8_t getQwenAnswer(const String _systemPrompt, const String _userPrompt, String _model_name, String* _response, bool useHistory);

// void reset_chat_history();

// // new_add
// // 讯飞STT 相关
// String getDateTime(bool print);
// // 构造讯飞ws连接url
// String make_XF_WSurl(const char *Secret, const char *Key, String request, String host);
// // 向讯飞STT发送音频数据
// void STTsend();

// void setupI2S();

// // new_add end


// // 0x00177fff
