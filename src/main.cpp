

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

	// WebSocket消息回调
	client.onMessage([&](websockets::WebsocketsMessage message) {
		// Serial.print("[Received] ");
		// Serial.println(message.data());
		
		JsonDocument tmp_doc;   // 临时 JSON 对象
		
		DeserializationError error = deserializeJson(tmp_doc, message.data());
		if (error) {
			Serial.print("JSON 解析错误: ");
			Serial.println(error.f_str());
			main_label_add_text( ( "#c81414 JSON 解析错误: #\n" + String(error.c_str()) ).c_str() );
			return;
		}
		
		const char* eventType = tmp_doc["type"];
		
		// 处理不同的事件类型
		if (strcmp(eventType, "session.created") == 0) {
			Serial.println("[事件] session.created");
			// main_label_add_text("[事件] session.created\n");
		}
		else if (strcmp(eventType, "session.updated") == 0) {
			Serial.println("[事件] session.updated");
			// main_label_add_text("[事件] session.updated\n");
		}
		else if (strcmp(eventType, "input_audio_buffer.speech_started") == 0) {
			Serial.println("[事件] Speech started detected (VAD)");
			// main_label_add_text("[事件] Speech started detected (VAD)\n");
		}
		else if (strcmp(eventType, "input_audio_buffer.speech_stopped") == 0) {
			Serial.println("[事件] Speech stopped detected (VAD)");
			// main_label_add_text("[事件] Speech stopped detected (VAD)\n");
		}
		else if (strcmp(eventType, "conversation.item.input_audio_transcription.text") == 0) {
			// 实时识别结果
			String fullText = tmp_doc["text"].as<String>() + tmp_doc["stash"].as<String>();
			Serial.print("[实时识别] ");
			Serial.println(fullText);
			// main_label_add_text("[实时识别] " + fullText + "\n");
		}
		else if (strcmp(eventType, "conversation.item.input_audio_transcription.completed") == 0) {
			// 最终识别结果
			asr_text = tmp_doc["transcript"].as<String>();
			asr_idle = 1;
			Serial.print("语音识别结果：");
			Serial.println(asr_text);
			// main_label_add_text("语音识别结果：" + asr_text + "\n");

			if (asr_text.length() > 0) {
				if (lvgl_mux_lock()) { // 上锁
					lv_textarea_add_text(ta, asr_text.c_str());
					lvgl_mutex_unlock(); // 解锁
				}
			}
		}
		else if (strcmp(eventType, "session.finished") == 0) {
			// 会话结束
			Serial.println("[事件] session.finished\n");
			// main_label_add_text("[事件] session.finished\n");
		}
		else if (strcmp(eventType, "error") == 0) {
			// 错误处理
			Serial.print("[错误] ");
			if (tmp_doc["error"]["message"]) {
				Serial.println(tmp_doc["error"]["message"].as<String>());
				main_label_add_text(("[错误] " + tmp_doc["error"]["message"].as<String>()).c_str());
			}
		}
	});

	Serial.println("Setup done");
	print_heap_free();

	// 主循环 (Core 0)
	while (1) {
		proc_key = read_key(); // 读取按键输入
		Serial.println("Key: " + proc_key);
		if (proc_key == "$12") {
			// print_heap_free();
			if (lvgl_mux_lock()) { // 上锁
				user_prompt = lv_textarea_get_text(ta);
				
				lv_obj_scroll_to_y(main_panel, 0, LV_ANIM_ON);

				lvgl_mutex_unlock(); // 解锁
			}

			Serial.println("等待结果中 ...");
			main_label_set_text("#7e00d2 等待结果中 ... #");

			getAnswer(user_prompt);
			if (answer.length()) main_label_set_text(answer.c_str()); // answer为空则不更新
		} else if (proc_key == "BCK") {
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

		} else if (proc_key == "UP") {
			send_key_to_ta(LV_KEY_UP);
		} else if (proc_key == "DOWN") {
			send_key_to_ta(LV_KEY_DOWN);
		} else if (proc_key == "LEFT") {
			send_key_to_ta(LV_KEY_LEFT);
		} else if (proc_key == "RIGT") {
			send_key_to_ta(LV_KEY_RIGHT);

		} else if (proc_key == "&S1") {
			use_proc = !use_proc;
			ta_tmp_show(use_proc ? "使用拼音预处理: 1" : "使用拼音预处理: 0");
		} else if (proc_key == "&S2") {
			show_proced = !show_proced;
			ta_tmp_show(show_proced ? "显示拼音处理结果: 1" : "显示拼音处理结果: 0");
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
		// } else if (proc_key == "&2") {
		// 	Serial.print("[WAV] 录音中 ... ");
		// main_label_set_text("[WAV] 录音中 ... ");
			
		// 	record_pcm("&2");
		// 	Serial.println("OK");

		// 	// ==========================================
		// 	// === 新增：生成 WAV 头并写入 SD 卡 ===
		// 	// ==========================================
		// Serial.print("[WAV] 写入中 ... ");
		// main_label_set_text("[WAV] 写入中 ... ");
			
		// 	uint32_t pcm_byte_size = recordingSize * sizeof(uint16_t); // 计算实际录制的字节长度

		// 	// 只有确实录到了数据才写文件
		// 	if (pcm_byte_size > 0) {
		// 		// 1. 准备 44 字节的 WAV 文件头 (WAVE_HEADER_SIZE 通常定义为 44)
		// 		char wav_header[WAVE_HEADER_SIZE];
		// 		generate_wav_header(wav_header, pcm_byte_size, SAMPLE_RATE);

		// 		// 2. 打开 SD 卡文件 (FILE_WRITE 会创建新文件，如果存在会覆盖)
		// 		File wav_file = SD.open("/recording.wav", FILE_WRITE);
		// 		if (wav_file) {
		// 			// 3. 写入 WAV 文件头
		// 			wav_file.write((uint8_t*)wav_header, WAVE_HEADER_SIZE);
					
		// 			// 4. 写入 PCM 纯音频数据
		// 			// 【注意】必须将 uint16_t* 强制转换为 uint8_t*，长度传入字节数 pcm_byte_size
		// 			size_t bytes_written = wav_file.write((uint8_t*)pcm_data, pcm_byte_size);
		// 			wav_file.close();

		// 			if (bytes_written == pcm_byte_size) {
		// 				Serial.printf("WAV文件保存成功! 总大小: %d 字节\n", pcm_byte_size + WAVE_HEADER_SIZE);
		// 			} else {
		// 				Serial.println("Error: 写入SD卡数据不完整");
		// 			}
		// 		} else {
		// 			Serial.println("Error: 无法打开SD卡文件");
		// 		}
		// 	} else {
		// 		Serial.println("未录制到有效音频数据");
		// 	}
		// 	// ==========================================

		// 	// 释放内存
		// 	free(pcm_data);
		// Serial.print("OK");
		// main_label_add_text("");
		} else if (proc_key == "&4") {
			calc_mode = !calc_mode;
			ta_tmp_show(calc_mode ? "计算器模式: 1" : "计算器模式: 0");
		} else if (proc_key.length() >= 3 && proc_key.startsWith("$S") && proc_key.substring(2).toInt() > 0 && proc_key.substring(2).toInt() <= MAX_CHAT_WINDOW) {
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
			if (lvgl_mux_lock()) { // 上锁
				lv_textarea_set_text(ta, current_window.ta_text_save.c_str());
				lv_textarea_set_cursor_pos(ta, current_window.ta_pos);
				lvgl_mutex_unlock(); // 解锁
			}
		} else if (proc_key.length() >= 2 && proc_key.startsWith("$") && proc_key.substring(1).toInt() > 0 && proc_key.substring(1).toInt() <= TEXT_SHORTCUT_SIZE) {
			ta_add_text(TEXT_SHORTCUT[ proc_key.substring(1).toInt() - 1 ]);
		} else if (proc_key == "$10") {
			reset_chat_history();
		} else if (proc_key == "$11") {
			ta_set_text("");
		} else if (proc_key == "&1") {
			if (!asr_idle) continue;

			// 保存当前文本, 用于后续恢复
			main_label_tmp_save();

			Serial.print("[ASR] 录音中 ... ");
			main_label_set_text("[ASR] 录音中 ... ");

			// 重置状态
			asr_text = "";
			asr_idle = 0;
			eventIdCounter = 0;
			
			record_pcm("&1");
			Serial.println("OK");
			
			main_label_set_text("ASR 识别中");
			// 发送音频到Qwen-ASR进行识别
			asr_send(pcm_data, recordingSize);
			
			// 释放内存
			free(pcm_data);

			// while (!asr_idle) {
			// 	client.poll(); // 处理接收的消息
			// 	vTaskDelay(2 / portTICK_PERIOD_MS);
			// }
			
			// if (asr_text.length() > 0) {
			// 	if (lvgl_mux_lock()) { // 上锁
			// 		lv_textarea_add_text(ta, asr_text.c_str());
			// 		lvgl_mutex_unlock(); // 解锁
			// 	}
			// }
			// main_label_set_text("ASR 识别完成: ");
			// main_label_add_text(asr_text);

			vTaskDelay(700 / portTICK_PERIOD_MS);
			// 恢复 main_label 上的文本
			main_label_tmp_recover();
		} else if (proc_key == "&5") {
			typing_pinyin = !typing_pinyin;

			if (lvgl_mux_lock()) { // 上锁
				if (typing_pinyin) { // 切换到了 拼音输入模式
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
		} else {
			// 拼音输入下的 字母输入
			if (typing_pinyin && proc_key[0] >= 'a' && proc_key[0] <= 'z') {
				pinyin_str += proc_key;
				if (lvgl_mux_lock()) { // 上锁
					lv_label_set_text(pinyin_input_l, pinyin_str.c_str());
					lvgl_mutex_unlock(); // 解锁
				}
				update_word_match();
			}
			// 选择候选词:  拼音输入模式  &&  word_result不为空     &&       proc_key 在 1 ~ 9 中
			else if (typing_pinyin && word_result.size() > 0 && proc_key[0] >= '1' && proc_key[0] <= '9') {
				if (proc_key.toInt()+candidate_offset <= word_result.size()) { // proc_key加偏移 超过 word_result 长度, 忽略
					if (lvgl_mux_lock()) { // 上锁
						lv_textarea_add_text(ta, word_result[ proc_key.toInt() + candidate_offset - 1 ].c_str());
						clear_pinyin();
						lvgl_mutex_unlock(); // 解锁
					}
				}
				
			}
			// 拼音输入下的 0 等同与 ETR(Enter)
			else if (typing_pinyin && proc_key == "0") {
				if (lvgl_mux_lock()) { // 上锁
					lv_textarea_add_text(ta, lv_label_get_text(pinyin_input_l));
					clear_pinyin();

					lvgl_mutex_unlock(); // 解锁
				}
			}
			// 拼音输入下的 [ -> offset-MOVE_WORDS (右移 MOVE_WORDS 项)
			else if (typing_pinyin && pinyin_str.length() && proc_key == "[") {
				if (candidate_offset <= MOVE_WORDS) candidate_offset = 0;
				else candidate_offset -= MOVE_WORDS;
				update_candidate();
			}
			// 拼音输入下的 ] -> offset+MOVE_WORDS (左移 MOVE_WORDS 项)
			else if (typing_pinyin && pinyin_str.length() && proc_key == "]") {
				if (candidate_offset+MOVE_WORDS*2 >= word_result.size()) {
					candidate_offset = word_result.size() - MOVE_WORDS;
				} else candidate_offset += MOVE_WORDS;
				update_candidate();
			}
			// 拼音输入下的 其他字符 及 正常输入
			else {
				ta_add_text(proc_key.c_str());
			}
			
		// } else {
			// // send_key_to_ta(proc_key[0]);
			// ta_add_text(proc_key.c_str());
		}

		// if (digitalRead(0) == LOW) {
		// 	vTaskDelay(15 / portTICK_PERIOD_MS);
		// 	if (digitalRead(0) == HIGH) continue;
		// 	Serial.println("Wait for recording");
		// }

		core0_loop_func();
		
		// // 清空串口
		// while (Serial1.available()) Serial1.read();
		
// 		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

// 根据情况发送不同请求
void getAnswer(String& _user_prompt) {
	if (_user_prompt == "") {
		Serial.println("输入为空");
		answer = "输入为空";
		return;
	} 
	if (_user_prompt == "-t") {
		if (syncing_sntp) {
			answer = "#bb5a14 正在同步时间 ";
			return;
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
			if (check_key()) break;
		}
		answer = "";
		return;
	}
	if (_user_prompt == "-imu") {
		while (1) {
			IMU.update();
			IMU.getAccel(&accelData); // accelData.accelX accelData.accelY accelData.accelZ
			IMU.getGyro(&gyroData);   // gyroData.gyroX   gyroData.gyroY   gyroData.gyroZ

			answer = "IMU 温度: " + String(IMU.getTemp());
			answer += "\r\n加速度计:\r\n - X: " + String(accelData.accelX) + "\r\n - Y: " + String(accelData.accelY) + "\r\n - Z: " + String(accelData.accelZ);
			answer += "\r\n陀螺仪:\r\n - X: " + String(gyroData.gyroX) + "\r\n - Y: " + String(gyroData.gyroY) + "\r\n - Z: " + String(gyroData.gyroZ);
			main_label_set_text(answer.c_str());
			vTaskDelay(50 / portTICK_PERIOD_MS);
			if (check_key()) break;
		}
		answer = "";
		return;
	}
	if (_user_prompt == "-i") {
		while (1) {
			answer = "CPU 温度: " + String(temperatureRead());
			// answer += "\r\n芯片版本: " + String(ESP.getChipRevision());
			answer += "\r\nCPU 频率: " + String(ESP.getCpuFreqMHz()) + " MHz";
			// answer += "\r\n循环计数: " + String(ESP.getCycleCount());
			
			answer += "\r\n堆 容量: " + String(ESP.getHeapSize() / 1024.0) + " KiB";
			answer += "\r\n堆 空闲: " + String(ESP.getFreeHeap() / 1024.0) + " KiB";
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
			vTaskDelay(100 / portTICK_PERIOD_MS);
			if (check_key()) break;
		}
		answer = "";
		return;
	}
	if (_user_prompt.startsWith("-bl=")) {
		ledc_duty = _user_prompt.substring(_user_prompt.indexOf("=") + 1).toDouble();
		ledc_duty = constrain(ledc_duty, 0, 100);
		ledcWrite(LCD_BL , ledc_duty ? (1 << LEDC_TIMER_10_BIT) / 100 * ledc_duty : 0.2);
		answer = "背光亮度: " + String(ledc_duty);
		return;
	}
	if (_user_prompt == "-rst") { esp_restart(); }
	if (_user_prompt == "-ds") {
		esp_deep_sleep_start();
	}
	if (_user_prompt == "-dsg") {
		esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_PIN_2, HIGH);
		esp_deep_sleep_start();
	}
	if (_user_prompt.startsWith("-dst=") && _user_prompt.length() > 5) { // -dst=1000 -> 休眠 1 秒
		uint64_t time_us = (_user_prompt.substring(_user_prompt.indexOf("=") + 1)).toInt() * 1000;
		esp_deep_sleep(time_us);
	}
	if (_user_prompt.startsWith("-sd")) {
		if (sd_status.length() != 0) {
			answer = sd_status;
			return;
		}
		answer = "";

		// 解析用户 (是否) 要读取的文件名
		String read_file_name;
		if (_user_prompt.indexOf('=') == 3 && _user_prompt.length() > 4) { // -sd=xxx -> 读取 SD 卡根目录下 xxx 文件
			read_file_name = _user_prompt.substring(_user_prompt.indexOf("=") + 1);
		}

		// 否则 解析用户 归递列出的层数
		else if (_user_prompt.indexOf('/') == 3) {
			uint8_t levels = 3;
			if (_user_prompt.length() > 4) { // -sd/x -> 归递列出 SD卡下 x层 目录与文件
				levels = (_user_prompt.substring(_user_prompt.indexOf('/') + 1)).toInt();
				Serial.println("归递列出 SD卡下 " + String(levels) + "层 目录与文件");
			} // else: -sd/ -> 归递列出 SD卡下 3层 目录与文件

			if (spi_mux_lock()) {   // 加锁
				listDir(SD, "/", levels, answer);
				spi_mux_unlock(); // 解锁
			}
			return;
		}
		// 继续解析用户 要读取的文件名    -sd-xx -> 读取 SD 卡根目录下 lcd-txx.txt 文件
		else if (_user_prompt.indexOf('-') == 3 && _user_prompt.length() > 4) {
			read_file_name = "lcd-t" + _user_prompt.substring(_user_prompt.indexOf('-') + 1) + ".txt";
		}
		else { // -sd -> 读取 SD 卡根目录下 lcd-t1.txt 文件
			read_file_name = "lcd-t1.txt";
		}
		
		// 读取 SD卡文件 内容
		if (spi_mux_lock()) {   // 加锁
			File file = SD.open("/" + read_file_name, FILE_READ);
			if (file.available()) {
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
	if (_user_prompt == "-wifi") {
		connect_wifi();
		answer = "";
		return;
	}
	if (_user_prompt == "-sntp") {
		sync_sntp();
		answer = "";
		return;
	}
	if (_user_prompt.startsWith("-ut")) {
		// uint8_t rx_pin = _user_prompt.substring(_user_prompt.indexOf("=") + 1);
		// uint8_t tx_pin = _user_prompt.substring(_user_prompt.indexOf("=") + 1);
		Serial2.begin(DEFAULT_BAUD, SERIAL_8N1, DEFAULT_RX_PIN, DEFAULT_TX_PIN); // 初始化 串口
		char tmp[80];
		snprintf(tmp, sizeof(tmp), "#10b166 UART 已连接: # \n #10b186 %d,8N1, RX=%d, TX=%d # \n #d6cc14 按 $12 退出 UART # \n", 
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
			if (Serial1.available()) {
				str_tmp = read_key();
				if (str_tmp == "$12") break;
				Serial2.write(str_tmp.c_str());
				// Serial.println("UART2 发送: " + str_tmp);
			}
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
		main_label_set_text("#10b166 UART 已断开");
		answer = "";
		return;
	}

	// 快捷文本
	if (_user_prompt == "-t-1") {answer = "";return;}
	if (_user_prompt == "-t-2") {answer = "";return;}
	if (_user_prompt == "-t-3") {answer = "";return;}

	answer = "";
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
	
	// bypass_proc = 0;
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

	// 初始化 IMU
	Wire.begin(48, 47);
  	Wire.setClock(400000); // 400 kHz clock
	int err = IMU.init({0}, IMU_ADDRESS);
	if (err != 0) {
		Serial.println("Error initializing IMU: " + err);
		// while (true) vTaskDelay(10000);
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

	ledcWrite(LCD_BL , ledc_duty ? (1 << LEDC_TIMER_10_BIT) / 100 * ledc_duty : 0);
	// pinMode(LCD_BL, OUTPUT);
	// digitalWrite(LCD_BL, HIGH);
#endif
}


void setup() {
	Serial.begin(115200);
	// print_heap_free();
	Serial1.begin(115200, SERIAL_8N1, 10, 7); // 初始化 C3从机串口
	
	hardware_init();  // 硬件 初始化
	
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
	
	// 创建 main_panel 和 main_label
    main_panel = lv_obj_create(lv_scr_act());
	lv_obj_align(main_panel, LV_ALIGN_TOP_LEFT, 4, 54);
    lv_obj_set_size(main_panel, LV_PCT(98), LV_PCT(77));
	lv_obj_align_to(main_panel, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_scroll_dir(main_panel, LV_DIR_VER);

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

	// 启动 C3 从机
	// Serial1.write("`");
	
	xTaskCreatePinnedToCore(
		my_loop,     // 任务函数
		"my_loop",   // 任务名称
		10000,        // 堆栈大小
		NULL,         // 参数
		1,            // 优先级
		&TASK_Handle_My_Loop,  // 任务句柄
		0             // 核心编号 (0或1)
	);
	// esp_task_wdt_delete(TASK_Handle_My_Loop);
}


// 0x001e4fff

