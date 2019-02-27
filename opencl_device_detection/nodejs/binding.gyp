{
  "targets": [{
    "target_name": "nhm2_detect_amd",
    "cflags": ["-Wall", "-fexceptions", "-O2"],
    # make sure to enable exceptions so it would build under Linux
    "cflags_cc": ["-Wall", "-std=c++11", "-fexceptions", "-O2"],
    "sources": [
      "nodejs_native_src/module.cpp",
      './AMDOpenCLDeviceDetection.cpp',
      './JSONHelpers.cpp',
    ],
    "include_dirs": [
      "<!(node -e \"require('nan')\")",
      './',
    ],
    "conditions": [
      ['OS=="win"', {
        'variables': {
          'amd_app_sdk_root%': '$(AMDAPPSDKROOT)'
        },
        'libraries': [
          '-l<(amd_app_sdk_root)/lib/x86_64/OpenCL.lib',
        ],
        "include_dirs": [
          "<(amd_app_sdk_root)/include",
        ],
        "msvs_settings": {
          "VCLinkerTool": {
            # Don't print a linker warning when no imports from either .exe are used.
            "AdditionalOptions": ["/ignore:4199"],
          },
        }
     }],
     [ 'OS=="linux"', {
          # if linker flags don't work make sure that the folders are set
          # To debug this use full library paths (with .so and .a extension)
          'libraries': [
            '/opt/AMDAPPSDK-3.0/lib/x86_64/sdk/libOpenCL.so'
          ],
          'include_dirs': [
            '/usr/local/include',
            '/opt/AMDAPPSDK-3.0/include'
          ],
          'library_dirs': [
            '/usr/local/lib',
          ]
       }
     ]
    ]
  }]
}