project(Main)
set(SRCS
    main.cc
    Epoller.cc
    Channel.cc
    EventLoop.cc
    EventLoopThread.cc
    Server.cc
    util.cc
    Log.cc
    LogFile.cc)
add_executable(main.out ${SRCS})


FIND_PACKAGE(Threads)
TARGET_LINK_LIBRARIES(main.out ${CMAKE_THREAD_LIBS_INIT} "profiler")