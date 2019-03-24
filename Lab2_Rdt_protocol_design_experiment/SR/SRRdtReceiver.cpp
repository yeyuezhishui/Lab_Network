#include "stdafx.h"
#include "Global.h"
#include "SRRdtReceiver.h"
#include<algorithm>

namespace ReceiverConfig {
	int windowLen = 4; //窗口长度
	int maxSeqNum = 8;	//因为是3位数字表示, 故序号模8
}


SRRdtReceiver::SRRdtReceiver() :base(0), nextSeqNum(0), N(ReceiverConfig::windowLen)
{
	Packet ackPkt;
	ackPkt.acknum = -1; //忽略该字段
	ackPkt.seqnum = -1;	//忽略该字段
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
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum) {
		//落在窗口内
		if (seqPos(base, N, packet.seqnum) == 0) {

			pUtils->printPacket("接收方正确收到发送方的报文", packet);

			if (pktQueue[((packet.seqnum - base) + ReceiverConfig::maxSeqNum) % ReceiverConfig::maxSeqNum].second == 0)//未确认的情况
			{
				pktQueue[((packet.seqnum - base) + ReceiverConfig::maxSeqNum) % ReceiverConfig::maxSeqNum] = { packet, 1 };
			}
			Packet ackPkt;
			ackPkt.acknum = packet.seqnum; //确认这个序号
			ackPkt.seqnum = -1;	//忽略该字段
			for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
				ackPkt.payload[i] = '.';
			}
			ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
			pUtils->printPacket("接收方发送确认报文", ackPkt);
			pns->sendToNetworkLayer(SENDER, ackPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

			if (packet.seqnum == base) //需要移动窗口
			{
				cout << "---》接收方移动窗口内待确认容：\t";
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
						Message msg; //取出Message，向上递交给应用层
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

				cout << "---》接收方移动窗口后内容：\t";
				cout<<(base+slideCount-1)% ReceiverConfig::maxSeqNum << " ";
				cout << (base + slideCount) % ReceiverConfig::maxSeqNum << " ";
				cout << (base + slideCount+1) % ReceiverConfig::maxSeqNum << " ";
				cout << (base + slideCount+2) % ReceiverConfig::maxSeqNum << " ";
				cout << endl;
			}
		}

		//落在已确认的区域
		else if (seqPos(base, N, packet.seqnum) == -1) 
		{
			//直接确认
			Packet ackPkt;
			ackPkt.acknum = packet.seqnum; //确认这个序号

			ackPkt.seqnum = -1;	//忽略该字段
			for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
				ackPkt.payload[i] = '.';
			}

			ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
			pUtils->printPacket("接收方发送确认报文", ackPkt);
			pns->sendToNetworkLayer(SENDER, ackPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		}
	}
	else {
		cout << "接收方接收到的报文损坏" << endl;
	}

}