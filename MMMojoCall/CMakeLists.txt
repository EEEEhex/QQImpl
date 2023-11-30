cmake_minimum_required (VERSION 3.5)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)

project (MMMojoCall LANGUAGES CXX VERSION 2.0.0)

# project use folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# set up option variable for cmake
option(XPLUGIN_WRAPPER "Enable this to build the OCR/Utility/Player Wrapper Code" ON)
option(BUILD_QQIPC "Enable this to build the QQ Mojo IPC code" OFF)
option(BUILD_CPPEXAMPLE "Enable this to build the c++ usage example" OFF)
option(EXAMPLE_USE_JSON "Example Code's callback function data using protobuf or json" ON)
option(BUILD_PURE_C_MODE "Export Pure C Interface" OFF)

# add mmmojo_call src file
aux_source_directory(src SRC_FILES)
aux_source_directory(src/mmmojocall SRC_FILES)

# add xplugin wrapper src file
if (XPLUGIN_WRAPPER)
	# check depend files
	if (NOT EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/libprotobuf_32_MT.lib")
    	message(FATAL_ERROR "the depends lib is not exist!")
	endif()

	if (NOT EXISTS "${CMAKE_SOURCE_DIR}/src/include/xplugin_protobuf/google/protobuf")
		message(FATAL_ERROR "the google protobuf header file is not exist!")
	endif()

	aux_source_directory(src/mmmojocall/wrapper SRC_FILES)
endif()

# add qq mojo ipc src file
if (BUILD_QQIPC)
	list(APPEND SRC_FILES src/qq_mojoipc/qq_ipc.cpp)

	# if x64 then need asm file
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	    list(APPEND SRC_FILES src/qq_mojoipc/qq_ipc_asm.asm)
	    enable_language(C ASM_MASM) # enable masm
	endif()
endif()

# create library
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

# add mmmojo_call include dir
include_directories(src/include)
include_directories(src/include/mmmojo_source)

# add xplugin wrapper include dir
if (XPLUGIN_WRAPPER)
	include_directories(src/include/xplugin_protobuf)	
endif()

# add qq mojo ipc include dir
if (BUILD_QQIPC)
	include_directories(src/include/qq_mojoipc)
endif()

# add xplugin wrapper marco
if (XPLUGIN_WRAPPER)
	add_definitions(-DUSE_WRAPPER)
endif()

# add PURE_C_MODE marco
if (BUILD_PURE_C_MODE)
	add_definitions(-DPURE_C_MODE)
endif()

# add protobuf link lib
if (XPLUGIN_WRAPPER)
	# which arch
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	    set(ARCH_SUFFIX "64")
	else()
	    set(ARCH_SUFFIX "32")
	endif()

	target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/libprotobuf_${ARCH_SUFFIX}_MT$<$<CONFIG:Debug>:d>.lib)
endif()

# set /MT or /MTd
# add 'd' to Debug output
set_target_properties(${PROJECT_NAME} PROPERTIES
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
	DEBUG_POSTFIX _d
	)

# build c++ usage example
if (BUILD_CPPEXAMPLE)
	if (EXAMPLE_USE_JSON)
		add_definitions(-DEXAMPLE_USE_JSON)
	endif()

	# add xplugin (without wrapper) example
	set(XPLUGIN_USAGE xplugin_test)
	set(XPLUGIN_USAGE_SRC examples/cpp/xplugin_test.cpp)
	add_executable(${XPLUGIN_USAGE} ${XPLUGIN_USAGE_SRC})
	set_property(TARGET ${XPLUGIN_USAGE} PROPERTY FOLDER "EXAMPLE")
	set_target_properties(${XPLUGIN_USAGE} PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
		DEBUG_POSTFIX _d)
	add_dependencies(${XPLUGIN_USAGE} ${PROJECT_NAME})
	target_link_libraries(${XPLUGIN_USAGE} PRIVATE $(SolutionDir)$<IF:$<CONFIG:Debug>,Debug,Release>/${PROJECT_NAME}$<$<CONFIG:Debug>:_d>.lib)

	# add xplugin wrapper example
	if (XPLUGIN_WRAPPER)
		#OCR TEST
		set(XPLUGIN_OCR_USAGE xplugin_ocr_test)
		set(XPLUGIN_OCR_USAGE_SRC examples/cpp/xplugin_ocr_test.cpp)

		if (NOT EXAMPLE_USE_JSON)
			list(APPEND XPLUGIN_OCR_USAGE_SRC src/mmmojocall/wrapper/ocr_protobuf.pb.cc)
		endif()
		
		add_executable(${XPLUGIN_OCR_USAGE} ${XPLUGIN_OCR_USAGE_SRC})
		set_property(TARGET ${XPLUGIN_OCR_USAGE} PROPERTY FOLDER "EXAMPLE")
		set_target_properties(${XPLUGIN_OCR_USAGE} PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
			DEBUG_POSTFIX _d)
		add_dependencies(${XPLUGIN_OCR_USAGE} ${PROJECT_NAME})
		target_link_libraries(${XPLUGIN_OCR_USAGE} PRIVATE $(SolutionDir)$<IF:$<CONFIG:Debug>,Debug,Release>/${PROJECT_NAME}$<$<CONFIG:Debug>:_d>.lib)
		if (NOT EXAMPLE_USE_JSON)
			target_link_libraries(${XPLUGIN_OCR_USAGE} PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/libprotobuf_${ARCH_SUFFIX}_MT$<$<CONFIG:Debug>:d>.lib)
		endif()

		#UTILITY TEST
		set(XPLUGIN_UTILITY_USAGE xplugin_utility_test)
		set(XPLUGIN_UTILITY_USAGE_SRC examples/cpp/xplugin_utility_test.cpp)
		
		if (NOT EXAMPLE_USE_JSON)
			list(APPEND XPLUGIN_UTILITY_USAGE_SRC src/mmmojocall/wrapper/utility_protobuf.pb.cc)
		endif()

		add_executable(${XPLUGIN_UTILITY_USAGE} ${XPLUGIN_UTILITY_USAGE_SRC})
		set_property(TARGET ${XPLUGIN_UTILITY_USAGE} PROPERTY FOLDER "EXAMPLE")
		set_target_properties(${XPLUGIN_UTILITY_USAGE} PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
			DEBUG_POSTFIX _d)
		add_dependencies(${XPLUGIN_UTILITY_USAGE} ${PROJECT_NAME})
		target_link_libraries(${XPLUGIN_UTILITY_USAGE} PRIVATE $(SolutionDir)$<IF:$<CONFIG:Debug>,Debug,Release>/${PROJECT_NAME}$<$<CONFIG:Debug>:_d>.lib)
		if (NOT EXAMPLE_USE_JSON)
			target_link_libraries(${XPLUGIN_UTILITY_USAGE} PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/libprotobuf_${ARCH_SUFFIX}_MT$<$<CONFIG:Debug>:d>.lib)
		endif()

		#PLAYER TEST
		set(XPLUGIN_PLAYER_USAGE xplugin_player_test)
		set(XPLUGIN_PLAYER_USAGE_SRC examples/cpp/xplugin_player_test.cpp)

		add_executable(${XPLUGIN_PLAYER_USAGE} ${XPLUGIN_PLAYER_USAGE_SRC})
		set_property(TARGET ${XPLUGIN_PLAYER_USAGE} PROPERTY FOLDER "EXAMPLE")
		set_target_properties(${XPLUGIN_PLAYER_USAGE} PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
			DEBUG_POSTFIX _d)
		add_dependencies(${XPLUGIN_PLAYER_USAGE} ${PROJECT_NAME})
		target_link_libraries(${XPLUGIN_PLAYER_USAGE} PRIVATE $(SolutionDir)$<IF:$<CONFIG:Debug>,Debug,Release>/${PROJECT_NAME}$<$<CONFIG:Debug>:_d>.lib)
	endif()

	if (BUILD_QQIPC)
		set(QQIPC_PARENT_USAGE qqipc_parent_test)
		set(QQIPC_PARENT_USAGE_SRC examples/cpp/parentipc_test.cpp)

		add_executable(${QQIPC_PARENT_USAGE} ${QQIPC_PARENT_USAGE_SRC})
		set_property(TARGET ${QQIPC_PARENT_USAGE} PROPERTY FOLDER "EXAMPLE")

		set_target_properties(${QQIPC_PARENT_USAGE} PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
			DEBUG_POSTFIX _d)
		add_dependencies(${QQIPC_PARENT_USAGE} ${PROJECT_NAME})
		target_link_libraries(${QQIPC_PARENT_USAGE} PRIVATE $(SolutionDir)$<IF:$<CONFIG:Debug>,Debug,Release>/${PROJECT_NAME}$<$<CONFIG:Debug>:_d>.lib)

		set(QQIPC_CHILD_USAGE qqipc_child_test)
		set(QQIPC_CHILD_USAGE_SRC examples/cpp/childipc_test.cpp)

		add_executable(${QQIPC_CHILD_USAGE} ${QQIPC_CHILD_USAGE_SRC})
		set_property(TARGET ${QQIPC_CHILD_USAGE} PROPERTY FOLDER "EXAMPLE")

		set_target_properties(${QQIPC_CHILD_USAGE} PROPERTIES
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
			DEBUG_POSTFIX _d)
		add_dependencies(${QQIPC_CHILD_USAGE} ${PROJECT_NAME})
		target_link_libraries(${QQIPC_CHILD_USAGE} PRIVATE $(SolutionDir)$<IF:$<CONFIG:Debug>,Debug,Release>/${PROJECT_NAME}$<$<CONFIG:Debug>:_d>.lib)
	endif()
endif()
