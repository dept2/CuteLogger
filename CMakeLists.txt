cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(CuteLogger LANGUAGES CXX)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Core5Compat REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Core5Compat REQUIRED)
ADD_DEFINITIONS(-DCUTELOGGER_LIBRARY)
add_library(CuteLogger SHARED "")
target_sources(CuteLogger PRIVATE
        src/Logger.cpp
        src/AbstractAppender.cpp
        src/AbstractStringAppender.cpp
        src/ConsoleAppender.cpp
        src/FileAppender.cpp
        src/RollingFileAppender.cpp
        )
target_compile_definitions(CuteLogger PRIVATE -DCUTELOGGER_LIBRARY)
target_include_directories(CuteLogger PUBLIC include)
target_link_libraries(CuteLogger PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat)

# OutputDebugAppender is only for Windows systems
if(WIN32)
  target_sources(CuteLogger PRIVATE
          src/OutputDebugAppender.cpp
          )
endif(WIN32)

install(TARGETS CuteLogger DESTINATION lib)

set(ENABLE_TESTS OFF CACHE BOOL "Enable building CuteLogger tests")
IF (ENABLE_TESTS)
  enable_testing()
  find_package(QT NAMES Qt6 Qt5 COMPONENTS Test REQUIRED)
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
  add_executable(basictest test/basictest.cpp)
  add_test(basictest basictest)
  set_target_properties(basictest PROPERTIES AUTOMOC ON)
  target_link_libraries(basictest Qt${QT_VERSION_MAJOR}::Test CuteLogger)
ENDIF ()
