add_library(orderbook
    orderbook.cpp
    pricelevel.cpp
    logger.cpp
)

target_include_directories(orderbook PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
