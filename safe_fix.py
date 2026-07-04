import io

def fix_cpp():
    with io.open('src/Renderer.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    new_lines = []
    skip = False
    
    for line in lines:
        # Load app logo
        if 'LoadBitmapResource(L"app_logo.png"' in line: continue
        if 'LoadBitmapResource(L"app_logo_hover.png"' in line: continue
        
        # Start of huge skip block 1: CreateTextFormat -> before m_bgDarkenBrush
        if 'hr = m_dwriteFactory->CreateTextFormat(' in line:
            skip = True
        if skip and 'hr = m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &m_bgDarkenBrush);' in line:
            skip = False
            # Don't continue, we want to KEEP this line
        
        # Start of huge skip block 2: m_seekBarBgBrush -> end of m_resizeGripGeometry
        if not skip and 'hr = m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_seekBarBgBrush);' in line:
            skip = True
        if skip and 'm_visualizer.SetConfig(m_config);' in line:
            skip = False
            # We want to KEEP this line

        # Skip blocks in UpdateAnimation
        if 'm_controlAlpha +=' in line or 'm_controlAlpha -=' in line or 'if (m_controlAlpha' in line:
            # wait, UpdateAnimation itself was using m_playlistSlideX!
            pass
            
        if not skip and 'if (m_config) {' in line:
            # Let's check if it's inside UpdateAnimation
            # UpdateAnimation starts at around line 550
            pass

        # We need to skip DrawTrackInfo, DrawNextTrack, DrawSeekBar, DrawPlaybackControls, DrawVolumeControl, DrawPlaylist, DrawResizeGrip
        if line.startswith('void Renderer::DrawAppLogo(') or \
           line.startswith('void Renderer::DrawTrackInfo(') or \
           line.startswith('void Renderer::DrawNextTrack(') or \
           line.startswith('void Renderer::DrawSeekBar(') or \
           line.startswith('void Renderer::DrawPlaybackControls(') or \
           line.startswith('void Renderer::DrawVolumeControl(') or \
           line.startswith('void Renderer::DrawPlaylist(') or \
           line.startswith('void Renderer::DrawResizeGrip('):
            skip = True
            
        if skip and line.startswith('}'):
            # This will close the method skip!
            # Wait, methods might have nested braces!
            # We know none of these methods have `}` at the start of the line except the final closing brace!
            # C++ formatting puts the final brace at the start of the line.
            # So `line.startswith('}')` is perfectly safe for method ends!
            skip = False
            continue

        # Skip AddPlaylistScroll and GetPlaylistManualScrollY
        if line.startswith('void Renderer::AddPlaylistScroll(') or \
           line.startswith('float Renderer::GetPlaylistManualScrollY('):
            skip = True

        # Skip TextLayout updates in UpdateTextLayouts
        if 'bool rebuildTime =' in line or 'bool rebuildVolume =' in line or 'bool rebuildTrackCount =' in line or 'bool rebuildPlaylistTime =' in line:
            continue
        if 'm_lastTimeString =' in line or 'm_lastVolume =' in line or 'm_lastCurrentTrackIndex =' in line or 'm_lastTotalTracks =' in line or 'm_forceTextLayoutUpdate =' in line:
            continue
        if 'if (rebuildTime) {' in line or 'if (rebuildVolume) {' in line or 'if (rebuildTrackCount) {' in line or 'if (rebuildPlaylistTime) {' in line:
            skip = True
            
        if not skip and 'DrawAppLogo(isHovered);' in line: continue
        if not skip and 'DrawTrackInfo();' in line: continue
        if not skip and 'DrawNextTrack();' in line: continue
        if not skip and 'DrawSeekBar(' in line: continue
        if not skip and 'DrawPlaybackControls(' in line: continue
        if not skip and 'DrawVolumeControl(' in line: continue
        if not skip and 'DrawPlaylist(' in line: continue
        if not skip and 'DrawResizeGrip();' in line: continue

        if not skip:
            new_lines.append(line)

    # Let's fix UpdateAnimation's playlist slide logic.
    # UpdateAnimation has:
    #     if (m_config) {
    #         float configPlaylistWidth = static_cast<float>(m_config->GetPlaylistWidth());
    #         ...
    #         }
    #     }
    # It's better to just write a clean UpdateAnimation method.
    final_lines = []
    in_update_anim = False
    for line in new_lines:
        if line.startswith('void Renderer::UpdateAnimation('):
            in_update_anim = True
            final_lines.append(line)
            final_lines.append('    if (isControlHovered) {\n')
            final_lines.append('        m_controlAlpha += 0.05f;\n')
            final_lines.append('        if (m_controlAlpha > 1.0f) m_controlAlpha = 1.0f;\n')
            final_lines.append('    } else {\n')
            final_lines.append('        m_controlAlpha -= 0.05f;\n')
            final_lines.append('        if (m_controlAlpha < 0.0f) m_controlAlpha = 0.0f;\n')
            final_lines.append('    }\n')
            final_lines.append('}\n\n')
            continue
            
        if in_update_anim:
            if line.startswith('}'):
                in_update_anim = False
            continue
            
        final_lines.append(line)

    with io.open('src/Renderer.cpp', 'w', encoding='utf-8') as f:
        f.writelines(final_lines)

fix_cpp()
print('Done cpp fix')
