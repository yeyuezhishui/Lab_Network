#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include<utility>
typedef std::pair<Packet, bool> RecvPkt;

class SRRdtReceiver :public RdtReceiver
{
private:
	int base;				// 窗口起始
	int nextSeqNum;			// 下一个发送的序号
	const int N;			// 窗口大小
	RecvPkt *pktQueue;		//缓存窗口队列

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:

	void receive(Packet &packet);	//接收报文，将被NetworkService调用
};

#endif

