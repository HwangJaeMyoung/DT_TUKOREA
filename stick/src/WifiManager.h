#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include "Globals.h"
#include "PowerManager.h"
#include "TimeManager.h"

void setup_wifi();
void reconnect(String clientId);
int connect_first(String clientId);

#endif
