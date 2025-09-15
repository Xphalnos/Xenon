/***************************************************************/
/* Copyright 2025 Xenon Emulator Project. All rights reserved. */
/***************************************************************/

#pragma once

#include "Core/PCI/PCIe.h"

#include "Core/PCI/Bridge/PCIBridge.h"

#include "Core/XGPU/XGPU.h"

/*
        PCI Configuration Space at Address 0xD0000000
        Bus0
        - Dev0  PCI-PCI Bridge    0xD0000000
        - Dev1  HostBridge        0xD0008000
*/

#define HOST_BRIDGE_SIZE 0x1FFFFFF // Maybe??

// Host Bridge regs, these control interrupts/etc
struct HOSTBRIDGE_REGS {
  u32 REG_E0020000;
  u32 REG_E0020004;
};

struct BIU_REGS {
  u32 REG_E1003000;
  u32 REG_E1003100;
  u32 REG_E1003200;
  u32 REG_E1003300;
  u32 REG_E1010000;
  u32 REG_E1010010;
  u32 REG_E1010020;
  u32 REG_E1013000;
  u32 REG_E1013100;
  u32 REG_E1013200;
  u32 REG_E1013300;
  u32 REG_E1018000;
  u32 REG_E1018020;
  u32 REG_E1020000;
  u32 REG_E1020004;
  u32 REG_E1020008;
  u32 ramSize;
  u32 REG_E1040074;
  u32 REG_E1040078;
};

class HostBridge {
public:
  HostBridge(u64 ramSize);
  ~HostBridge();

  // Xbox GPU Register
  void RegisterXGPU(std::shared_ptr<Xe::Xenos::XGPU> xgpu);

  // PCI Bridge Register
  void RegisterPCIBridge(std::shared_ptr<PCIBridge> bridge);

  // Read
  bool Read(u64 readAddress, u8 *data, u64 size);

  // Write
  bool Write(u64 writeAddress, const u8 *data, u64 size);

  // MemSet
  bool MemSet(u64 writeAddress, s32 data, u64 size);

  // Configuration Read
  bool ConfigRead(u64 readAddress, u8 *data, u64 size);

  // Configuration Write
  bool ConfigWrite(u64 writeAddress, const u8 *data, u64 size);

private:
  std::mutex mutex{};

  GENRAL_PCI_DEVICE_CONFIG_SPACE hostBridgeConfigSpace{};

  // Pointer to the registered XCGPU
  std::shared_ptr<Xe::Xenos::XGPU> xGPU{};

  // Pointer to the registered PCI Bridge
  std::shared_ptr<PCIBridge> pciBridge{};

  // Helpers
  bool isAddressMappedinBAR(u32 address);

  HOSTBRIDGE_REGS hostBridgeRegs{};
  BIU_REGS biuRegs{};
};
