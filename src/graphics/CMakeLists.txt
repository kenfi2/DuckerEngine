add_subdirectory(buffer)
add_subdirectory(texture)

set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/drawbuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/drawbuffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/image.h
	${CMAKE_CURRENT_SOURCE_DIR}/painter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/painter.h
	${CMAKE_CURRENT_SOURCE_DIR}/program.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/program.h
	${CMAKE_CURRENT_SOURCE_DIR}/shaders.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shaders.h
)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
