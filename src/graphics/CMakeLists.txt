add_subdirectory(shaders)
add_subdirectory(texture)

set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/buffermanager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffermanager.h
	${CMAKE_CURRENT_SOURCE_DIR}/framebuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/framebuffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/image.h
	${CMAKE_CURRENT_SOURCE_DIR}/painter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/painter.h
	${CMAKE_CURRENT_SOURCE_DIR}/renderbuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/renderbuffer.h
)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
