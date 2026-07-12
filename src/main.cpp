

#include "app_common.hpp"
#include "tools_func.hpp"


// 初始化 WiFi 并 启动按键接收循环 (Core 0)
void my_loop(void *param) {
	connect_wifi();

	if (lvgl_mux_lock()) { // 上锁
		lv_textarea_set_placeholder_text(ta, "请输入");
		lvgl_mutex_unlock(); // 解锁
	}

	// 初始化 SD 卡
	init_SDcard();

	// 初始化 I2S
	setupI2S();

	// 初始化 拼音解码器
	zh_pinyin_begin();

	// 注册 WebSocket消息回调
	client.onMessage(ws_callback);

	Serial.println("Setup done");
	print_heap_free("初始化完毕");

	// 主循环 (Core 0)
	while (1) {
		proc_key = wait_until_read_key(); // 读取按键输入
		Serial.println("Key: " + proc_key);
		
		// 顶部功能键
		/* $1-$9 */if (proc_key.length() >= 2 && proc_key.startsWith("$") && proc_key.substring(1).toInt() > 0 && proc_key.substring(1).toInt() <= TEXT_SHORTCUT_SIZE) {
			ta_add_text(TEXT_SHORTCUT[ proc_key.substring(1).toInt() - 1 ]);
		} else if (proc_key == "$10") {
			reset_chat_history();
		} else if (proc_key == "$11") {
			ta_set_text("");
		/* 获取结果 */} else if (proc_key == "$12") {
			if (lvgl_mux_lock()) { // 上锁
				user_prompt = lv_textarea_get_text(ta);
				if (!user_prompt.startsWith("-scr")) {
					scroll_main_p(0, 0);
				}

				lvgl_mutex_unlock(); // 解锁
			}

			if (!user_prompt.startsWith("-scr")) {
				Serial.println("等待结果中 ...");
				main_label_set_text("#7e00d2 等待结果中 ... #");
			}

			getAnswer(user_prompt);
			if (answer.length()) main_label_set_text(answer.c_str()); // answer为空则不更新
			if (!user_prompt.startsWith("-scr")) {
				scroll_main_p(0, 0);
			}
		/* $S1-$S9 */} else if (proc_key.length() >= 3 && proc_key.startsWith("$S") && proc_key.substring(2).toInt() > 0 && proc_key.substring(2).toInt() <= MAX_CHAT_WINDOW) {
			if (lvgl_mux_lock()) { // 上锁
				// 保存当前窗口 所有状态
				current_window.ta_text_save = lv_textarea_get_text(ta);
				current_window.main_label_text_save = lv_label_get_text(main_label);
				current_window.ta_pos = lv_textarea_get_cursor_pos(ta);
				current_window.main_label_pos = lv_obj_get_scroll_y(main_panel);
				
				lv_textarea_set_text(ta, 
					(String("当前对话窗口: ") + (uint16_t)(chat_window_select+1) + 
					" -> " + proc_key.substring(2).toInt()).c_str()
				);
				
				chat_window_select = proc_key.substring(2).toInt() - 1;

				// 切换到新窗口 main_label的状态
				lv_label_set_text(main_label, current_window.main_label_text_save.c_str());
				lv_obj_scroll_to_y(main_panel, current_window.main_label_pos, LV_ANIM_ON);
				Serial.println(current_window.main_label_pos);
				lvgl_mutex_unlock(); // 解锁
			}

			vTaskDelay(700 / portTICK_PERIOD_MS);
			
			// 切换到新窗口 ta的状态
			ta_tmp_recover();
		}
		// 中部功能键
		  else if (proc_key == "BCK") {
			if (typing_pinyin && pinyin_str.length()) { // 拼音输入模式
				candidate_offset = 0;
				pinyin_str = pinyin_str.substring(0, pinyin_str.length() - 1);
				if (lvgl_mux_lock()) { // 上锁
					lv_label_set_text(pinyin_input_l, pinyin_str.c_str());
					if (!pinyin_str.length()) clear_pinyin(); // pinyin_str为空，清空所有变量
					
					lvgl_mutex_unlock(); // 解锁
				}
				if (pinyin_str.length()) update_word_match();
			} else { // 正常输入模式
				send_key_to_ta(LV_KEY_BACKSPACE);
			}
		} else if (proc_key == "DEL") {
			send_key_to_ta(LV_KEY_DEL);
		} else if (proc_key == "ETR") {
			if (typing_pinyin) { // 拼音输入模式: 将 拼音输入框 的文本 转移到 文本文本框(ta), 并关闭 拼音输入模式
				if (lvgl_mux_lock()) { // 上锁
					lv_textarea_add_text(ta, lv_label_get_text(pinyin_input_l));
					clear_pinyin();

					lvgl_mutex_unlock(); // 解锁
				}
			} else { // 正常输入模式
				send_key_to_ta(LV_KEY_ENTER);
			}
		}
		// 方向键
		  else if (proc_key == "UP") {
			send_key_to_ta(LV_KEY_UP);
		} else if (proc_key == "DOWN") {
			send_key_to_ta(LV_KEY_DOWN);
		} else if (proc_key == "LEFT") {
			send_key_to_ta(LV_KEY_LEFT);
		} else if (proc_key == "RIGT") {
			send_key_to_ta(LV_KEY_RIGHT);
		}
		// 底部功能键
		/* ASR */   else if (proc_key == "&1") {
			scroll_main_p();
			run_asr("&1");
			scroll_main_p();
		/* WAV */ } else if (proc_key == "&2") {
			scroll_main_p();
			record_wav("&2");
			scroll_main_p();
		/*     */ } else if (proc_key == "&3") {
		/* 计算器 */}else if (proc_key == "&4") {
			ta_tmp_show((calc_mode = !calc_mode) ? "计算器模式: 1" : "计算器模式: 0");
		
		/* 拼音 */ } else if (proc_key == "&5") {
			if (lvgl_mux_lock()) { // 上锁
				if ((typing_pinyin = !typing_pinyin)) { // 切换到了 拼音输入模式
					clear_pinyin();
					lv_obj_clear_flag(candidate_l, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(pinyin_input_l, LV_OBJ_FLAG_HIDDEN);
				} else { // 退出了 拼音输入模式
					lv_obj_add_flag(candidate_l, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(pinyin_input_l, LV_OBJ_FLAG_HIDDEN);
					clear_pinyin();
				}
				lvgl_mutex_unlock(); // 解锁
			}
		/* 拼音预处理 */}else if (proc_key == "&S1") {
			ta_tmp_show((use_proc = !use_proc) ? "拼音预处理: 1" : "拼音预处理: 0");
		/* 显示处理结果 */}else if (proc_key == "&$2") {
			ta_tmp_show((show_proced = !show_proced) ? "显示处理结果: 1" : "显示处理结果: 0");
		/* 其他 */ } else { // 处理其他输入的键
			proc_other_input_key();
		}

		// if (digitalRead(BTN_PIN_1) == HIGH) {
		// 	vTaskDelay(15 / portTICK_PERIOD_MS);
		// 	if (digitalRead(BTN_PIN_1) == LOW) continue;
		// 	Serial.println("Recording");
		// }

		core0_loop_func();
	}
	main_label_set_text("循环已终止");
	vTaskDelete(NULL); // 防止循环终止
}

// 根据情况发送不同请求
void getAnswer(String& _user_prompt) {
	print_heap_free("getAnswer 前");
	// 输入为空
	if (_user_prompt == "") {
		Serial.println("输入为空");
		answer = "输入为空";
		return;
	} 
	// 查看 时间 (自动刷新)
	if (_user_prompt == "-t") {
		if (syncing_sntp) {
			answer = "#bb5a14 正在同步时间 "; return;
		}

		char buffer[80];

		while (1) {
			if(!getLocalTime(&timeinfo)){
				Serial.println("获取时间失败");
				answer = "获取时间失败";
				return;
			}
			strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S\r\n", &timeinfo);
			main_label_set_text(buffer);
			vTaskDelay(100 / portTICK_PERIOD_MS);
			if (read_key() != "") {
				main_label_set_text("#db6319 已停止刷新 #\r\n");
				main_label_add_text(answer.c_str());
				break;
			}
		}
		answer = ""; return;
	}
	// 查看 IMU信息 (自动刷新)
	if (_user_prompt == "-imu") {
		while (1) {
			IMU.update();
			IMU.getAccel(&accelData); // accelData.accelX accelData.accelY accelData.accelZ
			IMU.getGyro(&gyroData);   // gyroData.gyroX   gyroData.gyroY   gyroData.gyroZ

			answer = "IMU 温度: " + String(IMU.getTemp());
			answer += "\r\n加速度计: \n - X: " + String(accelData.accelX) + "\r\n - Y: " + String(accelData.accelY) + "\r\n - Z: " + String(accelData.accelZ);
			answer += "\r\n陀螺仪: \n - X: " + String(gyroData.gyroX) + "\r\n - Y: " + String(gyroData.gyroY) + "\r\n - Z: " + String(gyroData.gyroZ);
			main_label_set_text(answer.c_str());
			vTaskDelay(50 / portTICK_PERIOD_MS);
			if (read_key() != "") {
				main_label_set_text("#db6319 已停止刷新 #\r\n");
				main_label_add_text(answer.c_str());
				break;
			}
		}
		answer = ""; return;
	}
	// 查看 运行信息 (自动刷新)
	if (_user_prompt == "-i") {
		while (1) {
			answer = "CPU 温度: " + String(temperatureRead());
			// answer += "\r\n芯片版本: " + String(ESP.getChipRevision());
			answer += "\r\nCPU 频率: " + String(ESP.getCpuFreqMHz()) + " MHz";
			// answer += "\r\n循环计数: " + String(ESP.getCycleCount());
			
			answer += "\r\n堆 容量: " + String(ESP.getHeapSize() / 1024.0) + " KiB";
			answer += "\r\n堆 空闲: " + String(ESP.getFreeHeap() / 1024.0) + " KiB";
			answer += "\r\n堆 最大分配: " + String(ESP.getMaxAllocHeap() / 1024.0) + " KiB";
			answer += "\r\nPSRAM 容量: " + String(ESP.getPsramSize() / 1024.0 / 1024.0, 3) + " MiB";
			answer += "\r\nPSRAM 空闲: " + String(ESP.getFreePsram() / 1024.0 / 1024.0, 3) + " MiB";
			
			answer += "\r\nFlash 模式: ";
			switch (ESP.getFlashChipMode()) {
				case FM_QIO:
					answer += "QIO";
					break;
				case FM_QOUT:
					answer += "QOUT";
					break;
				case FM_DIO:
					answer += "DIO";
					break;
				case FM_DOUT:
					answer += "DOUT";
					break;
				case FM_FAST_READ:
					answer += "FAST_READ";
					break;
				case FM_SLOW_READ:
					answer += "SLOW_READ";
					break;
				case FM_UNKNOWN:
					answer += "UNKNOWN";
					break;
				default:
					answer += "UNKNOWN";
					break;
			}
			answer += "\r\nFlash 大小: " + String(ESP.getFlashChipSize() / 1024.0 / 1024.0) + " MiB";
			answer += "\r\nFlash 频率: " + String(ESP.getFlashChipSpeed() / 1000000.0) + " MHz";
			
			answer += "\r\nSDK 版本: " + String(ESP.getSdkVersion());

			main_label_set_text(answer.c_str());
			vTaskDelay(200 / portTICK_PERIOD_MS);
			if (read_key() != "") {
				main_label_set_text("#db6319 已停止刷新 #\r\n");
				main_label_add_text(answer.c_str());
				break;
			}
		}
		answer = ""; return;
	}
	// 设置 背光亮度
	if (_user_prompt.startsWith("-bl=")) {
		bl_duty = _user_prompt.substring(_user_prompt.indexOf("=") + 1).toDouble();
		set_bl_duty();
		answer = "背光亮度: " + String(bl_duty); return;
	}
	// 重启
	if (_user_prompt == "-rst") { esp_restart(); }
	// 深度睡眠
	if (_user_prompt == "-ds") { esp_deep_sleep_start(); }
	// 深度睡眠 直到 WAKEUP_BTN == HIGH
	if (_user_prompt == "-dsg") {
		esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKEUP_BTN, HIGH);
		esp_deep_sleep_start();
	}
	// 深度睡眠 __ms
	if (_user_prompt.startsWith("-dst=") && _user_prompt.length() > 5) { // -dst=1000 -> 休眠 1 秒
		uint64_t time_us = (_user_prompt.substring(_user_prompt.indexOf("=") + 1)).toInt() * 1000;
		esp_deep_sleep(time_us);
	}
	// 读取 SD卡
	if (_user_prompt.startsWith("-sd")) {
		if (sd_status != nullptr) {
			answer = sd_status;
			return; // 如有错误 则返回错误原因
		}

		// -sd=xxx -> 读取 SD_PREFIX目录下 xxx 文件
		String read_file_name = "";
		if (_user_prompt.indexOf('=') == 3 && _user_prompt.length() > 4) { 
			read_file_name = SD_PREFIX + _user_prompt.substring(_user_prompt.indexOf("=") + 1);
		}
		// -sd|xxx -> 读取 SD 卡根目录下 xxx 文件
		else if (_user_prompt.indexOf('|') == 3 && _user_prompt.length() > 4) { 
			read_file_name = "/" + _user_prompt.substring(_user_prompt.indexOf("|") + 1);
		}

		// 解析 要归递列出的层数
		// -sd/x -> 归递列出 SD卡根目录下 x层 目录与文件
		// -sd/  -> 归递列出 SD卡根目录下 3层 目录与文件
		else if (_user_prompt.indexOf('/') == 3) {
			uint8_t levels = 3;
			if (_user_prompt.length() > 4) { 
				levels = (_user_prompt.substring(_user_prompt.indexOf('/') + 1)).toInt();
				Serial.println("归递列出 SD卡根目录下 " + String(levels) + "层 目录与文件");
			}
			if (spi_mux_lock()) {   // 加锁
				listDir(SD, "/", levels, answer);
				spi_mux_unlock(); // 解锁
			}
			return;
		}
		// -sd#x -> 归递列出 SD_PREFIX下 x层 目录与文件
		// -sd#  -> 归递列出 SD_PREFIX下 3层 目录与文件
		else if (_user_prompt.indexOf('#') == 3) {
			uint8_t levels = 3;
			if (_user_prompt.length() > 4) { 
				levels = (_user_prompt.substring(_user_prompt.indexOf('#') + 1)).toInt();
			}
			Serial.println("归递列出 SD卡 " SD_PREFIX " 下 " + String(levels) + "层 目录与文件");
			
			if (spi_mux_lock()) {   // 加锁
				listDir(SD, SD_PREFIX, levels, answer);
				spi_mux_unlock(); // 解锁
			}
			return;
		}

		// 解析 要读取的文件名
		// -sd-xx -> 读取 SD_PREFIX目录下 textxx.txt 文件
		else if (_user_prompt.indexOf('-') == 3 && _user_prompt.length() > 4) {
			read_file_name = "text" + _user_prompt.substring(_user_prompt.indexOf('-') + 1) + ".txt";
		}
		// -sd -> 读取 SD_PREFIX下 text1.txt 文件
		else {
			read_file_name = SD_PREFIX "text1.txt";
		}
		
		// 读取 SD卡文件内容
		if (spi_mux_lock()) {   // 加锁
			File file = SD.open(read_file_name, FILE_READ);
			if (file.available()) {
				answer = "";
				while (file.available()) answer += (char)file.read();
				file.close();
			} else {
				Serial.printf("无法读取 %s", read_file_name.c_str());
				answer = "无法读取 " + read_file_name;
			}
			spi_mux_unlock(); // 解锁
		}
		return;
	}
	// 连接 WiFi
	if (_user_prompt == "-wifi") {
		connect_wifi();
		answer = ""; return;
	}
	// 同步 SNTP 时间
	if (_user_prompt == "-sntp") {
		sync_sntp();
		answer = ""; return;
	}
	// UART模式
	if (_user_prompt.startsWith("-ut")) {
		// uint8_t rx_pin = _user_prompt.substring(_user_prompt.indexOf("=") + 1);
		// uint8_t tx_pin = _user_prompt.substring(_user_prompt.indexOf("=") + 1);
		Serial2.begin(DEFAULT_BAUD, SERIAL_8N1, DEFAULT_RX_PIN, DEFAULT_TX_PIN); // 初始化 串口
		char tmp[120];
		snprintf(tmp, sizeof(tmp), "#10b166 UART 已连接: # \r\n#10b186 %d,8N1, RX=%d, TX=%d # \r\n#d6cc14 按 $12 退出 UART # \r\n", 
			DEFAULT_BAUD, DEFAULT_RX_PIN, DEFAULT_TX_PIN);
		main_label_set_text(tmp);
		vTaskDelay(80 / portTICK_PERIOD_MS);
		char c_tmp;
		String str_tmp;
		while (1) {
			if (Serial2.available()) {
				c_tmp = (char)Serial2.read();
				main_label_add_text(&c_tmp);
				// Serial.println("UART2 接收: " + String(c_tmp));
			}
			str_tmp = read_key();
			if (str_tmp == "$12") {
				break;
			} else {
				Serial2.write(str_tmp.c_str());
				// Serial.println("UART2 发送: " + str_tmp);
			}
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
		main_label_set_text("#10b166 UART 已断开");
		answer = ""; return;
	}
	// 滚动 主文本框 
	if (_user_prompt.startsWith("-scr")) {
		// -scr -> 滚动 主文本框 到y=0
		int16_t scroll_to = 0;
		// -scr=xx -> 滚动 主文本框 到y=xx
		if (_user_prompt.startsWith("-scr=") && _user_prompt.length() > 5) {
			scroll_to = _user_prompt.substring(_user_prompt.indexOf('=') + 1).toInt();
		}
		scroll_main_p(scroll_to);
		answer = ""; return;
	}

	// 快捷文本
	if (_user_prompt == "-t-1") { answer = ""; return; }
	if (_user_prompt == "-t-2") { answer = ""; return; }
	if (_user_prompt == "-t-3") { answer = ""; return; }

	Serial.println("_user_prompt: " + _user_prompt);

	if (calc_mode == 1) {  // 计算器模式
		double result;
        String errMsg;  // 用于接收错误信息的字符串
        bool success = calculate(_user_prompt, result, errMsg);
        
        if (success) {
            Serial.println("输入: " + _user_prompt);
            Serial.println("结果: " + String(result, 4)); // 输出结果，保留4位小数
			answer = "结果: " + String(result, 4);
        } else {
            // 如果失败，统一输出累积的错误信息
            Serial.println("输入: " + _user_prompt);
            Serial.print(errMsg);
			answer = errMsg;
        }
	} else if (use_proc == 1) {  // 使用 拼音预处理
		String proced;
		if (getAPIanswer(PROC_SYS_PROMPT, _user_prompt, PROC_MODEL_NAME, proced, false) != 0) {
			answer = proced;
			return;
		} else {
			Serial.println("处理后: " + proced);
			String tmp_ans;
			getAPIanswer(SYS_PROMPT_WITH_PROC, proced, MAIN_MODEL_NAME, tmp_ans, true);
			if (show_proced) answer = "处理后: " + proced + "\n" + tmp_ans;
			else answer = tmp_ans;
		}
	} else  if (use_proc == 0) {  // 默认模式
		getAPIanswer(SYS_PROMPT_NO_PROC, _user_prompt, MAIN_MODEL_NAME, answer, true);
	}

	Serial.print("\r\n--- start answer ---\r\n");
	Serial.println(answer);
	Serial.println("\r\n--- end answer ---");
	
	// use_proc = 0;
	// show_proced = 0;
	// calc_mode   = 0;
}

// 主循环 (Core 1)
void loop() {
	if (lvgl_mux_lock()) { // 上锁
		lv_timer_handler(); // LVGL任务处理
		lvgl_mutex_unlock();
	}

// 	if (spi_mux_lock()) {   // 加锁
// #if (LV_COLOR_16_SWAP != 0)
//  		gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
// #else
//  		gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
// #endif
// 		spi_mux_unlock(); // 解锁
// 	}
	
	my_read_imu();
  	vTaskDelay(1 / portTICK_PERIOD_MS);
}

// 硬件初始化
void hardware_init() {
	// 鼠标左键 (没有右键)
    pinMode(0, INPUT_PULLUP);
	pinMode(BTN_PIN_1, INPUT_PULLDOWN);
	pinMode(BTN_PIN_2, INPUT_PULLDOWN);
	
	// 初始化 Display
	if (spi_mux_lock()) {   // 加锁
#ifdef GFX_EXTRA_PRE_INIT
    	GFX_EXTRA_PRE_INIT();
#endif
		if (!gfx->begin(80E6)) {
			Serial.println("gfx begin failed!");
			while (true) vTaskDelay(10000);
		}
		gfx->fillScreen(0);
		spi_mux_unlock(); // 解锁
	}

	vTaskDelay(70 / portTICK_PERIOD_MS);

	// 初始化 触摸, 本人没有
	// touch_init(gfx->width(), gfx->height(), gfx->getRotation());
  	
	// 初始化 LVGL
	lv_init();
#if LV_USE_LOG != 0
	lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif
	screenWidth = gfx->width();
	screenHeight = gfx->height();
	bufSize = screenWidth * screenHeight;

	uint32_t malloc_local = MALLOC_CAP_INTERNAL;
	if (psramFound()) malloc_local = MALLOC_CAP_SPIRAM;
	disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, malloc_local | MALLOC_CAP_8BIT);
	if (!disp_draw_buf) {
		// remove MALLOC_CAP_INTERNAL flag try again
		Serial.println("Remove MALLOC_CAP_INTERNAL flag try again!");
		disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
	}
	if (!disp_draw_buf) {
		Serial.println("LVGL disp_draw_buf allocate failed!");
		while (true) vTaskDelay(10000 / portTICK_PERIOD_MS);
	} else {
		lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);

		// 初始化 display 设备
		lv_disp_drv_init(&disp_drv);
		disp_drv.hor_res = screenWidth;
		disp_drv.ver_res = screenHeight;
		disp_drv.flush_cb = my_disp_flush;
		disp_drv.draw_buf = &draw_buf;
		disp_drv.direct_mode = true;

		lv_disp_drv_register(&disp_drv);
	}

	// 初始化 BLE 连接
    bleLink.begin(BLE_PEER_MAC, BLE_ROLE, BLE_NAME);

    // 注册回调 (顺序无关, 可在 begin 之后任意时刻注册)
    bleLink.onConnect(BLEonConnect);
    bleLink.onDisconnect(BLEonDisconnect);
    bleLink.onReceive(BLEonReceive);

	Serial.println("======= BLE 信息 =======");
	Serial.printf("Local BLE MAC: %s\n", bleLink.localAddress().c_str());
    Serial.printf("Peer  BLE MAC: %s\n", bleLink.peerAddress().c_str());
    Serial.printf("Role         : %s\n", bleLink.role() == BLETextLink::MASTER ? "MASTER" : "SLAVE");
	Serial.println("=======================");

	// 初始化 IMU
	Wire.begin(48, 47);
  	Wire.setClock(400000); // 400 kHz clock
	int err = IMU.init({0}, IMU_ADDRESS);
	if (err != 0) {
		Serial.println("Error initializing IMU: " + err);
	}
	
	// // 初始化 鼠标设备(IMU + GPIO 0 模拟的) 与 鼠标指针 
	// static lv_indev_drv_t indev_drv;
	// lv_indev_drv_init(&indev_drv);
	// indev_drv.type = LV_INDEV_TYPE_POINTER;
	// indev_drv.read_cb = my_input_read;
	// lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);
	// lv_obj_t *cursor_img = lv_img_create(lv_scr_act());
	// lv_img_set_src(cursor_img, LV_SYMBOL_OK);
	// lv_indev_set_cursor(mouse_indev, cursor_img);

#if (LCD_BL >= 0)
	ledcAttach(LCD_BL , LEDC_FREQ, LEDC_TIMER_10_BIT);
	// ledcAttach(LEDC_CHANNEL , LEDC_FREQ, LEDC_TIMER_10_BIT);
	// ledcAttachPin(, LEDC_CHANNEL);

	ledcWrite(LCD_BL , bl_duty ? (1 << LEDC_TIMER_10_BIT) / 100 * bl_duty : 0);
	// pinMode(LCD_BL, OUTPUT);
	// digitalWrite(LCD_BL, HIGH);
#endif
}


void setup() {
	heap_caps_malloc_extmem_enable(128);
	Serial.begin(115200);
	print_heap_free("初始化前");
	
	hardware_init();  // 硬件 初始化
	
	// // 预分配对话历史到PSRAM，降低SRAM碎片化
	// if (psramFound()) {
	// 	for (int i = 0; i < MAX_CHAT_WINDOW; i++) {
	// 		chat_windows[i].chatHistory.reserve(MAX_MESSAGES * 2 + 1);
	// 	}
	// 	response.reserve(2048);
	// 	answer.reserve(2048);
	// 	proced.reserve(2048);
	// }
	
	// 声明字体
	LV_FONT_DECLARE(cgr_yuyag_w2_ext4);

	// ----- 定义样式 -----
	lv_style_init(&style_pinyin);
	/* 背景 */
	lv_style_set_bg_color(&style_pinyin, lv_color_white());
	lv_style_set_bg_opa(&style_pinyin, LV_OPA_COVER);
	lv_style_set_radius(&style_pinyin, 6);
	/* 边框 */
	lv_style_set_border_width(&style_pinyin, 2);
	lv_style_set_border_color(&style_pinyin, lv_color_hex(0xD2D2D2));
	/* 阴影 */
	lv_style_set_shadow_width(&style_pinyin, 6);
	lv_style_set_shadow_color(&style_pinyin, lv_color_black());
	lv_style_set_shadow_opa(&style_pinyin, LV_OPA_10);
	lv_style_set_shadow_ofs_y(&style_pinyin, 2);
	/* 内边距 */
	lv_style_set_pad_left(&style_pinyin, 4);
	lv_style_set_pad_right(&style_pinyin, 4);
	lv_style_set_pad_top(&style_pinyin, 1);
	lv_style_set_pad_bottom(&style_pinyin, 6);
	/* 文字 */
	lv_style_set_text_font(&style_pinyin, &cgr_yuyag_w2_ext4);

	// 创建 输入框
    ta = lv_textarea_create(lv_scr_act());
    lv_textarea_set_one_line(ta, false);
    lv_obj_set_style_text_font(ta, &cgr_yuyag_w2_ext4, 0);
    lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 4, 4);
    lv_obj_set_size(ta, LV_PCT(98), 46);
    lv_obj_add_state(ta, LV_STATE_FOCUSED);  /* 默认聚焦 */
	
	// 创建 main_panel
    main_panel = lv_obj_create(lv_scr_act());
	lv_obj_align(main_panel, LV_ALIGN_TOP_LEFT, 4, 54);
    lv_obj_set_size(main_panel, LV_PCT(98), LV_PCT(77));
	lv_obj_align_to(main_panel, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_scroll_dir(main_panel, LV_DIR_VER);
	// 创建 main_label
    main_label = lv_label_create(main_panel);
    lv_obj_set_size(main_label, LV_PCT(101), 4500);
	lv_label_set_recolor(main_label, true);
	lv_obj_set_style_text_font(main_label, &cgr_yuyag_w2_ext4, 0);
	lv_obj_align(main_label, LV_ALIGN_TOP_LEFT, -6, -6);
	lv_label_set_text(main_label, "");

    // 创建键盘 (隐藏)
	kb = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(kb, ta);
	lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);

	// 创建 拼音输入框 (隐藏)
	pinyin_input_l = lv_label_create(lv_scr_act());
	lv_obj_add_flag(pinyin_input_l, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(pinyin_input_l, LV_PCT(95), 30);
	lv_obj_align_to(pinyin_input_l, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
	lv_obj_add_style(pinyin_input_l, &style_pinyin, LV_STATE_DEFAULT);
	lv_label_set_text(pinyin_input_l, "");
	
	// 创建 候选词栏 (隐藏)
	candidate_l = lv_label_create(lv_scr_act());
	lv_obj_add_flag(candidate_l, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(candidate_l, LV_PCT(95), 54);
	lv_obj_align_to(candidate_l, pinyin_input_l, LV_ALIGN_OUT_BOTTOM_MID, 0, -2);
	lv_obj_add_style(candidate_l, &style_pinyin, LV_STATE_DEFAULT);
	lv_label_set_text(candidate_l, "");
	
	xTaskCreatePinnedToCore(
		my_loop,     // 任务函数
		"my_loop",   // 任务名称
		8000,        // 堆栈大小
		NULL,         // 参数
		1,            // 优先级
		&TASK_Handle_My_Loop,  // 任务句柄
		0             // 核心编号 (0或1)
	);
	// esp_task_wdt_delete(TASK_Handle_My_Loop);
}


// 0x001e4fff

