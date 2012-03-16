// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_CLIENT_ACTIVATION_CLIENT_H_
#define UI_AURA_CLIENT_ACTIVATION_CLIENT_H_
#pragma once

#include "ui/aura/aura_export.h"
#include "ui/aura/window.h"

namespace aura {

class Event;
class RootWindow;

namespace client {

// An interface implemented by an object that manages window activation.
class AURA_EXPORT ActivationClient {
 public:
  // Activates |window|. If |window| is NULL, nothing happens.
  virtual void ActivateWindow(Window* window) = 0;

  // Deactivates |window|. What (if anything) is activated next is up to the
  // client. If |window| is NULL, nothing happens.
  virtual void DeactivateWindow(Window* window) = 0;

  // Retrieves the active window, or NULL if there is none.
  virtual aura::Window* GetActiveWindow() = 0;

  // Invoked prior to |window| getting focus as a result of the |event|. |event|
  // may be NULL. Returning false blocks |window| from getting focus.
  virtual bool OnWillFocusWindow(Window* window, const Event* event) = 0;

 protected:
  virtual ~ActivationClient() {}
};

// Sets/Gets the activation client on the RootWindow.
AURA_EXPORT void SetActivationClient(RootWindow* root_window,
                                     ActivationClient* client);
AURA_EXPORT ActivationClient* GetActivationClient(RootWindow* root_window);

// A property key to store what the client defines as the active window on the
// RootWindow.
AURA_EXPORT extern const WindowProperty<Window*>* const
    kRootWindowActiveWindowKey;

}  // namespace clients
}  // namespace aura

#endif  // UI_AURA_CLIENT_ACTIVATION_CLIENT_H_
