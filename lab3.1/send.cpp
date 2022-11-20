#include<winsock2.h>
#include<stdio.h>
#include<iostream>
#include<assert.h>
#include"myhead.h"
#include"send.h"
#include<windows.h>
#pragma comment(lib,"ws2_32.lib")
datagram receiveData,sendData;
SOCKADDR_IN destinationAddrInfo;
SOCKET clientSocket;
FILE* p;
const char ip[15]="127.0.0.1";
const int routerPort=5000;
int nextSeqNum;
int tid;
timeval *start=new timeval();
timeval *stop=new timeval();
double durationTime=0.0;
int main()
{
    init();
    while(1)
    {
        shakeHands();
        fseek(p,0,SEEK_END);
        long long fileLen=ftell(p);
        long long tempFileLen=fileLen;
        fseek(p,0,SEEK_SET);
        printf("start send filelen=%d\n",fileLen);
        gettimeofday(start,NULL);
        while(fileLen>0)
        {
            memset(&sendData,0,datagramLen);
            fread(&sendData.data,maxBufferSize,1,p);
            if(fileLen>=maxBufferSize)
            {
                sendData.dataLen=maxBufferSize;
            }
            else
            {
                sendData.dataLen=fileLen;
            }
            sendData.seqnum=nextSeqNum;
            rsd();
            fileLen-=sendData.dataLen;
            nextSeqNum+=sendData.dataLen;
        }
        waveHands();
        gettimeofday(stop,NULL);
        durationTime =stop->tv_sec*1000+double(stop->tv_usec)/1000-start->tv_sec*1000-double(start->tv_usec)/1000;
        printf("file size:%d bytes\n",tempFileLen);
        printf("file transmission time:%f ms\n",durationTime);
        printf("throughput:%f Mbit\\s\n",tempFileLen*8000/durationTime/1024/1024);
        int t;
        printf("send next picture?\n1:yes 2:no\n");
        scanf("%d",&t);
        fclose(p);
        p = nullptr;
        if(t==2)break;
    }
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
void shakeHands()
{
    memset(&sendData,0,datagramLen);
    sendData.syn=1;
    sd();

    memset(&receiveData,0,datagramHeaderLen);
    rd();
    assert(receiveData.ack==1&&receiveData.syn==1&&receiveData.acknum==1);
    showDataInfo(receiveData);

    memset(&sendData,0,datagramLen);
    nextSeqNum=1;
    sendData.ack=1;sendData.acknum=1;sendData.seqnum=1;
    sd();

    char filePath[50]="";
    printf("please enter file path\n");
    scanf("%s",&filePath);
    if(!(p=fopen(filePath,"rb+")))
    {
        printf("wrong file name\n");
    }
    char fileName[50]={0};
    getFileName(filePath,fileName);
    printf("file name:%s\n",fileName);
    memset(&sendData,0,datagramLen);
    strcpy(sendData.data,fileName);

    sendData.seqnum=nextSeqNum;sendData.dataLen=strlen(fileName);
    sd();
    memset(&receiveData,0,datagramHeaderLen);
    rd();
    assert(receiveData.ack==1&&receiveData.acknum==nextSeqNum+sendData.dataLen);
    nextSeqNum+=sendData.dataLen;
}
void waveHands()
{
    memset(&sendData,0,datagramLen);
    sendData.fin=1;sendData.seqnum=nextSeqNum;
    rsd();
}
DWORD WINAPI handleTimer(LPVOID lparam)
{
    int tempId=(NULL,50,timeInterval*maxResend,NULL);
    tid=SetTimer(NULL,timerId,timeInterval,(TIMERPROC)TimerProc);
    MSG msg;
    for(int i=0;i<maxResend;i++)
    {
        GetMessage(&msg, NULL, 0, 0);
        DispatchMessage(&msg);
    }
    KillTimer(NULL,tempId);
}
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
    sd();
    printf("time out!!!\n");
}
void rsd()
{
    checkSum(sendData);
    sd();
    CreateThread(NULL,NULL,&handleTimer,NULL,0,NULL);
    while(1)
    {
        memset(&receiveData,0,datagramHeaderLen);
        rd();
        checkSum(receiveData);
        if(receiveData.ack==1&&receiveData.acknum==nextSeqNum+sendData.dataLen)
        {
            KillTimer(NULL,tid);
            showDataInfo(receiveData);
            break;
        }
        else
        {
            KillTimer(NULL,tid);
            printf("wrong ack! ");
            showDataInfo(receiveData);
            sd();
            CreateThread(NULL,NULL,&handleTimer,NULL,0,NULL);
        }
    }
}
void sd()
{
    if(SOCKET_ERROR==sendto(clientSocket,(char*)&sendData,datagramLen,0,(SOCKADDR*)&destinationAddrInfo,addrLen))
    {
        printf("send error\n");
    }
}
void rd()
{
    if(SOCKET_ERROR==recvfrom(clientSocket,(char*)&receiveData,datagramLen,0,(SOCKADDR*)&destinationAddrInfo,&addrLen))
    {
        printf("receive wrong\n");
    }
}
void init()
{
    WSADATA wsaData;
    int err;
    err=WSAStartup(MAKEWORD(1,1),&wsaData);
    if(!err)printf("succeed to open send socket\n");
    else printf("wrong \n");

    clientSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(clientSocket==-1)
    {
        printf("worong!\n");
    }
    destinationAddrInfo.sin_addr.S_un.S_addr=inet_addr(ip);
    destinationAddrInfo.sin_family=AF_INET;
    destinationAddrInfo.sin_port=htons(routerPort);
}
void getFileName(char* filePath,char* fileName)
{
    int len=strlen(filePath);
    int count=0;
    for(int i=len-1;i>0;i--)
    {
        if(filePath[i]!='\\'&&filePath[i]!='/')
            count++;
        else break;
    }
    int j=0;
    int pos=len-count;
    for(int i=pos;i<len;i++)
    {
        fileName[j++]=filePath[i];
    }
}