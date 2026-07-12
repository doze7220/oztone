#include "Widget_GlobalHotkeys.h"
#include "WidgetCommon.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include "resource.h"
#include <filesystem>

namespace {
enum ActionID {
  ACTION_NEXT_TRACK = 0,
  ACTION_PREV_TRACK,
  ACTION_PLAY_PAUSE,
  ACTION_STOP,
  ACTION_VOL_UP_5,
  ACTION_VOL_DOWN_5,
  ACTION_VOL_UP_25,
  ACTION_VOL_DOWN_25,
  ACTION_NEXT_PLAYLIST,
  ACTION_PREV_PLAYLIST,
  ACTION_PIN_TOP,
  ACTION_PIN_BOTTOM,
  ACTION_EXIT_APP
};
} // namespace

void GlobalHotkeysWidget::CreateResources(ID2D1DeviceContext *context,
                                          IWICImagingFactory *wicFactory,
                                          IDWriteFactory *dwriteFactory,
                                          const ConfigManager *config) {
  if (!config)
    return;
  m_dwriteFactory = dwriteFactory;

  m_keyTextLayout.Reset();
  m_actionTextLayout
      .Reset(); // Force recreation of text layout when resources are recreated

  dwriteFactory->CreateTextFormat(
      config->GetGlobalHotkeysFontFamily().c_str(), nullptr,
      DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, config->GetGlobalHotkeysFontSize(), L"en-us",
      &m_textFormat);

  if (m_textFormat) {
    m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  }

  context->CreateSolidColorBrush(
      WidgetCommon::HexToColorF(config->GetGlobalHotkeysCoreColor(), 0.9f), &m_coreBrush);
  context->CreateSolidColorBrush(
      WidgetCommon::HexToColorF(config->GetGlobalHotkeysGlowColor(),
                                config->GetGlobalHotkeysGlowOpacity()),
      &m_glowBrush);
  context->CreateSolidColorBrush(
      WidgetCommon::HexToColorF(config->GetGlobalHotkeysShadowColor(), 1.0f), &m_shadowBrush);
}

void GlobalHotkeysWidget::ReleaseResources() {
  m_textFormat.Reset();
  m_keyTextLayout.Reset();
  m_actionTextLayout.Reset();
  m_dwriteFactory.Reset();
  m_coreBrush.Reset();
  m_glowBrush.Reset();
  m_shadowBrush.Reset();
}

void GlobalHotkeysWidget::UpdateAnimation(const WidgetContext &ctx) {}

std::wstring GlobalHotkeysWidget::GetKeyName(int mod, int vk) {
  if (vk == 0)
    return L"None";
  std::wstring result;
  if (mod & MOD_CONTROL)
    result += L"CTRL + ";
  if (mod & MOD_SHIFT)
    result += L"SHIFT + ";
  if (mod & MOD_ALT)
    result += L"ALT + ";
  if (mod & MOD_WIN)
    result += L"WIN + ";

  if (vk >= 'A' && vk <= 'Z') {
    result += static_cast<wchar_t>(vk);
  } else if (vk >= '0' && vk <= '9') {
    result += static_cast<wchar_t>(vk);
  } else {
    switch (vk) {
    case VK_LEFT:
      result += L"←";
      break;
    case VK_RIGHT:
      result += L"→";
      break;
    case VK_UP:
      result += L"↑";
      break;
    case VK_DOWN:
      result += L"↓";
      break;
    case VK_SPACE:
      result += L"SPACE";
      break;
    case VK_ESCAPE:
      result += L"ESC";
      break;
    case VK_RETURN:
      result += L"ENTER";
      break;
    case VK_HOME:
      result += L"HOME";
      break;
    case VK_END:
      result += L"END";
      break;
    case VK_PRIOR:
      result += L"PAGE UP";
      break;
    case VK_NEXT:
      result += L"PAGE DOWN";
      break;
    case VK_INSERT:
      result += L"INSERT";
      break;
    case VK_DELETE:
      result += L"DELETE";
      break;
    case VK_OEM_PLUS:
      result += L"+";
      break;
    case VK_OEM_MINUS:
      result += L"-";
      break;
    default:
      result += L"Key(" + std::to_wstring(vk) + L")";
      break;
    }
  }
  return result;
}

std::wstring GlobalHotkeysWidget::GetActionName(int actionId) {
  switch (actionId) {
  case ACTION_NEXT_TRACK:
    return L": 次の曲";
  case ACTION_PREV_TRACK:
    return L": 前の曲";
  case ACTION_PLAY_PAUSE:
    return L": 再生/一時停止";
  case ACTION_STOP:
    return L": 停止";
  case ACTION_VOL_UP_5:
    return L": 音量 +5%";
  case ACTION_VOL_DOWN_5:
    return L": 音量 -5%";
  case ACTION_VOL_UP_25:
    return L": 音量 +25%";
  case ACTION_VOL_DOWN_25:
    return L": 音量 -25%";
  case ACTION_NEXT_PLAYLIST:
    return L": 前プレイリスト";
  case ACTION_PREV_PLAYLIST:
    return L": 次プレイリスト";
  case ACTION_PIN_TOP:
    return L": 最前面固定";
  case ACTION_PIN_BOTTOM:
    return L": 最背面固定";
  case ACTION_EXIT_APP:
    return L": 終了";
  default:
    return L"";
  }
}

void GlobalHotkeysWidget::GenerateHotkeysStrings(const ConfigManager *config,
                                                 std::wstring &outKeys,
                                                 std::wstring &outActions) {
  outKeys.clear();
  outActions.clear();
  auto append = [&](int actionId, int mod, int vk) {
    if (vk != 0) {
      outKeys += GetKeyName(mod, vk) + L"\n";
      outActions += GetActionName(actionId) + L"\n";
    }
  };
  append(ACTION_NEXT_TRACK, config->GetModifierNextTrack(),
         config->GetVKNextTrack());
  append(ACTION_PREV_TRACK, config->GetModifierPrevTrack(),
         config->GetVKPrevTrack());
  append(ACTION_PLAY_PAUSE, config->GetModifierPlayPause(),
         config->GetVKPlayPause());
  append(ACTION_STOP, config->GetModifierStop(), config->GetVKStop());
  append(ACTION_VOL_UP_5, config->GetModifierVolUp5(), config->GetVKVolUp5());
  append(ACTION_VOL_DOWN_5, config->GetModifierVolDown5(),
         config->GetVKVolDown5());
  append(ACTION_VOL_UP_25, config->GetModifierVolUp25(),
         config->GetVKVolUp25());
  append(ACTION_VOL_DOWN_25, config->GetModifierVolDown25(),
         config->GetVKVolDown25());
  append(ACTION_NEXT_PLAYLIST, config->GetModifierNextPlaylist(),
         config->GetVKNextPlaylist());
  append(ACTION_PREV_PLAYLIST, config->GetModifierPrevPlaylist(),
         config->GetVKPrevPlaylist());
  append(ACTION_PIN_TOP, config->GetModifierActiveTopMost(),
         config->GetVKActiveTopMost());
  append(ACTION_PIN_BOTTOM, config->GetModifierActiveBottom(),
         config->GetVKActiveBottom());
  append(ACTION_EXIT_APP, config->GetModifierExitApp(), config->GetVKExitApp());
}

void GlobalHotkeysWidget::UpdateLayout(const WidgetContext &ctx,
                                       const ConfigManager *config) {
  // Keep it empty because we don't have logicalWidth here.
  // We will update text layout in Draw.
}

void GlobalHotkeysWidget::Draw(ID2D1DeviceContext *context,
                               const WidgetContext &ctx,
                               const ConfigManager *config) {
  if (!config || !config->GetShowHotkeys())
    return;

  D2D1_SIZE_F rtSize = context->GetSize();
  float logicWidth = rtSize.width / ctx.dpiScale;

  bool show = config->GetShowHotkeys();
  std::wstring keyStr, actionStr;
  if (show) {
    GenerateHotkeysStrings(config, keyStr, actionStr);
  }

  if (keyStr != m_lastKeyString || actionStr != m_lastActionString ||
      show != m_wasShow || !m_keyTextLayout || !m_actionTextLayout) {
    m_keyTextLayout.Reset();
    m_actionTextLayout.Reset();
    if (show && (!keyStr.empty() || !actionStr.empty())) {
      GlobalHotkeysLayout layout =
          LayoutCalculator::CalculateGlobalHotkeysLayout(logicWidth, config);

      auto createLayout =
          [&](const std::wstring &text, D2D1_RECT_F rect,
              Microsoft::WRL::ComPtr<IDWriteTextLayout> &outLayout) {
            float w = rect.right - rect.left;
            float h = rect.bottom - rect.top;
            m_dwriteFactory->CreateTextLayout(
                text.c_str(), static_cast<UINT32>(text.length()),
                m_textFormat.Get(), w, h, &outLayout);
            if (outLayout) {
              Microsoft::WRL::ComPtr<IDWriteTextLayout1> textLayout1;
              if (SUCCEEDED(outLayout.As(&textLayout1))) {
                textLayout1->SetLineSpacing(
                    DWRITE_LINE_SPACING_METHOD_UNIFORM,
                    config->GetGlobalHotkeysLineSpacing(),
                    config->GetGlobalHotkeysFontSize());
              }
            }
          };

      createLayout(keyStr, layout.keyColumnRect, m_keyTextLayout);
      createLayout(actionStr, layout.actionColumnRect, m_actionTextLayout);
    }
    m_lastKeyString = keyStr;
    m_lastActionString = actionStr;
    m_wasShow = show;
  }

  if (!m_keyTextLayout || !m_actionTextLayout)
    return;

  GlobalHotkeysLayout layout =
      LayoutCalculator::CalculateGlobalHotkeysLayout(logicWidth, config);

  auto drawText = [&](Microsoft::WRL::ComPtr<IDWriteTextLayout> &textLayout,
                      D2D1_POINT_2F origin) {
    WidgetCommon::DrawShadowedTextLayout(
        context, textLayout.Get(), nullptr, m_shadowBrush.Get(),
        origin, D2D1::Point2F(origin.x + 2.0f, origin.y + 2.0f), config->GetGlobalHotkeysShadowOpacity());

    if (m_glowBrush) {
      float glowOffsets[] = {1.5f, 3.0f};
      for (float offset : glowOffsets) {
        context->DrawTextLayout(D2D1::Point2F(origin.x - offset, origin.y),
                                textLayout.Get(), m_glowBrush.Get());
        context->DrawTextLayout(D2D1::Point2F(origin.x + offset, origin.y),
                                textLayout.Get(), m_glowBrush.Get());
        context->DrawTextLayout(D2D1::Point2F(origin.x, origin.y - offset),
                                textLayout.Get(), m_glowBrush.Get());
        context->DrawTextLayout(D2D1::Point2F(origin.x, origin.y + offset),
                                textLayout.Get(), m_glowBrush.Get());
      }
    }

    if (m_coreBrush) {
      context->DrawTextLayout(origin, textLayout.Get(), m_coreBrush.Get());
    }
  };

  drawText(m_keyTextLayout,
           D2D1::Point2F(layout.keyColumnRect.left, layout.keyColumnRect.top));
  drawText(m_actionTextLayout, D2D1::Point2F(layout.actionColumnRect.left,
                                             layout.actionColumnRect.top));
}
