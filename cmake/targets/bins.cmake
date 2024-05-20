include_guard()

include("${PROJECT_SOURCE_DIR}/cmake/library.cmake")

file(GLOB_RECURSE CHAT_BINS_SOURCES bin_src/**.cpp)
foreach (CHAT_BIN_SOURCE ${CHAT_BINS_SOURCES})
	string(FIND ${CHAT_BIN_SOURCE} "/" CHAT_BIN_DIR_POS REVERSE)
	math(EXPR CHAT_BIN_DIR_POS "${CHAT_BIN_DIR_POS}+1")
	string(FIND ${CHAT_BIN_SOURCE} "." CHAT_BIN_EXT_POS REVERSE)
	math(EXPR CHAT_BIN_LEN "${CHAT_BIN_EXT_POS}-${CHAT_BIN_DIR_POS}")
	string(SUBSTRING ${CHAT_BIN_SOURCE} ${CHAT_BIN_DIR_POS} ${CHAT_BIN_LEN} CHAT_BIN)

    add_executable(${CHAT_BIN} ${CHAT_BIN_SOURCE})
	target_link_libraries(${CHAT_BIN} PUBLIC ${PROJECT_NAME})
endforeach ()
