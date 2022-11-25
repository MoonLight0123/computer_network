#include<winsock2.h>
#include<stdio.h>
#include<iostream>
#include<assert.h>
#include"myhead.h"
#include"send.h"
#include<algorithm>
#include<windows.h>
#pragma comment(lib,"ws2_32.lib")
int windowSize=25000;
int timeoutData;
int fileNameLen;
int i;
bool acked[dataBufferNum];
int lastAckDataNum;
int main()
{
    init();
    while(1)
    {
        shakeHands();
        fseek(p,0,SEEK_END);
        int fileLen=ftell(p);
        long long tempFileLen=fileLen;
        fseek(p,0,SEEK_SET);
        //fread(&buffer,maxBuffersize,1,p);
        printf("start send filelen=%d\n",fileLen);
        dataNum=0;
        while(fileLen>0)
        {
            //buffer[dataNum].dataLen=std::min(maxDataBufferSize,fileLen);
            buffer[dataNum].dataLen=maxDataBufferSize<fileLen?maxDataBufferSize:fileLen;
            fread(&buffer[dataNum].data,buffer[dataNum].dataLen,1,p);
            buffer[dataNum].seqnum=nextSeqNum;
            buffer[dataNum].rwnd=dataNum;
            nextSeqNum+=buffer[dataNum].dataLen;
            fileLen-=buffer[dataNum].dataLen;
            //showDataInfo(buffer[dataNum]);
            //printf("fileLen:%d dataNum:%d ",fileLen,dataNum);
            dataNum++;
        }
        gettimeofday(start,NULL);
        CreateThread(NULL,NULL,&receiveThread,NULL,0,NULL);
        i=0;
        while(i<dataNum)
        {
            if(buffer[i].seqnum+maxDataBufferSize>=sendBase+windowSize)
            {
                Sleep(200);
                continue;
            }
            sd(i);
            if(noTimer)
            {
                noTimer=0;
                //timeoutData=&buffer[i];
                timeoutData=i;
                //printf("set timer for1 %d\n",i);
                CreateThread(NULL,NULL,&handleTimer,NULL,0,NULL);
            }
            i++;
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
DWORD WINAPI receiveThread(LPVOID lparam)
{
    while(1)
    {
        memset(&receiveData,0,datagramHeaderLen);
        rd();
        if(receiveData.fin==1)
        {
            KillTimer(NULL,tid);
            printf("finish!\n");
            break;
        }
        checkSum(receiveData);
        if(receiveData.ack==1&&receiveData.acknum>sendBase)
        {
            int ackDataNum=(receiveData.acknum-fileNameLen-1)/maxDataBufferSize-1;
            for(int j=lastAckDataNum;j<=ackDataNum;j++)
                acked[j]=1;
            lastAckDataNum=ackDataNum;
            KillTimer(NULL,tid);
            //printf("kill timer for %d\n",ackDataNum);
            sendBase=receiveData.acknum;
            showDataInfo(receiveData);
            //printf("i:%d,ackDataNum%d\n",i,ackDataNum);
            if(i!=ackDataNum)
            {
                //timeoutData=&buffer[ackDataNum+1];
                timeoutData=ackDataNum+1;
                //printf("set timer for2 %d\n",ackDataNum+1);
                CreateThread(NULL,NULL,&handleTimer,NULL,0,NULL);
                continue;
            }
            noTimer=1;
        }
    }
}
void shakeHands()
{
    memset(&sendData,0,datagramLen);
    sendData.syn=1;
    sendto(clientSocket,(char*)&sendData,datagramLen,0,(SOCKADDR*)&destinationAddrInfo,addrLen);

    memset(&receiveData,0,datagramHeaderLen);
    rd();
    assert(receiveData.ack==1&&receiveData.syn==1&&receiveData.acknum==1);
    showDataInfo(receiveData);

    memset(&sendData,0,datagramLen);
    nextSeqNum=1;//sendBase=1;
    sendData.ack=1;sendData.acknum=1;sendData.seqnum=1;
    sendto(clientSocket,(char*)&sendData,datagramLen,0,(SOCKADDR*)&destinationAddrInfo,addrLen);

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

    sendData.seqnum=nextSeqNum;fileNameLen=sendData.dataLen=strlen(fileName);
    sendto(clientSocket,(char*)&sendData,datagramLen,0,(SOCKADDR*)&destinationAddrInfo,addrLen);
    memset(&receiveData,0,datagramHeaderLen);
    rd();
    assert(receiveData.ack==1&&receiveData.acknum==nextSeqNum+sendData.dataLen);
    nextSeqNum+=sendData.dataLen;
    sendBase=nextSeqNum;
    noTimer=1;
}
void waveHands()
{
    memset(&sendData,0,datagramLen);
    sendData.fin=1;sendData.seqnum=nextSeqNum;
    sendto(clientSocket,(char*)&sendData,datagramLen,0,(SOCKADDR*)&destinationAddrInfo,addrLen);
}
DWORD WINAPI handleTimer(LPVOID lparam)
{
    int tempId=SetTimer(NULL,50,timeInterval*maxResend*3,NULL);
    tid=SetTimer(NULL,timerId,timeInterval,(TIMERPROC)TimerProc);
    MSG msg;
    for(int i=0;i<maxResend;i++)
    {
        GetMessage(&msg, NULL, 0, 0);
        DispatchMessage(&msg);
    }
    KillTimer(NULL,tempId);
}
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent,DWORD dwTime)
{
    int t=i;
    printf("timeout!\n");
    for(int j=timeoutData;j<=t;j++){
        printf("resend ");
        showDataInfo(buffer[j]);
        sd(j);
    }
}
void sd(int i)
{
    if(SOCKET_ERROR==sendto(clientSocket,(char*)&buffer[i],datagramLen,0,(SOCKADDR*)&destinationAddrInfo,addrLen))
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