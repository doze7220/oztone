#include "Widget_Playlist.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include <filesystem>
void PlaylistWidget::BuildToolbarText(const WidgetContext& ctx, const ConfigManager* config, std::wstring& outText) {
  std::wstring hoverText = L"";
  if (ctx.playlistToolbarHoveredIndex >= 0 &&
      ctx.playlistToolbarHoveredIndex <= 2) {
    int idx = ctx.playlistToolbarHoveredIndex;
    if (ctx.isPlaylistListViewMode) {
      if (idx == 0)
        hoverText = L""; // (非表示)
      if (idx == 1)
        hoverText = L"プレイリストを新規作成する";
      if (idx == 2)
        hoverText = L"プレイリストを削除する";
    } else {
      if (idx == 0)
        hoverText = L"プレイリスト一覧を開く";
      if (idx == 1)
        hoverText = L"再生中の曲をプレイリストから削除する";
      if (idx == 2)
        hoverText = L"プレイリストの全曲を削除する";
    }
  }
  
  if (ctx.isPlaylistPinnedHovered) {
    hoverText = config->GetIsPlaylistPinned() ? L"表示モード: 画面固定" : L"表示モード: 自動格納";
  }

  std::wstring toolbarCenterText = hoverText;
  if (toolbarCenterText.empty() && !ctx.isPlaylistListViewMode) {
    std::wstring currentPlaylist = config->GetDefaultPlaylistPath();
    if (!currentPlaylist.empty()) {
      try {
        toolbarCenterText = std::filesystem::path(currentPlaylist).stem().wstring();
      } catch (...) {
        toolbarCenterText = L"Playlist";
      }
    }
  }
  outText = toolbarCenterText;
}

void PlaylistWidget::DrawPinButton(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config, const PlaylistLayout& layout) {
  if (ctx.isPlaylistPinnedHovered && m_playlistHighlightBrush) {
    m_playlistHighlightBrush->SetOpacity(0.2f);
    context->FillRectangle(&layout.toolbarLayout.pinButtonHitRect, m_playlistHighlightBrush.Get());
  }

  bool isPinned = config->GetIsPlaylistPinned();
  std::wstring pinBaseIcon = L"📌";
  std::wstring pinSubIcon = isPinned ? L"🔒" : L"🔓";

  if (m_toolbarIconFormat && m_textBrush) {
    m_textBrush->SetOpacity(isPinned ? 1.0f : 0.4f);
    context->DrawText(pinBaseIcon.c_str(), static_cast<UINT32>(pinBaseIcon.length()),
                      m_toolbarIconFormat.Get(), &layout.toolbarLayout.pinButtonHitRect,
                      m_textBrush.Get());
    m_textBrush->SetOpacity(1.0f);
  }

  if (m_pinSubIconFormat && m_textBrush && m_shadowBrush) {
    D2D1_RECT_F subRect = layout.toolbarLayout.pinButtonHitRect;
    subRect.left += config->GetPinSubIconOffsetX();
    subRect.right += config->GetPinSubIconOffsetX();
    subRect.top += config->GetPinSubIconOffsetY();
    subRect.bottom += config->GetPinSubIconOffsetY();

    m_shadowBrush->SetOpacity(1.0f);
    D2D1_RECT_F outlineRects[4] = {
        {subRect.left - 1, subRect.top, subRect.right - 1, subRect.bottom},
        {subRect.left + 1, subRect.top, subRect.right + 1, subRect.bottom},
        {subRect.left, subRect.top - 1, subRect.right, subRect.bottom - 1},
        {subRect.left, subRect.top + 1, subRect.right, subRect.bottom + 1}};
    for (int i = 0; i < 4; ++i) {
      context->DrawText(pinSubIcon.c_str(), static_cast<UINT32>(pinSubIcon.length()),
                        m_pinSubIconFormat.Get(), &outlineRects[i], m_shadowBrush.Get());
    }
    context->DrawText(pinSubIcon.c_str(), static_cast<UINT32>(pinSubIcon.length()),
                      m_pinSubIconFormat.Get(), &subRect, m_textBrush.Get());
  }
}

void PlaylistWidget::DrawToolbar(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config, const PlaylistLayout& layout) {
  std::wstring icons[3];
  if (ctx.isPlaylistListViewMode) {
    icons[0] = L""; // (非表示)
    icons[1] = L"➕";
    icons[2] = L"🗑️";
  } else {
    icons[0] = L"📁";
    icons[1] = L"➖";
    icons[2] = L"🗑️";
  }

  std::wstring toolbarCenterText;
  BuildToolbarText(ctx, config, toolbarCenterText);

  if (!toolbarCenterText.empty() && m_toolbarTextFormat && m_textBrush) {
    if (m_shadowBrush && config->GetEnableShadow()) {
      m_shadowBrush->SetOpacity(config->GetShadowOpacity());
      D2D1_RECT_F sRect = layout.toolbarLayout.textRect;
      sRect.left += 1.0f;
      sRect.top += 1.0f;
      sRect.right += 1.0f;
      sRect.bottom += 1.0f;
      context->DrawText(
          toolbarCenterText.c_str(), static_cast<UINT32>(toolbarCenterText.length()),
          m_toolbarTextFormat.Get(), &sRect, m_shadowBrush.Get());
    }
    context->DrawText(toolbarCenterText.c_str(),
                      static_cast<UINT32>(toolbarCenterText.length()),
                      m_toolbarTextFormat.Get(),
                      &layout.toolbarLayout.textRect, m_textBrush.Get());
  }

  for (int i = 0; i < 3; ++i) {
    if (ctx.isPlaylistListViewMode && i == 0)
      continue; // (非表示)

    if (ctx.playlistToolbarHoveredIndex == i && m_playlistHighlightBrush) {
      m_playlistHighlightBrush->SetOpacity(0.2f);
      context->FillRectangle(&layout.toolbarLayout.buttonHitRects[i],
                             m_playlistHighlightBrush.Get());
    }

    if (m_toolbarIconFormat && !icons[i].empty()) {
      context->DrawText(
          icons[i].c_str(), static_cast<UINT32>(icons[i].length()),
          m_toolbarIconFormat.Get(), &layout.toolbarLayout.buttonHitRects[i],
          m_textBrush.Get());
    }
  }

  DrawPinButton(context, ctx, config, layout);
}
