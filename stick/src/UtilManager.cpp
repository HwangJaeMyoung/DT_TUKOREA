#include "UtilManager.h"

String str(const char* rc){
	char rcstr[50];
	sprintf(rcstr, "ICCMS/%s/%s/%s/%s/%d/%s", location, subLocation, part, sensorType, sensorIndex, rc);

	return String(rcstr);
}