include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(orderbook_tests
    test_orderbook.cpp
    test_order.cpp
    test_pricelevel.cpp
    test_logger.cpp
)

target_link_libraries(orderbook_tests
    PRIVATE
        orderbook
        GTest::gtest_main
)

add_executable(demo 
    demo.cpp
)
target_link_libraries(demo 
    PRIVATE 
        orderbook
)

include(GoogleTest)
gtest_discover_tests(orderbook_tests)
