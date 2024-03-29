// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/formfiller/cffl_textfield.h"

#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cba_fontmap.h"
#include "fpdfsdk/fsdk_common.h"
#include "third_party/base/ptr_util.h"

CFFL_TextField::CFFL_TextField(CPDFSDK_FormFillEnvironment* pApp,
                               CPDFSDK_Widget* pWidget)
    : CFFL_FormFiller(pApp, pWidget) {}

CFFL_TextField::~CFFL_TextField() {
  for (const auto& it : m_Maps)
    it.second->InvalidateFocusHandler(this);

  // See comment in cffl_formfiller.h.
  // The font map should be stored somewhere more appropriate so it will live
  // until the PWL_Edit is done with it. pdfium:566
  DestroyWindows();
}

PWL_CREATEPARAM CFFL_TextField::GetCreateParam() {
  PWL_CREATEPARAM cp = CFFL_FormFiller::GetCreateParam();
  int nFlags = m_pWidget->GetFieldFlags();
  if (nFlags & FIELDFLAG_PASSWORD)
    cp.dwFlags |= PES_PASSWORD;

  if (nFlags & FIELDFLAG_MULTILINE) {
    cp.dwFlags |= PES_MULTILINE | PES_AUTORETURN | PES_TOP;
    if (!(nFlags & FIELDFLAG_DONOTSCROLL))
      cp.dwFlags |= PWS_VSCROLL | PES_AUTOSCROLL;
  } else {
    cp.dwFlags |= PES_CENTER;
    if (!(nFlags & FIELDFLAG_DONOTSCROLL))
      cp.dwFlags |= PES_AUTOSCROLL;
  }

  if (nFlags & FIELDFLAG_COMB)
    cp.dwFlags |= PES_CHARARRAY;

  if (nFlags & FIELDFLAG_RICHTEXT)
    cp.dwFlags |= PES_RICH;

  cp.dwFlags |= PES_UNDO;

  switch (m_pWidget->GetAlignment()) {
    default:
    case BF_ALIGN_LEFT:
      cp.dwFlags |= PES_LEFT;
      break;
    case BF_ALIGN_MIDDLE:
      cp.dwFlags |= PES_MIDDLE;
      break;
    case BF_ALIGN_RIGHT:
      cp.dwFlags |= PES_RIGHT;
      break;
  }
  if (!m_pFontMap) {
    m_pFontMap = pdfium::MakeUnique<CBA_FontMap>(
        m_pWidget.Get(), m_pFormFillEnv->GetSysHandler());
  }
  cp.pFontMap = m_pFontMap.get();
  cp.pFocusHandler = this;
  return cp;
}

CPWL_Wnd* CFFL_TextField::NewPDFWindow(const PWL_CREATEPARAM& cp) {
  CPWL_Edit* pWnd = new CPWL_Edit();
  pWnd->AttachFFLData(this);
  pWnd->Create(cp);
  pWnd->SetFillerNotify(m_pFormFillEnv->GetInteractiveFormFiller());

  int32_t nMaxLen = m_pWidget->GetMaxLen();
  CFX_WideString swValue = m_pWidget->GetValue();

  if (nMaxLen > 0) {
    if (pWnd->HasFlag(PES_CHARARRAY)) {
      pWnd->SetCharArray(nMaxLen);
      pWnd->SetAlignFormatV(PEAV_CENTER);
    } else {
      pWnd->SetLimitChar(nMaxLen);
    }
  }

  pWnd->SetText(swValue);
  return pWnd;
}

bool CFFL_TextField::OnChar(CPDFSDK_Annot* pAnnot,
                            uint32_t nChar,
                            uint32_t nFlags) {
  switch (nChar) {
    case FWL_VKEY_Return: {
      if (m_pWidget->GetFieldFlags() & FIELDFLAG_MULTILINE)
        break;

      CPDFSDK_PageView* pPageView = GetCurPageView(true);
      ASSERT(pPageView);
      m_bValid = !m_bValid;
      m_pFormFillEnv->Invalidate(pAnnot->GetUnderlyingPage(),
                                 pAnnot->GetRect().ToFxRect());

      if (m_bValid) {
        if (CPWL_Wnd* pWnd = GetPDFWindow(pPageView, true))
          pWnd->SetFocus();
        break;
      }

      if (!CommitData(pPageView, nFlags))
        return false;

      DestroyPDFWindow(pPageView);
      return true;
    }
    case FWL_VKEY_Escape: {
      CPDFSDK_PageView* pPageView = GetCurPageView(true);
      ASSERT(pPageView);
      EscapeFiller(pPageView, true);
      return true;
    }
  }

  return CFFL_FormFiller::OnChar(pAnnot, nChar, nFlags);
}

bool CFFL_TextField::IsDataChanged(CPDFSDK_PageView* pPageView) {
  CPWL_Edit* pEdit = GetEdit(pPageView, false);
  return pEdit && pEdit->GetText() != m_pWidget->GetValue();
}

void CFFL_TextField::SaveData(CPDFSDK_PageView* pPageView) {
  CPWL_Edit* pWnd = GetEdit(pPageView, false);
  if (!pWnd)
    return;

  CFX_WideString sOldValue = m_pWidget->GetValue();
  CFX_WideString sNewValue = pWnd->GetText();

  m_pWidget->SetValue(sNewValue, false);
  m_pWidget->ResetFieldAppearance(true);
  m_pWidget->UpdateField();
  SetChangeMark();
}

void CFFL_TextField::GetActionData(CPDFSDK_PageView* pPageView,
                                   CPDF_AAction::AActionType type,
                                   PDFSDK_FieldAction& fa) {
  switch (type) {
    case CPDF_AAction::KeyStroke:
      if (CPWL_Edit* pWnd = GetEdit(pPageView, false)) {
        fa.bFieldFull = pWnd->IsTextFull();

        fa.sValue = pWnd->GetText();

        if (fa.bFieldFull) {
          fa.sChange = L"";
          fa.sChangeEx = L"";
        }
      }
      break;
    case CPDF_AAction::Validate:
      if (CPWL_Edit* pWnd = GetEdit(pPageView, false)) {
        fa.sValue = pWnd->GetText();
      }
      break;
    case CPDF_AAction::LoseFocus:
    case CPDF_AAction::GetFocus:
      fa.sValue = m_pWidget->GetValue();
      break;
    default:
      break;
  }
}

void CFFL_TextField::SetActionData(CPDFSDK_PageView* pPageView,
                                   CPDF_AAction::AActionType type,
                                   const PDFSDK_FieldAction& fa) {
  switch (type) {
    case CPDF_AAction::KeyStroke:
      if (CPWL_Edit* pEdit = GetEdit(pPageView, false)) {
        pEdit->SetFocus();
        pEdit->SetSel(fa.nSelStart, fa.nSelEnd);
        pEdit->ReplaceSel(fa.sChange);
      }
      break;
    default:
      break;
  }
}

bool CFFL_TextField::IsActionDataChanged(CPDF_AAction::AActionType type,
                                         const PDFSDK_FieldAction& faOld,
                                         const PDFSDK_FieldAction& faNew) {
  switch (type) {
    case CPDF_AAction::KeyStroke:
      return (!faOld.bFieldFull && faOld.nSelEnd != faNew.nSelEnd) ||
             faOld.nSelStart != faNew.nSelStart ||
             faOld.sChange != faNew.sChange;
    default:
      break;
  }

  return false;
}

void CFFL_TextField::SaveState(CPDFSDK_PageView* pPageView) {
  ASSERT(pPageView);

  CPWL_Edit* pWnd = GetEdit(pPageView, false);
  if (!pWnd)
    return;

  pWnd->GetSel(m_State.nStart, m_State.nEnd);
  m_State.sValue = pWnd->GetText();
}

void CFFL_TextField::RestoreState(CPDFSDK_PageView* pPageView) {
  ASSERT(pPageView);

  CPWL_Edit* pWnd = GetEdit(pPageView, true);
  if (!pWnd)
    return;

  pWnd->SetText(m_State.sValue);
  pWnd->SetSel(m_State.nStart, m_State.nEnd);
}

CPWL_Wnd* CFFL_TextField::ResetPDFWindow(CPDFSDK_PageView* pPageView,
                                         bool bRestoreValue) {
  if (bRestoreValue)
    SaveState(pPageView);

  DestroyPDFWindow(pPageView);
  if (bRestoreValue)
    RestoreState(pPageView);

  CPWL_Wnd::ObservedPtr pRet(GetPDFWindow(pPageView, !bRestoreValue));
  m_pWidget->UpdateField();  // May invoke JS, invalidating pRet.
  return pRet.Get();
}

#ifdef PDF_ENABLE_XFA
bool CFFL_TextField::IsFieldFull(CPDFSDK_PageView* pPageView) {
  CPWL_Edit* pWnd = GetEdit(pPageView, false);
  return pWnd && pWnd->IsTextFull();
}
#endif  // PDF_ENABLE_XFA

void CFFL_TextField::OnSetFocus(CPWL_Edit* pEdit) {
  pEdit->SetCharSet(FX_CHARSET_ChineseSimplified);
  pEdit->SetReadyToInput();

  CFX_WideString wsText = pEdit->GetText();
  int nCharacters = wsText.GetLength();
  CFX_ByteString bsUTFText = wsText.UTF16LE_Encode();
  auto* pBuffer = reinterpret_cast<const unsigned short*>(bsUTFText.c_str());
  m_pFormFillEnv->OnSetFieldInputFocus(pBuffer, nCharacters, true);
}

CPWL_Edit* CFFL_TextField::GetEdit(CPDFSDK_PageView* pPageView, bool bNew) {
  return static_cast<CPWL_Edit*>(GetPDFWindow(pPageView, bNew));
}
