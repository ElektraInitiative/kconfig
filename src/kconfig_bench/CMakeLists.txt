
set(kconf_bench_SRC main.cpp)

add_executable(kconf_bench ${kconf_bench_SRC})
target_link_libraries(kconf_bench Qt5::Core KF5::ConfigCore )

if (ELEKTRA_FOUND)
    message(STATUS "Elektra ${ELEKTRA_VERSION} found")
    include_directories(${ELEKTRA_INCLUDE_DIR})

    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

    target_link_libraries(kconf_bench PRIVATE ${ELEKTRA_LIBRARIES})
    add_definitions(-DFEAT_ELEKTRA)
endif (ELEKTRA_FOUND)