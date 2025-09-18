add_executable(${PROJECT_NAME})

find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    message(STATUS "Vulkan found: ${Vulkan_LIBRARY}")
    include_directories(${Vulkan_INCLUDE_DIR})
	set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})
    set(SHADERC_LIB_DIR ${VULKAN_SDK_PATH}/Lib)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(SHADERC_LIB ${SHADERC_LIB_DIR}/shaderc_combinedd.lib)
		# set(SHADERC_LIB ${SHADERC_LIB_DIR}/shaderc_combined.lib)
	else()
    	set(SHADERC_LIB ${SHADERC_LIB_DIR}/shaderc_combined.lib)
	endif()
else()
    message(FATAL_ERROR "Vulkan not found!")
endif()

find_package(unofficial-spirv-reflect CONFIG REQUIRED)

set(Boost_INCLUDE_DIR "E:/vcpkg/installed/x64-windows/include")

set(SOURCES
	engine.cpp
	engine.h
	frametimer.cpp
	frametimer.h
	window.cpp
	window.h
	main.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/dxc_files/bin/x64/dxcompiler.dll"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_SOURCE_DIR}/dxc_files/bin/x64/dxil.dll"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	# set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /MP /D_DEBUG")
	# set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
	# set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
else()
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

add_subdirectory(graphics)
add_subdirectory(ui)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE NDEBUG)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /W4 /EHsc")

target_link_libraries(${PROJECT_NAME} PRIVATE vendor Vulkan::Vulkan ${SHADERC_LIB} unofficial::spirv-reflect)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
