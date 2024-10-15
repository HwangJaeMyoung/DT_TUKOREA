#include "TimeManager.h"

String checkTime() {
    // 현재 시간 계산
	unsigned long currentMillis = millis();
	unsigned long elapsedMillis = currentMillis - initialMillis;
	unsigned long currentEpoch = initialEpoch + elapsedMillis / 1000;

	// 시간 구조체 생성
	tmElements_t tm;
	breakTime(currentEpoch, tm);

	// 밀리초 단위 계산
	int milliseconds = elapsedMillis % 1000;

	// 현재 시간 문자열로 변환
	char time[30];
	sprintf(time, "%04d_%02d_%02d-%02d_%02d_%02d.%03d",
		tmYearToCalendar(tm.Year), tm.Month, tm.Day,
		tm.Hour, tm.Minute, tm.Second, milliseconds);

	return String(time);
}