#include "stdafx.h"
#include "Global.h"
#include "SRRdtReceiver.h"
#include<algorithm>

namespace ReceiverConfig {
	int windowLen = 4; //���ڳ���
	int maxSeqNum = 8;	//��Ϊ��3λ���ֱ�ʾ, �����ģ8
}


SRRdtReceiver::SRRdtReceiver() :base(0), nextSeqNum(0), N(ReceiverConfig::windowLen)
{
	Packet ackPkt;
	ackPkt.acknum = -1; //���Ը��ֶ�
	ackPkt.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		ackPkt.payload[i] = '.';
	}
	ackPkt.checksum = -1;

	pktQueue = new RecvPkt[N];
	for (int i = 0; i < N; i++) {
		pktQueue[i] = { ackPkt , 0 };
	}
}

SRRdtReceiver::~SRRdtReceiver()
{
	delete[]pktQueue;
}

int seqPos(int base, int N, int seq) {
	if (base < (base + N) % ReceiverConfig::maxSeqNum) {
		if (seq < base)
			return -1;
		else if (base <= seq && seq < base + N)
			return 0;
		else
			return -1;
	}
	else {
		if (seq < base && seq >= base - N)
			return -1;
		else if (ReceiverConfig::maxSeqNum - seq <= ReceiverConfig::maxSeqNum - base || seq < (base + N) % ReceiverConfig::maxSeqNum)
			return 0;
		else
			return -1;
	}
}

void SRRdtReceiver::receive(Packet &packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum) {
		//���ڴ�����
		if (seqPos(base, N, packet.seqnum) == 0) {

			pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);

			if (pktQueue[((packet.seqnum - base) + ReceiverConfig::maxSeqNum) % ReceiverConfig::maxSeqNum].second == 0)//δȷ�ϵ����
			{
				pktQueue[((packet.seqnum - base) + ReceiverConfig::maxSeqNum) % ReceiverConfig::maxSeqNum] = { packet, 1 };
			}
			Packet ackPkt;
			ackPkt.acknum = packet.seqnum; //ȷ��������
			ackPkt.seqnum = -1;	//���Ը��ֶ�
			for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
				ackPkt.payload[i] = '.';
			}
			ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", ackPkt);
			pns->sendToNetworkLayer(SENDER, ackPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�

			if (packet.seqnum == base) //��Ҫ�ƶ�����
			{
				cout << "---�����շ��ƶ������ڴ�ȷ���ݣ�\t";
				for (int i = 0; i < N; i++) {
					if (pktQueue[i].second)
					{
						cout << pktQueue[i].first.seqnum% ReceiverConfig::maxSeqNum << " ";
					}
					else
						break;
				}
				cout << endl;

				int slideCount = 0;
				for (int i = 0; i < N; i++) {
					if (pktQueue[i].second) {
						Message msg; //ȡ��Message�����ϵݽ���Ӧ�ò�
						memcpy(msg.data, pktQueue[i].first.payload, sizeof(pktQueue[i].first.payload));
						pns->delivertoAppLayer(RECEIVER, msg);
						pktQueue[i].second = 0;
						base = (base + 1) % ReceiverConfig::maxSeqNum;
						slideCount++;
					}
					else
						break;
				}
				for (int i = 0; i < N; i++) {
					if (pktQueue[i].second) 
					{
						if (i >= slideCount)
						{
							pktQueue[i].second = 0;
							pktQueue[i - slideCount].second = 1;
							pktQueue[i - slideCount].first = pktQueue[i].first;
						}
					}
				}

				cout << "---�����շ��ƶ����ں����ݣ�\t";
				cout<<(base+slideCount-1)% ReceiverConfig::maxSeqNum << " ";
				cout << (base + slideCount) % ReceiverConfig::maxSeqNum << " ";
				cout << (base + slideCount+1) % ReceiverConfig::maxSeqNum << " ";
				cout << (base + slideCount+2) % ReceiverConfig::maxSeqNum << " ";
				cout << endl;
			}
		}

		//������ȷ�ϵ�����
		else if (seqPos(base, N, packet.seqnum) == -1) 
		{
			//ֱ��ȷ��
			Packet ackPkt;
			ackPkt.acknum = packet.seqnum; //ȷ��������

			ackPkt.seqnum = -1;	//���Ը��ֶ�
			for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
				ackPkt.payload[i] = '.';
			}

			ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", ackPkt);
			pns->sendToNetworkLayer(SENDER, ackPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		}
	}
	else {
		cout << "���շ����յ��ı�����" << endl;
	}

}