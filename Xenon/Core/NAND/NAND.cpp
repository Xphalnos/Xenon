// Copyright 2025 Xenon Emulator Project

#include "NAND.h"
#include "Base/Logging/Log.h"

//#define NAND_DEBUG

/********************Responsible for loading the NAND file********************/
NAND::NAND(const std::string &deviceName, SFCX* sfcxDevicePtr,
  u64 startAddress, u64 endAddress,
  bool isSOCDevice) : SystemDevice(deviceName, startAddress, endAddress, isSOCDevice) {
  // Set the SFCX device pointer.
  sfcxDevice = sfcxDevicePtr;
}

NAND::~NAND() {
}

/************Responsible for reading the NAND************/
void NAND::Read(u64 readAddress, u8 *data, u64 size) {
  sfcxDevice->ReadRaw(readAddress, data, size);
}

/************Responsible for writing the NAND************/
void NAND::Write(u64 writeAddress, const u8 *data, u64 size) {
  sfcxDevice->WriteRaw(writeAddress, data, size);
}

/************Responsible for writing the NAND************/
void NAND::MemSet(u64 writeAddress, s32 data, u64 size) {
  sfcxDevice->MemSetRaw(writeAddress, data, size);
}