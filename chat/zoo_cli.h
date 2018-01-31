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
#include <zookeeper/zookeeper.h>
#include <zookeeper/proto.h>

using namespace std;
class Zoo_cli{
public:
	//初始构造函数获取zookeeper
	Zoo_cli(char *ip);//指定在zookeeper地址 默认127.0.0.1:2181
	Zoo_cli();
	~Zoo_cli();
	int init();
/***********broker***********************/
	int register_broker(char *br_name);//初始化建立此brokerd的临时节点
	vector<string> get_brokers();//获取所有brokers列表
	int register_topic(char *topic_name,char*data );//注册添加一个topic,
	string get_topic_data(char * topic_name);//获取topic节点上的数据
	int delete_topic(char *topic_name);
	vector<string> get_topics();//获取所有topic列表
	vector<string> get_topic_cosumers(char *topic);//获取某个topic下所有订阅者

	//int pub(char* topic);	//发布
/************consumer*******************/
	int register_consumer(char * consumer_id);
	int delete_consumer(char *consumer);
	vector<string> get_consumers();//获取所有订阅者信息
	int sub_(char * consumer,char* topic);	//consumer订阅topic
	int delete_topic_consumer(char * consumer,char* topic);
public:
	char* IP="127.0.0.1:2181";
	char* broker_id="1234";
private:
	zhandle_t *zh;
	char consumer_id[512];
	bool log_Debug =true;
//	struct String_vector {
//	    int32_t count;
//	    char * *data;
//	};
};

#endif /* CHAT_ZOO_CLI_H_ */
