### 本 example 包含所有测试

littlefs_data 目录 位于 本库的根目录下(example 内不存放副本),  存放的是 LittleFS 的数据
ffat_data 目录 位于 本库的根目录下(example 内不存放副本),  存放的是 FatFS(FFat) 的数据

### 使用方法：

#### 1.在 platformio.ini中 配置 你的开发板

#### 2.将 zh_pinyin_decoder-arduino库 复制到 ./lib 中

#### \[可选\] 3.在 user_config.h 中修改 Flash内用于存储 拼音码表和词库(json)文件 的文件系统:  \[推荐\] FatFS(FFat) 或 LittleFS：

```cpp
// 选择文件系统 (不要同时选择)
#define USE_FAT_FS               1  // [推荐] 使用 FatFS(FFat) 文件系统
#define USE_LITTLE_FS            0  // 使用LittleFS 文件系统
```

#### \[可选\] 4.在 user_config.h 中修改 是否使用日志 及 日志输出方式

#### 5.烧录文件系统：

> 注: 本 example 的根目录下提供了 推荐的 ESP32 分区表 (zh_pinyin_ffat_partition.csv) 与 推荐的 ESP32-S3 配置

##### (1) 使用 默认拼音码表与词库 或 制作文件系统
** \[推荐\] 如果使用 默认拼音码表与词库， 就无需制作文件系统，直接跳至 (2) 即可 **

自定义拼音码表与词库，此处以 Linux 中为例, 其他系统可根据 [esp32_fatfsimage](https://github.com/marcmerlin/esp32_fatfsimage) 的源代码 (.cpp) 自行编译

使用 marcmerlin/esp32_fatfsimage 提供的工具 fatfsimage (Linux版 在本库的根目录下有其副本) 来制作 FatFS(FFat) 文件系统:

首先, 将 分区表中 ffat分区 的大小\(Size\)转换为 10进制, 再除以 1024, 该结果为 FFat 文件系统的大小 (单位: KiB)
下一步, 运行以下命令 制作文件系统镜像:
```bash
./fatfsimage -l5 <要制作的镜像文件名> <FFat 文件系统的大小 (单位: KiB)> <存放要写入数据的目录路径>
```
例如: 
```bash
./fatfsimage -l5 img.ffat 5008 ./ffat_data
```

##### (2) 根据单片机类型 烧录文件系统，此处以 ESP32 为例
**如使用 默认镜像 和 默认分区表, 直接在 本库的根目录下 运行 下面的示例 即可**

运行以下命令 烧录文件系统到 ESP32 (确保已安装 esptool 工具):
```bash
esptool --baud 921600 write_flash <分区表中 ffat分区 的偏移位置\(Offset\)> <镜像文件路径>
```
示例:
```bash
esptool --baud 921600 write_flash 0x310000 img.ffat
```

#### 6.Build and Flash 吧 ！

#### 使用技巧: 

**注意：在使用前，需要先初始化拼音识别器，调用** `zh_pinyin_begin()` **函数。**
**在使用完成后，需要调用** `zh_pinyin_end()` **函数关闭拼音识别器。**





