#ifndef GBN_SENDER_H
#define GBN_SENDER_H
#include "RdtSender.h"
#include <vector>


class GBNRdtSender :public RdtSender
{
private:
	int base;		//窗口起始，最早未被确认或最初未发送的报文段
	int nextseqnum;			//最早未发送
	int timerseq;		//timer的序号
	const int N;		//窗口的大小
	std::vector<Packet> pktQueue;		//窗口中的报文段队列
public:

	bool getWaitingState();
	bool send(Message &message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(Packet &ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif

