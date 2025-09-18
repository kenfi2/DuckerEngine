set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/program.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/program.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders_vulkan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders.h
)

if(WIN32)
    set(SOURCES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/shaders_d3d12.cpp)
elseif(APPLE)
    set(SOURCES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/shaders_metal.cpp)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
