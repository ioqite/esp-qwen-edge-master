// ##########################################################
// 这个文件是一个模板!!!
// 请复制一份 并 命名为 user_config.h，然后再修改
// ##########################################################

#pragma once
// ################## 修改为自己的 API密钥, 获取: (如此处教程不可用, 请参考阿里云百炼官网)#########################
// 1. 注册登录阿里云百炼, 官网链接：https://bailian.console.aliyun.com
// 2. 实名认证: https://myaccount.console.aliyun.com/cert-info
// 3. 获取API密钥：https://bailian.console.aliyun.com/cn-beijing/?tab=model#/api-key
#define apiKey "sk-xxxxxxxxxxxxxxxx"

// ##################### 修改为 键盘的 BLE MAC ################################
// !!!!! 是 蓝牙(BLE) MAC, 不是WiFi MAC !!!!!
#define BLE_PEER_MAC "00:00:00:00:00:00"
#define BLE_ROLE     BLETextLink::MASTER
#define BLE_NAME     "ESP-Edge"

// ######################## 修改为自己的 讯飞STT API密钥 ################################

// 讯飞STT 的key (使用 Qwen-ASR 可不配置)
#define STTAPPID     "xxxxxxx"
#define STTAPISecret "xxxxxxx"
#define STTAPIKey    "xxxxxxx"

// ###################### 修改为自己的 WiFi名称 与 密码 #########################
// 注意: SSID 数量与密码数量必须一致
const char* ssids[] = {"", // 索引从1开始, 0留空
	"your_wifi_1", 
	"your_wifi_2",
	"your_wifi_3",
};
const char* passwords[] = {"", // 索引从1开始, 0留空
	"your_password_1",
	"your_password_2",
	"your_password_3",
};

// ##########################################################################
// 摄像头 相关配置

#define PWDN_GPIO_NUM  17   // power down is not used
#define RESET_GPIO_NUM -1  // software reset will be performed
#define XCLK_GPIO_NUM   8
#define SIOD_GPIO_NUM  21
#define SIOC_GPIO_NUM  16

#define Y9_GPIO_NUM   2
#define Y8_GPIO_NUM   7
#define Y7_GPIO_NUM  10
#define Y6_GPIO_NUM  14
#define Y5_GPIO_NUM  11
#define Y4_GPIO_NUM  15
#define Y3_GPIO_NUM  13
#define Y2_GPIO_NUM  12
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM  4
#define PCLK_GPIO_NUM  9

// ##########################################################################
// GMT 时间偏移量 (秒)
#define GMT_OFFSET_SEC 8 * 3600
// SNTP 服务器
#define SNTP_SERVER "ntp.cnnic.cn" // CNNIC

// 文本快捷键
#define TEXT_SHORTCUT_SIZE 9
const char* TEXT_SHORTCUT[TEXT_SHORTCUT_SIZE] = {"用", "控制", "是", "什么", "如何", "有", "中", "的", "详细说一下"};


