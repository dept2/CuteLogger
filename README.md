# Build

__Note:__ Omit `-D USE_Qt5=ON` below if you want to build against Qt4

```
git clone <this-repo> CuteLogger
mkdir CuteLogger-build
cd CuteLogger-build
cmake -D USE_Qt5=ON \
      -D Qt5Core_DIR="/path/to/your/Qt5CoreConfig.cmake/file" \
      ../CuteLogger/src
make -j
sudo make install # because default CMAKE_INSTALL_PREFIX is /usr/local
```

# Use

1. Put `FIND_PACKAGE(CuteLogger CONFIG REQUIRED)` into your project's CMake file.
1. Specify `CuteLogger_DIR` as `/usr/local/include/cutelogger` (i.e. `${CMAKE_INSTALL_PREFIX}/include/cutelogger`) for CMake.
1. See `cmake/CuteLoggerConfig.cmake` (installed in `/usr/local/include/cutelogger/CuteLoggerConfig.cmake`) to see which CMake variables to use.