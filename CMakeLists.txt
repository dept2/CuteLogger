CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

PROJECT(CuteLogger VERSION 1.0.0)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Network Xml Svg Concurrent LinguistTools)
if (QT_VERSION_MAJOR EQUAL 5)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network Xml Svg Concurrent LinguistTools)
elseif (QT_VERSION_MAJOR EQUAL 6)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network Xml Svg Concurrent LinguistTools Core5Compat)
endif()

ADD_DEFINITIONS(-DCUTELOGGER_LIBRARY)

INCLUDE_DIRECTORIES(BEFORE include)

SET(sources
  src/Logger.cpp
  src/AbstractAppender.cpp
  src/AbstractStringAppender.cpp
  src/ConsoleAppender.cpp
  src/FileAppender.cpp
  src/RollingFileAppender.cpp
  src/SignalAppender.cpp
)

SET(includes
  include/Logger.h
  include/FileAppender.h
  include/CuteLogger_global.h
  include/ConsoleAppender.h
  include/AbstractStringAppender.h
  include/AbstractAppender.h
  include/RollingFileAppender.h
  include/SignalAppender.h
)

# OutputDebugAppender is only for Windows systems
IF(WIN32)
  SET(sources ${sources} src/OutputDebugAppender.cpp)
  SET(includes ${includes} include/OutputDebugAppender.h)
ENDIF(WIN32)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${sources} ${includes})

if (QT_VERSION_MAJOR EQUAL 5)
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} Qt::Core)
elseif (QT_VERSION_MAJOR EQUAL 6)
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} Qt::Core Qt::Core5Compat)
endif()

SET(ENABLE_TESTS OFF CACHE BOOL "Enable building CuteLogger tests")
IF (ENABLE_TESTS)
  SET(CMAKE_AUTOMOC ON)
  FIND_PACKAGE(QtTest REQUIRED)

  ADD_EXECUTABLE(basictest test/basictest.cpp)
  TARGET_LINK_LIBRARIES(basictest Qt:Core Qt::Test ${PROJECT_NAME})
ENDIF ()


########################################################################
## The section includes the install section.
########################################################################
# GNUInstallDirs creates a set of fairly standard paths for installation
# Works for Linux/Unix, Windows and Mac
include(GNUInstallDirs)

target_include_directories(${PROJECT_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# sets up install paths for libraries and executables
install(TARGETS
      ${PROJECT_NAME}
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" # shared libraries
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" # static libraries
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" # applications
)
# marks the library target for export under the name "projectNameTargets"
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
install(
    TARGETS
      ${PROJECT_NAME}
    EXPORT
      ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# saves the export target to file as "projectnameTargets.cmake" in a folder
# "projectname"
install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${INSTALL_CONFIGDIR}
)
install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  FILES_MATCHING PATTERN "*.h*"
)


###Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

#Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

### install additional files
#set(INSTALL_INCLUDE_DIR "include")
#install(FILES ${SMWIDGETS_EXTRAS}
#  DESTINATION ${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}
#  CONFIGURATIONS Release
#  )

###Register package in the User Package Registry
export(PACKAGE ${PROJECT_NAME})
