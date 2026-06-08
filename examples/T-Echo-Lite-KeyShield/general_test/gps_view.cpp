/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-08 18:02:42
 * @License: GPL 3.0
 */
#include "gps_view.h"

#include <Arduino.h>
#include <math.h>

#include "ble_uart_log.h"
#include "cpp_bus_driver_library.h"
#include "t_echo_lite_keyshield_config.h"

// GPS demo 模式：无硬件模块时生成模拟定位数据用于 UI 测试。
// 有模块后注释掉此行即可切回真实 GPS。
// #define GPS_DEMO_MODE

namespace {

static constexpr int32_t kDefaultBaudRate = 9600;
static constexpr size_t kGpsBufferSize = 1024;

#ifdef GPS_DEMO_MODE
static constexpr double kDemoBaseLat = 25.7777;
static constexpr double kDemoBaseLon = 153.5555;
static constexpr float kDemoBaseAlt = 25.0f;
static constexpr uint32_t kDemoRefreshIntervalMs = 5000;
uint32_t demo_start_ms = 0;
uint32_t demo_last_refresh_ms = 0;
#endif

cpp_bus_driver::GnssParser gps_parser;
GpsInfo gps_info;
bool gps_initialized = false;
uint32_t gps_page_enter_ms = 0;

#ifndef GPS_DEMO_MODE
uint8_t gps_rx_buffer[kGpsBufferSize];
size_t gps_rx_length = 0;
#endif

bool UpdateGpsInfo(const cpp_bus_driver::GnssParser::Info& info) {
  bool changed = false;

  if (info.gga.gps_mode_status != 0xFF) {
    const bool new_fix = (info.gga.gps_mode_status >= 1);
    if (new_fix != gps_info.has_fix) {
      if (new_fix) {
        gps_info.time_to_first_fix_s =
            (millis() - gps_page_enter_ms) / 1000;
      }
      gps_info.has_fix = new_fix;
      changed = true;
    }
  }

  // 未定位时持续更新已等待时间。
  if (!gps_info.has_fix && gps_page_enter_ms > 0) {
    const uint32_t elapsed = (millis() - gps_page_enter_ms) / 1000;
    if (elapsed != gps_info.time_to_first_fix_s) {
      gps_info.time_to_first_fix_s = elapsed;
      changed = true;
    }
  }

  if (info.gga.location.lat.update_flag &&
      info.gga.location.lat.direction_update_flag &&
      info.gga.location.lon.update_flag &&
      info.gga.location.lon.direction_update_flag) {
    double lat = info.gga.location.lat.degrees_minutes;
    double lon = info.gga.location.lon.degrees_minutes;
    if (info.gga.location.lat.direction == "S") lat = -lat;
    if (info.gga.location.lon.direction == "W") lon = -lon;
    if (lat != gps_info.latitude || lon != gps_info.longitude) {
      gps_info.latitude = lat;
      gps_info.longitude = lon;
      changed = true;
    }
  }

  if (!info.gga.altitude_unit.empty() &&
      info.gga.altitude != gps_info.altitude) {
    gps_info.altitude = info.gga.altitude;
    changed = true;
  }

  if (info.gga.online_satellite_count != 0xFF &&
      info.gga.online_satellite_count != gps_info.satellites_used) {
    gps_info.satellites_used = info.gga.online_satellite_count;
    changed = true;
  }

  if (info.gsv.update_flag &&
      info.gsv.total_satellite_count != gps_info.satellites_visible) {
    gps_info.satellites_visible = info.gsv.total_satellite_count;
    changed = true;
  }

  if (info.gga.hdop >= 0 && info.gga.hdop != gps_info.hdop) {
    gps_info.hdop = info.gga.hdop;
    changed = true;
  }

  if (info.gsa.update_flag && !info.gsa.sentences.empty()) {
    const auto& s = info.gsa.sentences[0];
    if (s.pdop >= 0 && s.pdop != gps_info.pdop) {
      gps_info.pdop = s.pdop;
      changed = true;
    }
    if (s.vdop >= 0 && s.vdop != gps_info.vdop) {
      gps_info.vdop = s.vdop;
      changed = true;
    }
  }

  if (info.gsv.update_flag) {
    int16_t best_cn0 = 0;
    for (const auto& sat : info.gsv.satellites) {
      if (sat.cn0 > best_cn0) best_cn0 = sat.cn0;
    }
    if (best_cn0 > 0 && best_cn0 != gps_info.max_cn0) {
      gps_info.max_cn0 = best_cn0;
      changed = true;
    }
  }

  if (info.vtg.update_flag && info.vtg.speed_kmh >= 0 &&
      info.vtg.speed_kmh != gps_info.speed_kmh) {
    gps_info.speed_kmh = info.vtg.speed_kmh;
    changed = true;
  }

  if (info.gga.utc.update_flag) {
    if (!gps_info.utc_valid ||
        info.gga.utc.hour != gps_info.utc_hour ||
        info.gga.utc.minute != gps_info.utc_minute ||
        info.gga.utc.second != gps_info.utc_second) {
      gps_info.utc_hour = info.gga.utc.hour;
      gps_info.utc_minute = info.gga.utc.minute;
      gps_info.utc_second = info.gga.utc.second;
      gps_info.utc_valid = true;
      changed = true;
    }
  }

  if (info.rmc.data.update_flag) {
    if (!gps_info.date_valid ||
        info.rmc.data.day != gps_info.utc_day ||
        info.rmc.data.month != gps_info.utc_month ||
        info.rmc.data.year != gps_info.utc_year) {
      gps_info.utc_day = info.rmc.data.day;
      gps_info.utc_month = info.rmc.data.month;
      gps_info.utc_year = info.rmc.data.year + 2000;
      gps_info.date_valid = true;
      changed = true;
    }
  }

  return changed;
}

}  // namespace

bool InitializeGps() {
  if (gps_initialized) return true;

#ifdef GPS_DEMO_MODE
  demo_start_ms = millis();
  gps_info.module_found = true;
  gps_initialized = true;
  gps_page_enter_ms = millis();
  return true;
#else
  pinMode(GPS_RT9080_EN, OUTPUT);
  digitalWrite(GPS_RT9080_EN, HIGH);
  delay(10);

  Serial1.setPins(GPS_UART_RX, GPS_UART_TX);
  Serial1.begin(kDefaultBaudRate);

  // 不立即标记 module_found，等 ProcessGps 中收到 $G 数据再确认。
  gps_info.module_found = false;
  gps_initialized = true;
  gps_rx_length = 0;
  gps_page_enter_ms = millis();
  return true;
#endif
}

void ShutdownGps() {
  if (!gps_initialized) return;

#ifndef GPS_DEMO_MODE
  Serial1.end();
  digitalWrite(GPS_RT9080_EN, LOW);
#endif
  gps_initialized = false;
  gps_info = GpsInfo{};
}

bool ProcessGps() {
#ifdef GPS_DEMO_MODE
  if (!gps_info.module_found) return false;
#else
  if (!gps_initialized) return false;

  // 模块检测：3 秒内无 $G 数据则判定模块不存在。
  if (!gps_info.module_found) {
    if (millis() - gps_page_enter_ms > 3000) {
      return false;  // 超时，保持 module_found=false，UI 显示未找到。
    }
  }
#endif

#ifdef GPS_DEMO_MODE
  const uint32_t now = millis();
  if (now - demo_last_refresh_ms < kDemoRefreshIntervalMs) return false;
  demo_last_refresh_ms = now;

  const uint32_t elapsed_s = (now - demo_start_ms) / 1000;

  // 模拟冷启动：前 3 秒无定位。
  if (elapsed_s < 3) {
    gps_info.has_fix = false;
    gps_info.satellites_used = 0;
    gps_info.satellites_visible = static_cast<uint8_t>(4 + elapsed_s);
    gps_info.utc_valid = false;
    gps_info.time_to_first_fix_s = elapsed_s;
    return true;
  }

  // 首次获取定位时记录耗时。
  if (!gps_info.has_fix) {
    gps_info.time_to_first_fix_s = elapsed_s;
  }

  // 有定位后模拟缓慢移动。
  const double t = static_cast<double>(elapsed_s) * 0.0001;
  gps_info.has_fix = true;
  gps_info.latitude = kDemoBaseLat + sin(t * 1.3) * 0.002;
  gps_info.longitude = kDemoBaseLon + cos(t * 0.9) * 0.003;
  gps_info.altitude = kDemoBaseAlt + sin(t * 2.1) * 3.0f;
  gps_info.satellites_used = static_cast<uint8_t>(8 + (elapsed_s % 4));
  gps_info.satellites_visible = static_cast<uint8_t>(12 + (elapsed_s % 6));
  gps_info.hdop = 1.2f + sin(t * 5.0f) * 0.3f;
  gps_info.vdop = 1.8f + cos(t * 4.0f) * 0.4f;
  gps_info.pdop = sqrtf(gps_info.hdop * gps_info.hdop +
                        gps_info.vdop * gps_info.vdop);
  gps_info.max_cn0 = static_cast<int16_t>(35 + (elapsed_s % 10));
  gps_info.speed_kmh = 3.0f + fabsf(sin(t * 3.0f)) * 8.0f;

  const uint32_t utc_total_sec = (elapsed_s + 3600 * 6);  // UTC 06:00 起
  gps_info.utc_hour = static_cast<uint8_t>((utc_total_sec / 3600) % 24);
  gps_info.utc_minute = static_cast<uint8_t>((utc_total_sec / 60) % 60);
  gps_info.utc_second = static_cast<float>(utc_total_sec % 60);
  gps_info.utc_valid = true;
  gps_info.utc_day = 7;
  gps_info.utc_month = 6;
  gps_info.utc_year = 2026;
  gps_info.date_valid = true;

  return true;
#else
  // 真实 GPS：从 Serial1 读取 NMEA 数据。
  while (Serial1.available() > 0 && gps_rx_length < kGpsBufferSize - 1) {
    gps_rx_buffer[gps_rx_length++] =
        static_cast<uint8_t>(Serial1.read());
  }

  const uint8_t* sentence_start = nullptr;
  size_t sentence_length = 0;
  for (size_t i = 0; i < gps_rx_length; i++) {
    if (gps_rx_buffer[i] != '$') continue;
    for (size_t j = i + 1; j < gps_rx_length; j++) {
      if (gps_rx_buffer[j] == '\n') {
        sentence_start = &gps_rx_buffer[i];
        sentence_length = j - i + 1;
        break;
      }
    }
  }

  if (sentence_start == nullptr || sentence_length < 6) return false;

  const size_t consumed =
      static_cast<size_t>(sentence_start - gps_rx_buffer) + sentence_length;
  const size_t remaining = gps_rx_length - consumed;
  if (remaining > 0) {
    memmove(gps_rx_buffer, gps_rx_buffer + consumed, remaining);
  }
  gps_rx_length = remaining;

  cpp_bus_driver::GnssParser::Info info;

  // 一旦收到完整的 $ 开头语句，确认模块存在。
  if (!gps_info.module_found) {
    gps_info.module_found = true;
    LogPrintln("GPS module detected");
  }

  if (!gps_parser.ParseInfo(sentence_start, sentence_length, info)) {
    return false;
  }

  return UpdateGpsInfo(info);
#endif
}

GpsInfo GetGpsInfo() { return gps_info; }
