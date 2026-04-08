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
include(apriltag)



add_executable(arsandbox
          src/audio.cpp
          src/stb_impl.cpp
          src/app.cpp
          src/pig.cpp
          src/frog.cpp
          src/goat.cpp
          src/eagle.cpp
          src/wolf.cpp
          src/fish.cpp
          src/volture.cpp
          src/Drop.cpp
          src/Simulation.cpp
          src/image.cpp
          src/depth.cpp
          src/overlay.cpp)
target_compile_definitions(arsandbox PRIVATE AR_IMAGE_FOLDER=\"${PROJECT_SOURCE_DIR}/images\" )
target_compile_definitions(arsandbox PRIVATE AR_SOUND_FOLDER=\"${PROJECT_SOURCE_DIR}/sounds\" )
target_compile_definitions(arsandbox PRIVATE AR_DEPTH_FOLDER=\"${PROJECT_SOURCE_DIR}\" )
target_link_libraries(arsandbox PRIVATE glfw::glfw imgui::imgui apriltag::apriltag)

if(LINUX)
  find_package(OpenGL REQUIRED)
  target_link_libraries(arsandbox PRIVATE OpenGL::GL)

  find_package(realsense2 CONFIG REQUIRED)
  target_link_libraries(arsandbox PRIVATE realsense2::realsense2)

  target_compile_definitions(arsandbox PRIVATE SANDBOX_WITH_REALSENSE)
endif()

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