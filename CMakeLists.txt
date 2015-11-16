CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(CuteLogger)

OPTION(USE_Qt5 "Use Qt5 instead of Qt4 (default)" OFF)
if(USE_Qt5)
  FIND_PACKAGE(Qt5Core REQUIRED)
else(USE_Qt5) # default is Qt4
  FIND_PACKAGE(Qt4 COMPONENTS QtCore REQUIRED)
  INCLUDE(${QT_USE_FILE})
endif(USE_Qt5)

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
if(USE_Qt5)
  QT5_USE_MODULES(${library_target} Core)
endif(USE_Qt5)
INSTALL(TARGETS ${library_target} DESTINATION lib)

SET(CuteLogger_PATH_SUFFIX cutelogger)
INSTALL(FILES
        ${includes} cmake/CuteLoggerConfig.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CuteLogger_PATH_SUFFIX})
INSTALL(TARGETS
        ${library_target}
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/${CuteLogger_PATH_SUFFIX}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${CuteLogger_PATH_SUFFIX}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${CuteLogger_PATH_SUFFIX})
