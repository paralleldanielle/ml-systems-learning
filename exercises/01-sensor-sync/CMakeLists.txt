learn_code_root(EX01_CODE "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(ex01_sensor_sync
    "${EX01_CODE}/src/odometryBuffer.cpp"
    "${EX01_CODE}/src/latencyStats.cpp")
target_include_directories(ex01_sensor_sync PUBLIC "${EX01_CODE}/include")
learn_set_warnings(ex01_sensor_sync)
learn_set_tidy(ex01_sensor_sync)

add_executable(ex01_replay app/replay.cpp)
target_link_libraries(ex01_replay PRIVATE ex01_sensor_sync)
learn_set_warnings(ex01_replay)
learn_set_tidy(ex01_replay)

# Provided tests, one executable per session so each session has its own ctest label.
# `ctest -L` takes a regex, so `-L ex01` selects every ex01 test.
function(ex01_add_tests target label)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE ex01_sensor_sync GTest::gtest_main)
    target_include_directories(${target} PRIVATE tests)
    learn_set_warnings(${target})
    gtest_discover_tests(${target} PROPERTIES LABELS ${label})
endfunction()

ex01_add_tests(ex01_session1_tests ex01-s1 tests/odometryBufferSession1Test.cpp)
ex01_add_tests(ex01_session2_tests ex01-s2 tests/odometryBufferSession2Test.cpp tests/latencyStatsTest.cpp)

file(GLOB EX01_STUDENT_TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/student_*.cpp")
if(EX01_STUDENT_TESTS)
    ex01_add_tests(ex01_student_tests ex01-student ${EX01_STUDENT_TESTS})
endif()
