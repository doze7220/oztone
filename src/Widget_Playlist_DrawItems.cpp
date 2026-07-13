#include "Widget_Playlist.h"
#include "ConfigManager.h"
#include "LayoutCalculator.h"
#include <filesystem>

void PlaylistWidget::DrawPlaylistItems(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config, const PlaylistLayout& layout) {
  context->PushAxisAlignedClip(&layout.clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

  if (m_playlistHighlightBrush) {
    m_playlistHighlightBrush->SetOpacity(0.2f);
  }

  if (ctx.isPlaylistListViewMode) {
    DrawPlaylistList(context, ctx, config, layout);
  } else {
    DrawTrackList(context, ctx, config, layout);
  }

  context->PopAxisAlignedClip();
}

void PlaylistWidget::DrawPlaylistList(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config, const PlaylistLayout& layout) {
  float currentY = layout.startY;
  D2D1_COLOR_F originalTextColor = m_textBrush->GetColor();
  auto GetBlendedTextColor = [&](int index, bool isPlaying) {
      D2D1_COLOR_F baseColor = isPlaying ? ParseHexColor(config->GetPlayingItemColor()) : originalTextColor;
      float t = 0.0f;
      auto it = m_hoverAlpha.find(index);
      if (it != m_hoverAlpha.end()) t = it->second;

      if (t <= 0.0f) return baseColor;

      D2D1_COLOR_F hoverColor = ParseHexColor(config->GetHoverItemColor());
      return D2D1_COLOR_F{
          baseColor.r + (hoverColor.r - baseColor.r) * t,
          baseColor.g + (hoverColor.g - baseColor.g) * t,
          baseColor.b + (hoverColor.b - baseColor.b) * t,
          baseColor.a
      };
  };

  if (ctx.availablePlaylistsCache) {
    std::wstring currentPlaylist = config->GetDefaultPlaylistPath();

    for (size_t i = 0; i < ctx.availablePlaylistsCache->size(); ++i) {
      if (currentY + layout.itemHeight > 0 &&
          currentY < layout.playlistHeight) {
        PlaylistItemLayout itemLayout =
            LayoutCalculator::CalculatePlaylistItemLayout(layout, config,
                                                          currentY);

        bool isPlaying = ((*ctx.availablePlaylistsCache)[i].filepath == currentPlaylist);
        bool isFocused = ctx.focusedPlaylistIndex && *ctx.focusedPlaylistIndex == i;
        if ((isPlaying || isFocused) && m_playlistHighlightBrush) {
          float opacity = isPlaying ? 0.2f : 0.1f;
          m_playlistHighlightBrush->SetOpacity(opacity);
          context->FillRectangle(&itemLayout.hlRect,
                                 m_playlistHighlightBrush.Get());
        }

        std::wstring title = (*ctx.availablePlaylistsCache)[i].displayName;
        std::wstring info = std::to_wstring((*ctx.availablePlaylistsCache)[i].trackCount) + L" tracks";
        std::wstring timeStr = (*ctx.availablePlaylistsCache)[i].totalTimeString;

        m_textBrush->SetColor(GetBlendedTextColor(static_cast<int>(i), isPlaying));

        context->DrawText(title.c_str(), static_cast<UINT32>(title.length()),
                          m_playlistTitleTextFormat.Get(),
                          &itemLayout.titleRect, m_textBrush.Get());

        if (!info.empty()) {
          context->DrawText(
              info.c_str(), static_cast<UINT32>(info.length()),
              m_playlistArtistTextFormat.Get(), &itemLayout.artistRect,
              m_playlistArtistBrush ? m_playlistArtistBrush.Get()
                                    : m_textBrush.Get());
        }

        if (!timeStr.empty() && m_playlistTimeTextFormat) {
          D2D1_RECT_F timeRect =
              D2D1::RectF(itemLayout.timeOrigin.x, itemLayout.timeOrigin.y,
                          itemLayout.timeOrigin.x + itemLayout.timeMaxWidth,
                          itemLayout.timeOrigin.y + itemLayout.timeMaxHeight);
          context->DrawText(timeStr.c_str(),
                            static_cast<UINT32>(timeStr.length()),
                            m_playlistTimeTextFormat.Get(), &timeRect,
                            m_playlistTimeBrush ? m_playlistTimeBrush.Get()
                                                : m_textBrush.Get());
        }
      }
      currentY += layout.itemHeight;
    }
  }
  m_textBrush->SetColor(originalTextColor); // Restore
}

void PlaylistWidget::DrawTrackList(ID2D1DeviceContext* context, const WidgetContext& ctx, const ConfigManager* config, const PlaylistLayout& layout) {
  float currentY = layout.startY;
  D2D1_COLOR_F originalTextColor = m_textBrush->GetColor();
  auto GetBlendedTextColor = [&](int index, bool isPlaying) {
      D2D1_COLOR_F baseColor = isPlaying ? ParseHexColor(config->GetPlayingItemColor()) : originalTextColor;
      float t = 0.0f;
      auto it = m_hoverAlpha.find(index);
      if (it != m_hoverAlpha.end()) t = it->second;

      if (t <= 0.0f) return baseColor;

      D2D1_COLOR_F hoverColor = ParseHexColor(config->GetHoverItemColor());
      return D2D1_COLOR_F{
          baseColor.r + (hoverColor.r - baseColor.r) * t,
          baseColor.g + (hoverColor.g - baseColor.g) * t,
          baseColor.b + (hoverColor.b - baseColor.b) * t,
          baseColor.a
      };
  };

  for (size_t i = 0; i < ctx.totalTracks && ctx.shuffleMetadataList &&
                     i < ctx.shuffleMetadataList->size();
       ++i) {
    if (currentY + layout.itemHeight > 0 &&
        currentY < layout.playlistHeight) {
      PlaylistItemLayout itemLayout =
          LayoutCalculator::CalculatePlaylistItemLayout(layout, config,
                                                        currentY);

      bool isPlaying = (i == ctx.currentTrackIndex);
      bool isFocused = ctx.focusedPlaylistIndex && *ctx.focusedPlaylistIndex == i;
      if ((isPlaying || isFocused) && m_playlistHighlightBrush) {
        float opacity = isPlaying ? 0.2f : 0.1f;
        m_playlistHighlightBrush->SetOpacity(opacity);
        context->FillRectangle(&itemLayout.hlRect,
                               m_playlistHighlightBrush.Get());
      }

      const TrackMetadata &meta = (*ctx.shuffleMetadataList)[i];
      std::wstring title;
      std::wstring artist;
      std::wstring timeStr;

      if (meta.isMetaLoaded) {
        title = meta.title;
        artist = meta.artist;
        timeStr = meta.timeString;
      } else {
        try {
          title = std::filesystem::path(meta.filepath).filename().wstring();
        } catch (...) {
          title = L"Unknown";
        }
        artist = L"Unknown Artist";
      }

      m_textBrush->SetColor(GetBlendedTextColor(static_cast<int>(i), isPlaying && !m_isScanlineActive));

      context->DrawText(title.c_str(), static_cast<UINT32>(title.length()),
                        m_playlistTitleTextFormat.Get(),
                        &itemLayout.titleRect, m_textBrush.Get());

      if (!artist.empty()) {
        context->DrawText(
            artist.c_str(), static_cast<UINT32>(artist.length()),
            m_playlistArtistTextFormat.Get(), &itemLayout.artistRect,
            m_playlistArtistBrush ? m_playlistArtistBrush.Get()
                                  : m_textBrush.Get());
      }

      if (!timeStr.empty() && m_playlistTimeTextFormat) {
        D2D1_RECT_F timeRect =
            D2D1::RectF(itemLayout.timeOrigin.x, itemLayout.timeOrigin.y,
                        itemLayout.timeOrigin.x + itemLayout.timeMaxWidth,
                        itemLayout.timeOrigin.y + itemLayout.timeMaxHeight);
        context->DrawText(timeStr.c_str(),
                          static_cast<UINT32>(timeStr.length()),
                          m_playlistTimeTextFormat.Get(), &timeRect,
                          m_playlistTimeBrush ? m_playlistTimeBrush.Get()
                                              : m_textBrush.Get());
      }

      if (isPlaying && m_isScanlineActive && m_radarGradientBrush) {
          float startX = itemLayout.hlRect.left;
          float endX = itemLayout.hlRect.right;
          float tailLength = 150.0f * ctx.dpiScale;
          
          float currentX = startX + (endX - startX + tailLength) * m_scanlineProgress;
          float tailStart = currentX - tailLength;
          
          context->PushAxisAlignedClip(&itemLayout.hlRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
          
          m_radarGradientBrush->SetStartPoint(D2D1::Point2F(tailStart, itemLayout.hlRect.top));
          m_radarGradientBrush->SetEndPoint(D2D1::Point2F(currentX, itemLayout.hlRect.top));
          D2D1_RECT_F tailRect = D2D1::RectF(tailStart, itemLayout.hlRect.top, currentX, itemLayout.hlRect.bottom);
          context->FillRectangle(&tailRect, m_radarGradientBrush.Get());
          
          D2D1_RECT_F textClipRect = D2D1::RectF(startX, itemLayout.hlRect.top, currentX, itemLayout.hlRect.bottom);
          context->PushAxisAlignedClip(&textClipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
          
          m_textBrush->SetColor(ParseHexColor(config->GetPlayingItemColor()));
          context->DrawText(title.c_str(), static_cast<UINT32>(title.length()),
                            m_playlistTitleTextFormat.Get(), &itemLayout.titleRect, m_textBrush.Get());
          if (!artist.empty()) {
              context->DrawText(artist.c_str(), static_cast<UINT32>(artist.length()),
                                m_playlistArtistTextFormat.Get(), &itemLayout.artistRect, m_textBrush.Get());
          }
          if (!timeStr.empty() && m_playlistTimeTextFormat) {
              D2D1_RECT_F timeRect = D2D1::RectF(itemLayout.timeOrigin.x, itemLayout.timeOrigin.y,
                                                 itemLayout.timeOrigin.x + itemLayout.timeMaxWidth,
                                                 itemLayout.timeOrigin.y + itemLayout.timeMaxHeight);
              context->DrawText(timeStr.c_str(), static_cast<UINT32>(timeStr.length()),
                                m_playlistTimeTextFormat.Get(), &timeRect, m_textBrush.Get());
          }
          
          context->PopAxisAlignedClip(); // Pop text clip
          context->PopAxisAlignedClip(); // Pop hlRect clip
      }
    }
    currentY += layout.itemHeight;
  }
  m_textBrush->SetColor(originalTextColor); // Restore
}

