# S3cam：仅摄像头拍照、本地浏览

Copyright (c) 2026 Keyes DIY Robot.

本示例用于 S3cam/Keyestudio MB0184 ESP32-S3 CAM 接线，目标为 **Arduino ESP32 Core 3.2.0**、ESP32-S3-WROOM-1-N16R8、16 MB Flash 和 8 MB OPI PSRAM。官方 MB0184 文档标注 OV3660；用户提供的 S3cam README 标注 OV2640，所提供 `config.h` 的摄像头 GPIO 与官方映射相同。[1] [2] 程序由 `esp_camera` 自动探测传感器并打印 PID 及 OV2640/OV3660 名称，不硬性限定 OV3660。实际硬件型号以检测结果和实物为准。

程序获得 JPEG QVGA 图片，在同一 Wi-Fi 的浏览器中点击按钮查看照片；不会自动向公网或云端上传。这是独立摄像头测试，不是 AI 聊天、AI 图像识别或语音示例。`ai_bot_camera` 命名只沿用仓库示例风格，不代表已经合并语音和 AI。

用户已提供外接音频和 LCD 接线，但本例刻意不初始化这些功能：MIC BCLK=2、WS=1、DIN=42；SPK BCLK=40、WS=41、DOUT=39；LCD BL=38、MOSI=20、CLK=19、DC=47、RST=21、CS=45。独立测试也不初始化马达、闪光灯或 LED。SD CMD=38、CLK=39、D0=40 与上述音频/LCD 接线冲突，因此不挂载、读写或格式化 SD 卡。[1] 不需要部署网站；固件中的 `WebServer` 就是本地 HTTP 服务。

## 文件与摄像头引脚

用 Arduino IDE 打开 `ai_bot_camera.ino`。它是与 `ai_bot_camera` 目录同名的空索引，实际 `setup()`、`loop()` 在 `main.cpp`。`camera_pins.h` 保留引脚映射参考；`build_opt.h` 是 Arduino 草图编译选项；`partitions.csv` 为仓库根目录同名文件的原样副本。

可复用摄像头实现位于仓库 `src/board_support/s3cam_camera.h`，本例用 `#include "board_support/s3cam_camera.h"` 引入。它是 header-only 类，不新增全局 `.cpp` 或全局摄像头对象，其他板卡只有主动包含并使用它才引入摄像头支持。Arduino IDE 需安装包含该新头文件的本仓库 Arduino 库，或按 PlatformIO 参考添加仓库 `src` 头文件搜索路径；不要只拷贝本示例目录后遗漏该头文件。

引脚采用 Core 3.2.0 官方 `CAMERA_MODEL_ESP32S3_EYE` 定义，与用户配置和 MB0184 文档一致：SIOD=4、SIOC=5、VSYNC=6、HREF=7、XCLK=15、PCLK=13，Y9=16、Y8=17、Y7=18、Y6=12、Y5=10、Y4=8、Y3=9、Y2=11。Y2～Y9 按顺序连接 `pin_d0`～`pin_d7`。[1] [2]

官方 ESP32S3_EYE 的 `PWDN_GPIO_NUM` 和 `RESET_GPIO_NUM` 均为 `-1`；用户配置也指定不连接这两个引脚。因此不擅自指定 GPIO。[2] 该模型名称只复用摄像头接线，不代表实物是 ESP-EYE，也不会开启其音频或识别功能。

## Wi-Fi 配置

默认 `WIFI_SSID` 与 `WIFI_PASSWORD` 都为空。在 `main.cpp` 的 `#ifndef` 内填写默认值，或者由编译参数提供带正确引号的字符串宏。源码方式例如：

```cpp
#ifndef WIFI_SSID
#define WIFI_SSID "your_wifi_ssid"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_wifi_password"
#endif
```

请将占位字符串替换为自己的 2.4 GHz Wi-Fi 信息，空密码仅用于确实开放的网络。不要把实际凭据提交到公开仓库；编译期凭据不是加密存储。本例不打印密码，不将凭据持久化到 Flash，不开启 AP、SmartConfig 或配网服务。

未填写 SSID 时串口提示填写方法，关闭 Wi-Fi 并保持 idle，不初始化摄像头或 HTTP 服务。连接最多等待 20 秒，失败后释放摄像头、关闭 Wi-Fi 并保持 idle。修正配置后重新上传并复位，不无限阻塞等待连接或反复重启。

## Arduino IDE

安装 Espressif **esp32 3.2.0** 开发板包和本仓库 Arduino 库，打开 `ai_bot_camera.ino`，切换到 `main.cpp` 填写 Wi-Fi 信息。选择 **ESP32S3 Dev Module**，设置 **Flash Size: 16 MB** 和 **PSRAM: OPI PSRAM**，不是 QSPI PSRAM。使用草图自带的 `partitions.csv`，不要使用 4 MB 板卡分区表。串口监视器为 **115200**。

完整用户接线中 LCD 使用 GPIO19/20，这两个 GPIO 与 ESP32-S3 原生 USB-OTG 冲突。建议统一使用板卡 **CH340 UART USB 口** 上传及查看串口，设置 **USB CDC On Boot: Disabled**；不要把完整 LCD 接线误当成可同时使用原生 USB 的方案。本独立测试不初始化 LCD，但若硬件仍按完整接线连接，也应遵循上述 UART 方式。

使用数据 USB 线和稳定电源。程序不会无限等待串口打开；错过启动日志时可先打开串口监视器再复位。出现 `PSRAM not detected` 请检查 OPI PSRAM、板型和实际模块，不要修改 `psramFound()` 或添加“假初始化”绕过错误。

## PlatformIO 独立构建参考

board 为 `esp32-s3-devkitc1-n16r8`。不要将独立摄像头主程序与其他 AI 示例入口一起编译。本例作者未改仓库根目录 `platformio.ini`；下面是仓库外临时独立配置的参考，绝对路径应替换为实际路径。

```ini
[platformio]
src_dir = /absolute/path/to/ai_bot/examples/esp32s3/s3cam/ai_bot_camera

[env:s3cam_ai_bot_camera]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/54.03.20/platform-espressif32.zip
framework = arduino
board = esp32-s3-devkitc1-n16r8
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.psram_type = opi
board_upload.flash_size = 16MB
board_build.partitions = /absolute/path/to/ai_bot/examples/esp32s3/s3cam/ai_bot_camera/partitions.csv
build_src_filter = +<main.cpp>
monitor_speed = 115200
build_flags =
  -I/absolute/path/to/ai_bot/src
  -DBOARD_HAS_PSRAM
  -DARDUINO_USB_CDC_ON_BOOT=0
  -fno-exceptions
  -Wall
  -Werror
  -DWIFI_SSID=\"your_wifi_ssid\"
  -DWIFI_PASSWORD=\"your_wifi_password\"
```

平台应提供 Arduino Core 3.2.0，构建时核对实际安装 framework 版本，不要用浮动 `stable` 代替确认。平台版本号与仓库原有 54.03.20 系列一致。带空格或特殊字符的凭据需要遵循 PlatformIO/Shell 转义规则，初次可直接使用源码方式。编译宏不要和源码重复定义。本固件仅需要 Core 的 `WiFi`、`WebServer`、`esp_camera` 以及本仓库的 header-only 头文件，不需要 AI、音频或额外网页库。

## 可复用 `ai_bot::S3camCamera` 接口

`Begin()` 可在联网前初始化摄像头，返回是否成功；`StartHttpServer()` 在 Wi-Fi 连接后开启本地服务；主循环持续调用 `Update()`；`Ready()` 只表示摄像头已经初始化，不表示网络/服务状态；`CaptureUrl()` 在摄像头、网络和服务就绪时返回 `http://<当前局域网IP>/capture`，否则返回空 `std::string`。**这个 URL 不是云端视觉识别接口，不能直接当作公网可抓取的 AI 图片地址。**

```cpp
#include "board_support/s3cam_camera.h"
ai_bot::S3camCamera camera;

// setup 早期：
bool cameraReady = camera.Begin();
// Wi-Fi 已连接后，在与 Update 相同的任务内：
if (cameraReady) camera.StartHttpServer();
// Arduino loop 内：
camera.Update();
// 本地照片端点：
std::string localPhotoUrl = camera.CaptureUrl();
```

只使用一个活动对象，所有调用应在 Arduino setup/loop 同一任务执行，不要从网络事件任务或音频任务并发调用。HTTP 的单次请求同步处理，可能短暂占用主循环。默认占用 **LEDC timer 3 / channel 7** 产生摄像头 XCLK；其他外设不能重复使用它。构造函数允许传入其他合法 `ledc_timer_t`/`ledc_channel_t`，但资源冲突仍由调用者排除。完整 LCD 示例应使用数字背光开关或另行隔离 LEDC 资源；I2S 音频本身不占 LEDC。

复用类不控制调用者的 Wi-Fi，不提供凭据、不主动断开网络。`Update()` 检测断网后停止 HTTP，但保留摄像头；重新联网后应再次调用 `StartHttpServer()`。可选的 `End()` 停止 HTTP 并释放摄像头，不断开调用者 Wi-Fi。独立 `main.cpp` 在失败或断网时另外调用 `End()`、关闭 Wi-Fi 并保持 idle，需要复位后再尝试。

## 使用与错误处理

有效配置启动后，先检测真实 PSRAM 并初始化摄像头，再连接 Wi-Fi。只有两者均成功才开启端口 80，串口打印 `http://<板卡局域网IP>/` 和 `/capture` 地址。手机或电脑连接同一可信 Wi-Fi，在浏览器打开首页并点击 **Capture photo**。`/capture` 返回 `image/jpeg`，也可直接访问。没有连续流，按钮一次请求一张照片；路由器的访客/客户端隔离可能阻止访问。

配置为 `PIXFORMAT_JPEG`、`FRAMESIZE_QVGA`、质量参数 12、20 MHz XCLK、一个 PSRAM 帧缓冲和 `CAMERA_GRAB_WHEN_EMPTY`，图片为 320 × 240，不提供 DRAM fallback。[3] 串口记录驱动探测到的传感器 PID，兼容驱动对 OV2640 和 OV3660 的探测，不固定某一型号。按官方 ESP32S3_EYE 示例设置垂直翻转；具体安装方向不同时可调整头文件内 `set_vflip`。[3]

响应具有准确的 `Content-Length` 和禁用缓存的头部。类内 `CheckedWebServer` 覆盖 Core 3.2.0 的 `_currentClientWrite` 钩子，检查 HTTP 头和 JPEG 正文实际写入长度，使用非阻塞 socket 处理短写和临时繁忙，共享 5 秒响应发送期限并让出 Wi-Fi 任务。失败会关闭客户端，不将错误文字追加到半张 JPEG。[4] [5] 该期限不包含驱动内部拍摄等待，也不是硬实时保证。每个成功获取的帧在退出时由作用域对象调用 `esp_camera_fb_return()`，包含无效帧及发送失败路径。

采集失败返回文字错误和 HTTP 状态，允许下一次点击重试。初始化错误码输出到串口，先排查电源和排线，不要上电插拔排线。初始化失败不会启动 HTTP 服务；本独立程序断网后释放资源、关闭 Wi-Fi 并保持 idle。

## 限制与验证状态

本地 HTTP 没有 HTTPS 或身份认证，同一网络可达此板卡的客户端可以请求照片，只应在可信局域网使用。不要做公网端口映射。网页没有第三方脚本、外部资源或云端接口。

单客户端、单帧缓冲，不提供并发拍照、流媒体或文件保存。单帧 `WHEN_EMPTY` 缓冲可能包含驱动已采集的帧，不保证曝光恰好发生在点击时刻，首次曝光/颜色也可能尚未稳定，可再次点击。分区表保留原仓库的 `model` 名称，但本例从不挂载或写入该分区；使用 factory 分区，不实现 OTA。

已核对官方 Core 3.2.0 引脚、`pin_sccb_sda`/`pin_sccb_scl`、JPEG/QVGA、WebServer/NetworkClient 接口和用户配置。**本示例创建阶段未完成工具链编译，没有连接真实 OV2640/OV3660 板卡实测**；后续仍需编译、上传并验证 PSRAM、照片质量、UART 接口和供电，源码核对不等于实机通过。

正文及原创代码版权为 Keyes DIY Robot。摄像头引脚注明 Espressif 来源，上游该引脚文件没有单独版权头；WebServer/NetworkClient 等库的原第三方版权仍保留在 Core 中。本例不修改用户提供的原 ESP-IDF 源码，也不把它整体移植成 AI 识别实现。

## References

[1]: https://docs.keyestudio.com/projects/MB0184/en/latest/docs/MB0184%20ESP32-S3%20CAM%20Development%20Board.html#pin-out "Keyestudio MB0184 ESP32-S3 CAM Development Board pin-out"
[2]: https://github.com/espressif/arduino-esp32/blob/3.2.0/libraries/ESP32/examples/Camera/CameraWebServer/camera_pins.h "Arduino ESP32 Core 3.2.0 official camera pin definitions"
[3]: https://github.com/espressif/arduino-esp32/blob/3.2.0/libraries/ESP32/examples/Camera/CameraWebServer/CameraWebServer.ino "Arduino ESP32 Core 3.2.0 official CameraWebServer initialization"
[4]: https://github.com/espressif/arduino-esp32/blob/3.2.0/libraries/WebServer/src/WebServer.h "Arduino ESP32 Core 3.2.0 WebServer response API and write hook"
[5]: https://github.com/espressif/arduino-esp32/blob/3.2.0/libraries/Network/src/NetworkClient.cpp "Arduino ESP32 Core 3.2.0 NetworkClient socket send implementation"

用户提供的本地配置来源为 `pasted_file_z0iyOq_config.h`、`pasted_file_jV96cp_README.md` 和 `pasted_file_jOIqQg_compact_wifi_board_s3cam.cc`；它们分别用于核对接线、OV2640 标注和原板卡初始化资源，不作为本例已完成语音/AI 功能的证据。
