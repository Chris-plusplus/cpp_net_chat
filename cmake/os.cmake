include_guard()

# check for OS
if(WIN32)
    set(CHAT_WINDOWS TRUE)
    set(CHAT_LINUX FALSE)
elseif(UNIX)
    set(CHAT_WINDOWS FALSE)
    set(CHAT_LINUX TRUE)
endif()

# set user home directory
if(CHAT_WINDOWS)    
    add_compile_definitions(CHAT_WINDOWS=1)
    add_compile_definitions(CHAT_LINUX=0)
    add_compile_definitions(CHAT_UNIX=0)

    message(STATUS "OS: Windows")
elseif(CHAT_LINUX)    
    add_compile_definitions(CHAT_WINDOWS=0)
    add_compile_definitions(CHAT_LINUX=1)
    add_compile_definitions(CHAT_UNIX=1)

    message(STATUS "OS: Linux")
endif()
