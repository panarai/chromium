# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },

  'targets': [
    {
      'target_name': 'oak',
      'type': 'static_library',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',
        '../../build/temp_gyp/googleurl.gyp:googleurl',
        '../../skia/skia.gyp:skia',
        '../aura/aura.gyp:aura',
        '../gfx/compositor/compositor.gyp:compositor',
        '../ui.gyp:ui',
        '../ui.gyp:ui_resources',
        '../ui.gyp:ui_resources_standard',
        '../views/views.gyp:views',
      ],
      'defines': [
        'OAK_IMPLEMENTATION',
      ],
      'sources': [
        # All .cc, .h under oak, except unittests
        'oak.h',
        'oak_aura_window_display.cc',
        'oak_aura_window_display.h',
        'oak_export.h',
        'oak_pretty_print.cc',
        'oak_pretty_print.h',
        'oak_tree_model.cc',
        'oak_tree_model.h',
        'oak_window.cc',
        'oak_window.h',
        'oak_details_model.h',
      ],
    },
  ],
}
