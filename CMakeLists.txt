cmake_minimum_required( VERSION 3.29 )
project( NtFunctionInvoker )

set( CMAKE_CXX_STANDARD 20 )

set(NtFunctionInvoker_src
	"src/main.cpp"
	"src/typedefs.h"
)

add_executable( NtFunctionInvoker ${NtFunctionInvoker_src} )