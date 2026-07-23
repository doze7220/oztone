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
        hoverText = L""; // (グレーアウト時は非表示)
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
    hoverText = config->GetLayoutPlaylist().IsPlaylistPinned ? L"表示モード: 画面固定" : L"表示モード: 自動格納";
  }

  std::wstring toolbarCenterText = hoverText;
  if (toolbarCenterText.empty() && !ctx.isPlaylistListViewMode) {
    std::wstring currentPlaylist = config->GetPlaylist().DefaultPlaylistPath;
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

  bool isPinned = config->GetLayoutPlaylist().IsPlaylistPinned;
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
    subRect.left += config->GetLayoutPlaylist().PinSubIconOffsetX;
    subRect.right += config->GetLayoutPlaylist().PinSubIconOffsetX;
    subRect.top += config->GetLayoutPlaylist().PinSubIconOffsetY;
    subRect.bottom += config->GetLayoutPlaylist().PinSubIconOffsetY;

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
    icons[0] = L"📁"; // グレーアウト表示
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
    D2D1_RECT_F textRect = layout.toolbarLayout.textRect;
    textRect.left += 2.0f;

    if (m_shadowBrush && config->GetUICommonParm().EnableShadow) {
      m_shadowBrush->SetOpacity(config->GetUICommonParm().ShadowOpacity);
      D2D1_RECT_F sRect = textRect;
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
                      &textRect, m_textBrush.Get());
  }

  if (!ctx.isPlaylistListViewMode && m_toolbarCountTextFormat && m_textBrush) {
    if (ctx.playlistToolbarHoveredIndex == -1) {
      std::wstring countText = std::to_wstring(ctx.totalTracks) + L" Tracks";
      if (m_shadowBrush && config->GetUICommonParm().EnableShadow) {
        m_shadowBrush->SetOpacity(config->GetUICommonParm().ShadowOpacity);
        D2D1_RECT_F sRect = layout.toolbarLayout.textRect;
        sRect.left += 1.0f;
        sRect.top += 1.0f;
        sRect.right += 1.0f;
        sRect.bottom += 1.0f;
        context->DrawText(
            countText.c_str(), static_cast<UINT32>(countText.length()),
            m_toolbarCountTextFormat.Get(), &sRect, m_shadowBrush.Get());
      }
      context->DrawText(countText.c_str(), static_cast<UINT32>(countText.length()),
                        m_toolbarCountTextFormat.Get(), &layout.toolbarLayout.textRect, m_textBrush.Get());
    }
  }

  for (int i = 0; i < 3; ++i) {
    bool isDisabled = (ctx.isPlaylistListViewMode && i == 0);

    if (ctx.playlistToolbarHoveredIndex == i && m_playlistHighlightBrush && !isDisabled) {
      m_playlistHighlightBrush->SetOpacity(0.2f);
      context->FillRectangle(&layout.toolbarLayout.buttonHitRects[i],
                             m_playlistHighlightBrush.Get());
    }

    if (m_toolbarIconFormat && !icons[i].empty()) {
      float originalOpacity = 1.0f;
      if (m_textBrush) {
        originalOpacity = m_textBrush->GetOpacity();
        if (isDisabled) {
          m_textBrush->SetOpacity(0.3f);
        }
      }

      context->DrawText(
          icons[i].c_str(), static_cast<UINT32>(icons[i].length()),
          m_toolbarIconFormat.Get(), &layout.toolbarLayout.buttonHitRects[i],
          m_textBrush.Get());

      if (m_textBrush && isDisabled) {
        m_textBrush->SetOpacity(originalOpacity);
      }
    }
  }

  DrawPinButton(context, ctx, config, layout);
}
