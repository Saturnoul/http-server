#include "server/server.h"

int main() {
    server s;
    s.ip(INADDR_ANY)
    .port(9119)
    .build();

    s.setStaticPath("/home/saturn/Pictures");

    s.post("/saturn", [](const HttpRequest& request, HttpResponse& response){
        auto* jsonData = dynamic_cast<JsonData*>(request.getBody());
        auto name = jsonData->mJson["name"].asString();
        response.getBody()->setData("Hello from saturn", 17);
    });

    s.start();

    return 0;
}