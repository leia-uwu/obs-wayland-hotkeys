cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

find_package(obs-frontend-api REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)

find_package(
  Qt6
  COMPONENTS Core Widgets Gui DBus WaylandClient
)
if(Qt6Gui_VERSION VERSION_GREATER_EQUAL "6.10.0")
  find_package(Qt6GuiPrivate ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
else()
  find_package(Qt6Gui ${QT_MIN_VERSION} CONFIG REQUIRED Private)
endif()

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::DBus Qt6::WaylandClient Qt6::GuiPrivate
)

target_compile_options(
  ${CMAKE_PROJECT_NAME}
  PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
)

set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/main.cpp src/shortcutsPortal.cpp)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
