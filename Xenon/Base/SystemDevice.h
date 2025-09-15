/***************************************************************/
/* Copyright 2025 Xenon Emulator Project. All rights reserved. */
/***************************************************************/

#pragma once

#include <string>

#include "Base/Types.h"

struct DeviceInfo {
  std::string deviceName = ""; // Device Name
  u64 startAddr = 0; // Start Address
  u64 endAddr = 0; // End Address
  bool socDevice = false; // SOC Device
};

class SystemDevice {
public:
  SystemDevice(const std::string &deviceName, u64 startAddress, u64 endAddress,
               bool isSOCDevice)
  {
    info.deviceName = deviceName;
    info.startAddr = startAddress;
    info.endAddr = endAddress;
    info.socDevice = isSOCDevice;
  }

  virtual void Read(u64 readAddress, u8 *data, u64 byteCount) {}
  virtual void Write(u64 writeAddress, const u8 *data, u64 byteCount) {}
  virtual void MemSet(u64 writeAddress, s32 data, u64 byteCount) {}

  std::string GetDeviceName() { return info.deviceName; }
  u64 GetStartAddress() { return info.startAddr; }
  u64 GetEndAddress() { return info.endAddr; }
  u64 GetSize() { return info.startAddr - info.endAddr; }
  bool IsSOCDevice() { return info.socDevice; }
  
  void UpdateEndAddress(u64 addr) { info.endAddr = addr; }
  void UpdateStartAddress(u64 addr) { info.startAddr = addr; }
private:
  DeviceInfo info{};
};
