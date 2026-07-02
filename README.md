
# ESP Qwen Edge - Master #

### 当前文档未完善 ###

## 项目介绍 
该项目是一个基于ESP32-S3的智能终端设备，集成了2英寸LCD显示屏, 已实现语音识别（ASR）、IMU传感器，并通过 阿里云Qwen大模型 实现AI对话功能, 可选择不同模型。支持WiFi连接、SD卡读取、LVGL图形界面和拼音输入法(正在实现)。

## 本项目可供自行DIY, 如要复刻可根据以下步骤 ##

### 1. 准备材料 (购买链接, 含参考价) : ###

 -  72.8元 \- [ESP32-S3-2寸LCD开发板](https://item.taobao.com/item.htm?id=965108138294&skuId=5908711489110)

### 2. 准备环境: ###
本项目使用 PlatformIO 开发，也可以用 Arduino。
推荐使用 VSCode + PlatformIO （编译速度比 Arduino 快）

安装配置详见：
PlatformIO：[ESP32开发环境搭建(VSCode+PlatformIO) - 知乎](https://zhuanlan.zhihu.com/p/683949878 "ESP32开发环境搭建(VSCode+PlatformIO)")
Arduino：[安装和配置 Arduino IDE - 微雪文档](https://docs.waveshare.net/ESP32-Arduino-Tutorials/Arduino-IDE-Setup "安装和配置 Arduino IDE - 微雪文档")

如果是第一次用，可以先试一下 Hello World。

### 3. 下载代码: ###

```shell
git clone https://github.com/ioqite/esp-qwen-edge-master.git
```

### 4. 配置变量: ###

将 `src/user_config-template.h` 头文件复制一份 并 命名为 `user_config.h`，然后再修改

- `QWEN_API_SECRET_KEY`: 从阿里云Qwen大模型获取的API密钥
- `ssids` 和 `passwords`: 可供启动时选择的 WiFi 的 SSID 和 密码

### 5. 编译烧录 ###

```shell
platformio  upload
```


### 未完待续 ###

