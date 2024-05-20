include_guard()

include("${PROJECT_SOURCE_DIR}/cmake/os.cmake")

# check if conan is installed
execute_process(
	COMMAND "conan" "--version"
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE CONAN_CHECK_OUT
)
if(NOT CONAN_CHECK_OUT MATCHES ".?Conan.?")
	message(FATAL_ERROR "Conan not found")
else()
	message(STATUS "Conan present")
endif()

SET(CHAT_CONAN_TOOLCHAIN_PATH "${PROJECT_SOURCE_DIR}/cmake/conan_files/${CHAT_BUILD_TYPE}/conan_toolchain.cmake")
SET(CHAT_CONAN_INSTALL_HASH_PATH "${PROJECT_SOURCE_DIR}/cmake/conanfile.hash")

file(SHA256 "${PROJECT_SOURCE_DIR}/conanfile.py" CHAT_CONANFILE_HASH)
if(EXISTS ${CHAT_CONAN_INSTALL_HASH_PATH})
	file(STRINGS ${CHAT_CONAN_INSTALL_HASH_PATH} CHAT_CONAN_INSTALL_HASH_LIST LIMIT_COUNT 1)
	list(GET CHAT_CONAN_INSTALL_HASH_LIST 0 CHAT_CONAN_INSTALL_HASH)

	if(${CHAT_CONAN_INSTALL_HASH} STREQUAL ${CHAT_CONANFILE_HASH})
		SET(CHAT_CONAN_INSTALL_HASH_NEQ FALSE)
	else()
		SET(CHAT_CONAN_INSTALL_HASH_NEQ TRUE)
	endif()
endif()

# check for conan files
if(CHAT_FORCE_CONAN_INSTALL OR CHAT_CONAN_INSTALL_HASH_NEQ OR NOT EXISTS ${CHAT_CONAN_TOOLCHAIN_PATH} OR NOT EXISTS ${CHAT_CONAN_INSTALL_HASH_PATH})
	if(CHAT_FORCE_CONAN_INSTALL)
		message(STATUS "Forced Conan configuration for ${CHAT_BUILD_TYPE} mode")
	elseif(CHAT_CONAN_INSTALL_HASH_NEQ)
		message(STATUS "conanfile.py was edited, configuring conan for ${CHAT_BUILD_TYPE} mode")
	else()
		message(STATUS "Conan files not found for ${CHAT_BUILD_TYPE} mode, configuring conan for ${CHAT_BUILD_TYPE} mode")
	endif()

	execute_process(
		COMMAND "conan" "profile" "detect" "--force"
		COMMAND_ERROR_IS_FATAL ANY
	)

	file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/cmake/conan_files/${CHAT_BUILD_TYPE}/")

	# option only for MSCV
	if(MSVC)
		set(CHAT_CONAN_COMPILER_RUNTIME_TYPE "-s:b compiler.runtime_type=${CHAT_BUILD_TYPE} -s:h compiler.runtime_type=${CHAT_BUILD_TYPE}")
	endif()

	if(${CHAT_LINUX})
		set(CHAT_CONAN_SYSTEM_PACKAGE_MANAGER_MODE "-c tools.system.package_manager:mode=install")
		set(CHAT_CONAN_SYSTEM_PACKAGE_MANAGER_SUDO "-c tools.system.package_manager:sudo=True")
	endif()

	# install conan requirements
	execute_process(
		COMMAND conan install . -s:b build_type=${CHAT_BUILD_TYPE} -s:b compiler.cppstd=20 -s:h build_type=${CHAT_BUILD_TYPE} -s:h compiler.cppstd=20 ${CHAT_CONAN_INSTALL_RUNTIME_TYPE} ${CHAT_CONAN_SYSTEM_PACKAGE_MANAGER_SUDO} ${CHAT_CONAN_SYSTEM_PACKAGE_MANAGER_MODE} -of=cmake/conan_files/${CHAT_BUILD_TYPE} --build=missing -pr default
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE CHAT_CONAN_INSTALL_OUTPUT
		RESULT_VARIABLE CHAT_CONAN_INSTALL_RESULT
	)

	# check conan result
	if(NOT ${CHAT_CONAN_INSTALL_RESULT} EQUAL 0)
		file(REMOVE_RECURSE "${PROJECT_SOURCE_DIR}/cmake/conan_files/${CHAT_BUILD_TYPE}/")
		message(FATAL_ERROR "Conan installation failed")
	else()
		file(WRITE ${CHAT_CONAN_INSTALL_HASH_PATH} ${CHAT_CONANFILE_HASH})

		message(STATUS "Conan installation succeded")
	endif()
else()
	message(STATUS "Conan files found for ${CHAT_BUILD_TYPE} mode") 
endif()

# manual include conan toolchain
include("${PROJECT_SOURCE_DIR}/cmake/conan_files/${CHAT_BUILD_TYPE}/conan_toolchain.cmake")
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/conan_files/${CHAT_BUILD_TYPE})

# includes from conan
include_directories(${CMAKE_INCLUDE_PATH})

# automatic find_package()
include("${PROJECT_SOURCE_DIR}/cmake/conan_files/${CHAT_BUILD_TYPE}/conandeps_legacy.cmake")
list(APPEND CHAT_LIBRARIES ${CONANDEPS_LEGACY})
