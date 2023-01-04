{
  "targets": [
    {
      "target_name": "fims",
      "sources": [
        "libfims.cpp",
        "libaes.cpp",
        "napifims.cpp",
        "napifims.h",
        "cJSON.c"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "defines": ["NAPI_CPP_EXCEPTIONS"]
    }
  ]
}