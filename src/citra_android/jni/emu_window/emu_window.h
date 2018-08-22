// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <utility>
#include "citra_android/jni/ndk_helper/GLContext.h"
#include "core/frontend/emu_window.h"

class EmuWindow_Android : public EmuWindow {
public:
    EmuWindow_Android(ANativeWindow* surface);
    ~EmuWindow_Android();

    /// Swap buffers to display the next frame
    void SwapBuffers() override;

    /// Polls window events
    void PollEvents() override;

    /// Makes the graphics context current for the caller thread
    void MakeCurrent() override;

    /// Releases the GL context from the caller thread
    void DoneCurrent() override;

    /// Called by the onSurfaceChanges() method to change the surface
    void OnSurfaceChanged(ANativeWindow* surface);

    /// Handles touch event that occur.(Touched or released)
    void OnTouchEvent(int x, int y, bool pressed);

    /// Handles movement of touch pointer
    void OnTouchMoved(int x, int y);

private:
    /// Called by PollEvents when a key is pressed or released.
    void OnKeyEvent(int key, u8 state);

    /// Called by PollEvents when the mouse moves.
    void OnMouseMotion(s32 x, s32 y);

    /// Called by PollEvents when a mouse button is pressed or released
    void OnMouseButton(u32 button, u8 state, s32 x, s32 y);

    /// Called when any event that may change the surface(Rotation)
    void OnFramebufferSizeChanged();

    /// Internal render window
    ANativeWindow* render_window;

    /// Initialise the EGL context associated with the window
    bool InitDisplay();

    /// A helper class for handling the EGL context
    ndk_helper::GLContext* gl_context;
};
