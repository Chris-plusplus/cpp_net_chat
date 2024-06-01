include_guard()

add_library(${PROJECT_NAME})

# find source files
file(GLOB_RECURSE ARCHIMEDES_SOURCE src/**.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${ARCHIMEDES_SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include)

# link conan libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${ARCHIMEDES_LIBRARIES})

if(ARCHIMEDES_WINDOWS)    
    target_compile_definitions(${PROJECT_NAME} PUBLIC ARCHIMEDES_WINDOWS=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ARCHIMEDES_LINUX=0)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ARCHIMEDES_UNIX=0)

    message(STATUS "OS: Windows")
elseif(ARCHIMEDES_LINUX)    
    target_compile_definitions(${PROJECT_NAME} PUBLIC ARCHIMEDES_WINDOWS=0)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ARCHIMEDES_LINUX=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ARCHIMEDES_UNIX=1)

    message(STATUS "OS: Linux")
endif()
