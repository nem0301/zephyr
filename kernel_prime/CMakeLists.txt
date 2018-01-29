include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)
target_include_directories(app PRIVATE hdr)
target_sources(app PRIVATE src/main.c src/Thread.c src/WorkQueue.c src/Timer.c)

