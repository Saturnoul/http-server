cmake_minimum_required(VERSION 3.17)
project(http)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-pthread")

add_executable(http main.cpp server/server.cpp server/server.h client/client.cpp client/client.h message/http/HttpRequest.cpp message/http/HttpRequest.h message/http/HttpResponse.cpp message/http/HttpResponse.h message/component/header.cpp message/component/header.h message/component/body.cpp message/component/body.h util/string_helper.h util/string_helper.cpp message/http/HttpMessage.cpp message/http/HttpMessage.h util/io_helper.h util/io_helper.cpp conf/Configuration.cpp conf/Configuration.h thirdparty/thread_pool.h util/algorithm.cpp util/algorithm.h thirdparty/base64.cpp thirdparty/base64.h server/websocket_server.cpp server/websocket_server.h message/websocket/WebsocketMessage.cpp message/websocket/WebsocketMessage.h server/http_server.cpp server/http_server.h server/http_and_websocket_server.cpp server/http_and_websocket_server.h)

target_link_libraries(http jsoncpp ssl crypto)