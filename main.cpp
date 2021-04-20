#include "server/server.h"
#include <stdio.h>
#include <dirent.h>

int main() {
//    server s;
//    s.ip(INADDR_ANY)
//    .port(9119)
//    .build();
//
//    s.post("/saturn", [](const HttpRequest& request, HttpResponse& response){
//        auto* jsonData = dynamic_cast<JsonData*>(request.getBody());
//        std::cout << jsonData->mJson["name"] << std::endl;
//        response.getBody()->setData("Hello from saturn", 17);
//    });
//
//    s.start();

    struct dirent *entry = nullptr;
    DIR *dp = nullptr;

    dp = opendir("/home/saturn/Pictures");
    if (dp != nullptr) {
        while ((entry = readdir(dp)))
            printf ("%s\n", entry->d_name);
    }

    closedir(dp);

    return 0;
}