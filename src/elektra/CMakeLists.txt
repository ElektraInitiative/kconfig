include_directories(${ELEKTRA_INCLUDE_DIR})

add_library(elektra-kconfig SHARED
        fileinputiterator.cpp
        parser.cpp
        serializer.cpp
        elektra_plugin.cpp)

add_executable(elektra-kconfig-test
        fileinputiterator.cpp
        parser.cpp
        elektra_plugin.cpp
        serializer.cpp
        test.cpp)

target_include_directories(elektra-kconfig PUBLIC ${PROJECT_SOURCE_DIR}/src/elektra/)
target_include_directories(elektra-kconfig-test PUBLIC ${PROJECT_SOURCE_DIR}/src/elektra/)

set_property(TARGET elektra-kconfig
        APPEND
        PROPERTY COMPILE_FLAGS
        "-fexceptions")

set_property(TARGET elektra-kconfig-test
        APPEND
        PROPERTY COMPILE_FLAGS
        "-fexceptions")

find_package(Elektra REQUIRED)
include_directories(${ELEKTRA_INCLUDE_DIR})
include_directories(elektra-plugin)
target_link_libraries(elektra-kconfig ${ELEKTRA_LIBRARIES};elektra-plugin)
target_link_libraries(elektra-kconfig-test ${ELEKTRA_LIBRARIES};elektra-plugin)

find_package(Qt5 REQUIRED COMPONENTS Core)
include_directories(${Qt5Core_INCLUDE_DIRS})
target_link_libraries(elektra-kconfig ${Qt5Core_LIBRARIES})
target_link_libraries(elektra-kconfig-test ${Qt5Core_LIBRARIES})

get_cmake_property(_variableNames VARIABLES)
list(SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach ()

