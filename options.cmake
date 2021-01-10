set(CMAKE_CXX_COMPILER_ID "AppleClang")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CPP_SRC_DIR ${PROJECT_SOURCE_DIR})


set(CUSTOM_FLAGS "-Wall -Wextra -Wpedantic -Werror -Wconversion -Woverloaded-virtual -Wswitch-enum -fstrict-aliasing -Wstrict-aliasing=1 -fdiagnostics-color=auto -ftemplate-backtrace-limit=0 -faligned-new -Wno-noexcept-type -Wno-sign-conversion -Wsign-promo -Wno-unused-private-field -Wno-unused-parameter -ferror-limit=1")
# UNKNOWN: -Wno-range-loop-construct 

set(CMAKE_FIND_ROOT_PATH "/usr/local/bin")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
message( "CMAKE_FIND_ROOT_PATH_MODE_PROGRAM: " "${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}" )
find_program(CMAKE_AR ar)

set(CUSTOM_FLAGS_DEBUG "-g")
set(CUSTOM_FLAGS_RELEASE "-g -O3 -DNDEBUG")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CUSTOM_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CUSTOM_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CUSTOM_FLAGS_RELEASE}")
