cmake_minimum_required(VERSION 2.8)
project(sake)

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 包含目录
include_directories(${PROJECT_SOURCE_DIR}/src) 
# yaml-cpp的头文件路径
include_directories(${PROJECT_SOURCE_DIR}/yaml-cpp/include) 
# yaml-cpp库的库文件路径
link_directories(${PROJECT_SOURCE_DIR}/yaml-cpp/build)  

# 设置编译参数
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -DCMAKE_VERBOSE_MAKEFILE=ON")

add_subdirectory(yaml-cpp)

# 添加源文件
set(SRC_CPP 
    src/log.cpp
    src/util.cpp
    src/config.cpp
    src/thread.cpp
    src/config.cpp
    src/fiber.cpp
)

# 生成sake库
add_library(sake SHARED ${SRC_CPP})

# 生成测试可执行文件 test_thread
add_executable(test_thread ${PROJECT_SOURCE_DIR}/test/test_thread.cpp)
add_dependencies(test_thread sake)

# 生成测试可执行文件 test_config
add_executable(test_config ${PROJECT_SOURCE_DIR}/test/test_config.cpp)
add_dependencies(test_config sake)

# 生成测试可执行文件 test_util
add_executable(test_util ${PROJECT_SOURCE_DIR}/test/test_util.cpp)
add_dependencies(test_util sake)

# 生成测试可执行文件 test_fiber
add_executable(test_fiber ${PROJECT_SOURCE_DIR}/test/test_fiber.cpp)
add_dependencies(test_fiber sake)
set(LIB_LIB
    sake
    pthread
    yaml-cpp
)

# 链接依赖库
target_link_libraries(test_thread ${LIB_LIB})
target_link_libraries(test_config ${LIB_LIB})
target_link_libraries(test_util ${LIB_LIB})
target_link_libraries(test_fiber ${LIB_LIB})