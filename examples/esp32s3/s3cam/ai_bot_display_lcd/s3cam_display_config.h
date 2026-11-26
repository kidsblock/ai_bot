/* Copyright (c) 2026 Keyes DIY Robot. */
#pragma once
// User config.h confirms the wiring, but not the selected LCD size.
// Default profile: ST7789 240x240, SPI mode 0. Adjust for your actual panel.
// Example: 240x320 -> HEIGHT 320; 170x320 -> WIDTH 170, HEIGHT 320, OFFSET_X 35.
#define AI_BOT_S3CAM_LCD_WIDTH 240
#define AI_BOT_S3CAM_LCD_HEIGHT 240
#define AI_BOT_S3CAM_LCD_OFFSET_X 0
#define AI_BOT_S3CAM_LCD_OFFSET_Y 0
#define AI_BOT_S3CAM_LCD_SPI_MODE 0
#define AI_BOT_S3CAM_LCD_MIRROR_X false
#define AI_BOT_S3CAM_LCD_MIRROR_Y false
#define AI_BOT_S3CAM_LCD_SWAP_XY false
#define AI_BOT_S3CAM_LCD_INVERT_COLOR true
