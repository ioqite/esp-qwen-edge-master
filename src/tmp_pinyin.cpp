// // // ======== esp32s3-lcd-2 ========

// #include "main.hpp"

// int wifi_choose = 0;

// // LVGL 线程锁
// static pthread_mutex_t lvgl_mutex = PTHREAD_MUTEX_INITIALIZER;

// // IMU 和 鼠标指针
// QMI8658 IMU;
// calData calib = { 0 };
// AccelData accelData;
// GyroData gyroData;
// float touchpad_x = 160;
// float touchpad_y = 120;

// // 屏幕刷新 与 LVGL 缓冲区
// uint32_t screenWidth;
// uint32_t screenHeight;
// uint32_t bufSize;
// lv_disp_draw_buf_t draw_buf;
// lv_color_t *disp_draw_buf;
// lv_disp_drv_t disp_drv;

// // 处理中的按键
// String proc_key;

// // int64_t couser_offset = 0;    // 用于移动输入位置, 暂时不想做

// // 定义 LVGL对象 和 显示的文本
// lv_obj_t * status_panel;
// lv_obj_t * main_panel;
// lv_obj_t * main_label;
// String main_label_text = "";
// lv_obj_t * g_ta;
// lv_obj_t * g_kb;

// ZhPinyinDecoder zh_pinyin_decoder; // 创建 中文拼音解码器
// std::vector<String> word_result; // 候选词列表
// lv_obj_t * g_candidate; // 候选词列表 显示框
// lv_obj_t * g_pinyin_input; // 拼音 输入框
// String pinyin_str = ""; // 正在输入的拼音 的 字符串
// String candidate_str = ""; // 候选词列表 的 字符串
// bool entering_pinyin = false; // 是否正在输入拼音标志

// // ----------------------- API相关 ------------------------
// const char* apiEndpoint = "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation";

// // --- 对话管理 ---
// const int MAX_MESSAGES = 20; // 最大保存的对话轮数
// // 使用一个数组来存储每一条JSON格式的消息
// String chatHistory[MAX_MESSAGES * 2 + 1]; 
// int messageCount = 0; // 当前历史消息数量
// // 系统提示词
// const char* systemPrompt = "你是一个乐于助人且语言简练的AI助手。请始终用中文简短地回答。";
// String aiAnswer;

// // 多模态API的URL, 请求体格式不一样: "https://dashscope.aliyuncs.com/api/v1/services/aigc/multimodal-generation/generation";

// // 带 拼音预处理 的 系统提示词:
// // 精简版
// String sys_prompt = "你是一个语言简练的AI助手。不要进入思考模式，尽可能不用生僻字，禁止使用Emoji和Markdown语法。用户输入可能有汉字错误，请自动纠正。要将一段文本染色，请在开始颜色时加上‘#RRGGBB ’，结束颜色时加上‘#’，RRGGBB为HEX颜色。";

// // 拼音预处理 提示词:
// String proc_prompt = "你是一个将拼音替换为文本的工具，如果输入中没有出现拼音则直接把输入原封不动地输出，禁止回答用户的问题！禁止回答用户的问题！禁止回答用户的问题！也不要进入思考模式。如果有拼音则联系附近字符理解用户意思，将输入中的拼音+声调（0为轻声）替换为对应的汉字（仅替换拼音+声调，如没有则不替换，不要替换任何数字与符号），请仅在用户的拼音错误时自动纠正，不要输出其他内容，拼音之间会用空格分隔，输出时请忽略，如果能则尽量保持输出的是一句正常的话。";
// // 用于测试拼音替换器: ni neng2 kan4 dao4 shang4 yi2 ju4 hua4 me0 ?

// // 不带 拼音预处理 的 系统提示词
// // 精简版
// String no_proc_prompt = "你是一个语言简练准确的AI助手。不要进入思考模式，尽可能不用生僻字，禁止使用Emoji和Markdown语法。要将一段文本染色，请在开始颜色时加上‘#RRGGBB ’，结束颜色时加上‘#’，RRGGBB为HEX颜色。";

// String model_name = "qwen-plus";      // 主模型
// String proc_model_name = "qwen-plus"; // 拼音预处理 模型

// // 旧版提示词，可供参考：
// // String sys_prompt = "你是一个语言简练准确的AI助手，需要时再思考。";
// // String sys_prompt = "你是一个语言简练准确的AI助手，需要时再思考，尽可能不用生僻字，且确保所有输出每行长度<=10中文字符或20英文字符（1中文字符与2英文字符长度相等），超出部分将被截断。用户只能使用拼音+声调表示汉字，用空格分隔汉字，请联系附近字符理解用户意思，例如：应将“ni3 hao3 !”理解为“你好！”。为确保准确，请先输出：“输入：{理解到的用户输入}\n”再回答。";
// // String sys_prompt = "你是一个语言简练准确的AI助手，需要时再思考，尽可能不用生僻字。用户只能使用拼音+声调表示汉字，用空格分隔汉字，请联系附近字符理解用户意思。";
// //，例如：应将ni3 hao3 !理解为你好！。为确保准确，请先输出：输入：{理解到的用户输入}再回答。
// // String proc_prompt = "你是一个文本替换工具，如果输入中没有出现拼音则把输入原封不动地输出，然后不要理会本输入后面的内容！不管输入是什么，都不要回答问题！不要理会用户的问题！也不要进入思考模式。请联系附近字符理解用户意思，将输入中的拼音+声调（0为轻声）替换为对应的汉字（仅替换拼音+声调，如没有则不替换，不要替换任何数字与符号），用户的拼音可能有错，请自动纠正，不要输出其他内容，汉字之间会用空格分隔，输出时请忽略，如果能则保持输出的是一句正常的话。";

// // 请求所需变量
// String payload;
// String response;
// String outputText;
// String answer; // 如果为 proc <error>, 则是 拼音预处理 失败, 将直接返回
// String proced;
// String user_prompt_g_ta;

// // 用户提示词
// String user_prompt = "";

// bool transferring_key = 0;     // 开始接受新的键 (有的键是多字母的)
// bool show_proced = 0;   // 是否显示 预处理过的 提示词, 由 $# 键切换
// uint8_t bypass_proc = 0; // 是否跳过 拼音预处理, 由 $@ 键切换
// bool enable_search = 0; // 是否启用 联网搜索
// // 欢迎文本 与 欢迎提示词
// // String helloTexts = "你好！介绍一下自己。";
// // String helloTexts = "我是一个语言简练准确的AI助手。我能理解并回答各种问题。我可以回答问题，创作文字，逻辑推理，编程等。";

// // ---------------------------- 屏幕 -------------------------------
// Arduino_DataBus *bus = new Arduino_ESP32SPI(EXAMPLE_PIN_NUM_LCD_DC, EXAMPLE_PIN_NUM_LCD_CS, EXAMPLE_PIN_NUM_LCD_SCLK, EXAMPLE_PIN_NUM_LCD_MOSI, EXAMPLE_PIN_NUM_LCD_MISO);
// Arduino_GFX *gfx = new Arduino_ST7789(bus, EXAMPLE_PIN_NUM_LCD_RST, EXAMPLE_LCD_ROTATION, true /* IPS */, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);

// void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) { lv_disp_flush_ready(disp_drv); }

// #if LV_USE_LOG != 0
// /* Serial debugging */
// void my_print(const char *buf) {
// 	Serial.printf(buf);
// 	Serial.flush();
// }
// #endif

// void setup() {
// 	Serial.begin(115200);
// 	// print_heap_free();
// 	Serial1.begin(115200, SERIAL_8N1, 10, 7);
	
// 	my_init();

// 	// LV_FONT_DECLARE(siyuan_normal_ext);
// 	LV_FONT_DECLARE(cgr_yuyag_w2_ext1);

// 	// 创建 main_panel 和 main_label
//     main_panel = lv_obj_create(lv_scr_act());
// 	lv_obj_align(main_panel, LV_ALIGN_TOP_LEFT, 5, 54);
//     lv_obj_set_size(main_panel, 310, 186);

//     main_label = lv_label_create(main_panel);
//     lv_obj_set_size(main_label, 295, 4500);
// 	lv_label_set_recolor(main_label, true);
// 	lv_obj_set_style_text_font(main_label, &cgr_yuyag_w2_ext1, 0);
// 	// lv_obj_set_style_text_font(main_label, &siyuan_normal_ext, 0);
// 	lv_obj_align(main_label, LV_ALIGN_TOP_MID, 0, 0);
// 	lv_label_set_text(main_label, "");

//     /* 2. 创建文本框 */
//     g_ta = lv_textarea_create(lv_scr_act());
//     lv_textarea_set_one_line(g_ta, false);
//     lv_obj_set_style_text_font(g_ta, &cgr_yuyag_w2_ext1, 0);
//     lv_obj_align(g_ta, LV_ALIGN_TOP_LEFT, 5, 3);
//     lv_obj_set_size(g_ta, 310, 46);// w:310
//     lv_obj_add_state(g_ta, LV_STATE_FOCUSED); /* 默认聚焦 */

//     /* 3. 创建键盘 (仅用于模拟输入，隐藏按键) */
// 	g_kb = lv_keyboard_create(lv_scr_act());
//     lv_obj_add_flag(g_kb, LV_OBJ_FLAG_HIDDEN);/* 直接隐藏键盘 */
//     lv_keyboard_set_textarea(g_kb, g_ta);

// 	// 创建候选输入框
// 	g_pinyin_input = lv_label_create(lv_scr_act());
//     lv_obj_set_size(g_pinyin_input, 320, 46);
// 	lv_obj_set_style_text_font(g_pinyin_input, &cgr_yuyag_w2_ext1, 0);
// 	// lv_obj_set_style_text_font(g_pinyin_input, &siyuan_normal_ext, 0);
// 	lv_obj_align(g_pinyin_input, LV_ALIGN_TOP_MID, 0, 0);
// 	lv_label_set_text(g_pinyin_input, "");
	
// 	// 创建候选词栏
// 	g_candidate = lv_label_create(lv_scr_act());
//     lv_obj_set_size(g_candidate, 320, 46);
// 	lv_obj_set_style_text_font(g_candidate, &cgr_yuyag_w2_ext1, 0);
// 	// lv_obj_set_style_text_font(g_candidate, &siyuan_normal_ext, 0);
// 	lv_obj_align(g_candidate, LV_ALIGN_TOP_MID, 0, 0);
// 	lv_label_set_text(g_candidate, "");

// 	// 添加 文本框事件回调
// 	lv_obj_add_event_cb(g_ta, ta_event_cb, LV_EVENT_ALL, g_kb);  

// 	// 启动 C3 从机
// 	Serial1.write("`");
	
// 	TaskHandle_t TASK_HandleOne = NULL;
// 	xTaskCreatePinnedToCore(
// 		my_loop,     // 任务函数
// 		"my_loop",   // 任务名称
// 		10000,        // 堆栈大小
// 		NULL,         // 参数
// 		1,            // 优先级
// 		&TASK_HandleOne,  // 任务句柄
// 		0             // CPU核心编号 (0或1)
// 	);
// 	esp_task_wdt_delete(TASK_HandleOne); // 删除 任务看门狗
// }

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

// void update_candidate() {
// 	word_result = zh_pinyin_decoder.word_match(pinyin_str);
// 	if (!word_result.size()) return;
// 	String candidate_str = "";
// 	for (int i=0; i<word_result.size(); i++) {
// 		candidate_str += String(i) + "." + word_result[i] + " ";
// 		// candidate_str += i;
// 		// candidate_str += ".";
// 		// candidate_str += String(i);
// 		// candidate_str += ".";
// 		// candidate_str += word_result[i];
// 		// candidate_str += " ";
// 	}
// 	pthread_mutex_lock(&lvgl_mutex); // 上线程锁
// 	lv_label_set_text(g_candidate, candidate_str.c_str());
// 	pthread_mutex_unlock(&lvgl_mutex); // 解线程锁
// }

// // 初始化 WiFi 并 启动按键接收循环 (Core 0)
// void my_loop(void *param) {
// 	// 选择 SSID 并初始化 WiFi
// 	Serial.println("初始化 WiFi");

// 	main_label_text = "#0060b9 请选择 WIFI：\n";
// 	for (int i=1; i<=6; i++) {
// 		main_label_text += i;
// 		main_label_text += ": ";
// 		main_label_text += ssids[i];
// 		main_label_text += "\n";
// 	}
// 	main_label_clear_print(main_label_text);

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
// 		else if (proc_key == "&1") wifi_choose = 101;
// 		else {
// 			proc_key = "";
// 			Serial.println("unknow SSID Key: " + proc_key);
// 		}
// 		if (wifi_choose && wifi_choose != 12 && wifi_choose != 101) {
// 			// Serial.println("true SSID Key: " + proc_key);
// 			WiFi.mode(WIFI_STA);
// 			WiFi.begin(ssids[wifi_choose], passwords[wifi_choose]);
// 			Serial.print("连接WiFi中 ..");
// 			main_label_clear_print("#1058b1 连接WiFi中 ..");
// 			while (WiFi.status() != WL_CONNECTED) {
// 				Serial.print('.');
// 				main_label_print(".");
// 				vTaskDelay(700 / portTICK_PERIOD_MS);
// 			}
// 			// Serial.println(WiFi.localIP());
// 			Serial.println(" OK");
// 			main_label_print("# #3add70 OK#");
// 			break;
// 		} else if (wifi_choose == 12) {
// 			main_label_clear_print("#115df5 跳过 WIFI#");
// 			break;
// 		} else if (wifi_choose == 101) {
// 			main_label_clear_print("#11c7f5 自定义 WIFI#, 输入 SSID: ");
// 			while (1) { // 输入自定义 SSID
// 				proc_key = read_key();
// 				if (proc_key == "$12") break;
// 				else if (proc_key == "BCK") lv_textarea_del_char(g_ta);
// 				else lv_textarea_add_text(g_ta, proc_key.c_str());
// 			}
// 			const char *custom_ssid = lv_textarea_get_text(g_ta);
// 			lv_textarea_set_text(g_ta, "");
// 			lv_textarea_set_password_mode(g_ta, 1);
// 			main_label_clear_print("#11c7f5 自定义 WIFI#, 输入 PASSWORD: ");
// 			while (1) { // 输入自定义 PASSWORD
// 				proc_key = read_key();
// 				if (proc_key == "$12") break;
// 				else if (proc_key == "BCK") physical_key_input(LV_KEY_BACKSPACE);
// 				else if (proc_key == "LEFT") physical_key_input(LV_KEY_LEFT);
// 				else if (proc_key == "RIGT") physical_key_input(LV_KEY_RIGHT);
// 				else lv_textarea_add_text(g_ta, proc_key.c_str());
// 			}
// 			const char *custom_password = lv_textarea_get_text(g_ta);
// 			lv_textarea_set_text(g_ta, "");
// 			lv_textarea_set_password_mode(g_ta, 0);
// 			WiFi.mode(WIFI_STA);
// 			WiFi.begin(custom_ssid, custom_password);
// 			Serial.print("连接自定义WiFi中 ..");
// 			main_label_clear_print("#1058b1 连接自定义WiFi中 ..");
// 			while (WiFi.status() != WL_CONNECTED) {
// 				Serial.print('.');
// 				main_label_print(".");
// 				vTaskDelay(700 / portTICK_PERIOD_MS);
// 			}
// 			// Serial.println(WiFi.localIP());
// 			Serial.println(" OK");
// 			main_label_print("# #3add70 OK#");
// 			break;
// 		}
// 	}

// 	pthread_mutex_lock(&lvgl_mutex);
// 	lv_textarea_set_placeholder_text(g_ta, "请输入");
// 	pthread_mutex_unlock(&lvgl_mutex);

// 	Serial.println("Setup done");

// 	while (1) {
// 		proc_key = read_key();
// 		Serial.println("Key: " + proc_key);
// 		if (proc_key == "$12") {
// 			Serial.println("等待结果中 ...");
// 			main_label_clear_print("#7e00d2 等待结果中 ...#");

// 			pthread_mutex_lock(&lvgl_mutex); // 上线程锁
// 			user_prompt = lv_textarea_get_text(g_ta);
// 			pthread_mutex_unlock(&lvgl_mutex); // 解线程锁

// 			getAnswer(user_prompt);
// 			main_label_clear_print(answer);
// 		} else if (proc_key == "BCK") {
// 			physical_key_input(LV_KEY_BACKSPACE); // user_prompt = user_prompt.substring(0, user_prompt.length() - 1);
// 		} else if (proc_key == "DEL") {
// 			physical_key_input(LV_KEY_DEL);
// 		} else if (proc_key == "UP") {
// 			physical_key_input(LV_KEY_UP);
// 		} else if (proc_key == "DOWN") {
// 			physical_key_input(LV_KEY_DOWN);
// 		} else if (proc_key == "LEFT") {
// 			physical_key_input(LV_KEY_LEFT);
// 		} else if (proc_key == "RIGT") {
// 			physical_key_input(LV_KEY_RIGHT);
// 		} else if (proc_key == "ETR") {
// 			physical_key_input(LV_KEY_ENTER);
// 		} else if (proc_key == "$@") { // 切换 bypass_proc 并 快速显示一下
// 			bypass_proc = !bypass_proc;

// 			pthread_mutex_lock(&lvgl_mutex); // 上线程锁
// 			user_prompt = lv_textarea_get_text(g_ta);
// 			lv_textarea_set_text(g_ta, bypass_proc ? "bypass_proc: 1" : "bypass_proc: 0");
// 			pthread_mutex_unlock(&lvgl_mutex); // 解线程锁
			
// 			vTaskDelay(800 / portTICK_PERIOD_MS);
			
// 			pthread_mutex_lock(&lvgl_mutex); // 上线程锁
// 			lv_textarea_set_text(g_ta, user_prompt.c_str());
// 			pthread_mutex_unlock(&lvgl_mutex); // 解线程锁
// 		} else if (proc_key == "$#") { // 切换 show_proced 并 快速显示一下
// 			show_proced = !show_proced;
// 			pthread_mutex_lock(&lvgl_mutex); // 上线程锁
// 			user_prompt = lv_textarea_get_text(g_ta);
// 			lv_textarea_set_text(g_ta, show_proced ? "show_proced: 1" : "show_proced: 0");
// 			pthread_mutex_unlock(&lvgl_mutex); // 解线程锁

// 			vTaskDelay(800 / portTICK_PERIOD_MS);

// 			pthread_mutex_lock(&lvgl_mutex); // 上线程锁
// 			lv_textarea_set_text(g_ta, user_prompt.c_str());
// 			pthread_mutex_unlock(&lvgl_mutex); // 解线程锁
// 		} else if (proc_key == "$3") {
// 			lv_textarea_add_text(g_ta, "是");
// 		} else if (proc_key == "$4") {
// 			lv_textarea_add_text(g_ta, "什么");
// 		} else if (proc_key == "$5") {
// 			lv_textarea_add_text(g_ta, "如何");
// 		} else if (proc_key == "$6") {
// 			lv_textarea_add_text(g_ta, "有");
// 		} else if (proc_key == "$7") {
// 			lv_textarea_add_text(g_ta, "中");
// 		} else if (proc_key == "$8") {
// 			lv_textarea_add_text(g_ta, "的");
// 		} else if (proc_key == "$9") {
// 			lv_textarea_add_text(g_ta, "详细说一下");
// 		} else if (proc_key == "$10") {
// 			reset_chat_history(); // 重置聊天记录
// 		} else if (proc_key == "$11") {
// 			lv_textarea_set_text(g_ta, ""); // 清空输入框
// 		} else if (proc_key == "&1") {
// 			entering_pinyin = !entering_pinyin;
// 			pthread_mutex_lock(&lvgl_mutex); // 上线程锁
// 			if (entering_pinyin) { // 切换到 拼音输入模式
// 				lv_label_set_text(g_candidate, "");
// 				lv_label_set_text(g_pinyin_input, "");
// 				lv_obj_clear_flag(g_candidate, LV_OBJ_FLAG_HIDDEN);
// 				lv_obj_clear_flag(g_pinyin_input, LV_OBJ_FLAG_HIDDEN);
// 				// update_candidate();
// 			} else { // 退出 拼音输入模式
// 				lv_label_set_text(g_candidate, "");
// 				lv_label_set_text(g_pinyin_input, "");
// 				lv_obj_add_flag(g_candidate, LV_OBJ_FLAG_HIDDEN);
// 				lv_obj_add_flag(g_pinyin_input, LV_OBJ_FLAG_HIDDEN);
// 			}
// 			pthread_mutex_unlock(&lvgl_mutex); // 解线程锁
// 		} else {
// 			pthread_mutex_lock(&lvgl_mutex); // 上线程锁

// 			// 拼音输入下的 字母输入
// 			if (entering_pinyin && proc_key[0] >= 'a' && proc_key[0] <= 'z') {
// 				pinyin_str += proc_key;
// 				lv_label_set_text(g_pinyin_input, pinyin_str.c_str());
// 				update_candidate();
// 			// 选择候选词
// 			} else if (entering_pinyin && proc_key[0] >= '0' && proc_key[0] <= '9') {
// 				lv_textarea_add_text(g_ta, word_result[proc_key.toInt()].c_str());
// 				pinyin_str = "";
// 				lv_label_set_text(g_pinyin_input, "");
// 			// 拼音输入下的 其他字符 及 正常输入
// 			} else {
// 				lv_textarea_add_text(g_ta, proc_key.c_str());
// 			}
// 			pthread_mutex_unlock(&lvgl_mutex); // 解线程锁
// 		}
// 	}
// }

// // 根据情况发送不同请求
// void getAnswer(String _user_prompt) {
// 	// 快捷命令, 可自行添加修改
// 	if (_user_prompt == "-t-1") {answer = "";return;}
// 	if (_user_prompt == "-t-2") {answer = "";return;}
// 	if (_user_prompt == "-t-3") {answer = "";return;}
// 	if (_user_prompt == "-tf")  {answer = "";return;}

// 	answer = "";
// 	Serial.println("_user_prompt: " + _user_prompt);

// 	if (bypass_proc == 1) {
// 		getQwenAnswer(sys_prompt, user_prompt, model_name, answer, true);
// 	} else if (bypass_proc == 0) {
// 		if (getQwenAnswer(proc_prompt, user_prompt, proc_model_name, proced, false) != 0) {
// 			answer = proced;
// 			return;
// 		} else {
// 			Serial.println("proced: " + proced);
// 			String tmp_ans;
// 			getQwenAnswer(sys_prompt, proced, model_name, tmp_ans, true);
// 			if (show_proced) answer = "PROCED: " + proced + "\n" + tmp_ans;
// 			else answer = tmp_ans;
// 		}
// 	}

// 	Serial.print("\n--- start answer ---\n`");
// 	Serial.println(answer);
// 	Serial.println("`\n--- end answer ---");
	
// 	// // 重置 发送选项
// 	// bypass_proc = 0;
// 	// show_proced = 0;
// }

// // 主循环 (Core 1)
// void loop() {
// 	pthread_mutex_lock(&lvgl_mutex);
// 	lv_timer_handler(); // LVGL任务处理
// 	pthread_mutex_unlock(&lvgl_mutex);

// #if (LV_COLOR_16_SWAP != 0)
//  	gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
// #else
// 	// Serial.println("draw begin");
//  	gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
// #endif

// 	my_read_imu(); // 读取IMU数据

//   	vTaskDelay(5 / portTICK_PERIOD_MS);
// }

// // 获取 剩余RAM (测试用)
// void print_heap_free() {
// 	// 查看 片内 RAM 和 PSRAM 剩余堆
// 	Serial.printf("\nFree Heap:\n - Heap free: %.2f KB\n", esp_get_free_heap_size() / 1024.0);
// 	Serial.printf(" - Internal RAM free: %.2f KB\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024.0);
// 	Serial.printf(" - SPI RAM free: %.2f KB\n\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024.0);
// }

// /**
//  * @brief 向对话历史中添加消息
//  * @param role 消息角色 ("system", "user", "assistant")
//  * @param content 消息内容
//  */
// void addMessageToHistory(const char* role, const String content) {
//     // 如果历史记录已满，则移除最早的一条用户和助手消息
//     if (messageCount >= (MAX_MESSAGES * 2 + 1)) {
//         for (int i = 1; i < messageCount - 2; i++) {
//         chatHistory[i] = chatHistory[i + 2];
//         }
//         messageCount -= 2;
//     }
//     // 将消息以JSON字符串的形式存入数组
//     chatHistory[messageCount++] = String("{\"role\":\"") + role + "\",\"content\":\"" + content + "\"}";
// }

// /**
//  * @brief 构建并发送 HTTP 请求到 API
//  * @param _systemPrompt 系统提示词
//  * @param _userPrompt 用户的当前问题
//  * @param _model_name 模型名称
//  * @param _response 存储回复内容的字符串 引用
//  * @param useHistory 是否使用轮次对话
//  * @return AI的回复内容字符串
//  */
// int8_t getQwenAnswer(const String _systemPrompt, const String _userPrompt, String _model_name, String& _response, bool useHistory) {
//     if (_response == nullptr) return -1;
// 	if (WiFi.status() != WL_CONNECTED) {
// 		_response = "未连接到Wi-Fi";
// 		return -2;
// 	}

//     WiFiClientSecure client;
//     client.setInsecure(); // 跳过SSL证书验证

//     HTTPClient http;
// 	http.setTimeout(65535);
// 	http.setConnectTimeout(4294967295);
//     http.begin(client, apiEndpoint);
//     // http.begin(apiEndpoint);
//     http.addHeader("Content-Type", "application/json");
//     http.addHeader("Authorization", "Bearer " + String(apiKey));

// 	// 使用 ArduinoJson 构建请求体
// 	JsonDocument doc;

// 	doc["model"] = _model_name;

// 	if (useHistory) {
// 		// 如果是第一次对话，先加入系统提示词
// 		if (messageCount == 0) {
// 			addMessageToHistory("system", _systemPrompt);
// 		}
		
// 		// 加入用户当前的问题
// 		addMessageToHistory("user", _userPrompt);

// 		// 将历史记录中的每一条消息解析并添加到JSON数组中
// 		for (int i = 0; i < messageCount; i++) {
// 			JsonDocument tmpDoc;
// 			DeserializationError error = deserializeJson(tmpDoc, chatHistory[i]);
// 			if (!error) doc["input"]["messages"].add(tmpDoc.as<JsonObject>());
// 		}
// 	} else {
// 		JsonDocument tmpDoc;
// 		tmpDoc["role"] = "system";
// 		tmpDoc["content"] = _systemPrompt;
// 		doc["input"]["messages"].add(tmpDoc.as<JsonObject>());
// 		tmpDoc["role"] = "user";
// 		tmpDoc["content"] = _userPrompt;
// 		doc["input"]["messages"].add(tmpDoc.as<JsonObject>());
// 	}

//     // 将构建好的JSON文档序列化为字符串
//     String jsonPayload;
//     serializeJson(doc, jsonPayload);
//     Serial.printf("开始发送POST请求, 请求体: %s\n", jsonPayload.c_str());
   
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

//                 _response = String(aiContent);
//                 return 0;
//             } else {
//                 _response = "JSON解析失败, Response: \n" + response;
// 				Serial.println();
// 				Serial.println(response);
// 				Serial.println();
// 				return -3;
//             }
// 		} else {
// 			_response = "响应码: " + String(httpResponseCode) + "\nResponse: \n" + response;
// 			Serial.println();
// 			Serial.println(response);
// 			Serial.println();
// 			return -5;
// 		}
// 	} else {
// 		_response = "请求失败, ";
// 		if (httpResponseCode == HTTPC_ERROR_TOO_LESS_RAM) {
// 			_response += "内存不足";
// 		} else if (httpResponseCode == HTTPC_ERROR_READ_TIMEOUT) {
// 			_response += "读取超时";
// 		} else if (httpResponseCode == HTTPC_ERROR_CONNECTION_REFUSED) {
// 			_response += "连接被拒绝";
// 		} else if (httpResponseCode == HTTPC_ERROR_CONNECTION_LOST) {
// 			_response += "连接丢失";
// 		} else {
// 			_response += "错误码: " + String(httpResponseCode);
// 		}
// 		return -5;
// 	}
// }

// void reset_chat_history() {
//     messageCount = 0;
//     aiAnswer = "";
//     memset(chatHistory, 0, sizeof(chatHistory));
// }

// // 修改 各个Label 的文本
// void main_label_print(String print_text) {
// 	main_label_text += print_text;
// 	pthread_mutex_lock(&lvgl_mutex);
// 	lv_label_set_text(main_label, main_label_text.c_str());
// 	pthread_mutex_unlock(&lvgl_mutex);
// }
// void main_label_clear_print(String print_text) {
// 	main_label_text = print_text;
// 	pthread_mutex_lock(&lvgl_mutex);
// 	lv_label_set_text(main_label, main_label_text.c_str());
// 	pthread_mutex_unlock(&lvgl_mutex);
// }

// // 鼠标回调
// void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data) {
// 	data->point.x = touchpad_x + 0.5;
//     data->point.y = touchpad_y + 0.5;
// 	if(digitalRead(0) == 0) {
// 		// Serial.println("PRESSED");
// 		data->state = LV_INDEV_STATE_PRESSED;
// 	} else {
// 		// Serial.println("Released");
// 		data->state = LV_INDEV_STATE_RELEASED;
// 	}
// }

// // 读取 IMU, 并 更新鼠标位置
// void my_read_imu() {
//     IMU.update();

//     IMU.getAccel(&accelData); // accelData.accelX accelData.accelY accelData.accelZ
//     // IMU.getGyro(&gyroData); // gyroData.gyroX gyroData.gyroY gyroData.gyroZ

//     // Serial.println(IMU.getTemp());
    
//     touchpad_x -= accelData.accelX * 8;
// 	touchpad_y -= accelData.accelY * 8;
// 	if (touchpad_x < 0) touchpad_x = 0;
// 	if (touchpad_x > 319) touchpad_x = 319;
// 	if (touchpad_y < 0) touchpad_y = 0;
// 	if (touchpad_y > 239) touchpad_y = 239;
// 	// Serial.println(touchpad_x);Serial.print(",");Serial.println(touchpad_y);
// 	// Serial.println(accelData.accelX);Serial.print(",");Serial.println(accelData.accelY);
// }

// static void ta_event_cb(lv_event_t * e) {
// 	lv_event_code_t code = lv_event_get_code(e);
// 	lv_obj_t * ta = (lv_obj_t *)lv_event_get_target(e);
// 	lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);	
// 	if(code == LV_EVENT_FOCUSED) {
// 		// lv_obj_add_state(ta, LV_STATE_FOCUSED);

// 		/* 1. 恢复键盘正常高度 (自动计算或设具体值)，确保虚拟按键可见 */
// 		// lv_obj_set_height(kb, LV_SIZE_CONTENT); 
//         // 但通常聚焦时需要全键盘以便输入
        
//         /* 2. 关联文本框并显示 */
//         // lv_keyboard_set_textarea(kb, ta);
//         // lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        
//         /* 3. (可选) 如果确实只需要候选栏，可以在这里调整，但不要固定为 10% 导致无法恢复 */
//         // lv_obj_set_style_height(kb, LV_PCT(10), 0); // <--- 删除这行，防止变矮
//     } else if(code == LV_EVENT_CANCEL) {
// 		/* 隐藏键盘 */
//         // lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

// 		pthread_mutex_lock(&lvgl_mutex); // 上线程锁
//         lv_obj_clear_state(ta, LV_STATE_FOCUSED);
// 		pthread_mutex_unlock(&lvgl_mutex); // 解线程锁

//         // lv_indev_reset(NULL, ta);
//     }
// 	// 【新增】确保每次有事件时，键盘都关联着当前文本框
//     // 防止因焦点切换导致的关联丢失
//     // if(code == LV_EVENT_KEY || code == LV_EVENT_CLICKED) {
//     //     lv_keyboard_set_textarea(kb, ta);
//     // }
// }

// /* 物理按键输入接口 */
// void physical_key_input(uint32_t key) {
// 	// 确保文本框有焦点
//     if(g_ta) {
//         lv_obj_add_state(g_ta, LV_STATE_FOCUSED);
// 		Serial.println("physical_key_input: g_ta: " + String((char)key));
// 		// 直接向文本框发送 KEY 事件
// 		pthread_mutex_lock(&lvgl_mutex);
// 		lv_event_send(g_ta, LV_EVENT_KEY, &key);
// 		pthread_mutex_unlock(&lvgl_mutex);
//         return;
//     } else {
// 		Serial.println("physical_key_input: g_ta is NULL!");
// 	}
// }

// // 全部初始化
// void my_init() {
// 	// 鼠标左键 (没有右键)
//     pinMode(0, INPUT_PULLUP);

// 	// 初始化 Display
// #ifdef GFX_EXTRA_PRE_INIT
//     GFX_EXTRA_PRE_INIT();
// #endif
// 	if (!gfx->begin()) {
// 		Serial.println("gfx->begin() failed!");
// 		while (true) vTaskDelay(10000);
// 	}
// 	gfx->fillScreen(BLACK);
// #if (EXAMPLE_PIN_NUM_LCD_BL >= 0)
// 	pinMode(EXAMPLE_PIN_NUM_LCD_BL, OUTPUT);
// 	digitalWrite(EXAMPLE_PIN_NUM_LCD_BL, HIGH);
// #endif

// 	// 初始化 触摸, 本人没有
// 	// touch_init(gfx->width(), gfx->height(), gfx->getRotation());
  	
// 	// 初始化 LVGL
// 	lv_init();
// #if LV_USE_LOG != 0
//   lv_log_register_print_cb(my_print); /* register print function for debugging */
// #endif
// 	screenWidth = gfx->width();
// 	screenHeight = gfx->height();
// 	bufSize = screenWidth * screenHeight;
// 	disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
// 	if (!disp_draw_buf) {
// 		// remove MALLOC_CAP_INTERNAL flag try again
// 		disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
// 		Serial.println("LVGL disp_draw_buf allocate failed!");
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
//   	Wire.setClock(400000); //400khz clock
// 	int err = IMU.init(calib, IMU_ADDRESS);
// 	if (err != 0) {
// 		Serial.println("Error initializing IMU: " + err);
// 		// while (true) vTaskDelay(10000);
// 	}
	
// 	// 初始化 鼠标设备(IMU + GPIO 0 模拟的) 与 鼠标指针 
// 	static lv_indev_drv_t indev_drv;
// 	lv_indev_drv_init(&indev_drv);
// 	indev_drv.type = LV_INDEV_TYPE_POINTER;
// 	indev_drv.read_cb = my_input_read;
	
// 	lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);
// 	lv_obj_t *cursor_img = lv_img_create(lv_scr_act());
// 	lv_img_set_src(cursor_img, LV_SYMBOL_OK);
// 	lv_indev_set_cursor(mouse_indev, cursor_img);
// }

// // 0x0017afff

// // 用arduino写一个单字拼音输入法，输入含有一个字的拼音的字符串，输出候选字列表。我的想法是：通过比较char的值来截取字符串末尾的所有英文字母
