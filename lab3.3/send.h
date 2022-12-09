#include<windows.h>
#include <time.h>
#define slowStart 1
#define congestionAvoidance 2
#define fastRecovery 3
#define timerId 100
#define tempTimerId 50
void getFileName(char* filePath,char* fileName);
void CALLBACK TimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime);
void sd(int i);
void rd();
void init();
void shakeHands();
void waveHands();
void printstate();
DWORD WINAPI receiveThread(LPVOID lparam);
DWORD WINAPI handleTimer(LPVOID lparam);
int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return 0;
}
SOCKET clientSocket;
SOCKADDR_IN destinationAddrInfo;
FILE* p;
const char ip[15]="127.0.0.1";
const int routerPort=5000;
datagram sendData,receiveData;
datagram buffer[dataBufferNum];
int dataNum;
int nextSeqNum;
int tid;
int timerNum;
timeval *start=new timeval();
timeval *stop=new timeval();
double durationTime=0.0;