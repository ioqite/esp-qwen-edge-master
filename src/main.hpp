// ======== esp32s3-lcd-2 ========
#pragma once

// 必要
#include <Arduino.h>
#include <string>
#include "esp_task_wdt.h"
#include <pthread.h>
#include "time.h"
// API请求 相关
#include <vector>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#if __has_include("user_config.h")
#include "user_config.h"
#else
#include "user_config-template.h"
#endif
// SD 卡相关
#include "FS.h"
#include "SD.h"
#include "SPI.h"
// IMU 相关
#include "FastIMU.h"
#include <Wire.h>
// LCD 相关
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
// #include <examples/lv_examples.h>
// #include <demos/lv_demos.h>
// I2S 相关
#include <base64.h>
#include <driver/i2s.h>
#include <ArduinoWebsockets.h>

// ###################### API #########################
#define API_ENDPOINT "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation"
// 多模态API的URL, 请求体格式不一样: "https://dashscope.aliyuncs.com/api/v1/services/aigc/multimodal-generation/generation";

// ###################### 引脚定义 及 设置参数 #########################

// 按键引脚定义
#define RECORD_BTN 11  // 录音按钮引脚
#define SEND_BTN 13    // 发送按钮引脚

// SD卡 引脚定义
#define SD_SCK 39
#define SD_MISO 40
#define SD_MOSI 38
#define SD_CS 41

// I2S 引脚定义 及 设置参数
#define I2S_WS 9    // WS 引脚
#define I2S_SD 12   // SD 引脚
#define I2S_SCK 14  // SCK 引脚
#define I2S_PORT I2S_NUM_0  // I2S 编号
#define SAMPLE_RATE 16000   // 采样率
#define MAX_RECORD_TIME_SECONDS 30    // 最大录音时间 (秒)
#define BUFFER_SIZE (SAMPLE_RATE * MAX_RECORD_TIME_SECONDS)  // 缓冲区大小 (16bit 个数)
#define CHUNK_SIZE 2048

// LCD 引脚定义
#define EXAMPLE_PIN_NUM_LCD_SCLK 39  // LCD 时钟 引脚
#define EXAMPLE_PIN_NUM_LCD_MOSI 38  // LCD MOSI 引脚
#define EXAMPLE_PIN_NUM_LCD_MISO 40 // LCD MISO 引脚
#define EXAMPLE_PIN_NUM_LCD_DC 42   // LCD 数据/指令 引脚
#define EXAMPLE_PIN_NUM_LCD_RST -1  // LCD 重启 引脚
#define EXAMPLE_PIN_NUM_LCD_CS 45   // LCD 片选 引脚
#define EXAMPLE_PIN_NUM_LCD_BL 1    // LCD 背光 引脚 (非LEDC通道)
// 背光 LEDC 参数
#define LEDC_FREQ             5000  // LEDC 频率
#define LEDC_TIMER_10_BIT     10    // LEDC 定时器精度
#define LEDC_DEFAULT_DUTY     80    // LEDC 默认占空比 (0-100)
// LVGL 屏幕参数
#define EXAMPLE_LCD_ROTATION 3    // LCD 旋转次数
#define EXAMPLE_LCD_H_RES 240     // LCD 水平分辨率
#define EXAMPLE_LCD_V_RES 320     // LCD 垂直分辨率

// IMU 引脚定义
#define IMU_PIN_SDA 48    // IMU SDA 引脚
#define IMU_PIN_SCL 47    // IMU SCL 引脚
#define IMU_ADDRESS 0x6B  // IMU 地址, 默认 0x6B

// 模型名称
#define MAIN_MODEL_NAME "qwen-plus"   // 主模型
#define PROC_MODEL_NAME "qwen-plus"   // 拼音预处理 模型

// ===== Qwen-ASR 配置 =====
#define QWEN_ASR_MODEL "qwen3-asr-flash-realtime"
// 北京地域 baseUrl
#define QWEN_ASR_BASE_URL "wss://dashscope.aliyuncs.com/api-ws/v1/realtime"
// VAD模式设置: true=VAD模式（服务端自动断句），false=Manual模式（客户端控制断句）
#define ENABLE_SERVER_VAD false
#define ASR_LANGUAGE "zh"

// ############################## 提示词 #################################
#define USE_COLOR_ANSWER 0 // 是否使用颜色回答

#if USE_COLOR_ANSWER
// 不带 拼音预处理 的 系统提示词
#define SYS_PROMPT_NO_PROC "你是一个语言简练准确的AI助手。不要进入思考模式，尽可能不用生僻字，禁止使用Emoji和Markdown语法。要将一段文本染色，请在开始颜色时加上‘#RRGGBB ’，结束颜色时加上‘#’，RRGGBB为HEX颜色。"

// 带 拼音预处理 的 系统提示词:
#define SYS_PROMPT "你是一个语言简练的AI助手。不要进入思考模式，尽可能不用生僻字，禁止使用Emoji和Markdown语法。用户输入可能有汉字错误，请自动纠正。要将一段文本染色，请在开始颜色时加上‘#RRGGBB ’，结束颜色时加上‘#’，RRGGBB为HEX颜色。"

// 拼音预处理 提示词:
#define PROC_SYS_PROMPT "你是一个将拼音替换为文本的工具，如果输入中没有出现拼音则直接把输入原封不动地输出，禁止回答用户的问题！禁止回答用户的问题！也不要进入思考模式。如果有拼音则联系附近字符理解用户意思，将输入中的拼音+声调（0为轻声）替换为对应的汉字（仅替换拼音+声调，如没有则不替换，不要替换任何数字与符号），请仅在用户的拼音错误时自动纠正，不要输出其他内容，拼音之间会用空格分隔，输出时请忽略，如果能则尽量保持输出的是一句正常的话。"
// 用于测试拼音替换器: ni neng2 kan4 dao4 shang4 yi2 ju4 hua4 me0 ?
#else
// 不带 拼音预处理 的 系统提示词
#define SYS_PROMPT_NO_PROC "你是一个语言简练准确的AI助手。不要进入思考模式，尽可能不用生僻字，禁止使用Emoji和Markdown语法。"

// 带 拼音预处理 的 系统提示词:
#define SYS_PROMPT "你是一个语言简练的AI助手。不要进入思考模式，尽可能不用生僻字，禁止使用Emoji和Markdown语法。用户输入可能有汉字错误，请自动纠正。"

// 拼音预处理 提示词:
#define PROC_SYS_PROMPT "你是一个将拼音替换为文本的工具，如果输入中没有出现拼音则直接把输入原封不动地输出，禁止回答用户的问题！禁止回答用户的问题！也不要进入思考模式。如果有拼音则联系附近字符理解用户意思，将输入中的拼音+声调（0为轻声）替换为对应的汉字（仅替换拼音+声调，如没有则不替换，不要替换任何数字与符号），请仅在用户的拼音错误时自动纠正，不要输出其他内容，拼音之间会用空格分隔，输出时请忽略，如果能则尽量保持输出的是一句正常的话。"
// 用于测试拼音替换器: ni neng2 kan4 dao4 shang4 yi2 ju4 hua4 me0 ?
#endif

// 旧版提示词，可供参考：
// String SYS_PROMPT = "你是一个语言简练准确的AI助手，需要时再思考。";
// String SYS_PROMPT = "你是一个语言简练准确的AI助手，需要时再思考，尽可能不用生僻字，且确保所有输出每行长度<=10中文字符或20英文字符（1中文字符与2英文字符长度相等），超出部分将被截断。用户只能使用拼音+声调表示汉字，用空格分隔汉字，请联系附近字符理解用户意思，例如：应将“ni3 hao3 !”理解为“你好！”。为确保准确，请先输出：“输入：{理解到的用户输入}\n”再回答。";
// String SYS_PROMPT = "你是一个语言简练准确的AI助手，需要时再思考，尽可能不用生僻字。用户只能使用拼音+声调表示汉字，用空格分隔汉字，请联系附近字符理解用户意思。";
//，例如：应将ni3 hao3 !理解为你好！。为确保准确，请先输出：输入：{理解到的用户输入}再回答。
// String proc_prompt = "你是一个文本替换工具，如果输入中没有出现拼音则把输入原封不动地输出，然后不要理会本输入后面的内容！不管输入是什么，都不要回答问题！不要理会用户的问题！也不要进入思考模式。请联系附近字符理解用户意思，将输入中的拼音+声调（0为轻声）替换为对应的汉字（仅替换拼音+声调，如没有则不替换，不要替换任何数字与符号），用户的拼音可能有错，请自动纠正，不要输出其他内容，汉字之间会用空格分隔，输出时请忽略，如果能则保持输出的是一句正常的话。";


// ########################## 函数声明 #########################

// 刷屏回调
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

// 用户内容 初始化
void setup();

// 初始化 WiFi 并 启动按键接收循环 (Core 0)
void my_loop(void *param);

// 根据情况发送不同请求
void getAnswer(String& _user_prompt);

// 主循环 (Core 1)
void loop();

// 获取 剩余RAM
void print_heap_free();

// ============== 文本输入框ta 操作 ===============
// 添加文本到 ta 上
void ta_add_text(const char* text);
// 设置 ta 上的文本
void ta_set_text(const char* text);
// 暂存 ta 上的文本
void ta_tmp_save();
// 恢复 ta 上的文本
void ta_tmp_recover();
// 在 ta 上临时显示文本
void ta_tmp_show(const char* text, uint16_t delay_ms);

// ============== main_label 操作 ===============
// 添加文本到 main_label 上
void main_label_add_text(const char* text);
// 设置 main_label 上的文本
void main_label_set_text(const char* text);
// 暂存 main_label 上的文本
void main_label_tmp_save();
// 恢复 main_label 上的文本
void main_label_tmp_recover();
// 在 main_label 上临时显示文本
void main_label_tmp_show(const char* text, uint16_t delay_ms);

// 检查是否有按键按下
bool check_key();

// // 鼠标回调
// void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data);

// 读取 IMU 数据, 并 更新滚动位置
void my_read_imu();

// 文本输入框 事件回调 (LVGL回调)
static void ta_event_cb(lv_event_t * e);

// 发送按键到文本输入框
void send_key_to_ta(uint32_t key);

// 硬件 初始化
void hardware_init();

/**
 * @brief 向对话历史中添加消息
 * @param role 消息角色 ("system", "user", "assistant")
 * @param content 消息内容
 */
void addMessageToHistory(const char* role, const String content);

/**
 * @brief 构建并发送 HTTPS 请求到 API
 * @param _SYSTEM_PROMPT 系统提示词 引用
 * @param _userPrompt 用户的当前问题 引用
 * @param _MAIN_MODEL_NAME 模型名称 引用
 * @param _response 存储 回复内容 或 错误信息 的字符串引用
 * @param useHistory 是否使用轮次对话
 * @return 运行结果: (详见 _response)
 *           -1: _response 为空
 *           -2: 无网络
 *           -3: JSON解析失败
 *           -4: 响应码
 *           -5: 请求失败
 */
int8_t getAPIanswer(const char* _SYSTEM_PROMPT, const String& _userPrompt, const char* _MAIN_MODEL_NAME, String& _response, bool useHistory);

// 重置对话历史
void reset_chat_history();


// ######### STT / ASR 相关 #########

// // 构造讯飞ws连接url
// String make_XF_WSurl(const char *Secret, const char *Key, String request, String host);

// // 向讯飞STT发送音频数据
// void STTsend();

// 初始化 I2S
void setupI2S();

// ############### 其他 ################
bool check_key();

// 初始化 SD卡
void init_SDcard();

// 列出 SD卡目录 及 文件
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

// 向 Qwen-ASR 发送音频数据
void asr_send(uint16_t* pcm_data, uint32_t size);

// 选择SSID 并 连接WiFi
void connect_wifi();

