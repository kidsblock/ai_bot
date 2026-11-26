# ai_bot

**Keyes DIY Robot · Arduino AI 语音库 · v1.0.0**

面向 ESP32 和 ESP32-S3 的 AI 语音对话库，提供无屏、LCD、OLED、摄像头及四端口电机控制示例。ESP32-S3 示例按 **S3mini、S3pro、S3rover、S3cam** 开发板分类，共 **15 个示例**。

- 语音入口：[`ai_bot_engine.h`](src/ai_bot_engine.h)，C++ 命名空间为 `ai_bot`。
- 独立摄像头入口：[`ai_bot_camera.h`](src/ai_bot_camera.h)。
- 固定编译环境：**ESP32 Arduino Core 3.2.0**；显示示例使用 **LVGL 9.2.2**。

> 已于 **2026-11-23** 完成 PlatformIO **15/15** 和 Arduino CLI **15/15** 编译、链接验证。尚未完成实机烧录测试；编译通过不代表音频、屏幕、电机、摄像头或云服务已经实测。详见 [构建验证记录](BUILD_VALIDATION.md)。

## 1. 选择示例

Arduino IDE 打开示例目录内同名 `.ino`；实际应用代码在同目录的 `main.cpp`。PlatformIO 在库根目录按表中环境名构建。

| 开发板 | 示例目录 | 功能 | PlatformIO 环境 |
| --- | --- | --- | --- |
| ESP32 | [ai_bot](examples/esp32/ai_bot) | 无屏语音 | `esp32_ai_bot` |
| ESP32 | [ai_bot_display_lcd](examples/esp32/ai_bot_display_lcd) | 语音 + ST7789 LCD | `esp32_ai_bot_display_lcd` |
| ESP32 | [ai_bot_display_oled](examples/esp32/ai_bot_display_oled) | 语音 + SSD1306 OLED | `esp32_ai_bot_display_oled` |
| S3mini | [ai_bot](examples/esp32s3/s3mini/ai_bot) | 无屏语音 | `s3mini_ai_bot` |
| S3mini | [ai_bot_display_lcd](examples/esp32s3/s3mini/ai_bot_display_lcd) | 语音 + LCD | `s3mini_ai_bot_display_lcd` |
| S3mini | [ai_bot_display_oled](examples/esp32s3/s3mini/ai_bot_display_oled) | 语音 + OLED | `s3mini_ai_bot_display_oled` |
| S3pro | [ai_bot](examples/esp32s3/s3pro/ai_bot) | 无屏语音 | `s3pro_ai_bot` |
| S3pro | [ai_bot_display_lcd](examples/esp32s3/s3pro/ai_bot_display_lcd) | 语音 + 1.54″ ST7789 240×240 | `s3pro_ai_bot_display_lcd` |
| S3pro | [ai_bot_display_oled](examples/esp32s3/s3pro/ai_bot_display_oled) | 语音 + OLED | `s3pro_ai_bot_display_oled` |
| S3rover | [ai_bot](examples/esp32s3/s3rover/ai_bot) | 语音 + 四端口电机 | `s3rover_ai_bot` |
| S3rover | [ai_bot_display_lcd](examples/esp32s3/s3rover/ai_bot_display_lcd) | 语音 + LCD + 四端口电机 | `s3rover_ai_bot_display_lcd` |
| S3rover | [ai_bot_display_oled](examples/esp32s3/s3rover/ai_bot_display_oled) | 语音 + OLED + 四端口电机 | `s3rover_ai_bot_display_oled` |
| S3cam | [ai_bot](examples/esp32s3/s3cam/ai_bot) | 语音 + 摄像头，无 LCD | `s3cam_ai_bot` |
| S3cam | [ai_bot_display_lcd](examples/esp32s3/s3cam/ai_bot_display_lcd) | 语音 + 摄像头 + LCD | `s3cam_ai_bot_display_lcd` |
| S3cam | [ai_bot_camera](examples/esp32s3/s3cam/ai_bot_camera) | 独立摄像头拍照，不启动 AI 引擎 | `s3cam_ai_bot_camera` |

S3mini 使用原通用 S3 接线。S3pro、S3rover、S3cam 使用各自板级接线，不能直接互换。S3cam 不提供 OLED 示例，避免未经确认的接线与摄像头 GPIO 冲突。

## 2. Arduino IDE 快速开始

1. 安装 **ESP32 Arduino Core 3.2.0**。LCD/OLED 示例另需安装 **LVGL 9.2.2**。
2. 将本仓库文件放入 Arduino 库目录下的 `ai_bot` 文件夹，确保 `ai_bot/library.properties` 与 `ai_bot/src/` 位于同一层。也可使用 Arduino IDE 的“添加 .ZIP 库”安装包含该结构的发布包。
3. 打开上表中所选示例的同名 `.ino`，按下表选择开发板和分区。
4. 核对接线、配置 Wi-Fi，编译并上传；串口波特率设为 **115200**。

| 项目 | 普通 ESP32 | ESP32-S3 示例 |
| --- | --- | --- |
| 开发板 | `ESP32 Dev Module` | `ESP32S3 Dev Module` |
| Flash | 与实物一致；示例按 4MB 配置 | 默认 16MB |
| Flash Mode | 按开发板设置 | `QIO` |
| PSRAM | 按开发板设置 | `OPI PSRAM`，默认 8MB |
| Partition Scheme | `Huge APP (3MB No OTA)` | `Custom`，使用所选示例的 `partitions.csv` |

S3 默认配置对应 **N16R8**。语音示例需要 **4MB APP 分区**；不要使用容量不足的默认分区。独立摄像头示例使用自身的分区表。实物容量不同时，必须同步调整板级配置和分区，不能只按开发板名称判断。

**USB 注意事项：** S3cam 的 LCD 占用 GPIO19/20，合并 LCD 示例应使用板上的 **CH340 UART** 下载/监视口，并关闭 `USB CDC On Boot`，不能同时使用 USB-OTG。S3pro LCD 的 CLK 占用 GPIO20，也不要启用占用该脚的原生 USB。

## 3. PlatformIO 编译与上传

从仓库根目录运行以下命令，将环境名替换成上表中的目标环境：

```bash
# 编译
pio run -e s3mini_ai_bot

# 编译并上传
pio run -e s3mini_ai_bot -t upload

# 串口监视
pio device monitor -b 115200

# 只测试 S3cam 摄像头
pio run -e s3cam_ai_bot_camera -t upload
```

编译全部 15 个环境：

```bash
pio run
```

[`platformio.ini`](platformio.ini) 固定使用 pioarduino [54.03.20](https://github.com/pioarduino/platform-espressif32/releases/tag/54.03.20)，对应 Arduino Core **3.2.0**、ESP-IDF **5.4.1**。首次构建需要下载平台、框架和显示依赖；本库包含必要的预编译音频库，不要随意混用其他 SDK/Core 版本。

## 4. Wi-Fi 与语音使用

语音示例默认使用 **SmartConfig（ESPTouch / AirKiss）** 配网。若要直接连接指定网络，在所选示例的 `main.cpp` 中取消注释并同时定义 `WIFI_SSID`、`WIFI_PASSWORD`，填写自己的网络信息。使用 **2.4GHz Wi-Fi**，不要将真实 Wi-Fi 密码提交到公开仓库。

联网后，按串口或屏幕提示完成小智服务激活。源码会显示激活事件，不会仅靠按键自动完成账号激活；云端服务、账号和网络可用性不属于本地编译验证范围。

- **普通 ESP32：** 按键接 **GPIO34**，低电平有效，需要外部上拉到 **3.3V**；不是通用开发板自带的 GPIO0 BOOT 键。
- **ESP32-S3：** 示例按键接 **GPIO0 / BOOT**。S3rover 按键还会提交停止电机的请求，由主循环执行。

配网阶段按键可重新触发 SmartConfig；配网后按键调用语音引擎的 `Advance()` 推进对话状态，不是音量或 LED 专用快捷键。

音频默认使用**标准 I2S**。普通 ESP32、S3mini、S3pro 保留可选 PDM 分支，但默认接线不能直接用于任意 PDM 麦克风；S3rover、S3cam 限定标准 I2S，避免与板上其他功能冲突。

## 5. 接线速查

以下数字均为 **GPIO 编号**，不是排针序号。ESP32 GPIO 不能直接接入 5V。

### 音频

麦克风顺序为 **BCLK / WS / DIN**，扬声器顺序为 **BCLK / WS / DOUT**。

| 开发板 | I2S 麦克风 | I2S 扬声器 |
| --- | --- | --- |
| ESP32 | 25 / 26 / 27 | 33 / 32 / 23 |
| S3mini | 6 / 5 / 4 | 9 / 10 / 11 |
| S3pro | 6 / 5 / 4 | 9 / 10 / 11 |
| S3rover | 5 / 4 / 6 | 15 / 16 / 7 |
| S3cam | 2 / 1 / 42 | 40 / 41 / 39 |

### ST7789 LCD

| 开发板 | BL | MOSI | CLK | DC | RST | CS |
| --- | --- | --- | --- | --- | --- | --- |
| ESP32 | 19 | 16 | 4 | 5 | 17 | 18 |
| S3mini | 46 | 17 | 16 | 8 | 18 | 3 |
| S3pro | 42 | 21 | 20 | 40 | 38 | 41 |
| S3rover | 42 | 47 | 21 | 40 | 45 | 41 |
| S3cam | 38 | 20 | 19 | 47 | 21 | 45 |

S3pro 已确认使用 **1.54″ ST7789 240×240**。S3cam 默认使用 ST7789 240×240 配置，但当前硬件资料未确认所选 LCD 尺寸；尺寸、偏移和 SPI 模式可在 [`s3cam_display_config.h`](examples/esp32s3/s3cam/ai_bot_display_lcd/s3cam_display_config.h) 调整。该示例不是原 ESP-IDF 工程所有 LCD 驱动选项的完整移植。

### SSD1306 OLED

示例要求 **128×64、I2C 地址 0x3C**。

| 开发板 | SDA | SCL |
| --- | --- | --- |
| ESP32 | 21 | 22 |
| S3mini | 14 | 13 |
| S3pro | 14 | 13 |
| S3rover | 8 | 9 |

S3rover 的 I2C 引脚已确认，但表中 SSD1306 是示例要求，不代表板载或实际外接 OLED 型号已确认。

## 6. S3cam 摄像头：设置在哪里？

**真正生效的摄像头引脚和图像参数，在 [`src/board_support/s3cam_camera.h`](src/board_support/s3cam_camera.h) 的 `S3camCamera::Begin()` 内。** 三个 S3cam 示例共享这个实现。

| 文件 | 用途 |
| --- | --- |
| [`src/ai_bot_camera.h`](src/ai_bot_camera.h) | 独立摄像头公开入口，便于 Arduino 正确发现库 |
| [`src/board_support/s3cam_camera.h`](src/board_support/s3cam_camera.h) | 摄像头初始化、实际引脚、JPEG 参数和局域网 HTTP 服务 |
| [`camera_pins.h`](examples/esp32s3/s3cam/ai_bot_camera/camera_pins.h) | 独立示例内的接线参考；**仅修改此文件不会改变实际配置** |
| [`s3cam_display_config.h`](examples/esp32s3/s3cam/ai_bot_display_lcd/s3cam_display_config.h) | LCD 尺寸、偏移与 SPI 配置，不是摄像头配置 |

### 默认摄像头接线与参数

```text
D0…D7 = 11, 9, 8, 10, 12, 18, 17, 16
XCLK=15   PCLK=13   VSYNC=6   HREF=7
SIOD=4    SIOC=5    PWDN=-1   RESET=-1
```

默认参数为 **XCLK 20MHz、JPEG、QVGA 320×240、JPEG quality 12、PSRAM 单帧缓冲**。需要修改时，调整共享头文件 `Begin()` 中对应的 `camera_config_t` 字段，再重新编译上传。

接线依据包括板级资料与 [MB0184 官方文档](https://docs.keyestudio.com/projects/MB0184/en/latest/docs/MB0184%20ESP32-S3%20CAM%20Development%20Board.html#pin-out)。资料分别出现 OV2640 和 OV3660；程序不强制传感器型号，驱动自动识别并在串口报告 PID，以实物为准。

### 独立摄像头测试

1. 打开 [`ai_bot_camera`](examples/esp32s3/s3cam/ai_bot_camera) 示例，在其 `main.cpp` 中填写以下宏的默认值：

   ```cpp
   #ifndef WIFI_SSID
   #define WIFI_SSID "your_wifi_ssid"
   #endif
   #ifndef WIFI_PASSWORD
   #define WIFI_PASSWORD "your_wifi_password"
   #endif
   ```

2. 按前述 S3 配置启用 OPI PSRAM，编译上传 `s3cam_ai_bot_camera`。
3. 打开 **115200** 波特率串口，检查摄像头初始化和 Wi-Fi 连接结果。
4. 在同一可信局域网中打开串口打印的 `http://<设备IP>/` 页面，点击拍照；`http://<设备IP>/capture` 直接返回 JPEG。

此独立示例不启动音频、LCD 或 AI 引擎，也不提供 AP/自动配网。SSID 为空时不会启动摄像头或网络服务；连接超时或运行中断网后进入停止状态，修正问题并复位重试。更多细节见 [独立摄像头说明](examples/esp32s3/s3cam/ai_bot_camera/README.md)。

### 当前功能范围与限制

语音合并示例提供 MCP 工具 `self.camera.get_capture_url`，**只返回局域网拍照地址**。目前没有云端图片上传、AI 视觉识别或 LCD 摄像头实时预览；LCD 版显示语音状态和聊天文字。

HTTP 拍照接口**没有认证**，只用于可信局域网，不要端口转发到公网。合并版的 LCD/扬声器与 SD 接口 GPIO38/39/40 冲突，因此未启用 SD 卡，也不会格式化 SD 卡。GPIO48 用于 RGB LED。

摄像头对象的 `Begin()`、`StartHttpServer()`、`Update()` 应在同一个 Arduino `setup()/loop()` 任务中调用，不要跨任务并发操作。

## 7. S3rover 四端口电机

三个 S3rover 语音示例都包含电机控制。驱动为 **DRV8835 × 2**，使用外部 **MODE 高电平**的 **PHASE/ENABLE** 模式。

| 端口 | DIR / PHASE | PWM / ENABLE |
| --- | --- | --- |
| 1 | GPIO10 | GPIO11 |
| 2 | GPIO12 | GPIO13 |
| 3 | GPIO17 | GPIO18 |
| 4 | GPIO46 | GPIO38 |

MCP 工具：

| 工具 | 参数与行为 |
| --- | --- |
| `self.motor.set` | `port` 为 1–4；`speed` 为 −100–100%；`duration_ms` 为 1–10000ms，默认 1000ms；`speed=0` 停止对应端口 |
| `self.motor.stop_all` | 停止所有端口 |

C++ 驱动类为 `ai_bot::S3roverMotors`，提供 `Begin()`、`SetMotor()`、`StopAll()`、`Update()`。正负速度仅表示端口电气方向，不假设车轮位置或安装方向。PWM 为 20kHz、8bit。

程序上电不主动转动；参数非法时拒绝新命令，驱动初始化失败时不允许运动。换向前先停止 PWM；限时到期、Wi-Fi 断开、连接/加载失败或按键停止请求由主循环执行停机。驱动仅在已核验的 ESP32-S3 / Arduino Core 3.2.0 配置下启用，其他构建保持禁用。

**第一次测试请架空车轮或断开电机。软件限时不等于硬件急停。** 主循环卡死、CPU 故障、复位高阻态等仍需要独立断电保护、硬件看门狗和 ENABLE 下拉。驱动 API 应从同一个串行任务调用，按键回调只提交停止请求。GPIO46 是启动配置脚，外接负载不能破坏上电电平；PHASE/ENABLE 模式下 PWM 低为制动，不是自由滑行。

舵机 GPIO14/48、超声波 GPIO19/20 仅保留引脚，目前未实现相应功能，也未假设超声波 TRIG/ECHO 的对应关系。

## 8. 验证与许可证

[BUILD_VALIDATION.md](BUILD_VALIDATION.md) 记录了 **2026-11-23** 的真实固件编译结果、分区容量和电机主机测试。本轮 README 更新没有新增实机测试，也不将历史本地构建结果当作已运行的远程 GitHub Actions。

**GitHub 自动构建：** 当前仓库暂未上传 `.github/workflows/` 下的 4 个 CI 配置文件，因此未启用 GitHub Actions 自动构建。这不影响本地 Arduino IDE / PlatformIO 编译；完整配置已保留，可在更新工作流授权后补上。

本库使用 [MIT License](LICENSE)，保留原作者 **nulllab** 的版权声明，并加入 **Keyes DIY Robot** 的修改声明。第三方组件、字体和预编译库保留其原有作者与许可证，不因本库改名而变更归属。

Copyright (c) 2026 Keyes DIY Robot.
