#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include<vector>
#include<list>

class TCPRdtSender :public RdtSender
{
private:
	int base;				// ������ʼ
	int nextSeqNum;			// ��һ�����͵����
	const int N;			// ���ڴ�С
	int timerSeq;			//���
	list<Packet> pktQueue;		// ָ�򴰿ڰ����Ķ��packet
	int *timeoutCount;		//��¼��ǰ����

public:

	bool getWaitingState();
	bool send(Message &message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(Packet &ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	TCPRdtSender();
	virtual ~TCPRdtSender();
};

#endif

