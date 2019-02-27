{
  "targets": [{
    "target_name": "nhm2_detect_cuda",
    "cflags": ["-Wall", "-fexceptions", "-O2"],
    # make sure to enable exceptions so it would build under Linux
    "cflags_cc": ["-Wall", "-std=c++11", "-fexceptions", "-O2"],
    "sources": [
      #"nodejs_native_src/linux_module_test.cpp",
      "nodejs_native_src/module.cpp",
      './CudaDetection.cpp',
      './nvidia_nvml_helper.cpp',
    ],
    "include_dirs": [
      "<!(node -e \"require('nan')\")",
      './'
    ],
    "conditions": [
      ['OS=="win"', {
        "defines": [
          # under Windows we load nvml library so we don't bundle the nvml.dll (we don't need to do this under Linux)
          "USE_DYNAMIC_LIB_LOAD=1"
        ],
        'variables': {
          'cuda_root%': '$(CUDA_PATH)'
        },
        'libraries': [
          '-l<(cuda_root)/lib/x64/cudart_static.lib',
        ],
        "include_dirs": [
          "<(cuda_root)/include",
        ],
        "msvs_settings": {
          "VCLinkerTool": {
            # Don't print a linker warning when no imports from either .exe are used.
            "AdditionalOptions": ["/ignore:4199"],
          },
        }
     }],
     # The linux library and include dirs usually have symlinks by default e.g. /usr/local/cuda this will corespond to the latest CUDA version
     [ 'OS=="linux"', {
          "defines": [
            "USE_DYNAMIC_LIB_LOAD=0"
          ],
          # if linker flags don't work make sure that the folders are set
          # To debug this use full library paths (with .so and .a extension)
          'libraries': [
            ## CUDA runtime library
            # STATIC using the full static path works
            '/usr/local/cuda/lib64/libcudart_static.a',
            # DYNAMIC use this for dynamic linking if needed for some reason
            #'-lcudart',

            # this is the NVML library link flag
            '-lnvidia-ml'
          ],
          'include_dirs': [
            '/usr/local/include',
            '/usr/local/cuda/include'
          ],
          'library_dirs': [
            '/usr/local/lib',
            '/usr/local/cuda/lib64',
            '/usr/lib/nvidia-384/'
          ]
       }
     ]
    ]
  }]
}