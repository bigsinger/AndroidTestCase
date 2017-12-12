# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

# 设置其他后缀文件为源文件
set_property(SOURCE src/main/cpp/art/art_quick_dexposed_invoke_handler.S PROPERTY LANGUAGE C)

# 设置源码文件列表
file(GLOB NATIVE_SRC "src/main/cpp/*.cpp" "src/main/cpp/dalvik/*.cpp" "src/main/cpp/demo/*.cpp" "src/main/cpp/art/*.cpp" "src/main/cpp/libcutils/*.c" "src/main/cpp/art/*.S")
add_library( # Sets the name of the library.
             hooktest

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ${NATIVE_SRC}
            )
include_directories(src/main/cpp/ src/main/cpp/include)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

# 设置链接的库文件列表
file(GLOB libs src/main/cpp/3rd/libsubstrate.so src/main/cpp/3rd/libsubstratedvm.so)
target_link_libraries( # Specifies the target library.
                       hooktest

                       # Links the target library to the log library
                       # included in the NDK.
                       ${libs}
                       ${log-lib} )