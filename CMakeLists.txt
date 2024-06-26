cmake_minimum_required(VERSION 3.19)

project(
        cpp_net_chat
        VERSION 0.1.0
        LANGUAGES CXX C
)

#set(CMAKE_VERBOSE_MAKEFILE 1)
#set(CMAKE_AUTOGEN_VERBOSE 1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include("cmake/build_mode.cmake")
include("cmake/os.cmake")
include("cmake/msvc.cmake")
include("cmake/defines.cmake")

# uncomment below to force conan reinstallation
#set(CHAT_FORCE_CONAN_INSTALL TRUE)
if(WIN32)
    include("cmake/conan.cmake")
else()
    include("cmake/linux_link.cmake")
endif()

add_subdirectory(deps/archimedes)

include("cmake/library.cmake")

# CMake targets
file(GLOB_RECURSE CHAT_TARGETS cmake/targets/*.cmake)
foreach (CHAT_TARGET ${CHAT_TARGETS})
    include(${CHAT_TARGET})
endforeach ()
