include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

file(GLOB SRCS "src/*.c")
target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/drivers)
target_include_directories(app PRIVATE hdr)
target_sources(app PRIVATE ${SRCS})


target_compile_definitions(app PRIVATE THREAD)

