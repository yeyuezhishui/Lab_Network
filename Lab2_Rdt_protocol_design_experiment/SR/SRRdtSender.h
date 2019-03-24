#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include<vector>

class SRRdtSender :public RdtSender
{
private:
	int base;				// ������ʼ
	int nextSeqNum;			// ��һ�����͵����
	const int N;			// ���ڴ�С
	int timerSeq;			//timer �����
	std::vector<std::pair<Packet, bool>> pktQueue;		// ָ�򴰿ڰ����Ķ��packet

public:

	bool getWaitingState();
	bool send(Message &message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(Packet &ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};

#endif

