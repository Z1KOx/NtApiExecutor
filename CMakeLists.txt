cmake_minimum_required( VERSION 3.29 )
project( NtFunctionInvoker 
         LANGUAGES C CXX
)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR )
    message( FATAL_ERROR "In-tree builds are not supported. Run CMake from a separate directory: cmake -B build" )
endif()

if( WIN32 )
	set( CMAKE_CXX_STANDARD 20 )
	set( CMAKE_CXX_STANDARD_REQUIRED ON )

	set( NtFunctionInvoker_src
		"src/main.cpp"
		"src/nt_api_executor.cpp"
	)

	add_executable( NtFunctionInvoker ${NtFunctionInvoker_src} )
else()
    message( FATAL_ERROR "This project is specifically designed for Windows. It cannot be built on ${CMAKE_SYSTEM_NAME}." )
endif()