/*
 * @Description: tca8418
 * @Author: LILYGO_L
 * @Date: 2025-06-13 14:20:16
 * @LastEditTime: 2026-06-01 17:30:00
 * @License: GPL 3.0
 */
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <Wire.h>

#include "cpp_bus_driver_library.h"
#include "t_echo_lite_keyshield_config.h"

using Tca8418 = cpp_bus_driver::Tca8418;

static volatile bool interrupt_flag = false;

std::shared_ptr<cpp_bus_driver::HardwareI2c2>& Tca8418I2cBus() {
  static auto tca8418_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c2>(
          TCA8418_SDA, TCA8418_SCL, &Wire);
  return tca8418_i2c_bus;
}

Tca8418& Tca8418Device() {
  static auto tca8418 =
      std::make_unique<Tca8418>(Tca8418I2cBus(), TCA8418_IIC_ADDRESS);
  return *tca8418;
}

void PowerOnKeyShield3v3() {
  pinMode(RT9080_EN, OUTPUT);
  digitalWrite(RT9080_EN, HIGH);
  delay(100);
  digitalWrite(RT9080_EN, LOW);
  delay(100);
  digitalWrite(RT9080_EN, HIGH);
  delay(1000);
}

void PrintKeyEvent(const Tca8418::TouchInfo& event, size_t index) {
  switch (event.event_type) {
    case Tca8418::EventType::kKeypad: {
      Tca8418::TouchPosition position;
      if (!Tca8418Device().ParseTouchNum(event.num, position)) {
        printf("keypad event parse failed, num: %u\n",
            static_cast<unsigned int>(event.num));
        break;
      }

      printf(
          "keypad event[%u]: num: %u x: %u y: %u pressed: %u\n",
          static_cast<unsigned int>(index),
          static_cast<unsigned int>(event.num),
          static_cast<unsigned int>(position.x),
          static_cast<unsigned int>(position.y),
          static_cast<unsigned int>(event.press_flag));

      const size_t key_count = sizeof(Tca8418_Map) / sizeof(Tca8418_Map[0]);
      if ((event.num > 0) && (event.num <= key_count)) {
        printf("keypad string: %s\n", Tca8418_Map[event.num - 1].c_str());
      }
      break;
    }

    case Tca8418::EventType::kGpio:
      printf("gpio event[%u]: num: %u pressed: %u\n",
          static_cast<unsigned int>(index),
          static_cast<unsigned int>(event.num),
          static_cast<unsigned int>(event.press_flag));
      break;

    default:
      printf("unknown event[%u]: num: %u pressed: %u\n",
          static_cast<unsigned int>(index),
          static_cast<unsigned int>(event.num),
          static_cast<unsigned int>(event.press_flag));
      break;
  }
}

void HandleTca8418Interrupt() {
  auto& tca8418 = Tca8418Device();

  const uint8_t irq_flag = tca8418.GetIrqFlag();
  Tca8418::IrqStatus irq_status;
  if (!tca8418.ParseIrqStatus(irq_flag, irq_status)) {
    printf("Tca8418 irq parse failed\n");
    return;
  }

  if (irq_status.fifo_overflow_flag) {
    printf("Tca8418 FIFO overflow\n");
    tca8418.ClearIrqFlag(Tca8418::IrqFlag::kFifoOverflow);
  }

  if (irq_status.keypad_lock_flag) {
    Tca8418::KeyLockInfo lock_info;
    if (tca8418.GetKeyLockInfo(&lock_info)) {
      printf("key lock interrupt, locked: %u events: %u\n",
          static_cast<unsigned int>(lock_info.locked),
          static_cast<unsigned int>(lock_info.event_count));
    }
    tca8418.ClearIrqFlag(Tca8418::IrqFlag::kKeypadLock);
  }

  if (irq_status.gpio_interrupt_flag) {
    uint32_t gpio_status = 0;
    if (tca8418.GetClearGpioIrqFlag(&gpio_status)) {
      printf("gpio irq status: %#lx\n", static_cast<unsigned long>(gpio_status));
    }
    tca8418.ClearIrqFlag(Tca8418::IrqFlag::kGpioInterrupt);
  }

  if (irq_status.key_events_flag) {
    Tca8418::TouchPoint touch_point;
    if (tca8418.GetMultipleTouchPoint(touch_point)) {
      printf("key events: %u\n",
          static_cast<unsigned int>(touch_point.finger_count));

      for (size_t i = 0; i < touch_point.info.size(); i++) {
        PrintKeyEvent(touch_point.info[i], i + 1);
      }
    }
    tca8418.ClearIrqFlag(Tca8418::IrqFlag::kKeyEvents);
  }

  if (irq_status.ctrl_alt_del_key_sequence_flag) {
    printf("ctrl-alt-del sequence interrupt\n");
    tca8418.ClearIrqFlag(Tca8418::IrqFlag::kCtrlAltDelKeySequence);
  }
}

void setup() {
  Serial.begin(115200);
  uint8_t serial_init_count = 0;
  while (!Serial) {
    delay(100);  // Wait for native USB serial.
    serial_init_count++;
    if (serial_init_count > 30) {
      break;
    }
  }
  printf("Ciallo\n");

  PowerOnKeyShield3v3();

  pinMode(TCA8418_INT, INPUT_PULLUP);
  attachInterrupt(
      TCA8418_INT, []() -> void { interrupt_flag = true; }, FALLING);

  auto& tca8418 = Tca8418Device();
  if (!tca8418.Init()) {
    printf("Tca8418 init failed\n");
    return;
  }

  bool config_result = true;
  config_result &= tca8418.SetKeypadScanWindow(
      0, 0, TCA8418_KEYPAD_SCAN_WIDTH, TCA8418_KEYPAD_SCAN_HEIGHT);
  config_result &= tca8418.SetInterruptPulseMode(true);
  config_result &= tca8418.SetInterruptEnable(
      static_cast<uint8_t>(Tca8418::IrqMask::kKeyEvents) |
      static_cast<uint8_t>(Tca8418::IrqMask::kFifoOverflow));
  config_result &= tca8418.ClearEventFifo();
  config_result &= tca8418.ClearIrqFlag(Tca8418::IrqFlag::kAll);
  if (!config_result) {
    printf("Tca8418 config failed\n");
    return;
  }

  printf("Tca8418 ready\n");
}

void loop() {
  if (interrupt_flag) {
    interrupt_flag = false;
    HandleTca8418Interrupt();
  }

  delay(10);
}
