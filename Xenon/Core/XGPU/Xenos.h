/***************************************************************/
/* Copyright 2025 Xenon Emulator Project. All rights reserved. */
/***************************************************************/

#pragma once

#include "Base/Types.h"

// Contains Xenos related enums and structures.
// Mostly taken from Xenia, as their research is much more consistent than other sources.

// Primitive Types used on the Xenos.
enum class ePrimitiveType : u32 {
  xeNone = 0x00,
  xePointList = 0x01,
  xeLineList = 0x02,
  xeLineStrip = 0x03,
  xeTriangleList = 0x04,
  xeTriangleFan = 0x05,
  xeTriangleStrip = 0x06,
  xeTriangleWithWFlags = 0x07,
  xeRectangleList = 0x08,
  xeLineLoop = 0x0C,
  xeQuadList = 0x0D,
  xeQuadStrip = 0x0E,
  xePolygon = 0x0F,

  // Note from Xenia devs:
  // Starting with this primitive type, explicit major mode is assumed (in the
  // R6xx/R7xx registers, k2DCopyRectListV0 is 22, and implicit major mode is
  // only used for primitive types 0 through 21) - and tessellation patches use
  // the range that starts from k2DCopyRectListV0.
  // TODO(bitsh1ft3r): Verify if this is also true for the Xenos.
  xeExplicitMajorModeForceStart = 0x10,

  xe2DCopyRectListV0 = 0x10,
  xe2DCopyRectListV1 = 0x11,
  xe2DCopyRectListV2 = 0x12,
  xe2DCopyRectListV3 = 0x13,
  xe2DFillRectList = 0x14,
  xe2DLineStrip = 0x15,
  xe2DTriStrip = 0x16,

  // Tessellation patches when VGT_OUTPUT_PATH_CNTL::path_select is
  // VGTOutputPath::kTessellationEnable. The vertex shader receives the patch
  // index rather than control point indices.
  // With non-adaptive tessellation, VGT_DRAW_INITIATOR::num_indices is the
  // patch count (4D5307F1 draws single ground patches by passing 1 as the index
  // count). VGT_INDX_OFFSET is also applied to the patch index - 4D5307F1 uses
  // auto-indexed patches with a nonzero VGT_INDX_OFFSET, which contains the
  // base patch index there.
  // With adaptive tessellation, however, num_indices is the number of
  // tessellation factors in the "index buffer" reused for tessellation factors,
  // which is the patch count multiplied by the edge count (if num_indices is
  // multiplied further by 4 for quad patches for the ground in 4D5307F2, for
  // example, some incorrect patches are drawn, so Xenia shouldn't do that; also
  // 4D5307E6 draws water triangle patches with the number of indices that is 3
  // times the invocation count of the memexporting shader that calculates the
  // tessellation factors for a single patch for each "point").
  xeLinePatch = 0x10,
  xeTrianglePatch = 0x11,
  xeQuadPatch = 0x12,
};

// VGT_DRAW_INITIATOR::DI_SRC_SEL_*
enum class eSourceSelect : u32 {
  xeDMA,
  xeImmediate,
  xeAutoIndex,
};

// VGT_DRAW_INITIATOR::DI_MAJOR_MODE_*
enum class eMajorMode : u32 {
  xeImplicit,
  xeExplicit,
};

enum class eIndexFormat : u32 {
  xeInt16,
  // Not very common, but used for some world draws in 545407E0.
  xeInt32,
};

enum class eEndian : u32 {
  xeNone = 0,
  xe8in16 = 1,
  xe8in32 = 2,
  xe16in32 = 3,
};

enum class eEndian128 : u32 {
  xeNone = 0,
  xe8in16 = 1,
  xe8in32 = 2,
  xe16in32 = 3,
  xe8in64 = 4,
  xe8in128 = 5,
};

enum class eModeControl : u32 {
  xeIgnore = 0,
  xeColorDepth = 4,
  // TODO: Verify whether kDepth means the pixel shader is ignored
  // completely even if it writes depth, exports to memory or kills pixels.
  // Hints suggesting that it should be completely ignored (which is desirable
  // on real hardware to avoid scheduling the pixel shader at all and waiting
  // for it especially since the Xbox 360 doesn't have early per-sample depth /
  // stencil, only early hi-Z / hi-stencil, and other registers possibly
  // toggling pixel shader execution are yet to be found):
  // - Most of depth pre-pass draws in 415607E6 use the kDepth more with a
  //   `oC0 = tfetch2D(tf0, r0.xy) * r1` shader, some use `oC0 = r0` though.
  //   However, when alphatested surfaces are drawn, kColorDepth is explicitly
  //   used with the same shader performing the texture fetch.
  // - 5454082B has some kDepth draws with alphatest enabled, but the shader is
  //   `oC0 = r0`, which makes no sense (alphatest based on an interpolant from
  //   the vertex shader) as no texture alpha cutout is involved.
  // - 5454082B also has kDepth draws with pretty complex shaders clearly for
  //   use only in the color pass - even fetching and filtering a shadowmap.
  // For now, based on these, let's assume the pixel shader is never used with
  // kDepth.
  xeDepth = 5,
  xeCopy = 6,
};

enum class eMSAASamples : u32 {
  MSAA1X = 0,
  MSAA2X = 1,
  MSAA4X = 2,
};

// a2xx_rb_copy_sample_select
enum class eCopySampleSelect : u32 {
  xe0,
  xe1,
  xe2,
  xe3,
  xe01,
  xe23,
  xe0123,
};

enum class eCopyCommand : u32 {
  xeRaw = 0,
  xeConvert = 1,
  xeConstantOne = 2,
  xeNull = 3,  // ?
};

// Subset of a2xx_sq_surfaceformat - formats that RTs can be resolved to.
enum class eColorFormat : u32 {
  xe_8 = 2,
  xe_1_5_5_5 = 3,
  xe_5_6_5 = 4,
  xe_6_5_5 = 5,
  xe_8_8_8_8 = 6,
  xe_2_10_10_10 = 7,
  xe_8_A = 8,
  xe_8_B = 9,
  xe_8_8 = 10,
  xe_8_8_8_8_A = 14,
  xe_4_4_4_4 = 15,
  xe_10_11_11 = 16,
  xe_11_11_10 = 17,
  xe_16 = 24,
  xe_16_16 = 25,
  xe_16_16_16_16 = 26,
  xe_16_FLOAT = 30,
  xe_16_16_FLOAT = 31,
  xe_16_16_16_16_FLOAT = 32,
  xe_32_FLOAT = 36,
  xe_32_32_FLOAT = 37,
  xe_32_32_32_32_FLOAT = 38,
  xe_8_8_8_8_AS_16_16_16_16 = 50,
  xe_2_10_10_10_AS_16_16_16_16 = 54,
  xe_10_11_11_AS_16_16_16_16 = 55,
  xe_11_11_10_AS_16_16_16_16 = 56,
};

// SurfaceNumberX from yamato_enum.h.
enum class eSurfaceNumberFormat : u32 {
  xeUnsignedRepeatingFraction = 0,
  // Microsoft-style, scale factor (2^(n-1))-1.
  xeSignedRepeatingFraction = 1,
  xeUnsignedInteger = 2,
  xeSignedInteger = 3,
  kFloat = 7,
};

inline uint16_t xeEndianSwap(uint16_t value, eEndian endianness) {
  switch (endianness) {
  case eEndian::xeNone:
    // No swap.
    return value;
  case eEndian::xe8in16:
    // Swap bytes in half words.
    return ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0x00FF00FF);
  default:
    LOG_ERROR(Xenos, "GPUSwap: Invalid endianness was passed in.");
    return value;
  }
}

inline uint32_t xeEndianSwap(uint32_t value, eEndian endianness) {
  switch (endianness) {
  default:
  case eEndian::xeNone:
    // No swap.
    return value;
  case eEndian::xe8in16:
    // Swap bytes in half words.
    return ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0x00FF00FF);
  case eEndian::xe8in32:
    // Swap bytes.
    // NOTE: we are likely doing two swaps here. Wasteful. Oh well.
    return byteswap_be<u32>(value);
  case eEndian::xe16in32:
    // Swap half words.
    return ((value >> 16) & 0xFFFF) | (value << 16);
  }
}

inline float xeEndianSwap(float value, eEndian endianness) {
  union {
    uint32_t i;
    float f;
  } v;
  v.f = value;
  v.i = xeEndianSwap(v.i, endianness);
  return v.f;
}