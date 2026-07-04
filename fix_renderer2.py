import io
import re

def fix_h():
    with io.open('src/Renderer.h', 'r', encoding='utf-8') as f:
        content = f.read()
        
    vars_to_remove = [
        'm_titleTextFormat', 'm_artistTextFormat',
        'm_nextLabelTextFormat', 'm_nextTitleTextFormat', 'm_nextArtistTextFormat',
        'm_textBrush', 'm_shadowBrush', 'm_shadowEffect',
        'm_appLogoBitmap', 'm_appLogoHoverBitmap',
        'DrawSeekBar', 'DrawPlaybackControls', 'DrawVolumeControl',
        
        # Task 4 & 3 leftovers
        'm_timeTextFormat', 'm_volumeTextFormat', 'm_trackCountTextFormat',
        'm_playlistTitleTextFormat', 'm_playlistArtistTextFormat', 'm_playlistTimeTextFormat',
        'm_playlistArtistBrush', 'm_playlistTimeBrush', 'm_playlistGripLineBrush', 'm_playlistGripArrowBrush',
        'm_playlistGripArrowGeometry', 'm_seekBarBgBrush', 'm_seekBarFgBrush', 'm_controlBrush',
        'm_playlistBgBrush', 'm_playlistHighlightBrush', 'm_resizeGripBrush',
        'm_playIconGeometry', 'm_prevIconGeometry', 'm_speakerIconGeometry', 'm_resizeGripGeometry',
        
        'DrawPlaylist', 'DrawResizeGrip', 'DrawTrackInfo', 'DrawNextTrack',
        'm_playlistSlideX', 'm_playlistManualScrollY', 'm_playlistTargetScrollY', 'm_playlistScrollVelocity',
        'm_trackCountTextLayout', 'm_playlistTimeTextLayout', 'm_volTextLayout'
    ]
    
    new_lines = []
    for line in content.splitlines():
        remove = False
        for v in vars_to_remove:
            if v in line:
                remove = True
                break
        if not remove:
            new_lines.append(line)
            
    with io.open('src/Renderer.h', 'w', encoding='utf-8') as f:
        f.write('\n'.join(new_lines) + '\n')

def fix_cpp():
    with io.open('src/Renderer.cpp', 'r', encoding='utf-8') as f:
        content = f.read()

    # Remove app_logo loads
    content = re.sub(r'^\s*if \(\!LoadBitmapResource\(L"app_logo\.png".*?\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*if \(\!LoadBitmapResource\(L"app_logo_hover\.png".*?\n', '', content, flags=re.MULTILINE)

    # Remove block creating TextFormat
    content = re.sub(r'^\s*hr = m_dwriteFactory->CreateTextFormat\([\s\S]*?\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)

    # Remove Trimming block
    content = re.sub(r'^\s*// 7\.5.*?\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*auto ApplyTrimming = \[\&\][\s\S]*?\}\;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*ApplyTrimming\(.*?\);\s*\n', '', content, flags=re.MULTILINE)
    
    # Remove specific SetTextAlignment
    content = re.sub(r'^\s*hr = m_.*?->SetTextAlignment.*?\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_.*?->SetParagraphAlignment.*?\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)

    # Remove CreateSolidColorBrush blocks
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_playlistArtistBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_playlistTimeBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([\s\S]*?\&m_textBrush\s*\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([\s\S]*?\&m_shadowBrush\s*\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateEffect\(CLSID_D2D1Shadow, \&m_shadowEffect\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    
    content = re.sub(r'^\s*D2D1_COLOR_F gripLineColor.*?\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_playlistGripLineBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*D2D1_COLOR_F gripArrowColor.*?\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_playlistGripArrowBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_seekBarBgBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_seekBarFgBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_controlBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_playlistBgBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_playlistHighlightBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dContext->CreateSolidColorBrush\([^,]+,\s*\&m_resizeGripBrush\);\s*\n\s*if \(FAILED\(hr\)\) return false;\s*\n', '', content, flags=re.MULTILINE)

    # Remove ParseHexColor lambda since it's unused
    content = re.sub(r'^\s*// HEXから D2D1_COLOR_F への変換ラムダ\s*\n\s*auto ParseHexColor = \[\][\s\S]*?return D2D1::ColorF\(D2D1::ColorF::White\);\s*\n\s*\};\s*\n', '', content, flags=re.MULTILINE)

    # Remove Geometry creations
    content = re.sub(r'^\s*hr = m_d2dFactory->CreatePathGeometry\(&m_playlistGripArrowGeometry\);[\s\S]*?\}\s*\n\s*\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dFactory->CreatePathGeometry\(&m_playIconGeometry\);[\s\S]*?\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dFactory->CreatePathGeometry\(&m_prevIconGeometry\);[\s\S]*?\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dFactory->CreatePathGeometry\(&m_speakerIconGeometry\);[\s\S]*?\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dFactory->CreatePathGeometry\(&m_resizeGripGeometry\);[\s\S]*?\}\s*\n\s*\n\s*\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*hr = m_d2dFactory->CreatePathGeometry\(&m_resizeGripGeometry\);[\s\S]*?\}\s*\n\s*\}\s*\n', '', content, flags=re.MULTILINE) # Handle potential slight variation

    # Remove methods at the end (DrawPlaylist, DrawResizeGrip, DrawSeekBar etc)
    content = re.sub(r'^\s*void Renderer::DrawTrackInfo\([\s\S]*?^\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*void Renderer::DrawNextTrack\(\)[\s\S]*?^\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*void Renderer::DrawPlaylist\([\s\S]*?^\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*void Renderer::DrawResizeGrip\(\)[\s\S]*?^\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*void Renderer::DrawSeekBar\([\s\S]*?^\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*void Renderer::DrawPlaybackControls\([\s\S]*?^\}\s*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*void Renderer::DrawVolumeControl\([\s\S]*?^\}\s*\n', '', content, flags=re.MULTILINE)

    with io.open('src/Renderer.cpp', 'w', encoding='utf-8') as f:
        f.write(content)

fix_h()
fix_cpp()
print('Done')
