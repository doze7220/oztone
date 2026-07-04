import io

def fix_h():
    with io.open('src/Renderer.h', 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    new_lines = []
    for line in lines:
        if 'm_titleTextFormat' in line or 'm_artistTextFormat' in line: continue
        if 'm_nextLabelTextFormat' in line or 'm_nextTitleTextFormat' in line or 'm_nextArtistTextFormat' in line: continue
        if 'm_textBrush' in line or 'm_shadowBrush' in line: continue
        if 'm_appLogoBitmap' in line or 'm_appLogoHoverBitmap' in line: continue
        if 'DrawSeekBar(' in line or 'DrawPlaybackControls(' in line or 'DrawVolumeControl(' in line: continue
        
        # Task 4 removals
        if 'DrawPlaylist(' in line or 'DrawResizeGrip(' in line: continue
        if 'm_trackCountTextFormat' in line or 'm_playlistTitleTextFormat' in line or 'm_playlistArtistTextFormat' in line: continue
        if 'm_playlistTimeTextLayout' in line or 'm_trackCountTextLayout' in line: continue
        if 'm_playlistBgBrush' in line or 'm_playlistHighlightBrush' in line or 'm_playlistArtistBrush' in line or 'm_playlistTimeBrush' in line: continue
        if 'm_playlistGripLineBrush' in line or 'm_playlistGripArrowBrush' in line: continue
        if 'm_playlistGripArrowGeometry' in line or 'm_resizeGripBrush' in line or 'm_resizeGripGeometry' in line: continue
        if 'm_playlistSlideX' in line or 'm_playlistManualScrollY' in line or 'm_playlistTargetScrollY' in line: continue
        if 'm_playlistScrollVelocity' in line: continue
        
        new_lines.append(line)
        
    with io.open('src/Renderer.h', 'w', encoding='utf-8') as f:
        f.writelines(new_lines)

def fix_cpp():
    with io.open('src/Renderer.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    new_lines = []
    skip = False
    in_format = False
    for line in lines:
        # Task 4 method removals
        if line.startswith('void Renderer::DrawPlaylist('):
            skip = True
        if line.startswith('void Renderer::DrawResizeGrip('):
            skip = True
            
        if skip and line.startswith('}'):
            skip = False
            continue
        if skip: continue

        # Task 5 variables init removals
        if 'LoadBitmapResource(L"app_logo' in line: continue
        
        if 'm_dwriteFactory->CreateTextFormat(' in line:
            in_format = True
            continue
            
        if in_format:
            if ');' in line:
                # Need to also skip the if (FAILED(hr)) return false; line
                pass
            if 'if (FAILED(hr)) return false;' in line:
                in_format = False
            continue
            
        if 'CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White)' in line:
            in_format = True
            continue
            
        if 'CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f)' in line and '&m_shadowBrush' in line:
            in_format = True
            continue

        # Trimming block skip
        if 'auto ApplyTrimming = [&]' in line or 'ApplyTrimming(m_titleTextFormat)' in line or 'ApplyTrimming(m_artistTextFormat)' in line or 'ApplyTrimming(m_nextLabelTextFormat)' in line or 'ApplyTrimming(m_nextTitleTextFormat)' in line or 'ApplyTrimming(m_nextArtistTextFormat)' in line:
            continue
        if 'DWRITE_TRIMMING trimmingOptions =' in line or 'Microsoft::WRL::ComPtr<IDWriteInlineObject> ellipsis;' in line or 'HRESULT hrTrim =' in line or 'if (SUCCEEDED(hrTrim)) {' in line or 'format->SetTrimming(&trimmingOptions, ellipsis.Get());' in line or 'format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);' in line or '};' in line and len(line) < 10:
            # this is a bit hacky but we just remove trimming block fully
            continue

        new_lines.append(line)

    with io.open('src/Renderer.cpp', 'w', encoding='utf-8') as f:
        f.writelines(new_lines)

fix_h()
fix_cpp()
print('Done')
