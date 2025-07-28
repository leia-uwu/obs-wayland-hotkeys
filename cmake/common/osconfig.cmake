# CMake operating system bootstrap module

include_guard(GLOBAL)

set(CMAKE_CXX_EXTENSIONS FALSE)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/linux")
string(TOUPPER "${CMAKE_HOST_SYSTEM_NAME}" _SYSTEM_NAME_U)
set(OS_${_SYSTEM_NAME_U} TRUE)
