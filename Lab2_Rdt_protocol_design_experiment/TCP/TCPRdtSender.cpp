#include "stdafx.h"
#include "Global.h"
#include "TCPRdtSender.h"

namespace Config {
	int windowLen = 4; //���ڳ���
	int maxSeqNum = 8;	//������
}

TCPRdtSender::TCPRdtSender():base(1), nextSeqNum(1), N(Config::windowLen)
{
	timeoutCount = new int[Config::maxSeqNum];
	for (int i = 0; i < Config::maxSeqNum; i++) 
	{
		timeoutCount[i] = 0;
	}
}


TCPRdtSender::~TCPRdtSender()
{
	delete[]timeoutCount;
}



bool TCPRdtSender::getWaitingState() {
	/*return (nextSeqNum - base + Config::maxSeqNum) % Config::maxSeqNum >= N;*/
	return (nextSeqNum - base + Config::maxSeqNum) % Config::maxSeqNum >= N;
}


bool TCPRdtSender::send(Message &message) {
	if (this->getWaitingState()) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}

	//����Ҫ���͵İ�
	Packet &sendPkt = *new Packet;
	sendPkt.acknum = -1; //���Ը��ֶ�
	sendPkt.seqnum = this->nextSeqNum;
	sendPkt.checksum = 0;
	memcpy(sendPkt.payload, message.data, sizeof(message.data));
	sendPkt.checksum = pUtils->calculateCheckSum(sendPkt);
	pUtils->printPacket("���ͷ����ͱ���", sendPkt);


	pktQueue.push_back(sendPkt);	//���뻺�����
	if (base == nextSeqNum) {
		timerSeq = base;
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);			//�������ͷ���ʱ��
	}
	pns->sendToNetworkLayer(RECEIVER, sendPkt);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	
	nextSeqNum = ++nextSeqNum % Config::maxSeqNum;
	return true;
}

//�ж��Ƿ��������
bool isDuplicate(int base, int N, int seq) {
	if (base < (base + N) % Config::maxSeqNum) {
		if (seq > base)
			return false;
		else
			return true;
	}
		
	else {
		if (seq <= base && seq > (base + N) % Config::maxSeqNum)
			return true;
		else
			return false;
	}
}

void TCPRdtSender::receive(Packet &ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum) {
		pUtils->printPacket("��ȷ���յ����շ�������ȷ�ϰ���", ackPkt);
		if(!isDuplicate(base, N, ackPkt.acknum))
		{
			//δ�������࣬�����ۼƼ�������������Ҫ��������
			cout << "---���ƶ�����ǰ���ݣ�\t";
			for (auto i : pktQueue)
			{
				cout << i.seqnum << " ";
			}
			cout << endl;

			for (int i = 0; i < (ackPkt.acknum - base + Config::maxSeqNum) % Config::maxSeqNum; i++)
				pktQueue.pop_front();
			base = (ackPkt.acknum) % Config::maxSeqNum;	
			if (base == nextSeqNum)
				pns->stopTimer(SENDER, timerSeq);		//�رն�ʱ��
			else {
				pns->stopTimer(SENDER, timerSeq);
				timerSeq = base;
				pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);
			}

			cout << "---���ƶ����ں����ݣ�\t";
			for (auto i : pktQueue) {
				cout << i.seqnum << " ";
			}
			cout << endl;
		}
		else {
			timeoutCount[ackPkt.acknum]++;
			if (timeoutCount[ackPkt.acknum] == 3) {	//3������ack
				cout << "3������ack";
				for (auto i : pktQueue) {
					if (i.seqnum == ackPkt.acknum) {
						pns->sendToNetworkLayer(RECEIVER, i);//�����ش�
						pUtils->printPacket("---�������ش�����", i);
					}
				}
				timeoutCount[ackPkt.acknum] = 0;
			}
		}
	}
}

void TCPRdtSender::timeoutHandler(int seqNum) {
	cout << "������ʱ";
	if (base != nextSeqNum) { //������������
		cout << "ֻ�ش��� �緢����û��ȷ�ϵı���";
		pns->stopTimer(SENDER, seqNum);
		pns->sendToNetworkLayer(RECEIVER, pktQueue.front());	//��ֻ�ش��� �緢����û��ȷ�ϵı���
		pns->startTimer(SENDER, Configuration::TIME_OUT, pktQueue.front().seqnum);
	}
	else {
		pns->stopTimer(SENDER, seqNum);
	}

}
