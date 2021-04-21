#include "server/server.h"
#include "util/io_helper.h"

int main() {
    server s;
    s.ip(INADDR_ANY)
    .port(9119)
    .build();

    s.setStaticPath("/home/saturn/Documents");

    s.post("/saturn", [](const HttpRequest& request, HttpResponse& response){
        auto* jsonData = dynamic_cast<JsonData*>(request.getBody());
        std::cout << jsonData->mJson["name"] << std::endl;
        response.getBody()->setData("Hello from saturn", 17);
    });

    s.start();

    return 0;
}