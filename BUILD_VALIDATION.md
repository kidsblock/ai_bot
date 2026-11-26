# ai_bot 1.0.0 构建验证

验证日期：2026-11-23。源码公司信息为 Keyes DIY Robot；普通 ESP32 示例及四组 ESP32-S3 示例均包含在本次发布包中。

## 实际固件构建

**PlatformIO 15/15 通过；Arduino CLI 15/15 通过。** 每项都完成了编译和链接，并生成 ELF 与 BIN；不是仅解析配置，也不是主机模拟。构建在云端沙箱执行，没有烧录开发板。

PlatformIO 使用官方固定平台 54.03.20，Arduino ESP32 Core 3.2.0、ESP-IDF 5.4.1、Xtensa GCC 14.2.0、esptool 4.8.9。Arduino CLI 使用 1.2.0 和同版本 ESP32 Core 3.2.0 的官方工具包。显示库均为 LVGL 9.2.2；Arduino 下载渠道出现失败后，复用了已安装且版本核验一致的 LVGL 9.2.2 源码。

| 编译环境 | PlatformIO | Arduino CLI | PIO 应用字节 / 分区上限 |
|---|---|---|---|
| `esp32_ai_bot` | PASS | PASS | 1466866 / 3145728 |
| `esp32_ai_bot_display_lcd` | PASS | PASS | 2802734 / 3145728 |
| `esp32_ai_bot_display_oled` | PASS | PASS | 1991458 / 3145728 |
| `s3mini_ai_bot` | PASS | PASS | 2275534 / 4194304 |
| `s3mini_ai_bot_display_lcd` | PASS | PASS | 3610226 / 4194304 |
| `s3mini_ai_bot_display_oled` | PASS | PASS | 2799726 / 4194304 |
| `s3pro_ai_bot` | PASS | PASS | 2275534 / 4194304 |
| `s3pro_ai_bot_display_lcd` | PASS | PASS | 3601718 / 4194304 |
| `s3pro_ai_bot_display_oled` | PASS | PASS | 2799726 / 4194304 |
| `s3rover_ai_bot` | PASS | PASS | 2286474 / 4194304 |
| `s3rover_ai_bot_display_lcd` | PASS | PASS | 3612802 / 4194304 |
| `s3rover_ai_bot_display_oled` | PASS | PASS | 2810650 / 4194304 |
| `s3cam_ai_bot` | PASS | PASS | 2367874 / 4194304 |
| `s3cam_ai_bot_camera` | PASS | PASS | 915610 / 8388608 |
| `s3cam_ai_bot_display_lcd` | PASS | PASS | 3695470 / 4194304 |

## 配置与电机测试

15 个环境的源码路径、同名 Arduino .ino 入口、开发板分组和 CI 矩阵一致。默认标准 I2S 接线下，S3 各示例的 GPIO 分配无重叠；Rover 校验包含电机、I2C、舵机及超声波接口保留脚。所有使用中的分区表均通过对齐、不重叠和 Flash 容量检查；实际应用大小均未超出分区。

Rover 驱动主机测试：普通构建 26/26，ASan + UBSan 构建 26/26；另有非 Arduino、非 S3、非核验 Core 版本的 fail-closed 测试通过。覆盖全部四端口的 201 种速度、参数边界、失败注入、换向顺序、独立限时和 millis 溢出，包含 256 种 LEDC 占用图及 512 个固定种子时间随机测试。测试不连接或驱动实际电机。

GitHub Actions 配置已覆盖全部示例，并保留板级 PSRAM/USB/CPU 定义。Arduino S3 使用 Custom 分区并将构建检查 APP 上限设为 4MB；普通 ESP32 使用 Huge APP 3MB。这里的 Arduino PASS 指本地 CLI 的同类编译命令，不表示已经触发用户远程仓库的 GitHub Actions。

## 实机仍需确认

未验证实物电源、麦克风收音、扬声器播放、屏幕方向/背光极性、电机安装方向及摄像头成像，也未验证云服务账号激活。首次 Rover 电机测试应架空车轮或断开电机；协作式软件超时不能代替硬件急停、ENABLE 下拉或独立断电保护。

S3cam 的用户资料与官方文档分别标注 OV2640 和 OV3660，驱动自动识别并报告实际传感器。LCD 尺寸选项尚未由用户资料确认，默认 ST7789 240×240，可在 s3cam_display_config.h 调整。合并 LCD 版占 GPIO19/20，应使用 CH340 UART 而非 USB-OTG；SD 引脚与 LCD/音频冲突，因此未启用 SD。

摄像头功能为可信局域网 HTTP JPEG 拍照和 MCP 返回拍照地址，不包含云端视觉识别、云端图片上传或 LCD 摄像头预览。

Copyright (c) 2026 Keyes DIY Robot. 原 MIT 与第三方许可声明保留。
