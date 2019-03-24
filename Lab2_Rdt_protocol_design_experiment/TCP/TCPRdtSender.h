#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include<vector>
#include<list>

class TCPRdtSender :public RdtSender
{
private:
	int base;				// 窗口起始
	int nextSeqNum;			// 下一个发送的序号
	const int N;			// 窗口大小
	int timerSeq;			//序号
	list<Packet> pktQueue;		// 指向窗口包含的多个packet
	int *timeoutCount;		//记录当前冗余

public:

	bool getWaitingState();
	bool send(Message &message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(Packet &ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用

public:
	TCPRdtSender();
	virtual ~TCPRdtSender();
};

#endif

