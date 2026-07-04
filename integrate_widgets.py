import io

def integrate():
    with io.open('src/Renderer.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()
        
    out = []
    skip = False
    
    for line in lines:
        if 'LoadBitmapResource(L"app_logo.png"' in line: continue
        if 'LoadBitmapResource(L"app_logo_hover.png"' in line: continue
        
        # Dead code blocks in Initialize
        if 'hr = m_dwriteFactory->CreateTextFormat(' in line:
            skip = True
        if skip and 'hr = m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &m_bgDarkenBrush);' in line:
            skip = False
            
        if not skip and 'hr = m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_seekBarBgBrush);' in line:
            skip = True
            
        # Stop skipping and inject widget creation
        if skip and 'm_visualizer.SetConfig(m_config);' in line:
            skip = False
            out.append(line)
            out.append('    m_widgets.clear();\n')
            out.append('    m_widgets.push_back(std::make_unique<AppLogoWidget>());\n')
            out.append('    m_widgets.push_back(std::make_unique<TrackInfoWidget>());\n')
            out.append('    m_widgets.push_back(std::make_unique<NextTrackWidget>());\n')
            out.append('    m_widgets.push_back(std::make_unique<SeekBarWidget>());\n')
            out.append('    m_widgets.push_back(std::make_unique<PlaybackControlsWidget>());\n')
            out.append('    m_widgets.push_back(std::make_unique<VolumeControlWidget>());\n')
            out.append('    m_widgets.push_back(std::make_unique<PlaylistWidget>());\n')
            out.append('    m_widgets.push_back(std::make_unique<ResizeGripWidget>());\n')
            out.append('    for (auto& widget : m_widgets) {\n')
            out.append('        widget->CreateResources(m_d2dContext.Get(), m_wicFactory.Get(), m_dwriteFactory.Get(), m_config);\n')
            out.append('    }\n')
            continue

        if not skip and line.startswith('void Renderer::AddPlaylistScroll('):
            skip = True
            
        # We also need to skip `float Renderer::GetPlaylistManualScrollY` until its end `}`
        if skip and line.startswith('float Renderer::GetPlaylistManualScrollY('):
            pass # just keep skipping
            
        if skip and line.startswith('void Renderer::UpdateAnimation('):
            skip = False
            
        # Skip original Draw methods
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
            # Close skip block. BUT we don't know if this is the end of DrawResizeGrip or something else.
            # It's safe if we only do this for the specific Draw methods.
            # Let's be careful. The python script will just use a naive approach.
            pass

        if not skip:
            out.append(line)

    # Now we need to manually rewrite UpdateAnimation, UpdateTextLayouts, and Render.
    # It's easier to just find their start lines and replace until `}`
    final_out = []
    in_update_anim = False
    in_update_text = False
    in_render = False
    
    # We will also skip AddPlaylistScroll and GetPlaylistManualScrollY by identifying them.
    in_scroll = False
    
    for i, line in enumerate(out):
        if line.startswith('void Renderer::AddPlaylistScroll(') or line.startswith('float Renderer::GetPlaylistManualScrollY('):
            in_scroll = True
            continue
        if in_scroll and line.startswith('}'):
            in_scroll = False
            continue
        if in_scroll:
            continue
            
        if line.startswith('void Renderer::UpdateAnimation('):
            in_update_anim = True
            final_out.append(line)
            final_out.append('''    if (isControlHovered) {
        m_controlAlpha += 0.05f;
        if (m_controlAlpha > 1.0f) m_controlAlpha = 1.0f;
    } else {
        m_controlAlpha -= 0.05f;
        if (m_controlAlpha < 0.0f) m_controlAlpha = 0.0f;
    }

    WidgetContext ctx = {};
    ctx.deltaTime = deltaTime;
    ctx.isControlHovered = isControlHovered;
    ctx.isPlaylistHovered = isPlaylistHovered;
    ctx.currentTrackIndex = currentTrackIndex;
    ctx.totalTracks = totalTracks;
    
    for (auto& widget : m_widgets) {
        widget->UpdateAnimation(ctx);
    }
}
''')
            continue
        if in_update_anim:
            if line.startswith('}'):
                in_update_anim = False
            continue
            
        if line.startswith('void Renderer::UpdateTextLayouts('):
            in_update_text = True
            final_out.append(line)
            final_out.append('''    m_lastTimeString = timeString;
    m_lastVolume = volume;
    m_lastCurrentTrackIndex = currentTrackIndex;
    m_lastTotalTracks = totalTracks;
    
    WidgetContext ctx = {};
    ctx.timeString = timeString;
    ctx.volume = volume;
    ctx.currentTrackIndex = currentTrackIndex;
    ctx.totalTracks = totalTracks;
    
    for (auto& widget : m_widgets) {
        widget->UpdateLayout(ctx, m_config);
    }
}
''')
            continue
        if in_update_text:
            if line.startswith('}'):
                in_update_text = False
            continue
            
        if line.startswith('void Renderer::Render('):
            in_render = True
            final_out.append(line)
            final_out.append('''    if (!m_d2dContext) return;

    m_d2dContext->BeginDraw();
    m_d2dContext->SetTransform(D2D1::Matrix3x2F::Scale(m_dpiScale, m_dpiScale));
    m_d2dContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
    
    DrawBackground();
    DrawVisualizer(spectrum);

    WidgetContext ctx = {};
    ctx.isHovered = isHovered;
    ctx.isControlHovered = isControlHovered;
    ctx.isPlaylistHovered = isPlaylistHovered;
    ctx.isPlaying = isPlaying;
    ctx.progress = progress;
    ctx.spectrum = &spectrum;
    ctx.volume = volume;
    ctx.currentTrackIndex = currentTrackIndex;
    ctx.totalTracks = totalTracks;
    ctx.shuffleList = &shuffleList;
    ctx.dpiScale = m_dpiScale;
    ctx.controlAlpha = m_controlAlpha;
    ctx.timeString = m_lastTimeString;
    ctx.trackTitle = m_trackTitle;
    ctx.trackArtist = m_trackArtist;
    ctx.currentArtBitmap = m_currentArtBitmap.Get();
    ctx.nextIsReady = m_nextIsReady;
    ctx.nextTrackTitle = m_nextTrackTitle;
    ctx.nextTrackArtist = m_nextTrackArtist;
    ctx.nextArtBitmap = m_nextArtBitmap.Get();

    for (auto& widget : m_widgets) {
        widget->Draw(m_d2dContext.Get(), ctx, m_config);
    }

    HRESULT hr = m_d2dContext->EndDraw();
    if (SUCCEEDED(hr) || hr == D2DERR_RECREATE_TARGET) {
        m_swapChain->Present(1, 0);
    }
}
''')
            continue
        if in_render:
            if line.startswith('}'):
                in_render = False
            continue

        # For Draw methods, we just filter them out completely
        if line.startswith('void Renderer::DrawAppLogo(') or \
           line.startswith('void Renderer::DrawTrackInfo(') or \
           line.startswith('void Renderer::DrawNextTrack(') or \
           line.startswith('void Renderer::DrawSeekBar(') or \
           line.startswith('void Renderer::DrawPlaybackControls(') or \
           line.startswith('void Renderer::DrawVolumeControl(') or \
           line.startswith('void Renderer::DrawPlaylist(') or \
           line.startswith('void Renderer::DrawResizeGrip('):
            # this shouldn't happen because we filtered them in the first pass, but just in case
            pass

        final_out.append(line)

    # Let's do a final pass to remove any leftover Draw functions if they were missed by the first pass
    final_pass = []
    skip_draw = False
    for line in final_out:
        if line.startswith('void Renderer::DrawAppLogo(') or \
           line.startswith('void Renderer::DrawTrackInfo(') or \
           line.startswith('void Renderer::DrawNextTrack(') or \
           line.startswith('void Renderer::DrawSeekBar(') or \
           line.startswith('void Renderer::DrawPlaybackControls(') or \
           line.startswith('void Renderer::DrawVolumeControl(') or \
           line.startswith('void Renderer::DrawPlaylist(') or \
           line.startswith('void Renderer::DrawResizeGrip('):
            skip_draw = True
            continue
        if skip_draw and line.startswith('}'):
            skip_draw = False
            continue
        if skip_draw:
            continue
            
        final_pass.append(line)

    with io.open('src/Renderer.cpp', 'w', encoding='utf-8') as f:
        f.writelines(final_pass)

integrate()
print('Integration complete')
