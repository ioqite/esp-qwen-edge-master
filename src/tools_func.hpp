#pragma once
#include "app_common.hpp"

#define IN_PSRAM _SECTION_ATTR_IMPL(".ext_ram.bss", __COUNTER__)

// ######################################===================##################################
// ##################################### | 变量声明 与 初始化 | #################################
// ######################################===================##################################

// ################## 线程锁 与 FreeRTOS Task ###################
// LVGL 线程锁
SemaphoreHandle_t lvgl_mutex = xSemaphoreCreateRecursiveMutex();
// SPI 线程锁
SemaphoreHandle_t spi_mux = xSemaphoreCreateRecursiveMutex();

bool lvgl_mux_lock() { return xSemaphoreTakeRecursive(lvgl_mutex, portMAX_DELAY) == pdTRUE; }
void lvgl_mux_unlock() { xSemaphoreGiveRecursive(lvgl_mutex); }
bool spi_mux_lock() { return xSemaphoreTakeRecursive(spi_mux, portMAX_DELAY) == pdTRUE; }
void spi_mux_unlock() { xSemaphoreGiveRecursive(spi_mux); }

TaskHandle_t TASK_Handle_WiFi = NULL;
TaskHandle_t TASK_Handle_My_Loop = NULL;

// ########################### 外设 ###########################

// SD卡状态 (正常为空, 否则为 错误信息)
IN_PSRAM String sd_status = "";

// 屏幕背光 占空比(%)
float bl_duty = LEDC_DEFAULT_DUTY;	// 默认占空比

// IMU 和 鼠标指针
IN_PSRAM QMI8658 IMU;
IN_PSRAM AccelData accelData;
IN_PSRAM GyroData gyroData;
// float touchpad_x = 160;
// float touchpad_y = 120;

// ###################### 屏幕刷新 与 LVGL ######################

IN_PSRAM uint32_t screenWidth;
IN_PSRAM uint32_t screenHeight;
IN_PSRAM uint32_t bufSize;
IN_PSRAM lv_disp_draw_buf_t draw_buf;
lv_color_t *disp_draw_buf;
IN_PSRAM lv_disp_drv_t disp_drv;

// 定义 LVGL对象 和 显示的文本
IN_PSRAM lv_obj_t * main_panel;
IN_PSRAM lv_obj_t * main_label;
IN_PSRAM lv_obj_t * ta;
IN_PSRAM lv_obj_t * kb;

IN_PSRAM lv_obj_t * camera_img; // 摄像头画面 显示框
IN_PSRAM camera_fb_t *pic;
IN_PSRAM lv_img_dsc_t img_dsc;

// ##################### 录音 与 Qwen-ASR #####################

IN_PSRAM uint16_t *pcm_data;         // 录音缓存区
IN_PSRAM size_t bytes_read = 0;
IN_PSRAM uint32_t recordingSize = 0;

IN_PSRAM websockets::WebsocketsClient client;
IN_PSRAM String asr_text = "";   // 最终识别结果文本
IN_PSRAM bool asr_idle = 1;      // 是否空闲
IN_PSRAM int eventIdCounter = 0; // 事件ID计数器

// ######################## 状态变量 ##########################

// bool transferring_key = 0;     // 开始接收新的键 (有的键是多字母的)
bool skip_wifi = 0;            // 是否跳过WiFi连接
bool connecting_wifi = false;  // 是否正在连接WiFi
bool syncing_sntp = false;     // 是否正在同步SNTP时间
String proc_key;   // 处理中的按键
String tmp_key;    // 读取时的临时按键 (仅限 wait_until_read_key()、read_key()、BLEonReceive() 访问)
std::vector<String> ta_history;
uint16_t ta_history_pos = 0;

// ######################## 拼音输入法 ########################

#define MOVE_WORDS 6  // 一次移动词数
#define MIN_CANDIDATE_NUM  9 // 移动时 最小候选词数
IN_PSRAM lv_style_t style_pinyin;
IN_PSRAM std::vector<String> word_result; // 候选词列表
IN_PSRAM String candidate_str = ""; // 候选词列表 的 字符串
IN_PSRAM lv_obj_t * candidate_l;    // 候选词显示框
IN_PSRAM lv_obj_t * pinyin_input_l; // 拼音输入框
IN_PSRAM String pinyin_str = "";    // 拼音输入框 的 字符串(已实时同步)
IN_PSRAM String split_result = "";  // 拼音分割结果
IN_PSRAM uint16_t candidate_offset = 0; // 候选词显示框 的 偏移词数
IN_PSRAM bool typing_pinyin = false; // 是否正在输入拼音标志

// ######################### 对话管理 #########################

// 最大对话窗口 个数
#define MAX_CHAT_WINDOW 5
// 最大 保存的对话轮数
#define MAX_MESSAGES 10

IN_PSRAM String main_label_text_tmp;

struct chat_window_t{
	String ta_text_save;
	String main_label_text_save;
	std::vector<String> chatHistory; // 使用一个数组来存储 每一条JSON格式的消息(String) max:MAX_MESSAGES * 2 + 1
	uint32_t ta_pos = 0;
	int16_t main_label_pos = 0;
};
chat_window_t IN_PSRAM chat_windows[MAX_CHAT_WINDOW];

// 当前选中的 对话窗口
IN_PSRAM uint8_t chat_window_select = 0;
#define current_window chat_windows[chat_window_select]

// ####################### 请求管理 ##########################

// 请求所需变量
IN_PSRAM String answer;

// 用户提示词
IN_PSRAM String user_prompt = "";

bool use_proc = 0;         // 是否使用 拼音预处理,       由 &2 键切换
bool show_proced = 0;      // 是否显示 预处理过的 提示词, 由 &3 键切换
bool calc_mode   = 0;      // 是否启用 计算器模式,       由 &4 键切换
// bool enable_search = 0; // 是否启用 联网搜索,         未实现

IN_PSRAM String tmp_output;      // 临时文本输出

// ######################## 其他 ###########################

// 蓝牙(BLE) 传输器
IN_PSRAM BLETextLink bleLink;

// 储存 时间信息
IN_PSRAM tm timeinfo;

// SD卡使用的 SPI总线 (需与 屏幕总线 一致)
SPIClass sd_spi_bus(FSPI);
// 屏幕, More: https://github.com/moononournation/Arduino_GFX/wiki/
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCLK, LCD_MOSI, LCD_MISO, FSPI /* spi_num */, true);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, EXAMPLE_LCD_ROTATION, true /* IPS */, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);


// GMT 时间偏移量 (秒)
#define GMT_OFFSET_SEC 8 * 3600
// SNTP 服务器
#define SNTP_SERVER "ntp.cnnic.cn" // CNNIC

// 文本快捷键
#define TEXT_SHORTCUT_SIZE 9
const char* TEXT_SHORTCUT[TEXT_SHORTCUT_SIZE] = {"用", "控制", "是", "什么", "如何", "有", "中", "的", "详细说一下"};



// ######################################===========#######################################
// ##################################### | 函数声明 | ######################################
// ######################################===========#######################################


// ############################### 外设 ##################################

// 设置背光亮度
void set_bl_duty() {
	bl_duty = constrain(bl_duty, 0, 100);
	ledcWrite(LCD_BL , bl_duty ? (1 << LEDC_TIMER_10_BIT) / 100 * bl_duty : 0.2);
}

// 检查是否有按键按下 并 读取, 无->"", 有->按下的按键
String read_key() {
	tmp_key = MSG_NONE;
	bleLink.loop(); // 读取按键输入
	if (tmp_key == MSG_NONE) return "";
	return tmp_key;
}

// 读取按键输入
String wait_until_read_key() {
	tmp_key = MSG_NONE;
	while (1) {
		bleLink.loop(); // 读取按键输入
		if (tmp_key != MSG_NONE) return tmp_key;
		else vTaskDelay(2 / portTICK_PERIOD_MS);
		core0_loop_func();
	}
}

// 列出目录下的所有文件和子目录 (未加锁)
void listDir(fs::FS &fs, const char *dirname, uint8_t levels, String& response) {
	File root = fs.open(dirname);
	if (!root) {
		response += "Failed to open directory: ";
		response += dirname;
		return;
	}
	if (!root.isDirectory()) {
		response += "Not a directory";
		return;
	}

	response += String("List: ") + dirname + "\r\n";

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			response += "  Dir: ";
			response += file.name();
			response += "\r\n";
			if (levels) {
				listDir(fs, file.path(), levels - 1, response);
			}
		} else {
			response += "  File: ";
			response += file.name();
			response += "  Size: ";
			response += file.size();
			response += "\r\n";
		}
		file = root.openNextFile();
	}
	response += "\r\n";
}

// 创建新目录 (未加锁)
void createDir(fs::FS &fs, const char * path){
    // 打印正在创建的目录
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
    // 尝试创建目录，如果成功则打印成功信息，否则打印失败信息
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

// 将 JPEG图片数据 写入文件 (未加锁)
void writejpg(fs::FS &fs, const char * path, const uint8_t *buf, size_t size){
	// 打开文件用于写入
	File file = fs.open(path, FILE_WRITE);
	if(!file){
		// 如果文件打开失败，打印错误信息
		Serial.printf("[ERROR] Failed to open '%s'\r\n for writing", path);
		return;
	}
	// 写入数据到文件
	file.write(buf, size);
	// 输出文件保存成功信息
	Serial.printf("[INFO] Saved file to path: %s\r\n", path);
}

// 读取 指定目录下的文件数量 (未加锁)
int readFileNum(fs::FS &fs, const char * dirname){
	File root = fs.open(dirname);
	if(!root){
		Serial.printf("[ERROR] Failed to open '%s'\r\n", dirname);
		return -1;
	}
	if(!root.isDirectory()){
		Serial.printf("[ERROR] '%s' not a directory\r\n", dirname);
		return -1;
	}

	File file = root.openNextFile();
	int num = 0;
	while(file){
		//遍历文件个数
		file = root.openNextFile();
		num++;
	}
	return num;  
}

// 获取 指定目录中 下一个WAV文件的索引(名称) (未加锁)
void getWavFileIdex(fs::FS &fs, const char * dirname, int &fileIndex){
	File root = fs.open(dirname);
	if(!root){
		Serial.printf("[ERROR] Failed to open '%s'\r\n", dirname);
		fileIndex = -1;
		return;
	}
	if(!root.isDirectory()){
		Serial.printf("[ERROR] '%s' not a directory\r\n", dirname);
		fileIndex = -1;
		return;
	}

	File file = root.openNextFile();
	fileIndex = 0;
	while(fs.exists(dirname + String(fileIndex++) + ".wav")) {}
}

// 初始化 SD 卡
void init_SDcard() {
	if (spi_mux_lock()) {   // 加锁
		sd_spi_bus.begin(SD_SCK, SD_MISO, SD_MOSI, -1);
		if (!SD.begin(SD_CS, sd_spi_bus)) {
			Serial.println("SD卡 挂载失败");
			sd_status = "SD卡 挂载失败";
			spi_mux_unlock();
			return;
		}
		uint8_t cardType = SD.cardType();

		if (cardType == CARD_NONE) {
			Serial.println("未检测到 SD卡");
			sd_status = "未检测到 SD卡";
			spi_mux_unlock();
			return;
		}

		Serial.print("SD卡类型: ");
		if (cardType == CARD_MMC) {
			Serial.println("MMC");
		} else if (cardType == CARD_SD) {
			Serial.println("SDSC");
		} else if (cardType == CARD_SDHC) {
			Serial.println("SDHC");
		} else {
			Serial.println("UNKNOWN");
		}

		Serial.printf("Total space: %lluMiB\r\n",  SD.totalBytes() / (1024 * 1024));
		Serial.printf( "Used space: %lluMiB\r\n",  SD.usedBytes()  / (1024 * 1024));
		
		// String list_res = "";
		// listDir(SD, SD_PREFIX, 3, list_res);
		// Serial.println(list_res);

		spi_mux_unlock();   // 解锁
	}
}

// 初始化 I2S
void setupI2S() {
	Serial.print("Setup I2S ...");
	esp_err_t err;

	const i2s_config_t i2s_config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate = SAMPLE_RATE,
		.bits_per_sample = i2s_bits_per_sample_t(16),
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
		.intr_alloc_flags = 0, // default interrupt priority
		.dma_buf_count = 4,
		.dma_buf_len = 512,
		.use_apll = false
	};

	err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
	// err = i2s_driver_uninstall(I2S_PORT);
	if (err != ESP_OK) {
		Serial.printf("I2S driver install failed (I2S_PORT): %d\r\n", err);
	}
	
	const i2s_pin_config_t pin_config = {
		.bck_io_num = I2S_SCK,
		.ws_io_num = I2S_WS,
		.data_out_num = I2S_PIN_NO_CHANGE,
		.data_in_num = I2S_SD
	};
	
	err = i2s_set_pin(I2S_PORT, &pin_config);
	if (err != ESP_OK) {
		Serial.printf("I2S set pin failed (I2S_PORT): %d\r\n", err);
		while (true);
	}
	err = i2s_start(I2S_PORT);
	// err = i2s_stop(I2S_PORT);
	if (err != ESP_OK) {
		Serial.printf("I2S start failed (I2S_PORT): %d\r\n", err);
	}
	Serial.println("OK!");
}
// 停止 I2S
void stopI2S() {
	Serial.print("Stop I2S ... ");
	esp_err_t err;

	// err = i2s_start(I2S_PORT);
	err = i2s_stop(I2S_PORT);
	if (err != ESP_OK) {
		Serial.printf("I2S stop failed (I2S_PORT): %d\r\n", err);
	}
	Serial.println("OK!");
	
	// err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
	err = i2s_driver_uninstall(I2S_PORT);
	if (err != ESP_OK) {
		Serial.printf("I2S driver uninstall failed, I2S_PORT: %d\r\n", err);
	}
}

// 创建 WAV 头
void generate_wav_header(char* wav_header, uint32_t wav_size, uint32_t sample_rate){
    // See this for reference: http://soundfile.sapp.org/doc/WaveFormat/
    uint32_t file_size = wav_size + WAVE_HEADER_SIZE - 8;
    const char set_wav_header[] = {
        'R','I','F','F', // ChunkID
        (char)file_size, (char)(file_size >> 8), (char)(file_size >> 16), (char)(file_size >> 24), // ChunkSize
        'W','A','V','E', // Format
        'f','m','t',' ', // Subchunk1ID
        0x10, 0x00, 0x00, 0x00, // Subchunk1Size (16 for PCM)
        0x01, 0x00, // AudioFormat (1 for PCM)
        0x01, 0x00, // NumChannels (1 channel)
        (char)sample_rate, (char)(sample_rate >> 8), (char)(sample_rate >> 16), (char)(sample_rate >> 24), // SampleRate
        (char)BYTE_RATE, (char)(BYTE_RATE >> 8), (char)(BYTE_RATE >> 16), (char)(BYTE_RATE >> 24), // ByteRate
        0x02, 0x00, // BlockAlign
        0x10, 0x00, // BitsPerSample (16 bits)
        'd','a','t','a', // Subchunk2ID
        (char)wav_size, (char)(wav_size >> 8), (char)(wav_size >> 16), (char)(wav_size >> 24), // Subchunk2Size
    };
    memcpy(wav_header, set_wav_header, sizeof(set_wav_header));
}

// 录音 PCM 音频 (需手动 释放pcm_data)
bool record_pcm(const char *record_key) {
	bytes_read = 0;
	recordingSize = 0;
	
	// 分配 pcm_data
	pcm_data = (uint16_t *)ps_malloc(BUFFER_SIZE * sizeof(uint16_t));
	if (!pcm_data) {
		Serial.println("无法从 PSRAM 给 pcm_data 分配内存");
		main_label_add_text("无法从 PSRAM 给 pcm_data 分配内存");
		return 1;
	}
	
	uint32_t start_time = millis();
	// 开始循环录音
	while (recordingSize < MAX_RECORD_TIME_SECONDS * SAMPLE_RATE) {
		esp_err_t err = i2s_read(I2S_PORT, pcm_data + recordingSize, CHUNK_SIZE * sizeof(uint16_t), &bytes_read, portMAX_DELAY);
		if (err != ESP_OK) continue;
		else Serial.println("I2S Read fail: 0x" + String(err, 16));
		recordingSize += bytes_read / 2;

		if (millis() - start_time > Check_Interval) { // 每 Check_Interval 检查一次是否松开录音按钮
			start_time = millis();					  //  (按键发送间隔是 240ms)
			if (read_key() != record_key) break;
		}
	}
	return 0;
}

// 录制 WAV 文件
void record_wav(const char *record_key) {
	Serial.print("[WAV] 录音中 ... ");
	main_label_set_text("[WAV] 录音中 ... ");
	
	if (record_pcm(record_key)) {
		Serial.println("录音失败");
		main_label_add_text("#df1f1f 录音失败 #");
		return;
	}
	Serial.print("OK\r\n");
	main_label_add_text("OK\r\n");

	// ============== 生成 WAV 头并写入 SD 卡 ==============
	
	uint64_t pcm_byte_size = recordingSize * sizeof(uint16_t); // 计算实际录制的字节长度

	// 只有确实录到了数据才写文件
	if (pcm_byte_size > 0) {
		File wav_file;
		int wav_index;
		if (spi_mux_lock()) {
			// 读取 SD卡上 WAV文件数量
			wav_index = readFileNum(SD, SD_PREFIX "records");
			// 如果目录不存在 则创建
			if (wav_index == -1) {
				createDir(SD, SD_PREFIX "records");
				wav_index = 0;
			}
			getWavFileIdex(SD, SD_PREFIX "records/", wav_index);
			if (wav_index == -1) { // 获取失败时，使用文件数量作名称
				wav_index = readFileNum(SD, SD_PREFIX "records");
			}
			// 1. 打开 SD 卡文件 (FILE_WRITE 会创建新文件，如果存在会覆盖)
			wav_file = SD.open(SD_PREFIX "records/" + String(wav_index) + ".wav", FILE_WRITE);
			spi_mux_unlock();
		}
		if (wav_file) {
			Serial.print("[WAV] 写入 WAV头 中 ... ");
			main_label_add_text("[WAV] 写入 WAV头 中 ... ");
			
			// 2. 准备 44 字节的 WAV 文件头 (WAVE_HEADER_SIZE 通常定义为 44)
			char wav_header[WAVE_HEADER_SIZE];
			generate_wav_header(wav_header, pcm_byte_size, SAMPLE_RATE);

			// 3. 写入 WAV 文件头
			if (spi_mux_lock()) {
				wav_file.write((uint8_t*)wav_header, WAVE_HEADER_SIZE);
				spi_mux_unlock();
			}
			
			// 4. 写入 PCM 纯音频数据
			// 【注意】必须将 uint16_t* 强制转换为 uint8_t*，长度传入字节数 pcm_byte_size
			Serial.print("OK\r\n[WAV] 写入 PCM数据 中 ... ");
			main_label_add_text("OK\r\n[WAV] 写入 PCM数据 中 ... ");
			if (spi_mux_lock()) {
				size_t bytes_written = wav_file.write((uint8_t*)pcm_data, pcm_byte_size);
				wav_file.close();
				spi_mux_unlock();

				if (bytes_written == pcm_byte_size) {
					Serial.print("OK\r\n[WAV] 写入成功\r\n");
					main_label_add_text("OK\r\n[WAV] 写入成功\r\n");
				} else {
					Serial.print("写入数据不完整\r\n");
					main_label_add_text("写入数据不完整\r\n");
				}
				String tmp_text = "[WAV] 文件: " + String(wav_index) + ".wav, 总大小: " + (pcm_byte_size + WAVE_HEADER_SIZE) + " B\r\n";
				Serial.print(tmp_text);
				main_label_add_text(tmp_text.c_str());
			}
		} else {
			Serial.print("[WAV] 无法打开SD卡文件\r\n");
			main_label_add_text("[WAV] 无法打开SD卡文件\r\n");
		}
	} else {
		Serial.print("[WAV] 未录制到有效音频数据\r\n");
		main_label_add_text("[WAV] 未录制到有效音频数据\r\n");
	}
	// 释放内存
	free(pcm_data);
}


// ####################### BLE 回调 #######################

// BLE设备连接 回调
void BLEonConnect() {
    Serial.println("[BLE事件] 已连接 键盘 或 其他设备");
	ta_tmp_show("BLE 已连接", 1000);
    // 这里可以做"上线后初始化"操作, 如发送握手消息
    // bleLink.send("Connected from " + bleLink.localAddress());
}

// BLE设备断开 回调
void BLEonDisconnect() {
    Serial.println("[BLE事件] 连接已断开 对方会自动重连");
	ta_tmp_show("BLE连接 已断开", 1000);
}

void BLEonReceive(const String& msg) {
    Serial.printf("[BLE事件 | 接收] %u bytes: %s\r\n", msg.length(), msg.c_str());
    // 示例: 收到 "ping" 回 "pong"
    // if (msg == "ping") bleLink.send("pong");

	if (msg == "") return;
	if (msg[0] != 0x02) return;
	if (msg[msg.length() - 1] != 0x03) return;

	tmp_key = msg.substring(1, msg.length() - 1);
}



// ############################### 请求 ##################################

// WebSocket 回调
void ws_callback(websockets::WebsocketsMessage message) {
	// Serial.print("[Received] ");
	// Serial.println(message.data());
	
	JsonDocument tmp_doc;
	
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
		Serial.println("[ASR | 事件] session.created");
	}
	else if (strcmp(eventType, "session.updated") == 0) {
		Serial.println("[ASR | 事件] session.updated");
	}
	else if (strcmp(eventType, "input_audio_buffer.speech_started") == 0) {
		Serial.println("[ASR | 事件] Speech started detected (VAD)");
		// main_label_add_text("[ASR | 事件] Speech started detected (VAD)\n");
	}
	else if (strcmp(eventType, "input_audio_buffer.speech_stopped") == 0) {
		Serial.println("[ASR | 事件] Speech stopped detected (VAD)");
		// main_label_add_text("[ASR | 事件] Speech stopped detected (VAD)\n");
	}
	else if (strcmp(eventType, "conversation.item.input_audio_transcription.text") == 0) {
		// 实时识别结果
		String fullText = tmp_doc["text"].as<String>() + tmp_doc["stash"].as<String>();
		Serial.print("[ASR | 实时识别] ");
		Serial.println(fullText);
		// main_label_set_text("[ASR | 实时识别] " + fullText + "\n");
	}
	else if (strcmp(eventType, "conversation.item.input_audio_transcription.completed") == 0) {
		// 最终识别结果
		asr_text = tmp_doc["transcript"].as<String>();
		asr_idle = 1;
		Serial.print("[ASR] 最终识别结果：");
		Serial.println(asr_text);
		// main_label_add_text("[ASR] 最终识别结果：" + asr_text + "\n");

		if (asr_text.length() > 0) {
			if (lvgl_mux_lock()) { // 上锁
				lv_textarea_add_text(ta, asr_text.c_str());
				lvgl_mux_unlock(); // 解锁
			}
		}
		client.cleanup();
		print_heap_free("ASR识别完成 并清理");
		// client.close(websockets::CloseReason_NormalClosure);
	}
	else if (strcmp(eventType, "session.finished") == 0) {
		// 会话结束
		Serial.println("[ASR | 事件] session.finished\n");
	}
	else if (strcmp(eventType, "error") == 0) {
		// 错误处理
		Serial.print("[ASR | 错误] ");
		if (tmp_doc["error"]["message"]) {
			Serial.println(tmp_doc["error"]["message"].as<String>());
			main_label_add_text(("[ASR | 错误] " + tmp_doc["error"]["message"].as<String>()).c_str());
		}
	}
}

// 录音 并 发送到ASR识别
void run_asr(const char *record_key) {
	// 保存当前文本, 用于后续恢复
	main_label_tmp_save();

	// 重置状态
	asr_text = "";
	asr_idle = 0;
	eventIdCounter = 0;

	Serial.print("[ASR] 录音中 ... ");
	main_label_set_text("[ASR] 录音中 ... ");
	
	if (record_pcm(record_key)) {
		Serial.println("录音失败");
		main_label_add_text("#df1f1f 录音失败 #");
		return;
	}
	
	if (connecting_wifi) {
		Serial.println("正在连接 WiFi");
		main_label_set_text("#b9450f 正在连接 WiFi #");
		return;
	}
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("未连接 WiFi");
		main_label_set_text("#e31919 未连接 WiFi #");
		return;
	}

	main_label_set_text("ASR 识别中");
	// 发送音频到Qwen-ASR进行识别
	asr_send(pcm_data, recordingSize);

	// 等待最终结果（在回调中处理session.finished事件）
	Serial.println("[ASR] 等待最终结果");
	main_label_set_text("[ASR] 等待最终结果\n");
	
	// 释放内存
	free(pcm_data);

	vTaskDelay(700 / portTICK_PERIOD_MS);
	// 恢复 main_label 上的文本
	main_label_tmp_recover();
}

// 向 Qwen-ASR 发送音频数据
void asr_send(uint16_t* pcm_data, uint32_t size) {
	if (connecting_wifi) {
		Serial.println("#b9450f 正在连接 WiFi ");
		main_label_set_text("#b9450f 正在连接 WiFi ");
		asr_idle = 1;
		return;
	}
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("#e31919 未连接 WiFi");
		main_label_set_text("#e31919 未连接 WiFi");
		asr_idle = 1;
		return;
	}
	
	JsonDocument tmp_doc;

	Serial.print("[ASR] 连接服务器中 ... ");
	main_label_set_text("[ASR] 连接服务器中 ... ");
	
	// 添加 Authorization 和 OpenAI-Beta 请求头
	client.addHeader("Authorization", "Bearer " apiKey);
	client.addHeader("OpenAI-Beta", "realtime=v1");
	
	// 如果需要，可以设置CA证书
	client.setInsecure();

	// 连接 WebSocket
	print_heap_free("WS连接 前");
	bool connected = client.connect(QWEN_ASR_BASE_URL "?model=" QWEN_ASR_MODEL);
	print_heap_free("WS连接 后");
	if (connected) {
		Serial.println("OK");
		main_label_add_text("OK\n");
	} else {
		Serial.println("失败");
		main_label_add_text("失败\n");
		asr_idle = 1;
		return;
	}
	
	// 等待session.created事件（在回调中处理）
	vTaskDelay(15 / portTICK_PERIOD_MS);

	// 发送session.update配置
	tmp_doc.clear();
	tmp_doc["event_id"] = eventIdCounter++;
	tmp_doc["type"] = "session.update";
	
	tmp_doc["session"]["modalities"].add("text");
	tmp_doc["session"]["input_audio_format"] = "pcm";
	tmp_doc["session"]["sample_rate"] = SAMPLE_RATE;
	
	tmp_doc["session"]["input_audio_transcription"]["language"] = ASR_LANGUAGE;
	
#if ENABLE_SERVER_VAD
		// VAD模式：服务端自动检测语音起止
		tmp_doc["session"]["turn_detection"]["type"] = "server_vad";
		tmp_doc["session"]["turn_detection"]["threshold"] = ASR_Threshold;
		tmp_doc["session"]["turn_detection"]["silence_duration_ms"] = ASR_Silence_Duration_MS;
#else
		// Manual模式：客户端控制断句
		tmp_doc["session"]["turn_detection"] = nullptr;
#endif
	
	serializeJson(tmp_doc, tmp_output);
	
	Serial.println("[ASR] session.update");
	// main_label_add_text("[ASR] session.update\n");
	client.send(tmp_output);
	
	vTaskDelay(15 / portTICK_PERIOD_MS);
	
	// 分段发送音频数据
	int audioDataSize = size * 2;  // 16bit = 2 bytes per sample
	uint32_t totalChunks = (audioDataSize + CHUNK_SIZE - 1) / CHUNK_SIZE;
	size_t offset = 0;
	for (uint32_t i = 0; i < totalChunks; i++) {
		size_t chunkSize = CHUNK_SIZE;
		if (offset + chunkSize > audioDataSize) {
			chunkSize = audioDataSize - offset;
		}
		
		// 发送音频数据块
		tmp_doc.clear();
		tmp_doc["event_id"] = eventIdCounter++;
		tmp_doc["type"] = "input_audio_buffer.append";
		
		// Base64编码音频数据
		tmp_doc["audio"] = base64::encode((uint8_t*)pcm_data + offset, chunkSize);
		
		serializeJson(tmp_doc, tmp_output);
		
		Serial.println("[ASR] input_audio_buffer.append [" + String(i) + " / " + String(totalChunks) + "]");
		// main_label_set_text( ("[ASR] input_audio_buffer.append [" + String(i) + " / " + String(totalChunks) + "]").c_str() );
		main_label_set_text( ("[ASR] 音频块发送中 [" + String(i) + " / " + String(totalChunks) + "]").c_str() );
		client.send(tmp_output);

		offset += chunkSize;
		
		// vTaskDelay(1 / portTICK_PERIOD_MS);
	}
	
	// 如果是Manual模式，需要发送commit事件
	if (!ENABLE_SERVER_VAD) {
		vTaskDelay(20 / portTICK_PERIOD_MS);
		tmp_doc.clear();
		tmp_doc["event_id"] = eventIdCounter++;
		tmp_doc["type"] = "input_audio_buffer.commit";
		
		serializeJson(tmp_doc, tmp_output);
		
		Serial.println("[ASR] input_audio_buffer.commit");
		// main_label_set_text("[ASR] input_audio_buffer.commit\n");
		client.send(tmp_output);
	}
	
	// 发送session.finish结束会话
	vTaskDelay(10 / portTICK_PERIOD_MS);
	
	tmp_doc.clear();
	tmp_doc["event_id"] = eventIdCounter++;
	tmp_doc["type"] = "session.finish";
	
	serializeJson(tmp_doc, tmp_output);
	
	Serial.println("[ASR] session.finish");
	// main_label_set_text("[ASR] session.finish\n");
	client.send(tmp_output);
}

/**
 * @brief 向对话历史中添加消息
 * @param role 消息角色 ("system", "user", "assistant")
 * @param content 消息内容
 */
void addMessageToHistory(const char* role, const String content) {
    // 如果历史记录已满，则移除最早的一条用户和助手消息
    if (current_window.chatHistory.size() >= (MAX_MESSAGES * 2 + 1)) {
		current_window.chatHistory.erase(
			current_window.chatHistory.begin()+1,
			current_window.chatHistory.begin()+2
		);
    }
    // 将消息以JSON字符串的形式存入数组
    current_window.chatHistory.push_back(
		String("{\"role\":\"") + role + "\",\"content\":\"" + content + "\"}"
	);
}

/**
 * @brief 构建并发送 HTTPS 请求到 API
 * @param _SYSTEM_PROMPT 系统提示词 字符串
 * @param _userPrompt 用户的当前问题 String引用
 * @param _MAIN_MODEL_NAME 模型名称 字符串
 * @param _response 存储 回复内容 或 错误信息 的String引用
 * @param useHistory 是否使用多轮次对话
 * @return 运行结果: (详见 _response)
 *            0: 成功
 *           -1: _response 为空
 *           -2: 无网络
 *           -3: JSON解析失败
 *           -4: 响应码
 *           -5: 请求失败
 */
int8_t getAPIanswer(const char* _SYSTEM_PROMPT, const String& _userPrompt, const char* _MAIN_MODEL_NAME, String& _response, bool useHistory) {
	// if (_response == NULL) return -1;
	if (connecting_wifi) {
		_response = "#b9450f 正在连接 WiFi #";
		Serial.println("正在连接 WiFi");
		main_label_set_text("#b9450f 正在连接 WiFi #");
		return -2;
	}
	if (WiFi.status() != WL_CONNECTED) {
		_response = "#e31919 未连接 WiFi #";
		Serial.println("未连接 WiFi");
		main_label_set_text("#e31919 未连接 WiFi #");
		return -2;
	}

    NetworkClientSecure networkClient;
    networkClient.setInsecure(); // 跳过SSL证书验证

    HTTPClient http;
	http.setTimeout(65535);
	http.setConnectTimeout(4294967295);
    http.begin(networkClient, API_ENDPOINT);
    // http.begin(API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " apiKey);

	// --- 构建请求体 ---
	JsonDocument tmp_doc;
	tmp_doc["model"] = _MAIN_MODEL_NAME;

	if (useHistory) {
		// 如果是第一次对话，先加入系统提示词
		if (current_window.chatHistory.empty()) {
			addMessageToHistory("system", _SYSTEM_PROMPT);
		}
		
		// 加入用户当前的问题
		addMessageToHistory("user", _userPrompt);

		JsonDocument msgDoc;
		// 将历史记录中的每一条消息解析并添加到JSON数组中
		for (String tmp : current_window.chatHistory) {
			DeserializationError error = deserializeJson(msgDoc, tmp);
			if (!error) tmp_doc["input"]["messages"].add(msgDoc.as<JsonObject>());
		}
	} else {
		JsonDocument msgDoc;
		msgDoc["role"] = "system";
		msgDoc["content"] = _SYSTEM_PROMPT;
		tmp_doc["input"]["messages"].add(msgDoc.as<JsonObject>());
		msgDoc["role"] = "user";
		msgDoc["content"] = _userPrompt;
		tmp_doc["input"]["messages"].add(msgDoc.as<JsonObject>());
	}

    // 将构建好的JSON文档序列化为字符串
    String jsonPayload;
    serializeJson(tmp_doc, jsonPayload);
    Serial.println("开始发送POST请求, 请求体: ");
	Serial.println(jsonPayload);
   
    // 发送POST请求
	print_heap_free("请求前");
    int httpResponseCode = http.POST(jsonPayload);
    
	String response = http.getString();
	http.end();
	
	if (httpResponseCode > 0){
		if (httpResponseCode == 200) {
			// 解析 JSON 响应
			tmp_doc.clear();
            DeserializationError error = deserializeJson(tmp_doc, response);

            if (!error) {
                const char* aiContent = tmp_doc["output"]["text"];
				// 将AI的回复也加入历史记录
				if (useHistory) addMessageToHistory("assistant", aiContent);

                _response = aiContent;
                return 0;
            } else {
                _response = "#e34819 JSON解析失败, Response: \r\n" + response + " #";
				Serial.println();
				Serial.println(response);
				Serial.println();
				return -3;
            }
		} else {
			_response = "#c5b910 响应码: " + String(httpResponseCode) + "\r\nResponse: \r\n" + response + " #";
			Serial.println();
			Serial.println(response);
			Serial.println();
			return -4;
		}
	} else {
		_response = "#e31919 请求失败, ";
		if (httpResponseCode == HTTPC_ERROR_TOO_LESS_RAM) {
			_response += "内存不足";
		} else if (httpResponseCode == HTTPC_ERROR_READ_TIMEOUT) {
			_response += "读取超时";
		} else if (httpResponseCode == HTTPC_ERROR_CONNECTION_REFUSED) {
			_response += "连接被拒绝";
		} else if (httpResponseCode == HTTPC_ERROR_CONNECTION_LOST) {
			_response += "连接丢失";
		} else {
			_response += "错误码: " + String(httpResponseCode);
		}
		_response += " #";
		return -5;
	}
}

// 重置对话历史
void reset_chat_history() {
    current_window.chatHistory.clear();
	std::vector<String>().swap(current_window.chatHistory);
}



// ############################### LVGL ##################################

void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
	if (spi_mux_lock()) {   // 加锁
#if (LV_COLOR_16_SWAP != 0)
 		gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#else
 		gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#endif
		spi_mux_unlock(); // 解锁
	}
	lv_disp_flush_ready(disp_drv);
}

#if LV_USE_LOG != 0
void my_print(const char *buf) {
	Serial.print(buf);
	Serial.flush();
}
#endif

// ============== 文本输入框ta 操作 ===============
// 添加文本到 ta 上
void ta_add_text(const char* text) {
	if (lvgl_mux_lock()) {  // 加锁
		lv_textarea_add_text(ta, text);
		lvgl_mux_unlock();  // 解锁
	}
}
// 设置 ta 上的文本
void ta_set_text(const char* text) {
	if (lvgl_mux_lock()) {  // 加锁
		lv_textarea_set_text(ta, text);
		lvgl_mux_unlock();  // 解锁
	}
}
// 暂存 ta 上的文本
void ta_tmp_save() {
	if (lvgl_mux_lock()) { // 上锁
		current_window.ta_text_save = lv_textarea_get_text(ta);
		current_window.ta_pos = lv_textarea_get_cursor_pos(ta);
		lvgl_mux_unlock(); // 解锁
	}
}
// 恢复 ta 上的文本
void ta_tmp_recover() {
	if (lvgl_mux_lock()) { // 上锁
		lv_textarea_set_text(ta, current_window.ta_text_save.c_str());
		lv_textarea_set_cursor_pos(ta, current_window.ta_pos);
		lvgl_mux_unlock(); // 解锁
	}
}
// 在 ta 上临时显示文本
void ta_tmp_show(const char* text, uint16_t delay_ms = 700) {
	if (lvgl_mux_lock()) { // 上锁
		current_window.ta_text_save = lv_textarea_get_text(ta);
		current_window.ta_pos = lv_textarea_get_cursor_pos(ta);
		lv_textarea_set_text(ta, text);
		lvgl_mux_unlock(); // 解锁
	}

	vTaskDelay(delay_ms / portTICK_PERIOD_MS);

	ta_set_text(current_window.ta_text_save.c_str());
	if (lvgl_mux_lock()) { // 上锁
		lv_textarea_set_cursor_pos(ta, current_window.ta_pos);
		lvgl_mux_unlock(); // 解锁
	}
}

// ============== main_label 操作 ===============
// 添加文本到 main_label 上
void main_label_add_text(const char* text) {
	main_label_text_tmp = lv_label_get_text(main_label);
	main_label_text_tmp += text;
	if (lvgl_mux_lock()) { // 上锁
		lv_label_set_text(main_label, main_label_text_tmp.c_str());
		lvgl_mux_unlock();
	}
}
// 设置 main_label 上的文本
void main_label_set_text(const char* text) {
	if (lvgl_mux_lock()) { // 上锁
		lv_label_set_text(main_label, text);
		lvgl_mux_unlock();
	}
}
// 暂存 main_label 上的文本
void main_label_tmp_save() {
	if (lvgl_mux_lock()) { // 上锁
		current_window.main_label_text_save = lv_label_get_text(main_label);
		current_window.main_label_pos = lv_obj_get_scroll_y(main_panel);
		Serial.println(current_window.main_label_pos);
		lvgl_mux_unlock(); // 解锁
	}
}
// 恢复 main_label 上的文本
void main_label_tmp_recover() {
	if (lvgl_mux_lock()) { // 上锁
		lv_label_set_text(main_label, current_window.main_label_text_save.c_str());
		lv_obj_scroll_to_y(main_panel, current_window.main_label_pos, LV_ANIM_ON);
		Serial.println(current_window.main_label_pos);
		lvgl_mux_unlock(); // 解锁
	}
}
// 在 main_label 上临时显示文本
void main_label_tmp_show(const char* text, uint16_t delay_ms = 700) {
	if (lvgl_mux_lock()) { // 上锁
		current_window.main_label_text_save = lv_label_get_text(main_label);
		current_window.main_label_pos = lv_obj_get_scroll_y(main_panel);
		Serial.println(current_window.main_label_pos);
		lv_label_set_text(main_label, text);
		lvgl_mux_unlock(); // 解锁
	}

	vTaskDelay(delay_ms / portTICK_PERIOD_MS);

	if (lvgl_mux_lock()) { // 上锁
		lv_label_set_text(main_label, current_window.main_label_text_save.c_str());
		lv_obj_scroll_to_y(main_panel, current_window.main_label_pos, LV_ANIM_ON);
		Serial.println(current_window.main_label_pos);
		lvgl_mux_unlock(); // 解锁
	}
}

// 滚动 主文本框 (Y默认0) (默认加锁)
void scroll_main_p(int16_t y = 0, bool lock = 1) {
	if (lock) {
		if (lvgl_mux_lock()) { // 上锁
			lv_obj_scroll_to_y(main_panel, y, LV_ANIM_ON);
			lv_obj_update_layout(main_panel);
			lvgl_mux_unlock(); // 解锁
		}
	} else {
		lv_obj_scroll_to_y(main_panel, y, LV_ANIM_ON);
		lv_obj_update_layout(main_panel);
	}
}

// 隐藏 所有对话相关 组件 (默认加锁)
void hide_all_dialog_components(bool lock = 1) {
	if (lock) {
		if (lvgl_mux_lock()) { // 上锁
			lv_obj_add_flag(ta, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(main_panel, LV_OBJ_FLAG_HIDDEN);
			hide_pinyin(0);
			lvgl_mux_unlock(); // 解锁
		}
	} else {
		lv_obj_add_flag(ta, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(main_panel, LV_OBJ_FLAG_HIDDEN);
		hide_pinyin(0);
	}
}
// 显示 所有对话相关 组件 (默认加锁)
void recover_all_dialog_components(bool lock = 1) {
	if (lock) {
		if (lvgl_mux_lock()) { // 上锁
			lv_obj_clear_flag(ta, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(main_panel, LV_OBJ_FLAG_HIDDEN);
			recover_pinyin(0);
			lvgl_mux_unlock(); // 解锁
		}
	} else {
		lv_obj_clear_flag(ta, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(main_panel, LV_OBJ_FLAG_HIDDEN);
		recover_pinyin(0);
	}
}

// 隐藏 拼音输入 相关 (默认加锁)
void hide_pinyin(bool lock = 1) {
	if (!typing_pinyin) return;
	if (lock) {
		if (lvgl_mux_lock()) { // 上锁
			lv_obj_add_flag(candidate_l, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(pinyin_input_l, LV_OBJ_FLAG_HIDDEN);
			lvgl_mux_unlock(); // 解锁
		}
	} else {
		lv_obj_add_flag(candidate_l, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(pinyin_input_l, LV_OBJ_FLAG_HIDDEN);
	}
}
// 恢复 拼音输入框 相关 (默认加锁)
void recover_pinyin(bool lock = 1) {
	if (!typing_pinyin) return;
	if (lock) {
		if (lvgl_mux_lock()) { // 上锁
			lv_obj_clear_flag(candidate_l, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(pinyin_input_l, LV_OBJ_FLAG_HIDDEN);
			lvgl_mux_unlock(); // 解锁
		}
	} else {
		lv_obj_clear_flag(candidate_l, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(pinyin_input_l, LV_OBJ_FLAG_HIDDEN);
	}
}

// 匹配拼音 -> 词
void word_match(const String &input_str, std::vector<String> &word_result, String &split_result) {
	split_result = "";
	word_result.clear();
	// word_result.shrink_to_fit();
	std::vector<String>().swap(word_result);

	if (!input_str.length()) {
		Serial.println("[Error] word_match(): input_str is empty!");
		return;
	}

	// print_heap_free();
	match_case_node_t sp;
	uint16_t idx = 0;

	Serial.print("[INFO] word_match(): input_str="); Serial.println(input_str);
	word_dict_search_result_t *blk = zh_match_word(input_str.c_str(), &sp);

	uint8_t loc = 0;
	for (int i = 0; i < strlen(input_str.c_str()); i++) {
		if (i == sp.spm[loc]) {
			split_result += "'";
			loc++;
		}
		split_result += input_str.c_str()[i];
	}
	char code_str[3 * MAX_WORD_LENGTH + 1];
	for (word_dict_search_result_t *w = blk; w != NULL; w = w->next) {
		if (w->type == WORD_BLK_TYPE_CODES) {
			for (int i = 0; i < w->num.code_nbr; i++) {
				idx++;
				strncpy(code_str, w->buf + 3 * i, 3);
				code_str[3] = '\0';
				word_result.push_back(code_str);
			}
		} else { /* WORD_BLK_TYPE_WORDS */
			uint16_t buf_idx = 0;
			for (int i = 0; w->num.word_nbr[i] != 0; i++) {
				idx++;
				// Serial.println();
				// Serial.println(3 * w->num.word_nbr[i]);
				strncpy(code_str, w->buf + buf_idx, 3 * w->num.word_nbr[i]);
				buf_idx += 3 * w->num.word_nbr[i];
				code_str[3 * w->num.word_nbr[i]] = '\0';
				word_result.push_back(code_str);
			}
		}
	}
	zh_word_free_match(blk);
	// print_heap_free();
}

// 清空 拼音输入框 与 候选栏 (未加锁)
void clear_pinyin() {
	lv_label_set_text(pinyin_input_l, "");
	lv_label_set_text(candidate_l, "");
	word_result.clear();
	// word_result.shrink_to_fit();
	std::vector<String>().swap(word_result);
	pinyin_str = "";
	candidate_offset = 0;
}

// 更新 候选栏
void update_candidate() {
	candidate_str = "";
	if (!pinyin_str.length() && word_result.empty()) {
		Serial.println("匹配结果为空");
		if (lvgl_mux_lock()) {  // 上锁
			lv_label_set_text(candidate_l, "");
			lvgl_mux_unlock(); // 解锁
		}
		return;
	}
	candidate_offset = constrain(candidate_offset, 0, word_result.size()-1);
	for (int i=candidate_offset, j=1; i<word_result.size(); i++, j++) {
		candidate_str += String(j) + "." + word_result[i] + " ";
	}
	if (lvgl_mux_lock()) {  // 上锁
		lv_label_set_text(candidate_l, candidate_str.c_str());
		lvgl_mux_unlock(); // 解锁
	}
}

// 更新 候选词 与 候选栏
void update_word_match() {
	candidate_offset = 0;
	word_match(pinyin_str, word_result, split_result);
	update_candidate();
}

// 处理其他输入的键
void proc_other_input_key() {
	// 拼音输入下的 字母输入
	if (typing_pinyin && proc_key[0] >= 'a' && proc_key[0] <= 'z') {
		pinyin_str += proc_key;
		if (lvgl_mux_lock()) { // 上锁
			lv_label_set_text(pinyin_input_l, pinyin_str.c_str());
			lvgl_mux_unlock(); // 解锁
		}
		update_word_match();
	}
	// 选择候选词:  拼音输入模式  &&  word_result不为空     &&       proc_key 在 1 ~ 9 中
	else if (typing_pinyin && word_result.size() && proc_key[0] >= '1' && proc_key[0] <= '9') {
		if (proc_key.toInt()+candidate_offset <= word_result.size()) { // proc_key加偏移 超过 word_result 长度, 忽略
			if (lvgl_mux_lock()) { // 上锁
				lv_textarea_add_text(ta, word_result[ proc_key.toInt() + candidate_offset - 1 ].c_str());
				clear_pinyin();
				lvgl_mux_unlock(); // 解锁
			}
		}
		
	}
	// 拼音输入下的 0 等同与 ETR(Enter)
	else if (typing_pinyin && pinyin_str.length() && proc_key == "0") {
		if (lvgl_mux_lock()) { // 上锁
			lv_textarea_add_text(ta, lv_label_get_text(pinyin_input_l));
			clear_pinyin();

			lvgl_mux_unlock(); // 解锁
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
		if (candidate_offset+MOVE_WORDS+MIN_CANDIDATE_NUM >= word_result.size()) {
			candidate_offset = word_result.size() - MIN_CANDIDATE_NUM;
		} else candidate_offset += MOVE_WORDS;
		update_candidate();
	}
	// 拼音输入下的 其他字符 及 正常输入
	else {
		ta_add_text(proc_key.c_str());
	}
}

// 读取 IMU 数据, 并 更新滚动位置
void my_read_imu() {
    IMU.update();
    IMU.getAccel(&accelData); // accelData.accelX accelData.accelY accelData.accelZ
    // IMU.getGyro(&gyroData); // gyroData.gyroX gyroData.gyroY) gyroData.gyroZ
    // IMU.getTemp();

    if (lvgl_mux_lock()) { // 上锁
		lv_obj_scroll_by_bounded(main_panel, 0, 0-accelData.accelY*SCROLL_MULTI, LV_ANIM_ON);
		lvgl_mux_unlock();
	}

    // touchpad_x -= accelData.accelX * 8;
	// touchpad_y -= accelData.accelY * 8;
	// if (touchpad_x < 0) touchpad_x = 0;
	// if (touchpad_x > 319) touchpad_x = 319;
	// if (touchpad_y < 0) touchpad_y = 0;
	// if (touchpad_y > 239) touchpad_y = 239;
	// Serial.println(touchpad_x);Serial.print(",");Serial.println(touchpad_y);
	// Serial.println(accelData.accelX);Serial.print(",");Serial.println(accelData.accelY);
}

static void ta_event_cb(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * ta = (lv_obj_t *)lv_event_get_target(e);
	lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);	
	if(code == LV_EVENT_FOCUSED) {
		/* 1. 恢复键盘正常高度 (自动计算或设具体值)，确保虚拟按键可见 */
		lv_obj_set_height(kb, LV_SIZE_CONTENT); 
		// 或者如果你只想显示候选栏，在这里判断是否需要显示全键盘
        // 但通常聚焦时需要全键盘以便输入
        
        /* 2. 关联文本框并显示 */
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        
        /* 3. (可选) 如果确实只需要候选栏，可以在这里调整，但不要固定为 10% 导致无法恢复 */
        // lv_obj_set_style_height(kb, LV_PCT(10), 0); // <--- 删除这行，防止变矮
    }
    else if(code == LV_EVENT_CANCEL) {
		/* 隐藏键盘 */
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta);
    }
}

/* 物理按键输入接口 */
void send_key_to_ta(uint32_t key) {
	// 确保文本框有焦点
    if(ta) {
		if (lvgl_mux_lock()) { // 上锁
			lv_obj_add_state(ta, LV_STATE_FOCUSED);
			// 发送给键盘 (kb)
			if(kb) {
				// Serial.printf("send_key_to_ta: kb: %c\r\n", (char)key);
				// 向文本框发送 KEY 事件
				lv_event_send(ta, LV_EVENT_KEY, &key);
			} else Serial.println("send_key_to_ta: kb is NULL!");
			
			lvgl_mux_unlock();
		}
	} else Serial.println("send_key_to_ta: ta is NULL!");
}

// // 鼠标回调
// void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data) {
// 	data->point.x = touchpad_x + 0.5;
// 	data->point.y = touchpad_y + 0.5;
// 	if(digitalRead(0) == 0) {
// 		// Serial.println("PRESSED");
// 		data->state = LV_INDEV_STATE_PRESSED;
// 	} else {
// 		// Serial.println("Released");
// 		data->state = LV_INDEV_STATE_RELEASED;
// 	}
// }


// 初始化 摄像头
void init_camera() {
	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_7; // 尽可能 避开 其他功能已使用的
	config.ledc_timer   = LEDC_TIMER_3;   // 尽可能 避开 其他功能已使用的
	config.pin_d0 = Y2_GPIO_NUM;
	config.pin_d1 = Y3_GPIO_NUM;
	config.pin_d2 = Y4_GPIO_NUM;
	config.pin_d3 = Y5_GPIO_NUM;
	config.pin_d4 = Y6_GPIO_NUM;
	config.pin_d5 = Y7_GPIO_NUM;
	config.pin_d6 = Y8_GPIO_NUM;
	config.pin_d7 = Y9_GPIO_NUM;
	config.pin_xclk   = XCLK_GPIO_NUM;
	config.pin_pclk   = PCLK_GPIO_NUM;
	config.pin_vsync  = VSYNC_GPIO_NUM;
	config.pin_href   = HREF_GPIO_NUM;
	config.pin_sccb_sda = SIOD_GPIO_NUM;
	config.pin_sccb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn   = PWDN_GPIO_NUM;
	config.pin_reset  = RESET_GPIO_NUM;
	config.xclk_freq_hz = 24000000;
	// config.xclk_freq_hz = 26400000; // 超频极限
	// config.frame_size = FRAMESIZE_HVGA;
	config.frame_size   = FRAMESIZE_QVGA;
	config.pixel_format = PIXFORMAT_RGB565;  // for face detection/recognition
	// config.pixel_format = PIXFORMAT_JPEG; // for streaming
	config.grab_mode   = CAMERA_GRAB_WHEN_EMPTY;
	config.fb_location = CAMERA_FB_IN_PSRAM;
	config.jpeg_quality = 6;
	config.fb_count = 1;

	esp_err_t err = esp_camera_init(&config);
	if (err != ESP_OK) {
		Serial.printf("Camera init failed with error 0x%x", err);
		return;
	}

	// sensor_t * s = esp_camera_sensor_get();
	// s->set_hmirror(s, 1);
	// s->set_vflip(s, 1);

	img_dsc.header.always_zero = 0;
	img_dsc.header.w = 480;
	img_dsc.header.h = 320;
	img_dsc.data_size = 320 * 480 * 2;
	// img_dsc.header.w = 320;
	// img_dsc.header.h = 240;
	// img_dsc.data_size = 240 * 320 * 2;
	img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
	img_dsc.data = NULL;

	// lv_img_set_src(img_camera, &pic);
}
// 反初始化 摄像头
void deinit_camera() {
	esp_err_t err = esp_camera_deinit();
	if (err != ESP_OK) {
		Serial.printf("Camera deinit failed with error 0x%x", err);
		return;
	}
}

void camera_loop() {
	uint64_t start_time = millis();
	while (1) {
		// esp_task_wdt_reset();
		pic = esp_camera_fb_get();

		if (NULL != pic) {
			img_dsc.data = pic->buf;
			if (lvgl_mux_lock()) {
				lv_img_set_src(camera_img, &img_dsc);
				lvgl_mux_unlock();
			}
		}
		esp_camera_fb_return(pic);
		// vTaskDelay(1 / portTICK_PERIOD_MS);

		if (millis() - start_time > 150) {
			if (read_key() != "$12") {
				start_time = millis();
				vTaskDelay(1 / portTICK_PERIOD_MS);
			} else break;
		}
	}
}


// ############################### 网络 ##################################

// WiFi 事件回调函数（由系统后台任务自动调用）
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
			Serial.printf(("WiFi已断开, 原因码: " + String(info.wifi_sta_disconnected.reason) + ")").c_str());
			ta_tmp_show(("#be203a WiFi已断开, 原因码: " + String(info.wifi_sta_disconnected.reason) + ")#").c_str());

            // Serial.printf("WiFi断开（原因码: %d）, 正在尝试自动重连...", info.wifi_sta_disconnected.reason);
            break;
        default:
            break;
    }
}

// 同步 SNTP 时间
void sync_sntp() {
	syncing_sntp = true;
	// SNTP 时间同步
	Serial.print("使用 SNTP 同步时间 ...");
	main_label_set_text("#10acb1 使用 SNTP 同步时间 ...");
	configTime(GMT_OFFSET_SEC, 0, SNTP_SERVER);
	while (!getLocalTime(&timeinfo)) {
		vTaskDelay(700 / portTICK_PERIOD_MS);
		Serial.print(".");
		main_label_add_text(".");
	}
	syncing_sntp = false;
	Serial.println(" OK");
	main_label_add_text("# #3add70 OK#");
}

// 等待 WiFi 连接
void wait_wifi_connection(void *param) {
	Serial.print("连接WiFi中 ..");
	main_label_set_text("#1058b1 连接WiFi中 ..");
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print('.');
		main_label_add_text(".");
		vTaskDelay(700 / portTICK_PERIOD_MS);
	}
	// Serial.println(WiFi.localIP());
	connecting_wifi = false;
	Serial.println(" OK");
	main_label_add_text("# #3add70 OK#");

	sync_sntp();
	vTaskDelete(TASK_Handle_WiFi);
}

// 选择SSID 并 连接WiFi
void connect_wifi() {
	Serial.println("初始化 WiFi");

	int16_t ssid_num = sizeof(ssids) / sizeof(ssids[0]) - 1;
	int16_t password_num = sizeof(passwords) / sizeof(passwords[0]) - 1;
	if (ssid_num != password_num) {
		Serial.println("SSID 数量与密码数量不一致");
		main_label_set_text("SSID 数量与密码数量不一致");
		while (1) vTaskDelay(10000 / portTICK_PERIOD_MS);
	}
	
	main_label_text_tmp = "#0060b9 请选择 WIFI：\n";
	for (int16_t i=1; i<=ssid_num; i++) {
		main_label_text_tmp += i;
		main_label_text_tmp += ": ";
		main_label_text_tmp += ssids[i];
		main_label_text_tmp += "\n";
	}
	main_label_set_text(main_label_text_tmp.c_str());

	// 等待选择 SSID 并初始化 WiFi
	while (1) {
		proc_key = wait_until_read_key();
		if (proc_key.toInt() > 0 && proc_key.toInt() <= ssid_num) {
			connecting_wifi = true;
			int wifi_choose = proc_key.toInt();
			
			Serial.println("选择 SSID: " + proc_key);
			WiFi.mode(WIFI_STA);
			WiFi.begin(ssids[wifi_choose], passwords[wifi_choose]);

			xTaskCreate(
				wait_wifi_connection,     // 任务函数
				"wait_wifi_connection",   // 任务名称
				2500,              // 堆栈大小
				NULL,              // 参数
				1,                 // 优先级
				&TASK_Handle_WiFi  // 任务句柄
			);
			break;
		}
		else if (proc_key == "$12") {
			skip_wifi = 1;
			Serial.println("跳过 WIFI 连接");
			main_label_set_text("#115df5 跳过 WIFI 连接#");
			break;
		}
		else Serial.println("未知 SSID");

		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}



// ############################### 其他 ##################################

// 获取 剩余RAM
void print_heap_free(String title = "========") {
	// 查看 片内 RAM 和 PSRAM 剩余堆
	Serial.println();
	Serial.println("========= " + title + " =========");
	Serial.printf ("== 堆 最大分配: %.2f KiB ====\r\n", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL) / 1024.0);
	Serial.printf ("== 堆 空闲: %.2f KiB =======\r\n",  heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024.0);
	Serial.printf ("== PSRAM 空闲: %.2f KiB ==\r\n",   heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024.0);
	Serial.println("============================");
}

// 在 Core0(my_loop) 空闲(等待)时 执行的代码
// 限制: 在处理 耗时较长的任务[等待结果，录音时]时, 无法执行
void core0_loop_func() {
	client.poll(); // 处理接收的消息
}


