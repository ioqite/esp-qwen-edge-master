// // ======== esp32s3-lcd-2 ========

// #include "main.hpp"

// int wifi_choose = 0;

// TaskHandle_t TASK_Handle_wifi = NULL;
// TaskHandle_t TASK_HandleOne = NULL;

// // LVGL 线程锁
// SemaphoreHandle_t lvgl_mutex = xSemaphoreCreateRecursiveMutex();
// // static pthread_mutex_t lvgl_mutex = PTHREAD_MUTEX_INITIALIZER;
// // SPI 线程锁
// SemaphoreHandle_t spi_mux = xSemaphoreCreateRecursiveMutex();

// bool lvgl_mux_lock() { return xSemaphoreTakeRecursive(lvgl_mutex, portMAX_DELAY) == pdTRUE; }
// void lvgl_mutex_unlock() { xSemaphoreGiveRecursive(lvgl_mutex); }
// bool spi_mux_lock() { return xSemaphoreTakeRecursive(spi_mux, portMAX_DELAY) == pdTRUE; }
// void spi_mux_unlock() { xSemaphoreGiveRecursive(spi_mux); }

// // 从 SD卡 /lcd-t.txt 读取的文本
// String sd_text = "";

// double ledc_duty = LEDC_DEFAULT_DUTY;	// 默认占空比

// // IMU 和 鼠标指针
// QMI8658 IMU;
// AccelData accelData;
// GyroData gyroData;
// // float touchpad_x = 160;
// // float touchpad_y = 120;

// // 屏幕刷新 与 LVGL 缓冲区
// uint32_t screenWidth;
// uint32_t screenHeight;
// uint32_t bufSize;
// lv_disp_draw_buf_t draw_buf;
// lv_color_t *disp_draw_buf;
// lv_disp_drv_t disp_drv;

// // 处理中的按键
// String proc_key;

// // 定义 LVGL对象 和 显示的文本
// lv_obj_t * main_panel;
// lv_obj_t * main_label;
// String main_label_text = "";
// lv_obj_t * g_ta;
// lv_obj_t * g_kb;

// // GMT 时间偏移量 (秒)
// #define GMT_OFFSET_SEC 8 * 3600
// // SNTP 服务器
// const char* sntpServers[] = {  // 实际只使用 第1个
// 	"ntp.cnnic.cn",              // CNNIC
// 	"ntp.tuna.tsinghua.edu.cn",  // TUNA
// 	"ntp.aliyun.com"             // 阿里云
// };

// int16_t audioData[2560];
// int16_t *pcm_data; // 录音缓存区
// uint recordingSize = 0;

// using namespace websockets;
// WebsocketsClient client;

// String stttext = "";

// bool transferring_key = 0;     // 开始接收新的键 (有的键是多字母的)
// bool skip_wifi = 0;            // 是否跳过WiFi连接
// bool connecting_wifi = false;  // 是否正在连接WiFi
// bool syncing_sntp = false;     // 是否正在同步SNTP时间

// // ###################### 对话管理 #########################
// // 最大对话窗口 个数
// #define MAX_CHAT_WINDOW 5
// // 最大 保存的对话轮数
// #define MAX_MESSAGES 20

// struct chat_window_t{
// 	String ta_text;
// 	String main_label_text;
// 	std::vector<String> chatHistory; // 使用一个数组来存储 每一条JSON格式的消息(String) max:MAX_MESSAGES * 2 + 1
// 	// uint8_t messageCount = 0;  // 当前历史消息数量
// } chat_windows[MAX_CHAT_WINDOW];

// // 当前选中的 对话窗口
// uint8_t chat_window_select = 0;

// // ###################### 请求管理 #########################
// // 请求所需变量
// String response;
// String answer;
// String proced;

// // 用户提示词
// String user_prompt = "";

// bool show_proced = 0;   // 是否显示 预处理过的 提示词, 由 $# 键切换
// uint8_t bypass_proc = 0; // 是否跳过 拼音预处理, 由 $@ 键切换
// // bool enable_search = 0; // 是否启用 联网搜索

// // ###################### other #########################
// // 储存 时间信息
// tm timeinfo;

// // 屏幕, More: https://github.com/moononournation/Arduino_GFX/wiki/
// Arduino_DataBus *bus = new Arduino_ESP32SPI(EXAMPLE_PIN_NUM_LCD_DC, EXAMPLE_PIN_NUM_LCD_CS, EXAMPLE_PIN_NUM_LCD_SCLK, EXAMPLE_PIN_NUM_LCD_MOSI, EXAMPLE_PIN_NUM_LCD_MISO, FSPI /* spi_num */, true);
// Arduino_GFX *gfx = new Arduino_ST7789(bus, EXAMPLE_PIN_NUM_LCD_RST, EXAMPLE_LCD_ROTATION, true /* IPS */, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);

// void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) { lv_disp_flush_ready(disp_drv); }

// #if LV_USE_LOG != 0
// void my_print(const char *buf) {
// 	Serial.printf(buf);
// 	Serial.flush();
// }
// #endif

// void setup() {
// 	Serial.begin(115200);
// 	// print_heap_free();
// 	Serial1.begin(115200, SERIAL_8N1, 10, 7); // 初始化 C3从机串口
	
// 	hardware_init();  // 硬件 初始化
	
// 	// 声明字体
// 	// LV_FONT_DECLARE(siyuan_normal_ext);
// 	LV_FONT_DECLARE(cgr_yuyag_w2_ext4);

// 	// 创建 main_panel 和 main_label
//     main_panel = lv_obj_create(lv_scr_act());
// 	lv_obj_align(main_panel, LV_ALIGN_TOP_LEFT, 5, 54);
//     lv_obj_set_size(main_panel, 310, 186);

//     main_label = lv_label_create(main_panel);
//     lv_obj_set_size(main_label, 295, 4500);
// 	lv_label_set_recolor(main_label, true);
// 	lv_obj_set_style_text_font(main_label, &cgr_yuyag_w2_ext4, 0);
// 	// lv_obj_set_style_text_font(main_label, &siyuan_normal_ext, 0);
// 	lv_obj_align(main_label, LV_ALIGN_TOP_MID, 0, 0);
// 	lv_label_set_text(main_label, "");

//     // 创建文本框
//     g_ta = lv_textarea_create(lv_scr_act());
//     lv_textarea_set_one_line(g_ta, false);
//     lv_obj_set_style_text_font(g_ta, &cgr_yuyag_w2_ext4, 0);
//     lv_obj_align(g_ta, LV_ALIGN_TOP_LEFT, 5, 3);
//     lv_obj_set_size(g_ta, 310, 46);// w:310
//     lv_obj_add_state(g_ta, LV_STATE_FOCUSED);  /* 默认聚焦 */

//     // 创建键盘 (隐藏按键)
// 	g_kb = lv_keyboard_create(lv_scr_act());
//     lv_obj_add_flag(g_kb, LV_OBJ_FLAG_HIDDEN);  /* 直接隐藏键盘 */
//     lv_keyboard_set_textarea(g_kb, g_ta);
// 	lv_obj_add_event_cb(g_ta, ta_event_cb, LV_EVENT_ALL, g_kb);  

// 	// 启动 C3 从机
// 	// Serial1.write("`");
	
// 	xTaskCreatePinnedToCore(
// 		my_loop,     // 任务函数
// 		"my_loop",   // 任务名称
// 		10000,        // 堆栈大小
// 		NULL,         // 参数
// 		1,            // 优先级
// 		&TASK_HandleOne,  // 任务句柄
// 		0             // 核心编号 (0或1)
// 	);
// 	esp_task_wdt_delete(TASK_HandleOne);
// }

// // 读取按键输入
// String read_key() {
// 	transferring_key = 0;
// 	String proc_key = "";
// 	while (1) {
// 		if (Serial1.available()) {
// 			char t = Serial1.read();
// 			if (t == '`') {
// 				transferring_key = !transferring_key;
// 				if (!transferring_key) {
// 					return proc_key;
// 				}
// 			} else if (transferring_key) proc_key += t;
// 		} else vTaskDelay(2 / portTICK_PERIOD_MS);
// 	}
// }

// // WiFi 事件回调函数（由系统后台任务自动调用）
// void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
//     switch (event) {
//         case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
//             Serial.printf("WiFi断开（原因码: %d）,正在尝试自动重连...", info.wifi_sta_disconnected.reason);
//             // isConnected = false; // 标记需要重连
//             break;
//         default:
//             break;
//     }
// }

// // 更改 文本输入框 文本
// void ta_add_text(const char* text) {
// 	if (lvgl_mux_lock()) {  // 加锁
// 		lv_textarea_add_text(g_ta, text);
// 		lvgl_mutex_unlock();  // 解锁
// 	}
// }
// void ta_set_text(const char* text) {
// 	if (lvgl_mux_lock()) {  // 加锁
// 		lv_textarea_set_text(g_ta, text);
// 		lvgl_mutex_unlock();  // 解锁
// 	}
// }

// // 等待 WiFi 连接
// void wait_wifi_connection(void *param) {
// 	connecting_wifi = true;
// 	Serial.print("连接WiFi中 ..");
// 	main_label_set_text("#1058b1 连接WiFi中 ..");
// 	while (WiFi.status() != WL_CONNECTED) {
// 		Serial.print('.');
// 		main_label_add_text(".");
// 		vTaskDelay(700 / portTICK_PERIOD_MS);
// 	}
// 	// Serial.println(WiFi.localIP());
// 	connecting_wifi = false;
// 	Serial.println(" OK");
// 	main_label_add_text("# #3add70 OK#");

// 	syncing_sntp = true;
// 	// SNTP 时间同步
// 	Serial.print("使用 SNTP 同步时间 ...");
// 	main_label_set_text("#10acb1 使用 SNTP 同步时间 ...");
// 	configTime(GMT_OFFSET_SEC, 0, sntpServers[0]); //, sntpServers[1], sntpServers[2]
// 	while (!getLocalTime(&timeinfo)) {
// 		vTaskDelay(700 / portTICK_PERIOD_MS);
// 		Serial.print(".");
// 		main_label_add_text(".");
// 	}
// 	syncing_sntp = false;
// 	Serial.println(" OK");
// 	main_label_add_text("# #3add70 OK#");
// 	vTaskDelete(TASK_Handle_wifi);
// }

// // 初始化 WiFi 并 启动按键接收循环 (Core 0)
// void my_loop(void *param) {
// 	// 等待选择 SSID 并初始化 WiFi
// 	Serial.println("初始化 WiFi");

// 	main_label_text = "#0060b9 请选择 WIFI：\n";
// 	for (int i=1; i<=6; i++) {
// 		main_label_text += i;
// 		main_label_text += ": ";
// 		main_label_text += ssids[i];
// 		main_label_text += "\n";
// 	}
// 	main_label_set_text(main_label_text);

// 	// 等待选择 SSID 并初始化 WiFi
// 	while (1) {
// 		proc_key = read_key();
// 		wifi_choose = 0;
// 		if (proc_key == "1") wifi_choose = 1;
// 		else if (proc_key == "2") wifi_choose = 2;
// 		else if (proc_key == "3") wifi_choose = 3;
// 		else if (proc_key == "4") wifi_choose = 4;
// 		else if (proc_key == "5") wifi_choose = 5;
// 		else if (proc_key == "6") wifi_choose = 6;
// 		else if (proc_key == "$12") wifi_choose = 12;
// 		else {
// 			proc_key = "";
// 			Serial.println("unknow SSID Key: " + proc_key);
// 		}
// 		if (wifi_choose && wifi_choose != 12) {
// 			Serial.println("Choose SSID Key: " + proc_key);
// 			WiFi.mode(WIFI_STA);
// 			WiFi.begin(ssids[wifi_choose], passwords[wifi_choose]);

// 			xTaskCreate(
// 				wait_wifi_connection,     // 任务函数
// 				"wait_wifi_connection",   // 任务名称
// 				4096,            // 堆栈大小
// 				NULL,            // 参数
// 				1,               // 优先级
// 				&TASK_Handle_wifi  // 任务句柄
// 			);
// 			proc_key = "";
// 			break;
// 		} else if (wifi_choose == 12) {
// 			proc_key = "Skip WIFI";
// 			skip_wifi = 1;
// 			main_label_set_text("#115df5 跳过 WIFI 连接#");
// 			break;
// 		} else vTaskDelay(10 / portTICK_PERIOD_MS);
// 	}

// 	// 初始化 I2S
// 	setupI2S();

// 	// 当接受到消息时 调用回调函数
// 	client.onMessage([&](WebsocketsMessage message) {
// 		// STT ws连接的回调函数
// 		Serial.print("Got Message: ");
// 		Serial.println(message.data());
// 		JsonDocument doc;
// 		DeserializationError error = deserializeJson(doc, message.data());
// 		if (error) {
// 			Serial.print(F("deserializeJson() failed: "));
// 			Serial.println(error.f_str());
// 			return;
// 		}
// 		JsonArray ws = doc["data"]["result"]["ws"];
// 		for (JsonObject word : ws) {
// 			int bg = word["bg"];
// 			const char *w = word["cw"][0]["w"];
// 			stttext += w;
// 		}
// 		if (doc["data"]["status"] == 2) {
// 			// 收到结束标志
// 			Serial.print("语音识别完毕：");
// 			Serial.println(stttext);
// 		}
//   	});

// 	if (lvgl_mux_lock()) { // 上锁
// 		lv_textarea_set_placeholder_text(g_ta, "请输入");
// 		lvgl_mutex_unlock(); // 解锁
// 	}

// 	Serial.println("Setup done");
// 	print_heap_free();

// 	// 主循环 (Core 1)
// 	while (1) {
// 		proc_key = read_key(); // 读取按键输入
// 		Serial.println("Key: " + proc_key);
// 		if (proc_key == "$12") {
// 			print_heap_free();

// 			if (lvgl_mux_lock()) { // 上锁
// 				user_prompt = lv_textarea_get_text(g_ta);
// 				if (user_prompt == "") continue;  // 输入为空，跳过

// 				lv_obj_scroll_to_y(main_panel, 0, LV_ANIM_ON);

// 				lvgl_mutex_unlock(); // 解锁
// 			}

// 			Serial.println("等待结果中 ...");
// 			main_label_set_text("#7e00d2 等待结果中 ...#");

// 			getAnswer(user_prompt);
// 			main_label_set_text(answer);
// 		} else if (proc_key == "BCK") {
// 			send_key_to_ta(LV_KEY_BACKSPACE); // user_prompt = user_prompt.substring(0, user_prompt.length() - 1);
// 		} else if (proc_key == "DEL") {
// 			send_key_to_ta(LV_KEY_DEL);
// 		} else if (proc_key == "UP") {
// 			send_key_to_ta(LV_KEY_UP);
// 		} else if (proc_key == "DOWN") {
// 			send_key_to_ta(LV_KEY_DOWN);
// 		} else if (proc_key == "LEFT") {
// 			send_key_to_ta(LV_KEY_LEFT);
// 		} else if (proc_key == "RIGT") {
// 			send_key_to_ta(LV_KEY_RIGHT);
// 		} else if (proc_key == "ETR") {
// 			send_key_to_ta(LV_KEY_ENTER);
// 		} else if (proc_key == "&2") {
// 			bypass_proc = !bypass_proc;

// 			if (lvgl_mux_lock()) { // 上锁
// 				user_prompt = lv_textarea_get_text(g_ta);
// 				lv_textarea_set_text(g_ta, bypass_proc ? "跳过拼音预处理: 1" : "跳过拼音预处理: 0");
// 				lvgl_mutex_unlock(); // 解锁
// 			}

// 			vTaskDelay(800 / portTICK_PERIOD_MS);

// 			ta_set_text(user_prompt.c_str());
// 		} else if (proc_key == "&3") {
// 			show_proced = !show_proced;

// 			if (lvgl_mux_lock()) { // 上锁
// 				user_prompt = lv_textarea_get_text(g_ta);
// 				lv_textarea_set_text(g_ta, show_proced ? "显示拼音处理结果: 1" : "显示拼音处理结果: 0");
// 				lvgl_mutex_unlock(); // 解锁
// 			}

// 			vTaskDelay(800 / portTICK_PERIOD_MS);

// 			ta_set_text(user_prompt.c_str());
// 		} else if (proc_key.length() >= 3 && proc_key.startsWith("$S") && proc_key[2]-'0' >= 0 && proc_key[2]-'0' <= MAX_CHAT_WINDOW) {
// 			if (lvgl_mux_lock()) { // 上锁
// 				chat_windows[chat_window_select].ta_text = lv_textarea_get_text(g_ta);
// 				chat_windows[chat_window_select].main_label_text = lv_label_get_text(main_label);
				
// 				lv_textarea_set_text(g_ta, (String("chat_window_select: ") + (uint16_t)chat_window_select + " -> " + (uint16_t)(proc_key[2]-'0')).c_str());
// 				lvgl_mutex_unlock(); // 解锁
// 			}

// 			vTaskDelay(800 / portTICK_PERIOD_MS);
			
// 			chat_window_select = proc_key[2]-'0';
// 			ta_set_text(chat_windows[chat_window_select].ta_text.c_str());
// 			main_label_set_text(chat_windows[chat_window_select].main_label_text.c_str());
// 		} else if (proc_key == "$1") {
// 			ta_add_text("用");
// 		} else if (proc_key == "$2") {
// 			ta_add_text("控制");
// 		} else if (proc_key == "$3") {
// 			ta_add_text("是");
// 		} else if (proc_key == "$4") {
// 			ta_add_text("什么");
// 		} else if (proc_key == "$5") {
// 			ta_add_text("如何");
// 		} else if (proc_key == "$6") {
// 			ta_add_text("有");
// 		} else if (proc_key == "$7") {
// 			ta_add_text("中");
// 		} else if (proc_key == "$8") {
// 			ta_add_text("的");
// 		} else if (proc_key == "$9") {
// 			ta_add_text("详细说一下");
// 		} else if (proc_key == "$10") {
// 			reset_chat_history();
// 		} else if (proc_key == "$11") {
// 			ta_set_text("");
// 		} else if (proc_key == "&1") {
// 			vTaskDelay(800 / portTICK_PERIOD_MS);

// 			String tmp = "";
// 			if (lvgl_mux_lock()) { // 上锁
// 				tmp = lv_label_get_text(main_label);
// 				lvgl_mutex_unlock(); // 解锁
// 			}

// 			stttext = "";
// 			Serial.print("录音中 ...");
// 			main_label_set_text("录音中 ...");

// 			size_t bytes_read = 0;
// 			recordingSize = 0;
// 			// 分配 pcm_data
// 			pcm_data = (int16_t *)ps_malloc(BUFFER_SIZE * sizeof(int16_t));
// 			if (!pcm_data) {
// 				Serial.println(" 无法从PSRAM分配内存");
// 				main_label_set_text(" 无法从PSRAM分配内存");
// 				return;
// 			}

// 			uint32_t start_time = millis();
// 			while (recordingSize < MAX_RECORD_TIME_SECONDS * SAMPLE_RATE) {
// 				// 开始循环录音，将录制结果保存在pcm_data中
// 				esp_err_t result = i2s_read(I2S_PORT, audioData, sizeof(audioData), &bytes_read, portMAX_DELAY);
// 				memcpy(pcm_data + recordingSize, audioData, bytes_read);
// 				recordingSize += bytes_read / 2;

// 				if (millis() - start_time > 240) {
// 					start_time = millis();
// 					if (!Serial1.available()) break;
// 					while (Serial1.available()) char t = Serial1.read();
// 				}
// 			} 
// 			// Serial.printf("Total bytes read: %d\r\n", recordingSize);
// 			Serial.println(" OK");

// 			main_label_set_text("STT 识别中");
// 			STTsend();   // STT请求开始
// 			free(pcm_data);
// 			lv_textarea_add_text(g_ta, stttext.c_str());
// 			main_label_set_text("STT 识别完成: |");
// 			main_label_add_text(stttext);
// 			main_label_add_text("|");

// 			vTaskDelay(800 / portTICK_PERIOD_MS);
// 			main_label_set_text(tmp);
// 		} else {
// 			// send_key_to_ta(proc_key[0]);
// 			ta_add_text(proc_key.c_str());
// 		}

// 		// if (digitalRead(0) == LOW) {
// 		// 	vTaskDelay(15 / portTICK_PERIOD_MS);
// 		// 	if (digitalRead(0) == HIGH) continue;

// 		// 	Serial.println("Wait for recording");
// 		// }
// 		if (client.available()) {
// 			client.poll(); // 处理接收的消息
// 		}
// // 		vTaskDelay(1 / portTICK_PERIOD_MS);
// 	}
// }

// // 根据情况发送不同请求
// void getAnswer(String& _user_prompt) {
// 	if (_user_prompt == "-t") {
// 		if (syncing_sntp) {
// 			answer = "#bb5a14 正在同步时间 ";
// 			return;
// 		}

// 		char buffer[80];
// 		if(!getLocalTime(&timeinfo)){
// 			Serial.println("获取时间失败");
// 			return;
// 		}
// 		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S\r\n", &timeinfo);
// 		answer = buffer;
		
// 		Serial.println(answer);
// 		return;
// 	}
// 	if (_user_prompt == "-imu") {
// 		IMU.update();
// 		IMU.getAccel(&accelData); // accelData.accelX accelData.accelY accelData.accelZ
// 		IMU.getGyro(&gyroData);   // gyroData.gyroX   gyroData.gyroY   gyroData.gyroZ
// 		// Serial.println(IMU.getTemp());

// 		answer = "IMU Temp: " + String(IMU.getTemp());
// 		answer += "\r\nAccel:\r\n - X: " + String(accelData.accelX) + "\r\n - Y: " + String(accelData.accelY) + "\r\n - Z: " + String(accelData.accelZ);
// 		answer += "\r\nGyro:\r\n - X: " + String(gyroData.gyroX) + "\r\n - Y: " + String(gyroData.gyroY) + "\r\n - Z: " + String(gyroData.gyroZ);
// 		Serial.println(answer);
// 		return;
// 	}
// 	if (_user_prompt == "-i") {
// 		answer = "CPU 温度: " + String(temperatureRead());
// 		// answer += "\r\n芯片版本: " + String(ESP.getChipRevision());
// 		answer += "\r\nCPU 频率: " + String(ESP.getCpuFreqMHz()) + " MHz";
// 		// answer += "\r\n循环计数: " + String(ESP.getCycleCount());
		
// 		answer += "\r\n堆 容量: " + String(ESP.getHeapSize() / 1024.0) + " KiB";
// 		answer += "\r\n堆 空闲: " + String(ESP.getFreeHeap() / 1024.0) + " KiB";
// 		answer += "\r\nPSRAM 容量: " + String(ESP.getPsramSize() / 1024.0 / 1024.0, 3) + " MiB";
// 		answer += "\r\nPSRAM 空闲: " + String(ESP.getFreePsram() / 1024.0 / 1024.0, 3) + " MiB";
		
// 		answer += "\r\nFlash 模式: ";
// 		switch (ESP.getFlashChipMode()) {
// 			case FM_QIO:
// 				answer += "QIO";
// 				break;
// 			case FM_QOUT:
// 				answer += "QOUT";
// 				break;
// 			case FM_DIO:
// 				answer += "DIO";
// 				break;
// 			case FM_DOUT:
// 				answer += "DOUT";
// 				break;
// 			case FM_FAST_READ:
// 				answer += "FAST_READ";
// 				break;
// 			case FM_SLOW_READ:
// 				answer += "SLOW_READ";
// 				break;
// 			case FM_UNKNOWN:
// 				answer += "UNKNOWN";
// 				break;
// 			default:
// 				answer += "UNKNOWN";
// 				break;
// 		}
// 		answer += "\r\nFlash 大小: " + String(ESP.getFlashChipSize() / 1024.0 / 1024.0) + " MiB";
// 		answer += "\r\nFlash 频率: " + String(ESP.getFlashChipSpeed() / 1000000.0) + " MHz";
		
// 		answer += "\r\nSDK 版本: " + String(ESP.getSdkVersion());
// 		Serial.println(answer);
// 		return;
// 	}
// 	if (_user_prompt.startsWith("-bl=")) {
// 		ledc_duty = _user_prompt.substring(_user_prompt.indexOf("=") + 1).toDouble();
// 		ledc_duty = constrain(ledc_duty, 0, 100);
// 		ledcWrite(LEDC_CHANNEL , ledc_duty ? (1 << LEDC_TIMER_10_BIT) / 100 * ledc_duty : 0.2);
// 		answer = "背光 亮度: " + String(ledc_duty);
// 		Serial.println(answer);
// 		return;
// 	}
// 	if (_user_prompt == "-rst") {ESP.restart();}
// 	if (_user_prompt == "-ds") {
// 		esp_deep_sleep_start();
// 	}
// 	if (_user_prompt == "-dsg") {
// 		esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, LOW);
// 		esp_deep_sleep_start();
// 	}
// 	if (_user_prompt == "-sd") {
// 		// if (spi_mux_lock()) {   // 加锁
// 		// 	sd_text = "";
// 		// 	File file = SD.open("/lcd-t.txt");
// 		// 	if (file.available()) {
// 		// 		while (file.available()) sd_text += (char)file.read();
// 		// 		file.close();
// 		// 	} else Serial.println("Error opening lcd-t.txt");
// 		// 	spi_mux_unlock(); // 解锁
// 		// }
// 		answer = sd_text;
// 		Serial.println(answer);
// 		return;
// 	}
// 	// 快捷文本
// 	if (_user_prompt == "-t-1") {answer = "";return;}
// 	if (_user_prompt == "-t-2") {answer = "";return;}
// 	if (_user_prompt == "-t-3") {answer = "";return;}

// 	answer = "";
// 	Serial.println("_user_prompt: " + _user_prompt);

// 	if (bypass_proc == 1) {
// 		getQwenAnswer(SYS_PROMPT_NO_PROC, _user_prompt, MAIN_MODEL_NAME, &answer, true);
// 	} else if (bypass_proc == 0) {
// 		if (getQwenAnswer(PROC_SYS_PROMPT, _user_prompt, PROC_MODEL_NAME, &proced, false) != 0) {
// 			answer = proced;
// 			return;
// 		} else {
// 			Serial.println("proced: " + proced);
// 			String tmp_ans;
// 			getQwenAnswer(SYS_PROMPT, proced, MAIN_MODEL_NAME, &tmp_ans, true);
// 			if (show_proced) answer = "PROCED: " + proced + "\n" + tmp_ans;
// 			else answer = tmp_ans;
// 		}
// 	}

// 	Serial.print("\r\n--- start answer ---\r\n`");
// 	Serial.println(answer);
// 	Serial.println("`\r\n--- end answer ---");
	
// 	// bypass_proc = 0;
// 	// show_proced = 0;
// }

// // 主循环 (Core 1)
// void loop() {
// 	if (lvgl_mux_lock()) { // 上锁
// 		// Serial.println("lv_timer begin");
// 		lv_timer_handler(); // LVGL任务处理
// 		lvgl_mutex_unlock();
// 	}

// 	if (spi_mux_lock()) {   // 加锁
// 		// Serial.println("spi lock begin");
// #if (LV_COLOR_16_SWAP != 0)
//  		gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
// #else
//  		gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
// #endif
// 		spi_mux_unlock(); // 解锁
// 	}
// 	// Serial.println("spi lock end");
	
// 	my_read_imu();
//   	vTaskDelay(1 / portTICK_PERIOD_MS);
// }

// // 获取 剩余RAM
// void print_heap_free() {
// 	// 查看 片内 RAM 和 PSRAM 剩余堆
// 	Serial.println();
// 	Serial.printf("\r\nFree Heap:\r\n - Heap free: %.2f KB\r\n", esp_get_free_heap_size() / 1024.0);
// 	Serial.printf(" - Internal RAM free: %.2f KB\r\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024.0);
// 	Serial.printf(" - SPI RAM free: %.2f KB\r\n\r\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024.0);
// }

// /**
//  * @brief 向对话历史中添加消息
//  * @param role 消息角色 ("system", "user", "assistant")
//  * @param content 消息内容
//  */
// void addMessageToHistory(const char* role, const String content) {
//     // 如果历史记录已满，则移除最早的一条用户和助手消息
//     if (chat_windows[chat_window_select].chatHistory.size() >= (MAX_MESSAGES * 2 + 1)) {
//         // for (int i = 1; i < messageCount - 2; i++) {
// 		// 	chat_windows[chat_window_select].chatHistory[i] = chat_windows[chat_window_select].chatHistory[i + 2];
// 		// }
// 		chat_windows[chat_window_select].chatHistory.erase(
// 			chat_windows[chat_window_select].chatHistory.begin()+1,
// 			chat_windows[chat_window_select].chatHistory.begin()+2
// 		);
//         // messageCount -= 2;
//     }
//     // 将消息以JSON字符串的形式存入数组
//     // chat_windows[chat_window_select].chatHistory[messageCount++] = String("{\"role\":\"") + role + "\",\"content\":\"" + content + "\"}";
//     chat_windows[chat_window_select].chatHistory.push_back(
// 		String("{\"role\":\"") + role + "\",\"content\":\"" + content + "\"}"
// 	);
//     // messageCount++;
// }

// /**
//  * @brief 构建并发送 HTTP 请求到 API
//  * @param _SYSTEM_PROMPT 系统提示词
//  * @param _userPrompt 用户的当前问题
//  * @param _MAIN_MODEL_NAME 模型名称
//  * @param _response 存储回复内容的字符串指针
//  * @param useHistory 是否使用轮次对话
//  * @return AI的回复内容字符串
//  */
// int8_t getQwenAnswer(const String _SYSTEM_PROMPT, const String _userPrompt, String _MAIN_MODEL_NAME, String* _response, bool useHistory) {
//     if (_response == nullptr) return -1;
// 	if (connecting_wifi) {
// 		*_response = "#b9450f 正在连接 WiFi ";
// 		return -2;
// 	}
// 	if (WiFi.status() != WL_CONNECTED) {
// 		*_response = "#e31919 未连接 WiFi";
// 		return -2;
// 	}

//     WiFiClientSecure client;
//     client.setInsecure(); // 跳过SSL证书验证

//     HTTPClient http;
// 	http.setTimeout(65535);
// 	http.setConnectTimeout(4294967295);
//     http.begin(client, API_ENDPOINT);
//     // http.begin(API_ENDPOINT);
//     http.addHeader("Content-Type", "application/json");
//     http.addHeader("Authorization", "Bearer " + String(apiKey));

// 	// --- 核心：使用 ArduinoJson 构建请求体 ---
// 	JsonDocument doc;

// 	doc["model"] = _MAIN_MODEL_NAME;

// 	if (useHistory) {
// 		// 如果是第一次对话，先加入系统提示词
// 		if (chat_windows[chat_window_select].chatHistory.empty()) {
// 			addMessageToHistory("system", _SYSTEM_PROMPT);
// 		}
		
// 		// 加入用户当前的问题
// 		addMessageToHistory("user", _userPrompt);

// 		// 将历史记录中的每一条消息解析并添加到JSON数组中
// 		// for (int i = 0; i < messageCount; i++) {
// 		for (String tmp : chat_windows[chat_window_select].chatHistory) {
// 			JsonDocument msgDoc;
// 			DeserializationError error = deserializeJson(msgDoc, tmp);
// 			if (!error) doc["input"]["messages"].add(msgDoc.as<JsonObject>());
// 		}
// 	} else {
// 		JsonDocument msgDoc;
// 		msgDoc["role"] = "system";
// 		msgDoc["content"] = _SYSTEM_PROMPT;
// 		doc["input"]["messages"].add(msgDoc.as<JsonObject>());
// 		msgDoc["role"] = "user";
// 		msgDoc["content"] = _userPrompt;
// 		doc["input"]["messages"].add(msgDoc.as<JsonObject>());
// 	}

//     // 将构建好的JSON文档序列化为字符串
//     String jsonPayload;
//     serializeJson(doc, jsonPayload);
//     Serial.printf("开始发送POST请求, 请求体: %s\r\n", jsonPayload.c_str());
   
//     // 发送POST请求
//     int httpResponseCode = http.POST(jsonPayload);
    
// 	response = http.getString();
// 	http.end();

	
// 	if (httpResponseCode > 0){
// 		if (httpResponseCode == 200) {
// 			// Parse JSON response
// 			JsonDocument responseDoc;
//             DeserializationError error = deserializeJson(responseDoc, response);

//             if (!error) {
//                 const char* aiContent = responseDoc["output"]["text"];
// 				// 将AI的回复也加入历史记录
// 				if (useHistory) addMessageToHistory("assistant", String(aiContent));

//                 *_response = String(aiContent);
//                 return 0;
//             } else {
//                 *_response = "JSON解析失败, Response: \r\n" + response;
// 				Serial.println();
// 				Serial.println(response);
// 				Serial.println();
// 				return -3;
//             }
// 		} else {
// 			*_response = "响应码: " + String(httpResponseCode) + "\r\nResponse: \r\n" + response;
// 			Serial.println();
// 			Serial.println(response);
// 			Serial.println();
// 			return -5;
// 		}
// 	} else {
// 		*_response = "请求失败, ";
// 		if (httpResponseCode == HTTPC_ERROR_TOO_LESS_RAM) {
// 			*_response += "内存不足";
// 		} else if (httpResponseCode == HTTPC_ERROR_READ_TIMEOUT) {
// 			*_response += "读取超时";
// 		} else if (httpResponseCode == HTTPC_ERROR_CONNECTION_REFUSED) {
// 			*_response += "连接被拒绝";
// 		} else if (httpResponseCode == HTTPC_ERROR_CONNECTION_LOST) {
// 			*_response += "连接丢失";
// 		} else {
// 			*_response += "错误码: " + String(httpResponseCode);
// 		}
// 		return -5;
// 	}
// }

// // 重置对话历史
// void reset_chat_history() {
//     // messageCount = 0;
//     // memset(chat_windows[chat_window_select].chatHistory, 0, sizeof(chatHistory));
//     chat_windows[chat_window_select].chatHistory.clear();
// }

// // 修改 main_label 的文本
// void main_label_add_text(String print_text) {
// 	main_label_text += print_text;
// 	if (lvgl_mux_lock()) { // 上锁
// 		lv_label_set_text(main_label, main_label_text.c_str());
// 		lvgl_mutex_unlock();
// 	}
// }
// void main_label_set_text(String print_text) {
// 	main_label_text = print_text;
// 	if (lvgl_mux_lock()) { // 上锁
// 		lv_label_set_text(main_label, main_label_text.c_str());
// 		lvgl_mutex_unlock();
// 	}
// }

// // // 鼠标回调
// // void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data) {
// // 	data->point.x = touchpad_x + 0.5;
// // 	data->point.y = touchpad_y + 0.5;
// // 	if(digitalRead(0) == 0) {
// // 		// Serial.println("PRESSED");
// // 		data->state = LV_INDEV_STATE_PRESSED;
// // 	} else {
// // 		// Serial.println("Released");
// // 		data->state = LV_INDEV_STATE_RELEASED;
// // 	}
// // }

// // 读取 IMU 数据, 并 更新滚动位置
// void my_read_imu() {
//     IMU.update();

//     IMU.getAccel(&accelData); // accelData.accelX accelData.accelY accelData.accelZ
//     // IMU.getGyro(&gyroData); // gyroData.gyroX gyroData.gyroY) gyroData.gyroZ

//     // Serial.println(IMU.getTemp());

//     if (lvgl_mux_lock()) { // 上锁
// 		lv_obj_scroll_by_bounded(main_panel, 0, 0-accelData.accelY*10, LV_ANIM_ON);
// 		lvgl_mutex_unlock();
// 	}

//     // touchpad_x -= accelData.accelX * 8;
// 	// touchpad_y -= accelData.accelY * 8;
// 	// if (touchpad_x < 0) touchpad_x = 0;
// 	// if (touchpad_x > 319) touchpad_x = 319;
// 	// if (touchpad_y < 0) touchpad_y = 0;
// 	// if (touchpad_y > 239) touchpad_y = 239;
// 	// Serial.println(touchpad_x);Serial.print(",");Serial.println(touchpad_y);
// 	// Serial.println(accelData.accelX);Serial.print(",");Serial.println(accelData.accelY);
// }

// static void ta_event_cb(lv_event_t * e) {
// 	lv_event_code_t code = lv_event_get_code(e);
// 	lv_obj_t * ta = (lv_obj_t *)lv_event_get_target(e);
// 	lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);	
// 	if(code == LV_EVENT_FOCUSED) {
// 		/* 1. 恢复键盘正常高度 (自动计算或设具体值)，确保虚拟按键可见 */
// 		lv_obj_set_height(kb, LV_SIZE_CONTENT); 
// 		// 或者如果你只想显示候选栏，在这里判断是否需要显示全键盘
//         // 但通常聚焦时需要全键盘以便输入
        
//         /* 2. 关联文本框并显示 */
//         lv_keyboard_set_textarea(kb, ta);
//         lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        
//         /* 3. (可选) 如果确实只需要候选栏，可以在这里调整，但不要固定为 10% 导致无法恢复 */
//         // lv_obj_set_style_height(kb, LV_PCT(10), 0); // <--- 删除这行，防止变矮
//     }
//     else if(code == LV_EVENT_CANCEL) {
// 		/* 隐藏键盘 */
//         lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_state(ta, LV_STATE_FOCUSED);
//         lv_indev_reset(NULL, ta);
//     }
// }

// /* 物理按键输入接口 */
// void send_key_to_ta(uint32_t key) {
// 	// 确保文本框有焦点
//     if(g_ta) {
// 		if (lvgl_mux_lock()) { // 上锁
// 			lv_obj_add_state(g_ta, LV_STATE_FOCUSED);
// 			// 发送给键盘 (g_kb)
// 			if(g_kb) {
// 				// Serial.printf("send_key_to_ta: g_kb: %c\r\n", (char)key);
// 				// 向文本框发送 KEY 事件
// 				lv_event_send(g_ta, LV_EVENT_KEY, &key);
// 			} else Serial.println("send_key_to_ta: g_kb is NULL!");
			
// 			lvgl_mutex_unlock();
// 		}
// 	} else Serial.println("send_key_to_ta: g_ta is NULL!");
// }

// // 全部初始化
// void hardware_init() {
// 	// 鼠标左键 (没有右键)
//     pinMode(0, INPUT_PULLUP);
// // 
// // 	pinMode(RECORD_BTN, INPUT_PULLDOWN);
// // 	pinMode(SEND_BTN, INPUT_PULLDOWN);

// 	// 初始化 SD 卡
// 	init_SDcard();
	
// 	// 初始化 Display
// 	if (spi_mux_lock()) {   // 加锁
// #ifdef GFX_EXTRA_PRE_INIT
//     	GFX_EXTRA_PRE_INIT();
// #endif
// 		if (!gfx->begin()) {
// 			Serial.println("gfx begin failed!");
// 			while (true) vTaskDelay(10000);
// 		}
// 		gfx->fillScreen(BLACK);
// 		spi_mux_unlock(); // 解锁
// 	}

// 	vTaskDelay(70 / portTICK_PERIOD_MS);

// #if (EXAMPLE_PIN_NUM_LCD_BL >= 0)
// 	ledcSetup(LEDC_CHANNEL , LEDC_FREQ, LEDC_TIMER_10_BIT);
// 	ledcAttachPin(EXAMPLE_PIN_NUM_LCD_BL, LEDC_CHANNEL);
// 	ledcWrite(LEDC_CHANNEL , ledc_duty ? (1 << LEDC_TIMER_10_BIT) / 100 * ledc_duty : 0);
// 	// pinMode(EXAMPLE_PIN_NUM_LCD_BL, OUTPUT);
// 	// digitalWrite(EXAMPLE_PIN_NUM_LCD_BL, HIGH);
// #endif

// 	// 初始化 触摸, 本人没有
// 	// touch_init(gfx->width(), gfx->height(), gfx->getRotation());
  	
// 	// 初始化 LVGL
// 	lv_init();
// #if LV_USE_LOG != 0
// 	lv_log_register_print_cb(my_print); /* register print function for debugging */
// #endif
// 	screenWidth = gfx->width();
// 	screenHeight = gfx->height();
// 	bufSize = screenWidth * screenHeight;

// 	uint32_t malloc_local = MALLOC_CAP_INTERNAL;
// 	if (psramFound()) malloc_local = MALLOC_CAP_SPIRAM;
// 	disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, malloc_local | MALLOC_CAP_8BIT);
// 	if (!disp_draw_buf) {
// 		// remove MALLOC_CAP_INTERNAL flag try again
// 		Serial.println("Remove MALLOC_CAP_INTERNAL flag try again!");
// 		disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
// 	}
// 	if (!disp_draw_buf) {
// 		Serial.println("LVGL disp_draw_buf allocate failed!");
// 		while (true) vTaskDelay(10000 / portTICK_PERIOD_MS);
// 	} else {
// 		lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);

// 		// 初始化 display 设备
// 		lv_disp_drv_init(&disp_drv);
// 		disp_drv.hor_res = screenWidth;
// 		disp_drv.ver_res = screenHeight;
// 		disp_drv.flush_cb = my_disp_flush;
// 		disp_drv.draw_buf = &draw_buf;
// 		disp_drv.direct_mode = true;

// 		lv_disp_drv_register(&disp_drv);
// 	}

// 	// 初始化 IMU
// 	Wire.begin(48, 47);
//   	Wire.setClock(400000); // 400 kHz clock
// 	int err = IMU.init({0}, IMU_ADDRESS);
// 	if (err != 0) {
// 		Serial.println("Error initializing IMU: " + err);
// 		// while (true) vTaskDelay(10000);
// 	}

	
// 	// // 初始化 鼠标设备(IMU + GPIO 0 模拟的) 与 鼠标指针 
// 	// static lv_indev_drv_t indev_drv;
// 	// lv_indev_drv_init(&indev_drv);
// 	// indev_drv.type = LV_INDEV_TYPE_POINTER;
// 	// indev_drv.read_cb = my_input_read;
// 	// lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);
// 	// lv_obj_t *cursor_img = lv_img_create(lv_scr_act());
// 	// lv_img_set_src(cursor_img, LV_SYMBOL_OK);
// 	// lv_indev_set_cursor(mouse_indev, cursor_img);
// }

// // 列出目录下的所有文件和子目录
// void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
// 	Serial.printf("List directory: %s\r\n", dirname);

// 	File root = fs.open(dirname);
// 	if (!root) {
// 		Serial.println("Failed to open directory");
// 		return;
// 	}
// 	if (!root.isDirectory()) {
// 		Serial.println("Not a directory");
// 		return;
// 	}

// 	File file = root.openNextFile();
// 	while (file) {
// 		if (file.isDirectory()) {
// 			Serial.print("  Dir : ");
// 			Serial.println(file.name());
// 			if (levels) {
// 				listDir(fs, file.path(), levels - 1);
// 			}
// 		} else {
// 			Serial.print("  File: ");
// 			Serial.print(file.name());
// 			Serial.print("  Size: ");
// 			Serial.println(file.size());
// 		}
// 		file = root.openNextFile();
// 	}
// 	Serial.println();
// }

// // 初始化 SD 卡
// void init_SDcard() {
// 	if (spi_mux_lock()) {   // 加锁
// 		SPIClass sd_spi_bus(FSPI);
// 		sd_spi_bus.begin(SD_SCK, SD_MISO, SD_MOSI, -1);
// 		if (!SD.begin(SD_CS, sd_spi_bus)) {
// 			Serial.println("Card Mount Failed");
// 			return;
// 		}
// 		uint8_t cardType = SD.cardType();

// 		if (cardType == CARD_NONE) {
// 			Serial.println("No SD card attached");
// 			return;
// 		}

// 		Serial.print("SD Card Type: ");
// 		if (cardType == CARD_MMC) {
// 			Serial.println("MMC");
// 		} else if (cardType == CARD_SD) {
// 			Serial.println("SDSC");
// 		} else if (cardType == CARD_SDHC) {
// 			Serial.println("SDHC");
// 		} else {
// 			Serial.println("UNKNOWN");
// 		}

// 		Serial.printf("Total space: %lluMiB\r\n",  SD.totalBytes() / (1024 * 1024));
// 		Serial.printf("Used space: %lluMiB\r\n",   SD.usedBytes()  / (1024 * 1024));
		
// 		listDir(SD, "/", 2);

// 		sd_text = "";
// 		File file = SD.open("/lcd-t.txt");
// 		if (file.available()) {
// 			while (file.available()) sd_text += (char)file.read();
// 			file.close();
// 		} else Serial.println("Error opening lcd-t.txt");

// 		spi_mux_unlock();   // 解锁
// 	}
// }


// // 初始化 I2S
// void setupI2S() {
// 	Serial.print("Setup I2S ...");
// 	esp_err_t err;

// 	// i2s_install();
// 	const i2s_config_t i2s_config = {
// 		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
// 		.sample_rate = SAMPLE_RATE,
// 		.bits_per_sample = i2s_bits_per_sample_t(16),
// 		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
// 		.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
// 		.intr_alloc_flags = 0, // default interrupt priority
// 		.dma_buf_count = 8,
// 		.dma_buf_len = 1024,
// 		.use_apll = false
// 	};

// 	err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
// 	if (err != ESP_OK) {
// 		Serial.printf("I2S driver install failed (I2S_PORT): %d\r\n", err);
// 		while (true);
// 	}
	
// 	const i2s_pin_config_t pin_config = {
// 		.bck_io_num = I2S_SCK,
// 		.ws_io_num = I2S_WS,
// 		.data_out_num = I2S_PIN_NO_CHANGE,
// 		.data_in_num = I2S_SD
// 	};
	
// 	err = i2s_set_pin(I2S_PORT, &pin_config);
// 	if (err != ESP_OK) {
// 		Serial.printf("I2S set pin failed (I2S_PORT): %d\r\n", err);
// 		while (true);
// 	}
// 	err = i2s_start(I2S_PORT);
// 	if (err != ESP_OK) {
// 		Serial.printf("I2S start failed (I2S_PORT): %d\r\n", err);
// 		while (true);
// 	}
// 	Serial.println("OK!");
// }

// // 构造讯飞ws连接url
// String make_XF_WSurl(const char *Secret, const char *Key, String request, String host) {
// 	// 获取当前时间
// 	char buffer[80];
// 	if(!getLocalTime(&timeinfo)){
// 		Serial.println("获取时间失败");
// 		return "GMT";
// 	}
// 	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
// 	String timeString = buffer;

// 	// 构造原字符串
// 	String signature_origin = "host: " + host;
// 	signature_origin += "\n";
// 	signature_origin += "date: ";
// 	signature_origin += timeString;
// 	signature_origin += "\n";
// 	signature_origin += "GET " + request + " HTTP/1.1";
	
// 	// 使用 mbedtls 计算 HMAC-SHA256
// 	unsigned char hmacResult[32]; // SHA256 产生的哈希结果长度为 32 字节
// 	mbedtls_md_context_t ctx;
// 	mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
// 	mbedtls_md_init(&ctx);
// 	mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1); // 1 表示 HMAC
// 	mbedtls_md_hmac_starts(&ctx, (const unsigned char *)Secret, strlen(Secret));
// 	mbedtls_md_hmac_update(&ctx, (const unsigned char *)signature_origin.c_str(), signature_origin.length());
// 	mbedtls_md_hmac_finish(&ctx, hmacResult);
// 	mbedtls_md_free(&ctx);
// 	// 对结果进行 Base64 编码
// 	String base64Result = base64::encode(hmacResult, 32);
	
// 	String authorization_origin = "api_key=\"";
// 	authorization_origin += Key;
// 	authorization_origin += "\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"";
// 	authorization_origin += base64Result;
// 	authorization_origin += "\"";
// 	String authorization = base64::encode(authorization_origin);
	
// 	String url = "ws://" + host + request;
// 	url += "?authorization=";
// 	url += authorization;
// 	url += "&date=";

// 	// 替换空格为 "+"
// 	timeString.replace(" ", "+");
// 	timeString.replace(",", "%2C");
// 	timeString.replace(":", "%3A");
// 	url += timeString;
	
// 	url += "&host=" + host;
// 	return url;
// }
 
// // 向讯飞STT发送音频数据
// void STTsend() {
// 	uint8_t status = 0;
// 	int dataSize = 1280 * 8;
// 	int audioDataSize = recordingSize * 2;
// 	uint lan = (audioDataSize) / dataSize;
// 	uint lan_end = (audioDataSize) % dataSize;
// 	if (lan_end > 0) {
// 		lan++;
// 	}
	
// 	String host_url = make_XF_WSurl(STTAPISecret, STTAPIKey, "/v2/iat", "ws-api.xfyun.cn");
// 	Serial.print("Connecting to server ... ");
// 	bool connected = client.connect(host_url);
// 	if (connected) {
// 		Serial.println("OK");
// 	} else {
// 		Serial.println("Not Connected!");
// 		return;
// 	}
// 	// 分段向STT发送PCM音频数据
// 	for (int i = 0; i < lan; i++) { 
// 		if (i == (lan - 1)) {
// 			status = 2;
// 		}
// 		if (status == 0) {
// 			String input = "{";
// 			input += "\"common\":{ \"app_id\":\"" STTAPPID "\" },";
// 			input += "\"business\":{\"domain\": \"iat\", \"language\": \"zh_cn\", \"accent\": \"mandarin\", \"vinfo\":1,\"vad_eos\":10000},";
// 			input += "\"data\":{\"status\": 0, \"format\": \"audio/L16;rate=16000\",\"encoding\": \"raw\",\"audio\":\"";
// 			String base64audioString = base64::encode((uint8_t *)pcm_data, dataSize);
// 			input += base64audioString;
// 			input += "\"}}";
// 			client.send(input);
// 			status = 1;
// 		}
// 		else if (status == 1) {
// 			String input = "{";
// 			input += "\"data\":{\"status\": 1, \"format\": \"audio/L16;rate=16000\",\"encoding\": \"raw\",\"audio\":\"";
// 			String base64audioString = base64::encode((uint8_t *)pcm_data + (i * dataSize), dataSize);
// 			input += base64audioString;
// 			input += "\"}}";
// 			client.send(input);
// 		}
// 		else if (status == 2) {
// 			if (lan_end == 0) {
// 				String input = "{";
// 				input += "\"data\":{\"status\": 2, \"format\": \"audio/L16;rate=16000\",\"encoding\": \"raw\",\"audio\":\"";
// 				String base64audioString = base64::encode((uint8_t *)pcm_data + (i * dataSize), dataSize);
// 				input += base64audioString;
// 				input += "\"}}";
// 				client.send(input);
// 			}
// 			if (lan_end > 0) {
// 				String input = "{";
// 				input += "\"data\":{\"status\": 2, \"format\": \"audio/L16;rate=16000\",\"encoding\": \"raw\",\"audio\":\"";
// 				String base64audioString = base64::encode((uint8_t *)pcm_data + (i * dataSize), lan_end);
// 				input += base64audioString;
// 				input += "\"}}";
// 				client.send(input);
// 			}
// 		}
// 		vTaskDelay(20 / portTICK_PERIOD_MS);
// 	}
// }

// // 0x00197fff

