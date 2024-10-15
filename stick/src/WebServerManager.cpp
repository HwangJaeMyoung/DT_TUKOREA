#include "WebServerManager.h"

void functionA() {
    webserver.send(200, "text/plain", "Power On");
}

void handleNotFound() {
    webserver.send(404, "text/plain", "404: Not Found");
}