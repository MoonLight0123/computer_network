#include<winsock2.h>
#include<stdio.h>
#include<iostream>
#include<assert.h>
#include"myhead.h"
#include"send.h"
#include<algorithm>
#include<windows.h>
#pragma comment(lib,"ws2_32.lib")
int rwnd=80000;
int timeoutData;
int fileNameLen;
int i;
bool acked[dataBufferNum];
int lastAckDataNum;
int sendBase;
float cwnd;
int ssthresh;
int duplicateAck;
int duplicateAckCount;
int state;
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
        printf("start send filelen=%d\n",fileLen);
        dataNum=0;
        while(fileLen>0)
        {
            buffer[dataNum].dataLen=maxDataBufferSize<fileLen?maxDataBufferSize:fileLen;
            fread(&buffer[dataNum].data,buffer[dataNum].dataLen,1,p);
            buffer[dataNum].seqnum=nextSeqNum;
            buffer[dataNum].num=dataNum;
            nextSeqNum+=buffer[dataNum].dataLen;
            fileLen-=buffer[dataNum].dataLen;
            dataNum++;
        }
        gettimeofday(start,NULL);
        CreateThread(NULL,NULL,&receiveThread,NULL,0,NULL);
        i=0;
        while(i<dataNum)
        {
            int cwndSize=int(cwnd)*datagramLen;
            int temp=cwndSize<rwnd?cwndSize:rwnd;
            if(buffer[i].seqnum>=sendBase+temp)
            {
                Sleep(150);
                continue;
            }
            for(int j=0;j<cwnd&&i<dataNum;j++)
            {
                sd(i);
                printf("send ");
                showDataInfo(buffer[i++]);
            }
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
        printf("receive ");
        showDataInfo(receiveData);
        checkSum(receiveData);
        if(receiveData.ack==1&&receiveData.acknum>sendBase)
        {
            KillTimer(NULL,tid);
            rwnd=receiveData.rwnd;
            int ackDataNum=receiveData.num;
            memset(acked+lastAckDataNum+1,1,ackDataNum-lastAckDataNum);
            while(acked[++ackDataNum]==1);
            int ackCount=ackDataNum-1-lastAckDataNum;
            lastAckDataNum=ackDataNum-1;
            switch(state)
            {
                case slowStart:
                    cwnd+=ackCount;
                    if(cwnd>=ssthresh)
                        state=congestionAvoidance;
                    break;
                case congestionAvoidance:
                    cwnd+=ackCount/cwnd;
                    break;
                case fastRecovery:
                    cwnd=ssthresh;
                    state=congestionAvoidance;
                    break;
                default:
                    printf("wrong");
            }
            printstate();
            duplicateAckCount=0;

            datagram* d=&buffer[lastAckDataNum];
            sendBase=d->seqnum+d->dataLen;

            timeoutData=ackDataNum;
            CreateThread(NULL,NULL,&handleTimer,NULL,0,NULL);
        }
        else
        {
            if(duplicateAckCount==0)
            {
                duplicateAck=receiveData.acknum;
            }
            if(receiveData.acknum==duplicateAck)
            {
                duplicateAckCount++;
                if(state==fastRecovery){
                    cwnd++;
                }
            }
            if(duplicateAckCount==3)
            {
                switch(state)
                {
                    case slowStart:
                        ssthresh=int(cwnd)>>1;
                        cwnd=ssthresh+3;
                        state=fastRecovery;
                        break;
                    case congestionAvoidance:
                        ssthresh=int(cwnd)>>1;
                        cwnd=ssthresh+3;
                        state=fastRecovery;
                        break;
                    case fastRecovery:
                        break;
                    default:
                        printf("wrong");
                }
                printstate();
                printf("quick resend ");
                KillTimer(NULL,tid);
                sd(receiveData.num+1);
                timeoutData=receiveData.num+1;
                CreateThread(NULL,NULL,&handleTimer,NULL,0,NULL);
                duplicateAckCount=0;
            }
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
    timerNum=0;
    receiveData.rwnd=35000;
    cwnd=2.0;
    state=slowStart;
    ssthresh=16;
}
void waveHands()
{
    memset(&sendData,0,datagramLen);
    sendData.fin=1;sendData.seqnum=nextSeqNum;
    sendto(clientSocket,(char*)&sendData,datagramLen,0,(SOCKADDR*)&destinationAddrInfo,addrLen);
}
DWORD WINAPI handleTimer(LPVOID lparam)
{
    int tempId=SetTimer(NULL,tempTimerId,timeInterval*maxResend*3,NULL);
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
    printf("timeout!\n");
    printf("resend ");
    showDataInfo(buffer[timeoutData]);
    sd(timeoutData);
    ssthresh=int(cwnd)>>1;
    cwnd=1;
    duplicateAckCount=0;
    state=slowStart;
    printstate();
    // switch(state)
    // {
    //     case slowStart:
    //         ssthresh=int(cwnd)>>1;
    //         cwnd=1;
    //         break;
    //     case congestionAvoidance:
    //         ssthresh=int(cwnd)>>1;
    //         cwnd=1;
    //         state=slowStart;
    //         break;
    //     case fastRecovery:
    //         ssthresh=int(cwnd)>>1;
    //         cwnd=1;
    //         state=slowStart;
    //         break;
    //     default:
    //         printf("wrong");
    // }
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
void printstate()
{
    printf("state:");
    if(state==slowStart)
        printf("slowStart ");
    else if(state==congestionAvoidance)
        printf("congestionAvoidance ");
    else printf("fastRecovery ");
    printf("cwnd:%f ssthresh:%d\n",cwnd,ssthresh);
}