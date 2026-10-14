# Firmware Modification Log

## 202610141155
### Changed firmware
- `[T-Echo-Lite-KeyShield_V1.0][general_test]_firmware_202610141155`
### Change content
- 修复低功耗休眠时电源使能引脚浮空导致电流回弹的问题，关闭 RT9080_EN 和 GPS_RT9080_EN 后使用下拉输入。
- 修复录音和播放的 I2S DMA 缓冲交接错误，避免过早读取录音缓冲和覆盖正在播放的缓冲，消除周期性的“哒哒”声。

## 2026070140923
### Changed firmware
- `[T-Echo-Lite_V1.0][general_test]_firmware_2026070140923`
### Change content
- 更新flash新型号兼容ZD25Q32D

## 202607010917
### Changed firmware
- `[T-Echo-Lite_V1.0][general_test]_firmware_202607010917`
### Change content
- 更新cpp_bus_driver

## 202606101202
### Changed firmware
- `[T-Echo-Lite_V1.0][general_test]_firmware_202606101202`
### Change content
- 添加volatile防止屏幕刷新变量被编译优化

## 202606081756
### Changed firmware
- `[T-Echo-Lite_V1.0][general_test]_firmware_202606081756`
### Change content
- 初始版本
