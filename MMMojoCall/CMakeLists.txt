cmake_minimum_required (VERSION 3.5)

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)

project (MMMojoCall LANGUAGES CXX VERSION 2.0.0)

# project use folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# set up option variable for cmake
option(XPLUGIN_WRAPPER "Enable this to build the OCR and Utility Wrapper Code" ON)
option(BUILD_QQIPC "Enable this to build the QQ Mojo IPC code" OFF)
option(BUILD_CPPEXAMPLE "Enable this to build the c++ usage example" OFF)
option(BUILD_WITH_SWIG_PY "Build with SWIG python" OFF)

# add mmmojo_call src file
aux_source_directory(src SRC_FILES)

# add xplugin wrapper src file
if (XPLUGIN_WRAPPER)
	# check depend files
	if (NOT EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/libprotobuf_32_MT.lib")
    	message(FATAL_ERROR "the depends lib is not exist!")
	endif()

	if (NOT EXISTS "${CMAKE_SOURCE_DIR}/src/include/xplugin_protobuf/google/protobuf")
		message(FATAL_ERROR "the google protobuf header file is not exist!")
	endif()

	aux_source_directory(src/wrapper SRC_FILES)
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

# build use swig c++ to python
if (BUILD_WITH_SWIG_PY)
	message(FATAL_ERROR "在SWIG中 不知道怎么把python函数转为void*传给c++ 试了好多方法都不行 先放弃了")
	if (NOT XPLUGIN_WRAPPER)
		message(FATAL_ERROR "这块没写 建议把xplugin wrapper code一块BUILD了")
	endif()

	# check swig and python env
	find_package(Python COMPONENTS Development)
	if (NOT Python_Development_FOUND)
		message(FATAL_ERROR "Can't Find Your Python Dir!")
	endif()

	find_package(SWIG REQUIRED)
	if (NOT SWIG_FOUND)	
		message(FATAL_ERROR "Can't Find Your Swig.exe!")
	endif()	

	# UseSWIG
	include (${SWIG_USE_FILE})

	# set .py swig wrapper output dir
	#set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release>)

	set(SWIG2PY mmmcall_wrapper)

	set_property(SOURCE swig/mmmojo_call.i PROPERTY CPLUSPLUS ON)
	swig_add_library(${SWIG2PY} LANGUAGE python SOURCES swig/mmmojo_call.i)
	set_property(TARGET ${SWIG2PY} PROPERTY FOLDER "SWIG")

	# include python embeded header
	target_include_directories(${SWIG2PY} PRIVATE ${Python_INCLUDE_DIRS})
	target_link_directories(${SWIG2PY} PRIVATE ${Python_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(${SWIG2PY} ${Python_LIBRARIES} ${PROJECT_NAME})
endif()



# build c++ usage example
if (BUILD_CPPEXAMPLE)
	set(XPLUGIN_USAGE xplugin_test)
	set(XPLUGIN_USAGE_SRC examples/cpp/xplugin_test.cpp)
	if (XPLUGIN_WRAPPER)
		list(APPEND XPLUGIN_USAGE_SRC src/wrapper/ocr_protobuf.pb.cc)
		list(APPEND XPLUGIN_USAGE_SRC src/wrapper/utility_protobuf.pb.cc)
	endif()

	add_executable(${XPLUGIN_USAGE} ${XPLUGIN_USAGE_SRC})
	set_property(TARGET ${XPLUGIN_USAGE} PROPERTY FOLDER "EXAMPLE")

	set_target_properties(${XPLUGIN_USAGE} PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
		DEBUG_POSTFIX _d)

	add_dependencies(${XPLUGIN_USAGE} ${PROJECT_NAME})
	
	target_link_libraries(${XPLUGIN_USAGE} PRIVATE $(SolutionDir)$<IF:$<CONFIG:Debug>,Debug,Release>/${PROJECT_NAME}$<$<CONFIG:Debug>:_d>.lib)
	if (XPLUGIN_WRAPPER)	
		target_link_libraries(${XPLUGIN_USAGE} PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/libprotobuf_${ARCH_SUFFIX}_MT$<$<CONFIG:Debug>:d>.lib)
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
