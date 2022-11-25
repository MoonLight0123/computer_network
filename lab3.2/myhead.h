#include<stdio.h>
#define timerId 100
const int maxDataBufferSize=4086;
const int timeInterval=200;
const int maxResend=2;
const int maxWindowSize=29999;
const int maxBuffersize=1e8;
struct datagram
{
    int ack,syn,fin;
    unsigned short checksum;
    int seqnum,acknum;
    int dataLen;
    int rwnd;
    char data[maxDataBufferSize];
};
int addrLen=sizeof(SOCKADDR);
int datagramLen=sizeof(datagram);
int datagramHeaderLen=datagramLen-maxDataBufferSize;
void showDataInfo(datagram& d)
{
    printf("datanum:%d ack:%d syn:%d fin:%d seqnum:%d acknum:%d checksum:%d datalen:%d\n",d.rwnd,d.ack,d.syn,d.fin,d.seqnum,d.acknum,d.checksum,d.dataLen);
}
void checkSum(datagram &d)
{
    unsigned short* buff=(unsigned short*)&d;
    int count=datagramLen/sizeof(short);
    d.checksum=0;
    unsigned long sum=0;
    while(count--)
    {
        sum+=*buff++;
        if(sum&0xffff0000)
        {
            sum&=0xffff;
            sum++;
        }
    }
    d.checksum=~(sum&0xffff);
}