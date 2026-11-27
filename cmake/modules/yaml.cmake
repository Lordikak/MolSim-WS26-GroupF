message(STATUS "Loading yaml.cmake")

#include yaml parsing framework
include(FetchContent)
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG yaml-cpp-0.9.0
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(yaml-cpp)

target_link_libraries(Lib PUBLIC yaml-cpp::yaml-cpp)