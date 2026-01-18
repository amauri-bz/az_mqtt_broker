
set(DOCTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/)

add_executable(az_mqtt_tests az_mqtt_protocol_test.cpp)

target_include_directories(az_mqtt_tests PUBLIC ${DOCTEST_INCLUDE_DIR})

enable_testing()
add_test(NAME run_mqtt_tests COMMAND az_mqtt_tests)
