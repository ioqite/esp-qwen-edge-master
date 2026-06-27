// ##########################################################
// 这个文件是一个模板!!!
// 请复制一份 并 命名为 user_config.h，然后再修改
// ##########################################################

#pragma once
// ###################### 修改为自己的 API密钥, 获取: (如此处教程不可用, 请参考阿里云百炼官网)#########################
// 1. 注册登录阿里云百炼, 官网链接：https://bailian.console.aliyun.com
// 2. 实名认证: https://myaccount.console.aliyun.com/cert-info
// 3. 获取API密钥：https://bailian.console.aliyun.com/cn-beijing/?tab=model#/api-key
#define apiKey "sk-xxxxxxxxxxxxxxxx"

// ######################## 修改为自己的 讯飞STT API密钥 ################################

// 讯飞STT 的key (使用 Qwen-ASR 可不配置)
#define STTAPPID "xxxxxxx"
#define STTAPISecret "xxxxxxx"
#define STTAPIKey "xxxxxxx"

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


