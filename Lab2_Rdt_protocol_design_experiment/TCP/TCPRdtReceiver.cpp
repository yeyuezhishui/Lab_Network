#include "stdafx.h"
#include "Global.h"
#include "TCPRdtReceiver.h"

namespace ReceiverConfig {
	int windowLen = 4; //窗口长度
	int maxSeqNum = 8;	//最大序号， 因为是3位数字表示
}


TCPRdtReceiver::TCPRdtReceiver():expectSequenceNumberRcvd(1)
{
	lastAckPkt.acknum = 1; //ack回应下一个希望收到的包序号
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	expectSequenceNumberRcvd = 1; //初始希望收到1
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

TCPRdtReceiver::~TCPRdtReceiver()
{
}

void TCPRdtReceiver::receive(Packet &packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && this->expectSequenceNumberRcvd == packet.seqnum) {
		pUtils->printPacket("接收方正确收到发送方的报文", packet);

		//取出Message，向上递交给应用层
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = (packet.seqnum + 1) % ReceiverConfig::maxSeqNum; //下一个希望收到的序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

		this->expectSequenceNumberRcvd = (packet.seqnum + 1) % ReceiverConfig::maxSeqNum; //期待收到下一个序号



	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
		}
		pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文

	}
}