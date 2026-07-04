# Phase 7-3: Circle Spectral Analyzer Implementation

This plan outlines the addition of the new "CircleParticle" visualizer mode and the architecture to switch between visualizers dynamically from the system tray.

## User Review Required
> [!IMPORTANT]  
> The new visualizer needs the track title and artist to calculate its unique color hash. I will change `Visualizer::Draw` to receive `title` and `artist` strings as parameters from `Renderer::Render`. 
> Also, to keep DPI scaling safe during circular transformations, I will save and restore the `ID2D1DeviceContext` transform matrix during rendering.
> Please review the approach.

## Proposed Changes

### Configuration
#### [MODIFY] [ConfigManager.h](file:///D:/ozlab/oztone/src/ConfigManager.h)
- Add `int m_visualizerMode;` (0: PrismBeat, 1: CircleParticle)
- Add `int GetVisualizerMode() const;`
- Add `void SetVisualizerMode(int mode);`

#### [MODIFY] [ConfigManager.cpp](file:///D:/ozlab/oztone/src/ConfigManager.cpp)
- Load/Save `VisualizerMode` in the `[Visualizer]` section of the INI file. Default is `0`.

---

### Window & System Tray
#### [MODIFY] [resource.h](file:///D:/ozlab/oztone/src/resource.h)
- Define new menu IDs: `ID_TRAY_VIS_PRISM` and `ID_TRAY_VIS_CIRCLE`.

#### [MODIFY] [Window.cpp](file:///D:/ozlab/oztone/src/Window.cpp)
- In `WindowProc` under `WM_TRAYICON`, add a submenu (or menu items) for "Visualizer" -> "PrismBeat" and "CircleParticle".
- Apply `MF_CHECKED` to the currently selected mode by querying `m_config->GetVisualizerMode()`.
- In `WM_COMMAND`, handle the new IDs, call `SetVisualizerMode`, and let the next frame render it automatically.

---

### Visualizer Engine
#### [MODIFY] [Visualizer.h](file:///D:/ozlab/oztone/src/Visualizer.h)
- Include `ConfigManager.h`.
- Add `const ConfigManager* m_config;` and `void SetConfig(const ConfigManager* config);`.
- Update `Draw` signature: `void Draw(ID2D1DeviceContext* context, const std::vector<float>& spectrum, D2D1_RECT_F drawRect, const std::wstring& trackTitle, const std::wstring& trackArtist);`
- Extract existing logic into `void DrawPrismBeat(...)`.
- Add new `void DrawCircleParticle(...)`.
- Define `struct Particle` for 3D spinning particles, and add `std::vector<Particle> m_particles;` and `std::vector<float> m_circleAmplitudes;`.

#### [MODIFY] [Visualizer.cpp](file:///D:/ozlab/oztone/src/Visualizer.cpp)
- Implement `DrawCircleParticle`:
  - Calculate color from `std::hash` of `title + artist`. Convert Hue to RGB.
  - Draw circular bars using polar coordinates (rotated via context transform).
  - Track amplitude drops to emit particles.
  - Update and draw particles: Use Rodrigues' rotation formula around a random 3D axis per particle. Project vertices to 2D and draw using `ID2D1PathGeometry` or `FillGeometry`.
  - Draw slow-rotating radial light rays in the background.

#### [MODIFY] [Renderer.cpp](file:///D:/ozlab/oztone/src/Renderer.cpp)
- In `Renderer::Initialize`, call `m_visualizer.SetConfig(m_config);`.
- In `Renderer::Render`, pass `m_trackTitle` and `m_trackArtist` to `m_visualizer.Draw(...)`.

## Verification Plan

### Manual Verification
- Build the project and run OZtone.
- Right-click the system tray icon, select "Visualizer" -> "CircleParticle".
- Verify that a circular visualizer is drawn in the center, and its color changes depending on the song currently playing.
- Verify that particles are emitted and spin in 3D when the amplitude drops.
- Verify that radial light rays rotate in the background.
- Switch back to "PrismBeat" and verify the old visualizer still works correctly.
