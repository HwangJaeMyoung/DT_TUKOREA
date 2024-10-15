#include "SensorManager.h"

void initialize_sensors() {
    WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitSerialWriteRegister(SensorUartSend);
	WitRegisterCallBack(SensorDataUpdata);
  	WitDelayMsRegister(Delayms);
	WitSetOutputRate(RRATE_200HZ);
	AutoScanSensor();
}

static void SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
	Serial2.write(p_data, uiSize);
	Serial2.flush();
}
static void Delayms(uint16_t ucMs)
{
	delay(ucMs);
}
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum)
{
	int i;
    for(i = 0; i < uiRegNum; i++)
    {
		if(uiReg == AZ){
			s_cDataUpdate |= ACC_UPDATE;
			break;
		} 
		uiReg++;
    }
}

static void AutoScanSensor(void)
{
	int i, iRetry;
	
	for(i = 0; i < sizeof(c_uiBaud)/sizeof(c_uiBaud[0]); i++)
	{
		Serial2.begin(c_uiBaud[i]);
    	Serial2.flush();
		iRetry = 2;
		s_cDataUpdate = 0;
		do
		{
			WitReadReg(AX, 3);
			delay(200);
		while (Serial2.available())
		{
			WitSerialDataIn(Serial2.read());
		}
			if(s_cDataUpdate != 0)
			{
				return ;
			}
			iRetry--;
		}while(iRetry);		
	}
}
