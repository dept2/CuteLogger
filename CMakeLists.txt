CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FIND_PACKAGE(Qt4 COMPONENTS QtCore REQUIRED)
INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(BEFORE include)

SET(sources
  src/Logger.cpp
  src/AbstractAppender.cpp
  src/AbstractStringAppender.cpp
  src/ConsoleAppender.cpp
  src/FileAppender.cpp
)

SET(library_target Logger)

ADD_LIBRARY(${library_target} SHARED ${sources})
TARGET_LINK_LIBRARIES(${library_target} ${QT_LIBRARIES})
INSTALL(TARGETS ${library_target} LIBRARY DESTINATION lib)
