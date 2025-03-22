set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/buffermanager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffermanager.h
	${CMAKE_CURRENT_SOURCE_DIR}/bufferchunk.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/bufferchunk.h
)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
