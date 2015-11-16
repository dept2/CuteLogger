CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(CuteLogger)

FIND_PACKAGE(Qt5Core REQUIRED)

ADD_DEFINITIONS(-DCUTELOGGER_LIBRARY)

INCLUDE_DIRECTORIES(BEFORE include)

SET(sources
  src/Logger.cpp
  src/AbstractAppender.cpp
  src/AbstractStringAppender.cpp
  src/ConsoleAppender.cpp
  src/FileAppender.cpp
  src/RollingFileAppender.cpp
)

SET(includes
  include/Logger.h
  include/FileAppender.h
  include/CuteLogger_global.h
  include/ConsoleAppender.h
  include/AbstractStringAppender.h
  include/AbstractAppender.h
  include/RollingFileAppender.h
 )


# OutputDebugAppender is only for Windows systems
IF(WIN32)
  SET(sources ${sources} src/OutputDebugAppender.cpp)
  SET(includes ${includes} include/OutputDebugAppender.h)
ENDIF(WIN32)


SET(library_target Logger)

ADD_LIBRARY(${library_target} SHARED ${sources} ${includes})
TARGET_LINK_LIBRARIES(${library_target} ${QT_LIBRARIES})
QT5_USE_MODULES(${library_target} Core)
INSTALL(TARGETS ${library_target} DESTINATION lib)

SET(executable_target LoggerEx)
ADD_EXECUTABLE(${executable_target} sample/main.cpp)
TARGET_LINK_LIBRARIES(${executable_target} ${library_target} ${QT_LIBRARIES})

SET(CuteLogger_PATH_SUFFIX cutelogger)
INSTALL(FILES
        ${includes} cmake/CuteLoggerConfig.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CuteLogger_PATH_SUFFIX})
INSTALL(TARGETS
        ${library_target}
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/${CuteLogger_PATH_SUFFIX}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${CuteLogger_PATH_SUFFIX}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${CuteLogger_PATH_SUFFIX})