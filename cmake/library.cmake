include_guard()

add_library(${PROJECT_NAME})

# find source files
file(GLOB_RECURSE CHAT_SOURCE src/**.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${CHAT_SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include ${CHAT_INCLUDES})

# link conan libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${CHAT_LIBRARIES} archimedes)
