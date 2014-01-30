import qbs 1.0

DynamicLibrary {
  name: "CuteLogger"

  files: [ "src/Logger.cpp", "include/Logger.h",
           "src/AbstractAppender.cpp", "include/AbstractAppender.h",
           "src/AbstractStringAppender.cpp", "include/AbstractStringAppender.h",
           "src/ConsoleAppender.cpp", "include/ConsoleAppender.h",
           "src/FileAppender.cpp", "include/FileAppender.h",
           "include/CuteLogger_global.h" ]

  Group {
    condition: qbs.targetOS == "windows"
    files: [ "src/OutputDebugAppender.cpp", "include/OutputDebugAppender.h" ]
  }

  Depends { name: "cpp" }
  cpp.includePaths: "include"
  cpp.defines: "CUTELOGGER_LIBRARY"

  Depends { name: "Qt.core" }
}
