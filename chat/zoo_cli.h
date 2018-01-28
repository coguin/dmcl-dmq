/*
 * zoo_cli.h
 *
 *  Created on: Jan 28, 2018
 *      Author: silence
 */

#ifndef CHAT_ZOO_CLI_H_
#define CHAT_ZOO_CLI_H_
#include<string>
#include<cstring>
#include<errno.h>
#include<vector>
#include<iostream>
#include<zookeeper/zookeeper.h>
#include <zookeeper/proto.h>
using namespace std;
class Zoo_cli{
public:
	//初始构造函数获取zookeeper
	Zoo_cli(char *p);
	Zoo_cli();
	~Zoo_cli();
	void zoo_init();
/***********broker***********************/
	void register_broker(char *);//初始化建立此brokerd的临时节点
	vector<string> get_brokers();//获取所有brokers列表
	void register_topic(char *p);//注册添加一个topic
	vector<string> get_topics();//获取所有topic列表
	vector<string> get_topic_cosumers(char *topic);//获取某个topic下所有订阅者
	//void pub(char* topic);	//发布
/************consumer*******************/
	void register_consumer(char * consumer_id);
	vector<string> get_consumers();//获取所有订阅者信息
	void sub_(char * consumer,char* topic);	//consumer订阅topic
public:
	char* IP="127.0.0.2";
	char *broker_id="1234";
private:
	zhandle_t *zh;
	char consumer_id[512];
//	struct String_vector {
//	    int32_t count;
//	    char * *data;
//	};
};

#endif /* CHAT_ZOO_CLI_H_ */
