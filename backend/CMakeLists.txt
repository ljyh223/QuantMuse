cmake_minimum_required(VERSION 3.12)
project(trading_engine)

# ---------- 语言和编译选项 ----------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- 子模块 ----------
add_subdirectory(Logger)          # 只调用一次

# ---------- 查找第三方包 ----------
find_package(pybind11 REQUIRED)          # Python 接口
find_package(spdlog REQUIRED)            # 日志
find_package(nlohmann_json REQUIRED)     # JSON
find_package(Boost REQUIRED COMPONENTS system)

# ---------- 生成可执行目标 ----------
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
     src/*.cpp)                       # 收集所有 .cpp
add_executable(trading_engine ${SOURCES})

# ---------- 头文件搜索路径 ----------
target_include_directories(trading_engine PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# ---------- 链接所有库 ----------
target_link_libraries(trading_engine
    PRIVATE
        Utils::Logger                 # 来自 Logger 子模块
        pybind11::embed
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        Boost::system
        Threads::Threads              # 跨平台 pthread
)