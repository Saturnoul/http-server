cmake_minimum_required(VERSION 3.17)
project(http)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-pthread")
add_executable(http main.cpp server/server.cpp server/server.h client/client.cpp client/client.h message/HttpRequest.cpp message/HttpRequest.h message/HttpResponse.cpp message/HttpResponse.h message/component/header.cpp message/component/header.h message/component/body.cpp message/component/body.h util/string_helper.h util/string_helper.cpp message/HttpMessage.cpp message/HttpMessage.h util/io_helper.h util/io_helper.cpp conf/Configuration.cpp conf/Configuration.h)

target_link_libraries(http "/usr/local/lib/libjsoncpp.a")