set(REQUIRED_CMAKE_VERSION "3.25.0")
cmake_minimum_required(VERSION ${REQUIRED_CMAKE_VERSION})


################################################################################
# CMake Policies
################################################################################

cmake_policy(SET CMP0054 NEW) # Only interpret if() arguments as variables or keywords when unquoted.
cmake_policy(SET CMP0076 NEW) # target_sources() command converts relative paths to absolute.
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
  cmake_policy(SET CMP0135 NEW) # Set the timestamps of all extracted contents to the time of the extraction.
endif()

################################################################################
project(ASSandbox DESCRIPTION "AR Sandbox" LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ASSANDBOX_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src")


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


include(imgui)
include(GLFW)



add_executable(arsandbox src/app.cpp src/image.cpp)
target_compile_definitions(arsandbox PRIVATE AR_IMAGE_FOLDER=\"${PROJECT_SOURCE_DIR}/images\" )
target_link_libraries(arsandbox PRIVATE glfw::glfw imgui::imgui)


set_target_properties(arsandbox PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
install(TARGETS arsandbox DESTINATION .)

if(NOT (${CMAKE_VERSION} VERSION_LESS "3.6.0"))
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT arsandbox)
endif()


if(APPLE)
  target_link_libraries(arsandbox PRIVATE
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
    "-framework OpenGL"
  )
endif()