#include "Config.h"
#include <string>

using namespace std;

Config::Config(void)
{
}

Config::~Config(void)
{
}

 string Config::SERVERADDRESS = "127.0.0.1";	//������IP��ַ
const int Config::MAXCONNECTION = 50;				//���������50
const int Config::BUFFERLENGTH = 65535;				//��������С65535�ֽ�
 int Config::PORT = 5050;						//�������˿�5050
const u_long Config::BLOCKMODE = 1;					//SOCKETΪ������ģʽ
