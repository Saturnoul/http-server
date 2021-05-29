#include "server/http_and_websocket_server.h"
#include <iostream>

int main() {
    http_and_websocket_server s;
    s.ip(INADDR_ANY)
    .port(9119)
    .build();

    websocket_handler handler;
    handler.onMessage = [](const WebsocketSession& session, const std::string& msg) {
        std::cout << "Message: " << msg << std::endl;
        session.sendMessage("已经收到了！");
    };

    handler.onOpen = [](const WebsocketSession& session) {
        session.sendMessage("Welcome");
    };

    handler.onClose = [](const WebsocketSession& session) {
        std::cout << session.getId() << " has left" << std::endl;
    };

    s.addEndPoint("/saturn", handler);

    s.setStaticPath("/home/saturn/Pictures");

    s.post("/saturn", [](const HttpRequest& request, HttpResponse& response){
//        std::string msg;
//        auto session = request.getSession();
//        auto name = session.get("name");
//        std::cout << "Welcome: " << name << std::endl;
        auto json = dynamic_cast<JsonData*>(response.getBody());
    });

    s.post("/login", [](const HttpRequest& request, HttpResponse& response){
        auto* jsonData = dynamic_cast<JsonData*>(request.getBody());
        auto name = jsonData->mJson["name"].asString();
        auto& session = request.getSession();
        session.set("name", "luohao");
    });

    s.start_with_epoll();

    return 0;
}
