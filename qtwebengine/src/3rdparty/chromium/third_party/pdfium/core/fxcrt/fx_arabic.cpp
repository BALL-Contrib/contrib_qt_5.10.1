// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_arabic.h"

#include <algorithm>
#include <vector>

#include "core/fxcrt/fx_ucd.h"
#include "third_party/base/stl_util.h"

#define FX_BIDIMAXLEVEL 61

namespace {

struct FX_ARBFORMTABLE {
  uint16_t wIsolated;
  uint16_t wFinal;
  uint16_t wInitial;
  uint16_t wMedial;
};

struct FX_ARAALEF {
  uint16_t wAlef;
  uint16_t wIsolated;
};

struct FX_ARASHADDA {
  uint16_t wShadda;
  uint16_t wIsolated;
};

enum FX_BIDIWEAKSTATE {
  FX_BWSxa = 0,
  FX_BWSxr,
  FX_BWSxl,
  FX_BWSao,
  FX_BWSro,
  FX_BWSlo,
  FX_BWSrt,
  FX_BWSlt,
  FX_BWScn,
  FX_BWSra,
  FX_BWSre,
  FX_BWSla,
  FX_BWSle,
  FX_BWSac,
  FX_BWSrc,
  FX_BWSrs,
  FX_BWSlc,
  FX_BWSls,
  FX_BWSret,
  FX_BWSlet
};

enum FX_BIDIWEAKACTION {
  FX_BWAIX = 0x100,
  FX_BWAXX = 0x0F,
  FX_BWAxxx = (0x0F << 4) + 0x0F,
  FX_BWAxIx = 0x100 + FX_BWAxxx,
  FX_BWAxxN = (0x0F << 4) + FX_BIDICLASS_ON,
  FX_BWAxxE = (0x0F << 4) + FX_BIDICLASS_EN,
  FX_BWAxxA = (0x0F << 4) + FX_BIDICLASS_AN,
  FX_BWAxxR = (0x0F << 4) + FX_BIDICLASS_R,
  FX_BWAxxL = (0x0F << 4) + FX_BIDICLASS_L,
  FX_BWANxx = (FX_BIDICLASS_ON << 4) + 0x0F,
  FX_BWAAxx = (FX_BIDICLASS_AN << 4) + 0x0F,
  FX_BWAExE = (FX_BIDICLASS_EN << 4) + FX_BIDICLASS_EN,
  FX_BWANIx = (FX_BIDICLASS_ON << 4) + 0x0F + 0x100,
  FX_BWANxN = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_ON,
  FX_BWANxR = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_R,
  FX_BWANxE = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_EN,
  FX_BWAAxA = (FX_BIDICLASS_AN << 4) + FX_BIDICLASS_AN,
  FX_BWANxL = (FX_BIDICLASS_ON << 4) + FX_BIDICLASS_L,
  FX_BWALxL = (FX_BIDICLASS_L << 4) + FX_BIDICLASS_L,
  FX_BWAxIL = (0x0F << 4) + FX_BIDICLASS_L + 0x100,
  FX_BWAAxR = (FX_BIDICLASS_AN << 4) + FX_BIDICLASS_R,
  FX_BWALxx = (FX_BIDICLASS_L << 4) + 0x0F,
};

enum FX_BIDINEUTRALSTATE {
  FX_BNSr = 0,
  FX_BNSl,
  FX_BNSrn,
  FX_BNSln,
  FX_BNSa,
  FX_BNSna
};

enum FX_BIDINEUTRALACTION {
  FX_BNAnL = FX_BIDICLASS_L,
  FX_BNAEn = (FX_BIDICLASS_AN << 4),
  FX_BNARn = (FX_BIDICLASS_R << 4),
  FX_BNALn = (FX_BIDICLASS_L << 4),
  FX_BNAIn = FX_BWAIX,
  FX_BNALnL = (FX_BIDICLASS_L << 4) + FX_BIDICLASS_L,
};

const FX_ARBFORMTABLE g_FX_ArabicFormTables[] = {
    {0xFE81, 0xFE82, 0xFE81, 0xFE82}, {0xFE83, 0xFE84, 0xFE83, 0xFE84},
    {0xFE85, 0xFE86, 0xFE85, 0xFE86}, {0xFE87, 0xFE88, 0xFE87, 0xFE88},
    {0xFE89, 0xFE8A, 0xFE8B, 0xFE8C}, {0xFE8D, 0xFE8E, 0xFE8D, 0xFE8E},
    {0xFE8F, 0xFE90, 0xFE91, 0xFE92}, {0xFE93, 0xFE94, 0xFE93, 0xFE94},
    {0xFE95, 0xFE96, 0xFE97, 0xFE98}, {0xFE99, 0xFE9A, 0xFE9B, 0xFE9C},
    {0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0}, {0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4},
    {0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8}, {0xFEA9, 0xFEAA, 0xFEA9, 0xFEAA},
    {0xFEAB, 0xFEAC, 0xFEAB, 0xFEAC}, {0xFEAD, 0xFEAE, 0xFEAD, 0xFEAE},
    {0xFEAF, 0xFEB0, 0xFEAF, 0xFEB0}, {0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4},
    {0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8}, {0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC},
    {0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0}, {0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4},
    {0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8}, {0xFEC9, 0xFECA, 0xFECB, 0xFECC},
    {0xFECD, 0xFECE, 0xFECF, 0xFED0}, {0x063B, 0x063B, 0x063B, 0x063B},
    {0x063C, 0x063C, 0x063C, 0x063C}, {0x063D, 0x063D, 0x063D, 0x063D},
    {0x063E, 0x063E, 0x063E, 0x063E}, {0x063F, 0x063F, 0x063F, 0x063F},
    {0x0640, 0x0640, 0x0640, 0x0640}, {0xFED1, 0xFED2, 0xFED3, 0xFED4},
    {0xFED5, 0xFED6, 0xFED7, 0xFED8}, {0xFED9, 0xFEDA, 0xFEDB, 0xFEDC},
    {0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0}, {0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4},
    {0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8}, {0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC},
    {0xFEED, 0xFEEE, 0xFEED, 0xFEEE}, {0xFEEF, 0xFEF0, 0xFBFE, 0xFBFF},
    {0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4}, {0x064B, 0x064B, 0x064B, 0x064B},
    {0x064C, 0x064C, 0x064C, 0x064C}, {0x064D, 0x064D, 0x064D, 0x064D},
    {0x064E, 0x064E, 0x064E, 0x064E}, {0x064F, 0x064F, 0x064F, 0x064F},
    {0x0650, 0x0650, 0x0650, 0x0650}, {0x0651, 0x0651, 0x0651, 0x0651},
    {0x0652, 0x0652, 0x0652, 0x0652}, {0x0653, 0x0653, 0x0653, 0x0653},
    {0x0654, 0x0654, 0x0654, 0x0654}, {0x0655, 0x0655, 0x0655, 0x0655},
    {0x0656, 0x0656, 0x0656, 0x0656}, {0x0657, 0x0657, 0x0657, 0x0657},
    {0x0658, 0x0658, 0x0658, 0x0658}, {0x0659, 0x0659, 0x0659, 0x0659},
    {0x065A, 0x065A, 0x065A, 0x065A}, {0x065B, 0x065B, 0x065B, 0x065B},
    {0x065C, 0x065C, 0x065C, 0x065C}, {0x065D, 0x065D, 0x065D, 0x065D},
    {0x065E, 0x065E, 0x065E, 0x065E}, {0x065F, 0x065F, 0x065F, 0x065F},
    {0x0660, 0x0660, 0x0660, 0x0660}, {0x0661, 0x0661, 0x0661, 0x0661},
    {0x0662, 0x0662, 0x0662, 0x0662}, {0x0663, 0x0663, 0x0663, 0x0663},
    {0x0664, 0x0664, 0x0664, 0x0664}, {0x0665, 0x0665, 0x0665, 0x0665},
    {0x0666, 0x0666, 0x0666, 0x0666}, {0x0667, 0x0667, 0x0667, 0x0667},
    {0x0668, 0x0668, 0x0668, 0x0668}, {0x0669, 0x0669, 0x0669, 0x0669},
    {0x066A, 0x066A, 0x066A, 0x066A}, {0x066B, 0x066B, 0x066B, 0x066B},
    {0x066C, 0x066C, 0x066C, 0x066C}, {0x066D, 0x066D, 0x066D, 0x066D},
    {0x066E, 0x066E, 0x066E, 0x066E}, {0x066F, 0x066F, 0x066F, 0x066F},
    {0x0670, 0x0670, 0x0670, 0x0670}, {0xFB50, 0xFB51, 0xFB50, 0xFB51},
    {0x0672, 0x0672, 0x0672, 0x0672}, {0x0673, 0x0673, 0x0673, 0x0673},
    {0x0674, 0x0674, 0x0674, 0x0674}, {0x0675, 0x0675, 0x0675, 0x0675},
    {0x0676, 0x0676, 0x0676, 0x0676}, {0x0677, 0x0677, 0x0677, 0x0677},
    {0x0678, 0x0678, 0x0678, 0x0678}, {0xFB66, 0xFB67, 0xFB68, 0xFB69},
    {0xFB5E, 0xFB5F, 0xFB60, 0xFB61}, {0xFB52, 0xFB53, 0xFB54, 0xFB55},
    {0x067C, 0x067C, 0x067C, 0x067C}, {0x067D, 0x067D, 0x067D, 0x067D},
    {0xFB56, 0xFB57, 0xFB58, 0xFB59}, {0xFB62, 0xFB63, 0xFB64, 0xFB65},
    {0xFB5A, 0xFB5B, 0xFB5C, 0xFB5D}, {0x0681, 0x0681, 0x0681, 0x0681},
    {0x0682, 0x0682, 0x0682, 0x0682}, {0xFB76, 0xFB77, 0xFB78, 0xFB79},
    {0xFB72, 0xFB73, 0xFB74, 0xFB75}, {0x0685, 0x0685, 0x0685, 0x0685},
    {0xFB7A, 0xFB7B, 0xFB7C, 0xFB7D}, {0xFB7E, 0xFB7F, 0xFB80, 0xFB81},
    {0xFB88, 0xFB89, 0xFB88, 0xFB89}, {0x0689, 0x0689, 0x0689, 0x0689},
    {0x068A, 0x068A, 0x068A, 0x068A}, {0x068B, 0x068B, 0x068B, 0x068B},
    {0xFB84, 0xFB85, 0xFB84, 0xFB85}, {0xFB82, 0xFB83, 0xFB82, 0xFB83},
    {0xFB86, 0xFB87, 0xFB86, 0xFB87}, {0x068F, 0x068F, 0x068F, 0x068F},
    {0x0690, 0x0690, 0x0690, 0x0690}, {0xFB8C, 0xFB8D, 0xFB8C, 0xFB8D},
    {0x0692, 0x0692, 0x0692, 0x0692}, {0x0693, 0x0693, 0x0693, 0x0693},
    {0x0694, 0x0694, 0x0694, 0x0694}, {0x0695, 0x0695, 0x0695, 0x0695},
    {0x0696, 0x0696, 0x0696, 0x0696}, {0x0697, 0x0697, 0x0697, 0x0697},
    {0xFB8A, 0xFB8B, 0xFB8A, 0xFB8B}, {0x0699, 0x0699, 0x0699, 0x0699},
    {0x069A, 0x069A, 0x069A, 0x069A}, {0x069B, 0x069B, 0x069B, 0x069B},
    {0x069C, 0x069C, 0x069C, 0x069C}, {0x069D, 0x069D, 0x069D, 0x069D},
    {0x069E, 0x069E, 0x069E, 0x069E}, {0x069F, 0x069F, 0x069F, 0x069F},
    {0x06A0, 0x06A0, 0x06A0, 0x06A0}, {0x06A1, 0x06A1, 0x06A1, 0x06A1},
    {0x06A2, 0x06A2, 0x06A2, 0x06A2}, {0x06A3, 0x06A3, 0x06A3, 0x06A3},
    {0xFB6A, 0xFB6B, 0xFB6C, 0xFB6D}, {0x06A5, 0x06A5, 0x06A5, 0x06A5},
    {0xFB6E, 0xFB6F, 0xFB70, 0xFB71}, {0x06A7, 0x06A7, 0x06A7, 0x06A7},
    {0x06A8, 0x06A8, 0x06A8, 0x06A8}, {0xFB8E, 0xFB8F, 0xFB90, 0xFB91},
    {0x06AA, 0x06AA, 0x06AA, 0x06AA}, {0x06AB, 0x06AB, 0x06AB, 0x06AB},
    {0x06AC, 0x06AC, 0x06AC, 0x06AC}, {0xFBD3, 0xFBD4, 0xFBD5, 0xFBD6},
    {0x06AE, 0x06AE, 0x06AE, 0x06AE}, {0xFB92, 0xFB93, 0xFB94, 0xFB95},
    {0x06B0, 0x06B0, 0x06B0, 0x06B0}, {0xFB9A, 0xFB9B, 0xFB9C, 0xFB9D},
    {0x06B2, 0x06B2, 0x06B2, 0x06B2}, {0xFB96, 0xFB97, 0xFB98, 0xFB99},
    {0x06B4, 0x06B4, 0x06B4, 0x06B4}, {0x06B5, 0x06B5, 0x06B5, 0x06B5},
    {0x06B6, 0x06B6, 0x06B6, 0x06B6}, {0x06B7, 0x06B7, 0x06B7, 0x06B7},
    {0x06B8, 0x06B8, 0x06B8, 0x06B8}, {0x06B9, 0x06B9, 0x06B9, 0x06B9},
    {0xFB9E, 0xFB9F, 0xFBE8, 0xFBE9}, {0xFBA0, 0xFBA1, 0xFBA2, 0xFBA3},
    {0x06BC, 0x06BC, 0x06BC, 0x06BC}, {0x06BD, 0x06BD, 0x06BD, 0x06BD},
    {0xFBAA, 0xFBAB, 0xFBAC, 0xFBAD}, {0x06BF, 0x06BF, 0x06BF, 0x06BF},
    {0xFBA4, 0xFBA5, 0xFBA4, 0xFBA5}, {0xFBA6, 0xFBA7, 0xFBA8, 0xFBA9},
    {0x06C2, 0x06C2, 0x06C2, 0x06C2}, {0x06C3, 0x06C3, 0x06C3, 0x06C3},
    {0x06C4, 0x06C4, 0x06C4, 0x06C4}, {0xFBE0, 0xFBE1, 0xFBE0, 0xFBE1},
    {0xFBD9, 0xFBDA, 0xFBD9, 0xFBDA}, {0xFBD7, 0xFBD8, 0xFBD7, 0xFBD8},
    {0xFBDB, 0xFBDC, 0xFBDB, 0xFBDC}, {0xFBE2, 0xFBE3, 0xFBE2, 0xFBE3},
    {0x06CA, 0x06CA, 0x06CA, 0x06CA}, {0xFBDE, 0xFBDF, 0xFBDE, 0xFBDF},
    {0xFBFC, 0xFBFD, 0xFBFE, 0xFBFF}, {0x06CD, 0x06CD, 0x06CD, 0x06CD},
    {0x06CE, 0x06CE, 0x06CE, 0x06CE}, {0x06CF, 0x06CF, 0x06CF, 0x06CF},
    {0xFBE4, 0xFBE5, 0xFBE6, 0xFBE7}, {0x06D1, 0x06D1, 0x06D1, 0x06D1},
    {0xFBAE, 0xFBAF, 0xFBAE, 0xFBAF}, {0xFBB0, 0xFBB1, 0xFBB0, 0xFBB1},
    {0x06D4, 0x06D4, 0x06D4, 0x06D4}, {0x06D5, 0x06D5, 0x06D5, 0x06D5},
};

const FX_ARAALEF gs_FX_AlefTable[] = {
    {0x0622, 0xFEF5},
    {0x0623, 0xFEF7},
    {0x0625, 0xFEF9},
    {0x0627, 0xFEFB},
};

const FX_ARASHADDA gs_FX_ShaddaTable[] = {
    {0x064C, 0xFC5E}, {0x064D, 0xFC5F}, {0x064E, 0xFC60},
    {0x064F, 0xFC61}, {0x0650, 0xFC62},
};

const int32_t gc_FX_BidiNTypes[] = {
    FX_BIDICLASS_N,   FX_BIDICLASS_L,   FX_BIDICLASS_R,   FX_BIDICLASS_AN,
    FX_BIDICLASS_EN,  FX_BIDICLASS_AL,  FX_BIDICLASS_NSM, FX_BIDICLASS_CS,
    FX_BIDICLASS_ES,  FX_BIDICLASS_ET,  FX_BIDICLASS_BN,  FX_BIDICLASS_BN,
    FX_BIDICLASS_N,   FX_BIDICLASS_B,   FX_BIDICLASS_RLO, FX_BIDICLASS_RLE,
    FX_BIDICLASS_LRO, FX_BIDICLASS_LRE, FX_BIDICLASS_PDF, FX_BIDICLASS_ON,
};

const int32_t gc_FX_BidiWeakStates[][10] = {
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWSxa,
     FX_BWSao, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSxr,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSxl,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWSao,
     FX_BWSao, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSro,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlo,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSrt,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlt,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWScn,
     FX_BWSac, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSra,
     FX_BWSrc, FX_BWSro, FX_BWSrt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSre,
     FX_BWSrs, FX_BWSrs, FX_BWSret},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSla,
     FX_BWSlc, FX_BWSlo, FX_BWSlt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSle,
     FX_BWSls, FX_BWSls, FX_BWSlet},
    {FX_BWSao, FX_BWSxl, FX_BWSxr, FX_BWScn, FX_BWScn, FX_BWSxa, FX_BWSao,
     FX_BWSao, FX_BWSao, FX_BWSao},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSro,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSro,
     FX_BWSro, FX_BWSro, FX_BWSrt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlo,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlo,
     FX_BWSlo, FX_BWSlo, FX_BWSlt},
    {FX_BWSro, FX_BWSxl, FX_BWSxr, FX_BWSra, FX_BWSre, FX_BWSxa, FX_BWSret,
     FX_BWSro, FX_BWSro, FX_BWSret},
    {FX_BWSlo, FX_BWSxl, FX_BWSxr, FX_BWSla, FX_BWSle, FX_BWSxa, FX_BWSlet,
     FX_BWSlo, FX_BWSlo, FX_BWSlet},
};

const int32_t gc_FX_BidiWeakActions[][10] = {
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxA, FX_BWAxxR,
     FX_BWAxxR, FX_BWAxxN, FX_BWAxxN, FX_BWAxxN},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxR, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxL, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxA, FX_BWAxxR,
     FX_BWAxxN, FX_BWAxxN, FX_BWAxxN, FX_BWAxxN},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxN, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxN, FX_BWAxxN, FX_BWAxxN, FX_BWAxIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAExE, FX_BWANxR,
     FX_BWAxIx, FX_BWANxN, FX_BWANxN, FX_BWAxIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWALxL, FX_BWANxR,
     FX_BWAxIx, FX_BWANxN, FX_BWANxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxA, FX_BWAxxR,
     FX_BWAxxA, FX_BWAxIx, FX_BWAxxN, FX_BWAxxN},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxA, FX_BWAxIx, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxE, FX_BWAxIx, FX_BWAxIx, FX_BWAxxE},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxA, FX_BWAxIx, FX_BWAxxN, FX_BWAxIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxL, FX_BWAxIx, FX_BWAxIx, FX_BWAxxL},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAAxx, FX_BWAAxA, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANxN},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAAxx, FX_BWANxE, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAExE, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWAAxx, FX_BWANxL, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWANxx, FX_BWALxL, FX_BWANxR,
     FX_BWANxN, FX_BWANxN, FX_BWANxN, FX_BWANIx},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxE, FX_BWAxxR,
     FX_BWAxxE, FX_BWAxxN, FX_BWAxxN, FX_BWAxxE},
    {FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxx, FX_BWAxxL, FX_BWAxxR,
     FX_BWAxxL, FX_BWAxxN, FX_BWAxxN, FX_BWAxxL},
};

const int32_t gc_FX_BidiNeutralStates[][5] = {
    {FX_BNSrn, FX_BNSl, FX_BNSr, FX_BNSr, FX_BNSr},
    {FX_BNSln, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
    {FX_BNSrn, FX_BNSl, FX_BNSr, FX_BNSr, FX_BNSr},
    {FX_BNSln, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
    {FX_BNSna, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
    {FX_BNSna, FX_BNSl, FX_BNSr, FX_BNSa, FX_BNSl},
};
const int32_t gc_FX_BidiNeutralActions[][5] = {
    {FX_BNAIn, 0, 0, 0, 0},
    {FX_BNAIn, 0, 0, 0, FX_BIDICLASS_L},
    {FX_BNAIn, FX_BNAEn, FX_BNARn, FX_BNARn, FX_BNARn},
    {FX_BNAIn, FX_BNALn, FX_BNAEn, FX_BNAEn, FX_BNALnL},
    {FX_BNAIn, 0, 0, 0, FX_BIDICLASS_L},
    {FX_BNAIn, FX_BNAEn, FX_BNARn, FX_BNARn, FX_BNAEn},
};

const int32_t gc_FX_BidiAddLevel[][4] = {
    {0, 1, 2, 2},
    {1, 0, 1, 1},
};

const FX_ARBFORMTABLE* GetArabicFormTable(wchar_t unicode) {
  if (unicode < 0x622 || unicode > 0x6d5)
    return nullptr;
  return g_FX_ArabicFormTables + unicode - 0x622;
}

const FX_ARBFORMTABLE* ParseChar(const CFX_Char* pTC,
                                 wchar_t* wChar,
                                 FX_CHARTYPE* eType) {
  if (!pTC) {
    *eType = FX_CHARTYPE_Unknown;
    *wChar = 0xFEFF;
    return nullptr;
  }

  *eType = pTC->GetCharType();
  *wChar = static_cast<wchar_t>(pTC->m_wCharCode);
  const FX_ARBFORMTABLE* pFT = GetArabicFormTable(*wChar);
  if (!pFT || *eType >= FX_CHARTYPE_ArabicNormal)
    *eType = FX_CHARTYPE_Unknown;

  return pFT;
}

wchar_t GetArabicFromAlefTable(wchar_t alef) {
  static const size_t s_iAlefCount = FX_ArraySize(gs_FX_AlefTable);
  for (size_t iStart = 0; iStart < s_iAlefCount; iStart++) {
    const FX_ARAALEF& v = gs_FX_AlefTable[iStart];
    if (v.wAlef == alef)
      return v.wIsolated;
  }
  return alef;
}

class CFX_BidiLine {
 public:
  void BidiLine(std::vector<CFX_Char>* chars,
                int32_t iCount,
                int32_t iBaseLevel) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    if (iCount < 2)
      return;

    Classify(chars, iCount, false);
    ResolveExplicit(chars, iCount, iBaseLevel);
    ResolveWeak(chars, iCount, iBaseLevel);
    ResolveNeutrals(chars, iCount, iBaseLevel);
    ResolveImplicit(chars, iCount);
    Classify(chars, iCount, true);
    ResolveWhitespace(chars, iCount, iBaseLevel);
    Reorder(chars, iCount, iBaseLevel);
    Position(chars, iCount);
  }

 private:
  int32_t Direction(int32_t val) {
    return FX_IsOdd(val) ? FX_BIDICLASS_R : FX_BIDICLASS_L;
  }

  int32_t GetDeferredType(int32_t val) { return (val >> 4) & 0x0F; }

  int32_t GetResolvedType(int32_t val) { return val & 0x0F; }

  int32_t GetDeferredNeutrals(int32_t iAction, int32_t iLevel) {
    iAction = (iAction >> 4) & 0xF;
    if (iAction == (FX_BNAEn >> 4))
      return Direction(iLevel);
    return iAction;
  }

  int32_t GetResolvedNeutrals(int32_t iAction) {
    iAction &= 0xF;
    return iAction == FX_BNAIn ? 0 : iAction;
  }

  void ReverseString(std::vector<CFX_Char>* chars,
                     int32_t iStart,
                     int32_t iCount) {
    ASSERT(pdfium::IndexInBounds(*chars, iStart));
    ASSERT(iCount >= 0 &&
           iStart + iCount <= pdfium::CollectionSize<int32_t>(*chars));
    std::reverse(chars->begin() + iStart, chars->begin() + iStart + iCount);
  }

  void SetDeferredRun(std::vector<CFX_Char>* chars,
                      bool bClass,
                      int32_t iStart,
                      int32_t iCount,
                      int32_t iValue) {
    ASSERT(iStart >= 0 && iStart <= pdfium::CollectionSize<int32_t>(*chars));
    ASSERT(iStart - iCount > -1);
    int32_t iLast = iStart - iCount;
    if (bClass) {
      for (int32_t i = iStart - 1; i >= iLast; i--)
        (*chars)[i].m_iBidiClass = static_cast<int16_t>(iValue);
      return;
    }

    for (int32_t i = iStart - 1; i >= iLast; i--)
      (*chars)[i].m_iBidiLevel = static_cast<int16_t>(iValue);
  }

  void Classify(std::vector<CFX_Char>* chars, int32_t iCount, bool bWS) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    if (bWS) {
      for (int32_t i = 0; i < iCount; i++) {
        (*chars)[i].m_iBidiClass =
            static_cast<int16_t>((*chars)[i].m_dwCharProps &
                                 FX_BIDICLASSBITSMASK) >>
            FX_BIDICLASSBITS;
      }
      return;
    }

    for (int32_t i = 0; i < iCount; i++) {
      (*chars)[i].m_iBidiClass = static_cast<int16_t>(
          gc_FX_BidiNTypes[((*chars)[i].m_dwCharProps & FX_BIDICLASSBITSMASK) >>
                           FX_BIDICLASSBITS]);
    }
  }

  void ResolveExplicit(std::vector<CFX_Char>* chars,
                       int32_t iCount,
                       int32_t iBaseLevel) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    ASSERT(iBaseLevel >= 0 && iBaseLevel <= FX_BIDIMAXLEVEL);
    for (int32_t i = 0; i < iCount; i++)
      (*chars)[i].m_iBidiLevel = static_cast<int16_t>(iBaseLevel);
  }

  void ResolveWeak(std::vector<CFX_Char>* chars,
                   int32_t iCount,
                   int32_t iBaseLevel) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    iCount--;
    if (iCount < 1)
      return;

    int32_t iLevelCur = iBaseLevel;
    int32_t iState = FX_IsOdd(iBaseLevel) ? FX_BWSxr : FX_BWSxl;
    int32_t i = 0;
    int32_t iNum = 0;
    int32_t iClsCur;
    int32_t iClsRun;
    int32_t iClsNew;
    int32_t iAction;
    for (; i <= iCount; i++) {
      CFX_Char* pTC = &(*chars)[i];
      iClsCur = pTC->m_iBidiClass;
      if (iClsCur == FX_BIDICLASS_BN) {
        pTC->m_iBidiLevel = (int16_t)iLevelCur;
        if (i == iCount && iLevelCur != iBaseLevel) {
          iClsCur = Direction(iLevelCur);
          pTC->m_iBidiClass = (int16_t)iClsCur;
        } else if (i < iCount) {
          CFX_Char* pTCNext = &(*chars)[i + 1];
          int32_t iLevelNext, iLevelNew;
          iClsNew = pTCNext->m_iBidiClass;
          iLevelNext = pTCNext->m_iBidiLevel;
          if (iClsNew != FX_BIDICLASS_BN && iLevelCur != iLevelNext) {
            iLevelNew = std::max(iLevelNext, iLevelCur);
            pTC->m_iBidiLevel = static_cast<int16_t>(iLevelNew);
            iClsCur = Direction(iLevelNew);
            pTC->m_iBidiClass = static_cast<int16_t>(iClsCur);
            iLevelCur = iLevelNext;
          } else {
            if (iNum > 0)
              iNum++;
            continue;
          }
        } else {
          if (iNum > 0)
            iNum++;
          continue;
        }
      }

      ASSERT(iClsCur <= FX_BIDICLASS_BN);
      iAction = gc_FX_BidiWeakActions[iState][iClsCur];
      iClsRun = GetDeferredType(iAction);
      if (iClsRun != FX_BWAXX && iNum > 0) {
        SetDeferredRun(chars, true, i, iNum, iClsRun);
        iNum = 0;
      }
      iClsNew = GetResolvedType(iAction);
      if (iClsNew != FX_BWAXX)
        pTC->m_iBidiClass = static_cast<int16_t>(iClsNew);
      if (FX_BWAIX & iAction)
        iNum++;

      iState = gc_FX_BidiWeakStates[iState][iClsCur];
    }
    if (iNum > 0) {
      iClsCur = Direction(iBaseLevel);
      iClsRun = GetDeferredType(gc_FX_BidiWeakActions[iState][iClsCur]);
      if (iClsRun != FX_BWAXX)
        SetDeferredRun(chars, true, i, iNum, iClsRun);
    }
  }

  void ResolveNeutrals(std::vector<CFX_Char>* chars,
                       int32_t iCount,
                       int32_t iBaseLevel) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    ASSERT(iBaseLevel >= 0 && iBaseLevel <= FX_BIDIMAXLEVEL);
    iCount--;
    if (iCount < 1)
      return;

    CFX_Char* pTC;
    int32_t iLevel = iBaseLevel;
    int32_t iState = FX_IsOdd(iBaseLevel) ? FX_BNSr : FX_BNSl;
    int32_t i = 0;
    int32_t iNum = 0;
    int32_t iClsCur;
    int32_t iClsRun;
    int32_t iClsNew;
    int32_t iAction;
    for (; i <= iCount; i++) {
      pTC = &(*chars)[i];
      iClsCur = pTC->m_iBidiClass;
      if (iClsCur == FX_BIDICLASS_BN) {
        if (iNum)
          iNum++;
        continue;
      }

      ASSERT(iClsCur < FX_BIDICLASS_AL);
      iAction = gc_FX_BidiNeutralActions[iState][iClsCur];
      iClsRun = GetDeferredNeutrals(iAction, iLevel);
      if (iClsRun != FX_BIDICLASS_N && iNum > 0) {
        SetDeferredRun(chars, true, i, iNum, iClsRun);
        iNum = 0;
      }

      iClsNew = GetResolvedNeutrals(iAction);
      if (iClsNew != FX_BIDICLASS_N)
        pTC->m_iBidiClass = (int16_t)iClsNew;
      if (FX_BNAIn & iAction)
        iNum++;

      iState = gc_FX_BidiNeutralStates[iState][iClsCur];
      iLevel = pTC->m_iBidiLevel;
    }
    if (iNum > 0) {
      iClsCur = Direction(iLevel);
      iClsRun = GetDeferredNeutrals(gc_FX_BidiNeutralActions[iState][iClsCur],
                                    iLevel);
      if (iClsRun != FX_BIDICLASS_N)
        SetDeferredRun(chars, true, i, iNum, iClsRun);
    }
  }

  void ResolveImplicit(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    for (int32_t i = 0; i < iCount; i++) {
      int32_t iCls = (*chars)[i].m_iBidiClass;
      if (iCls == FX_BIDICLASS_BN)
        continue;

      ASSERT(iCls > FX_BIDICLASS_ON && iCls < FX_BIDICLASS_AL);
      int32_t iLevel = (*chars)[i].m_iBidiLevel;
      iLevel += gc_FX_BidiAddLevel[FX_IsOdd(iLevel)][iCls - 1];
      (*chars)[i].m_iBidiLevel = (int16_t)iLevel;
    }
  }

  void ResolveWhitespace(std::vector<CFX_Char>* chars,
                         int32_t iCount,
                         int32_t iBaseLevel) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    ASSERT(iBaseLevel >= 0 && iBaseLevel <= FX_BIDIMAXLEVEL);
    if (iCount < 1)
      return;

    iCount--;
    int32_t iLevel = iBaseLevel;
    int32_t i = 0;
    int32_t iNum = 0;
    for (; i <= iCount; i++) {
      switch ((*chars)[i].m_iBidiClass) {
        case FX_BIDICLASS_WS:
          iNum++;
          break;
        case FX_BIDICLASS_RLE:
        case FX_BIDICLASS_LRE:
        case FX_BIDICLASS_LRO:
        case FX_BIDICLASS_RLO:
        case FX_BIDICLASS_PDF:
        case FX_BIDICLASS_BN:
          (*chars)[i].m_iBidiLevel = (int16_t)iLevel;
          iNum++;
          break;
        case FX_BIDICLASS_S:
        case FX_BIDICLASS_B:
          if (iNum > 0)
            SetDeferredRun(chars, false, i, iNum, iBaseLevel);

          (*chars)[i].m_iBidiLevel = static_cast<int16_t>(iBaseLevel);
          iNum = 0;
          break;
        default:
          iNum = 0;
          break;
      }
      iLevel = (*chars)[i].m_iBidiLevel;
    }
    if (iNum > 0)
      SetDeferredRun(chars, false, i, iNum, iBaseLevel);
  }

  int32_t ReorderLevel(std::vector<CFX_Char>* chars,
                       int32_t iCount,
                       int32_t iBaseLevel,
                       int32_t iStart,
                       bool bReverse) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    ASSERT(iBaseLevel >= 0 && iBaseLevel <= FX_BIDIMAXLEVEL);
    ASSERT(iStart >= 0 && iStart < iCount);

    if (iCount < 1)
      return 0;

    bReverse = bReverse || FX_IsOdd(iBaseLevel);
    int32_t i = iStart;
    for (; i < iCount; i++) {
      int32_t iLevel = (*chars)[i].m_iBidiLevel;
      if (iLevel == iBaseLevel)
        continue;
      if (iLevel < iBaseLevel)
        break;
      i += ReorderLevel(chars, iCount, iBaseLevel + 1, i, bReverse) - 1;
    }
    int32_t iNum = i - iStart;
    if (bReverse && iNum > 1)
      ReverseString(chars, iStart, iNum);

    return iNum;
  }

  void Reorder(std::vector<CFX_Char>* chars,
               int32_t iCount,
               int32_t iBaseLevel) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    ASSERT(iBaseLevel >= 0 && iBaseLevel <= FX_BIDIMAXLEVEL);
    int32_t i = 0;
    while (i < iCount)
      i += ReorderLevel(chars, iCount, iBaseLevel, i, false);
  }

  void Position(std::vector<CFX_Char>* chars, int32_t iCount) {
    ASSERT(iCount >= 0 && iCount <= pdfium::CollectionSize<int32_t>(*chars));
    for (int32_t i = 0; i < iCount; ++i)
      (*chars)[(*chars)[i].m_iBidiPos].m_iBidiOrder = i;
  }
};

}  // namespace

namespace pdfium {
namespace arabic {

wchar_t GetFormChar(wchar_t wch, wchar_t prev, wchar_t next) {
  CFX_Char c(wch, kTextLayoutCodeProperties[static_cast<uint16_t>(wch)]);
  CFX_Char p(prev, kTextLayoutCodeProperties[static_cast<uint16_t>(prev)]);
  CFX_Char n(next, kTextLayoutCodeProperties[static_cast<uint16_t>(next)]);
  return GetFormChar(&c, &p, &n);
}

wchar_t GetFormChar(const CFX_Char* cur,
                    const CFX_Char* prev,
                    const CFX_Char* next) {
  FX_CHARTYPE eCur;
  wchar_t wCur;
  const FX_ARBFORMTABLE* ft = ParseChar(cur, &wCur, &eCur);
  if (eCur < FX_CHARTYPE_ArabicAlef || eCur >= FX_CHARTYPE_ArabicNormal)
    return wCur;

  FX_CHARTYPE ePrev;
  wchar_t wPrev;
  ParseChar(prev, &wPrev, &ePrev);
  if (wPrev == 0x0644 && eCur == FX_CHARTYPE_ArabicAlef)
    return 0xFEFF;

  FX_CHARTYPE eNext;
  wchar_t wNext;
  ParseChar(next, &wNext, &eNext);
  bool bAlef = (eNext == FX_CHARTYPE_ArabicAlef && wCur == 0x644);
  if (ePrev < FX_CHARTYPE_ArabicAlef) {
    if (bAlef)
      return GetArabicFromAlefTable(wNext);
    return (eNext < FX_CHARTYPE_ArabicAlef) ? ft->wIsolated : ft->wInitial;
  }

  if (bAlef) {
    wCur = GetArabicFromAlefTable(wNext);
    return (ePrev != FX_CHARTYPE_ArabicDistortion) ? wCur : ++wCur;
  }

  if (ePrev == FX_CHARTYPE_ArabicAlef || ePrev == FX_CHARTYPE_ArabicSpecial)
    return (eNext < FX_CHARTYPE_ArabicAlef) ? ft->wIsolated : ft->wInitial;
  return (eNext < FX_CHARTYPE_ArabicAlef) ? ft->wFinal : ft->wMedial;
}

}  // namespace arabic
}  // namespace pdfium

wchar_t FX_GetArabicFromShaddaTable(wchar_t shadda) {
  static const size_t s_iShaddaCount = FX_ArraySize(gs_FX_ShaddaTable);
  for (size_t iStart = 0; iStart < s_iShaddaCount; iStart++) {
    const FX_ARASHADDA& v = gs_FX_ShaddaTable[iStart];
    if (v.wShadda == shadda)
      return v.wIsolated;
  }
  return shadda;
}

void FX_BidiLine(std::vector<CFX_Char>* chars, int32_t iCount) {
  CFX_BidiLine blt;
  blt.BidiLine(chars, iCount, 0);
}