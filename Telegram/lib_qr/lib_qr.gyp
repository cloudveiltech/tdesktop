# This file is part of Desktop App Toolkit,
# a set of libraries for developing nice desktop applications.
#
# For license and copyright information please follow this link:
# https://github.com/desktop-app/legal/blob/master/LEGAL

{
  'includes': [
    '../gyp/helpers/common/common.gypi',
  ],
  'targets': [{
    'target_name': 'lib_qr',
    'includes': [
      '../gyp/helpers/common/library.gypi',
      '../gyp/helpers/modules/qt.gypi',
    ],
    'variables': {
      'src_loc': '.',
      'qr_loc': '<(third_party_loc)/QR',
      'qr_src': '<(qr_loc)/cpp',
    },
    'dependencies': [
      '<(submodules_loc)/lib_base/lib_base.gyp:lib_base',
    ],
    'export_dependent_settings': [
      '<(submodules_loc)/lib_base/lib_base.gyp:lib_base',
    ],
    'defines': [
    ],
    'include_dirs': [
      '<(src_loc)',
      '<(qr_src)',
    ],
    'direct_dependent_settings': {
      'include_dirs': [
        '<(src_loc)',
      ],
    },
    'sources': [
      '<(src_loc)/qr/qr_generate.cpp',
      '<(src_loc)/qr/qr_generate.h',

      '<(qr_src)/BitBuffer.cpp',
      '<(qr_src)/BitBuffer.hpp',
      '<(qr_src)/QrCode.cpp',
      '<(qr_src)/QrCode.hpp',
      '<(qr_src)/QrSegment.cpp',
      '<(qr_src)/QrSegment.hpp',
    ],
  }],
}
