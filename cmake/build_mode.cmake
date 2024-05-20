include_guard()

# check build type
if(
    "${CMAKE_CONFIGURATION_TYPES}" STREQUAL Debug
    OR "${CMAKE_BUILD_TYPE}" STREQUAL Debug
    OR "${CMAKE_CONFIGURATION_TYPES}" STREQUAL "RelWithDebInfo"
    OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"
)
    set(CHAT_DEBUG TRUE)
    set(CHAT_RELEASE FALSE)

    set(CHAT_BUILD_TYPE "Debug")
    
    add_compile_definitions(CHAT_DEBUG=1)
    add_compile_definitions(CHAT_RELEASE=0)
else()
    set(CHAT_DEBUG FALSE)
    set(CHAT_RELEASE TRUE)
    
    set(CHAT_BUILD_TYPE "Release")
    
    add_compile_definitions(CHAT_DEBUG=0)
    add_compile_definitions(CHAT_RELEASE=1)
endif()

message(STATUS "Build mode: ${CHAT_BUILD_TYPE}")

set(CMAKE_BUILD_TYPE ${CHAT_BUILD_TYPE})
