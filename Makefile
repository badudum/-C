cmake_minumum_required(3.14)

find_program(CCACHE_PROTRAM ccache)
if(CCACHE_PROGRAM)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

project(Cminmin C)

add_executable(
	func
	src/main.c
)