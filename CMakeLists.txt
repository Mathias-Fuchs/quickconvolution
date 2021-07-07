cmake_minimum_required(VERSION 3.1)
project(quickconv C)


add_library(fftw SHARED IMPORTED)
set_property(TARGET fftw PROPERTY IMPORTED_LOCATION
   "${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/fftw335WIN64/libfftw3f-3.dll"
	)
set_property(TARGET fftw PROPERTY IMPORTED_IMPLIB
  	"${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/fftw335WIN64/libfftw3f-3.lib"
    )
set_property(TARGET fftw PROPERTY INTERFACE_INCLUDE_DIRECTORIES
	"${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/fftw335WIN64/"
    )


if (WIN32)
else()
find_package(FFTW3f CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)
endif()


add_executable(blurimage
	
	blurimage.c
	)


add_executable(quickconv
	main.c
	Timeline.c
	box.c
	readnumerictable.c
	staticHelpers.c
)

if (WIN32)
target_link_libraries(quickconv PRIVATE
	fftw
	sqlite3
)
target_link_libraries(blurimage PRIVATE
	fftw
)
else()
target_link_libraries(quickconv PRIVATE
	FFTW3::fftw3f
	sqlite3
)
target_link_libraries(blurimage
	FFTW3::fftw3f
	)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/forConv.db ${CMAKE_CURRENT_BINARY_DIR}/forConv.db COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hania.png ${CMAKE_CURRENT_BINARY_DIR}/hania.png COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/fftw335WIN64/libfftw3f-3.dll ${CMAKE_CURRENT_BINARY_DIR}/libfftw3f-3.dll COPYONLY)