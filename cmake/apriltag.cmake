# apriltag::apriltag
# License: Zlib

if(TARGET apriltag::apriltag)
    return()
endif()



message(STATUS "Third-party: creating target 'apriltag::apriltag'")

set(BUILD_PYTHON_WRAPPER OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include(CPM)
CPMAddPackage("gh:AprilRobotics/apriltag#305766652af34cafa5bab68fc2ebb2ca272e1482")

add_library(apriltag::apriltag ALIAS apriltag)

