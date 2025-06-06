// Copyright 2025 Xenon Emulator Project. All rights reserved.

#pragma once

#include "Base/Logging/Log.h"

#include "Core/XCPU/Interpreter/PPCInterpreter.h"
#include "Core/XCPU/PPU/PPU_JIT.h"

#if defined(ARCH_X86) || defined(ARCH_X86_64)
using namespace asmjit;

#define COMP b->compiler

//
// Allocates a new general purpose x86 register
//
#define newGP64()  b->compiler->newGpq()
#define newGP32()  b->compiler->newGpd()
#define newGP16()  b->compiler->newGpw()
#define newGP8()   b->compiler->newGpb()
#define newGPptr() b->compiler->newGpz()

//
// Pointer Helpers
//

#define GPRPtr(x) b->threadCtx->array(&PPU_THREAD_REGISTERS::GPR).Ptr(x)
#define SPRStruct(x) b->threadCtx->substruct(&PPU_THREAD_REGISTERS::SPR).substruct(&PPU_THREAD_SPRS::x)
#define SPRPtr(x) b->threadCtx->substruct(&PPU_THREAD_REGISTERS::SPR).scalar(&PPU_THREAD_SPRS::x)
#define SharedSPRStruct(x) b->ppuState->substruct(&PPU_STATE::SPR).substruct(&PPU_STATE_SPRS::x)
#define SharedSPRPtr(x) b->ppuState->substruct(&PPU_STATE::SPR).scalar(&PPU_STATE_SPRS::x)
#define CRValPtr() b->threadCtx->scalar(&PPU_THREAD_REGISTERS::CR)
#define CIAPtr() b->threadCtx->scalar(&PPU_THREAD_REGISTERS::CIA)
#define NIAPtr() b->threadCtx->scalar(&PPU_THREAD_REGISTERS::NIA)
#define LRPtr() SPRPtr(LR)

inline x86::Gp Jrotl32(JITBlockBuilder *b, x86::Mem x, u32 n) {
  x86::Gp tmp = newGP32();
  COMP->mov(tmp, x); // Cast value to 32 bit register
  COMP->rol(tmp, n);
  return tmp;
}

// Duplicates a u32 value left, used in rotate instructions that duplicate the
// lower 32 bits
inline x86::Gp Jduplicate32(JITBlockBuilder *b, x86::Gp origin) {
  x86::Gp cast64 = newGP64();
  COMP->mov(cast64, origin.r64());  // copy and cast to 64 bit
  COMP->shl(cast64, 32);        // shift left 32 bits
  COMP->or_(cast64, origin.r64());  // or with original value
  return cast64;
}

inline x86::Gp J_BuildCR(JITBlockBuilder *b, x86::Gp lhs, x86::Gp rhs) {
  x86::Gp crValue = newGP32();
  x86::Gp tmp = newGP8();

  COMP->cmp(lhs, rhs); // Compare lhs and rhs
  COMP->xor_(crValue, crValue);

  // lt (less than)
  COMP->setl(tmp);
  COMP->shl(tmp, imm(3 - CR_BIT_LT));
  COMP->or_(crValue.r8(), tmp.r8());

  // gt (greater than)
  COMP->setg(tmp);
  COMP->shl(tmp, imm(3 - CR_BIT_GT));
  COMP->or_(crValue.r8(), tmp.r8());

  // eq (equal)
  COMP->sete(tmp);
  COMP->shl(tmp, imm(3 - CR_BIT_EQ));
  COMP->or_(crValue.r8(), tmp.r8());

  // so (summary overflow)
#ifdef __LITTLE_ENDIAN__
  COMP->mov(tmp.r32(), SPRPtr(XER));
  COMP->shr(tmp.r32(), imm(31));
#else
  COMP->mov(tmp.r32(), SPRPtr(XER));
  COMP->and_(tmp.r32(), imm(1));
#endif
  COMP->shl(tmp, imm(3 - CR_BIT_SO));
  COMP->or_(crValue.r8(), tmp.r8());

  return crValue;
}

inline void J_SetCRField(JITBlockBuilder *b, x86::Gp field, u32 index) {
  x86::Gp tempCR = newGP32();

  u32 sh = (7 - index) * 4; // Shift formula
  uint32_t clearMask = ~(0xF << sh);

  COMP->mov(tempCR, CRValPtr());
  COMP->and_(tempCR, clearMask); // Clear field 
  COMP->shl(field, sh); // Shift field bits to position
  COMP->or_(tempCR, field); // Apply bits

  COMP->mov(CRValPtr(), tempCR); // Store updated value back
}

inline void J_ppuSetCR(JITBlockBuilder *b, x86::Gp value, u32 index) {
  Label use64 = COMP->newLabel();
  Label done = COMP->newLabel();

  x86::Gp tempMSR = newGP32();
  x86::Gp tempCR = newGP32();

  // Load MSR and check SF bit
  COMP->mov(tempMSR, SPRPtr(MSR));
  COMP->bt(tempMSR, 0); // Bit 0 (SF) on MSR
  COMP->jc(use64); // If set, use 64-bit compare

  // 32-bit compare
  {
    x86::Gp zero32 = newGP32();
    COMP->xor_(zero32, zero32);
    x86::Gp field = J_BuildCR(b, value.r32(), zero32);
    J_SetCRField(b, field, index);
    COMP->jmp(done);
  }

  // 64-bit compare
  COMP->bind(use64);
  {
    x86::Gp zero64 = newGP64();
    COMP->xor_(zero64, zero64);
    x86::Gp field = J_BuildCR(b, value.r64(), zero64);
    J_SetCRField(b, field, index);
  }

  COMP->bind(done);
}

inline void J_ppuSetCR_LOGICAL(JITBlockBuilder *b, x86::Gp value, u32 index) {
  x86::Gp tempCR = newGP32();
  x86::Gp field = newGP32();

  // Zero compare (logical)
  Label is_zero = COMP->newLabel();
  Label done = COMP->newLabel();

  COMP->xor_(field, field); // Clear field

  COMP->test(value, value); // Logical test
  COMP->jz(is_zero);

  // result != 0: GT = 1
  COMP->mov(field, imm(1 << (3 - CR_BIT_GT)));
  COMP->jmp(done);

  COMP->bind(is_zero);
  // result == 0: EQ = 1
  COMP->mov(field, imm(1 << (3 - CR_BIT_EQ)));

  COMP->bind(done);

  // Set field in CR
  J_SetCRField(b, field, index);
}

#endif