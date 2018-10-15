// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <cstdlib>
#include <string>
#include <glad/glad.h>
#include "citra_android/jni/button_manager.h"
#include "citra_android/jni/emu_window/emu_window.h"
#include "citra_android/jni/ndk_helper/GLContext.h"
#include "common/logging/log.h"
#include "common/scm_rev.h"
#include "common/string_util.h"
#include "core/3ds.h"
#include "core/settings.h"
#include "input_common/keyboard.h"
#include "input_common/main.h"
#include "input_common/motion_emu.h"
#include "network/network.h"

void EmuWindow_Android::OnSurfaceChanged(ANativeWindow* surface) {
    render_window = surface;
}

void EmuWindow_Android::OnTouchEvent(int x, int y, bool pressed) {
    if (pressed) {
        TouchPressed((unsigned)std::max(x, 0), (unsigned)std::max(y, 0));
    } else {
        TouchReleased();
    }
}

void EmuWindow_Android::OnTouchMoved(int x, int y) {
    TouchMoved((unsigned)std::max(x, 0), (unsigned)std::max(y, 0));
}

void EmuWindow_Android::OnFramebufferSizeChanged() {
    int width, height;
    width = gl_context->GetScreenWidth();
    height = gl_context->GetScreenHeight();
    UpdateCurrentFramebufferLayout(width, height);
}

EmuWindow_Android::EmuWindow_Android(ANativeWindow* surface) {
    LOG_DEBUG(Frontend, "Initializing Emuwindow");

    Network::Init();
    gl_context = ndk_helper::GLContext::GetInstance();
    render_window = surface;

    LOG_INFO(Frontend, "InitDisplay");
    gl_context->Init(render_window);

    if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(eglGetProcAddress))) {
        LOG_CRITICAL(Frontend, "Failed to initialize GL functions: %d", eglGetError());
    }

    OnFramebufferSizeChanged();
    DoneCurrent();
}

EmuWindow_Android::~EmuWindow_Android() {
    gl_context->Invalidate();
    Network::Shutdown();
}

void EmuWindow_Android::SwapBuffers() {

    if (EGL_SUCCESS != gl_context->Swap())
        LOG_ERROR(Frontend, "Swap failed");
}

void EmuWindow_Android::PollEvents() {
    if (render_window != gl_context->GetANativeWindow()) {
        MakeCurrent();
        OnFramebufferSizeChanged();
    }
}

void EmuWindow_Android::MakeCurrent() {
    gl_context->Resume(render_window);
}

void EmuWindow_Android::DoneCurrent() {
    gl_context->Suspend();
}
