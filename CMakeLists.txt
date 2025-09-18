cmake_minimum_required (VERSION 3.5)

cmake_policy(SET CMP0167 OLD)

set(CMAKE_TOOLCHAIN_FILE E:/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(Ducker)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SAVE_FOLDER "${CMAKE_SOURCE_DIR}/dxc_files")
set(PS_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/dxc.ps1")

if(WIN32)
    add_custom_target(
        RunDXCDownload
        COMMAND powershell -ExecutionPolicy Bypass -File "${PS_SCRIPT}" "${SAVE_FOLDER}"
        COMMENT "Downloading and extract DXC files to ${SAVE_FOLDER}"
    )
    
    set(DXC_LIB_PATH "${CMAKE_SOURCE_DIR}/dxc_files/lib/x64")
    link_directories(${DXC_LIB_PATH})
endif()

add_subdirectory(android-project/app/jni)
add_subdirectory(src)
target_include_directories(${PROJECT_NAME} PRIVATE src)
cmake_policy(SET CMP0079 NEW)
target_link_libraries(${PROJECT_NAME} PRIVATE dxcompiler)
add_dependencies(${PROJECT_NAME} RunDXCDownload)
