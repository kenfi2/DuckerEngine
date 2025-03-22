add_executable(${PROJECT_NAME})

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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/dxc_files/bin/x64/dxcompiler.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/dxc_files/bin/x64/dxil.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>)

add_subdirectory(graphics)
add_subdirectory(ui)

target_link_libraries(${PROJECT_NAME} PRIVATE vendor)
