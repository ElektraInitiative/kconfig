
set(kconf_bench_SRC main.cpp)

add_executable(kconf_bench ${kconf_bench_SRC})
target_link_libraries(kconf_bench Qt5::Core KF5::ConfigCore)