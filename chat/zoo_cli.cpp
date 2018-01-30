/*
 * zoo_cli.cpp
 *
 *  Created on: Jan 25, 2018
 *      Author: silence
 */
#include <string.h>
#include<cstring>
#include<iostream>
#include <errno.h>
#include <zookeeper/zookeeper.h>
#include <zookeeper/proto.h>
#include"zoo_cli.h"

char* state2String(int state){
  if (state == 0)
    return "CLOSED_STATE";
  if (state == ZOO_CONNECTING_STATE)
    return "CONNECTING_STATE";
  if (state == ZOO_ASSOCIATING_STATE)
    return "ASSOCIATING_STATE";
  if (state == ZOO_CONNECTED_STATE)
    return "CONNECTED_STATE";
  if (state == ZOO_EXPIRED_SESSION_STATE)
    return "EXPIRED_SESSION_STATE";
  if (state == ZOO_AUTH_FAILED_STATE)
    return "AUTH_FAILED_STATE";

  return "INVALID_STATE";
}

char* type2String(int state){
  if (state == ZOO_CREATED_EVENT)
    return "CREATED_EVENT";
  if (state == ZOO_DELETED_EVENT)
    return "DELETED_EVENT";
  if (state == ZOO_CHANGED_EVENT)
    return "CHANGED_EVENT";
  if (state == ZOO_CHILD_EVENT)
    return "CHILD_EVENT";
  if (state == ZOO_SESSION_EVENT)
    return "SESSION_EVENT";
  if (state == ZOO_NOTWATCHING_EVENT)
    return "NOTWATCHING_EVENT";

  return "UNKNOWN_EVENT_TYPE";
}
void watcher(zhandle_t *zzh,int type,int state,const char*path,void *watcherCtx){
	std::cout<<"zookeeper connected"<<endl;
	//std::cout<<"Watcher "<<type2String(type) <<"state ="<<state2String(state);
}
void my_string_completion_free_data(int rc, const char *name, const void *data){
	std::cout<<(data==0?"null":data)<<": rc = "<<rc<<std::endl;
	if (!rc) {
       std::cout<<"name = "<<name<<std::endl;
	}
}
Zoo_cli::Zoo_cli(){
	if(this->log_Debug){
		zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
	}
	else{
		zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
	}
	zh = zookeeper_init("localhost:	2181", watcher, 10000, 0, 0, 0);
	if(!zh){
		std::cout<<errno;
	}
}
Zoo_cli::Zoo_cli(char *ip){
	if(this->log_Debug){
		zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
	}
	else{
		zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
	}
	this->IP=ip;
	zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
	this->zh = zookeeper_init(ip, watcher, 10000, 0, 0, 0);
	if(!this->zh){
		std::cout<<"f";
	}
}
/*****
 * 初始化节点,即建立brokes,topics,consumers节点目录
 * 系统只运行一次
 * *********/
void Zoo_cli::init(){
	int rc = zoo_create(zh,"/brokers","",0,&ZOO_OPEN_ACL_UNSAFE,0,
				0,0);
	if(rc){
		std::cout<<"init error :brokers!rc = "<<rc<<endl;
	}
	rc = zoo_create(zh,"/consumers","",0,&ZOO_OPEN_ACL_UNSAFE,0,
				0,0);
	if(rc){
			std::cout<<"init error :consumers! rc="<<rc<<endl;
		}
	rc = zoo_create(zh,"/topics","",0,&ZOO_OPEN_ACL_UNSAFE,0,
					0,0);
	if(rc){
				std::cout<<"init error :topics! rc = "<<rc<<endl;;
			}
}
Zoo_cli::~Zoo_cli(){
	zookeeper_close(this->zh);
}
/*********broker*******
 * br_name 要建立broker节点的name 一般可为ip地址
 * 创建的broker为临时节点,掉线自动删除
 * ************/
void Zoo_cli::register_broker(char * br_name){
	char p[30]="/brokers/";
	strcat(p,br_name);
//	 char str[]="/xyz6";
//	std::cout<<"register_broker p"<<p<<std::endl;
	int rc = zoo_create(zh,p,broker_id,sizeof(broker_id),&ZOO_OPEN_ACL_UNSAFE,ZOO_EPHEMERAL,
			0,0);
	if(rc){
		std::cout<<"Register broker error! rc = "<<rc<<endl;
	}
	else{
		cout<<"Register broker success"<<endl;
	}
}
/************Get Brokers*****
 * 获取当前系统所有在线的Broker
 * 结果放在返回的vector中
 * */
vector<string> Zoo_cli::get_brokers(){
	vector<string> brokers;
	struct String_vector strings;
    struct Stat stat;
    int flag = zoo_wget_children2(zh,"/brokers",
                                  watcher,"brokers getChildren",
                                  &strings,&stat);
	//cout<<"rc= "<<rc<<"count "<<strings<<endl;
	if (flag==ZOK){
		int32_t i=0;
		for (;i<strings.count;++i){
			string s(strings.data[i]);
//			cout<<s<<endl;
			brokers.push_back(s);
		}
	}
	return brokers;
}
/***********Register Topic*******
 * 即在Topics节点下创建指定的topic节点
 * 用于保存当前系统所有topic信息
 * 生产者创建topic时调用一次
 * ****/
void Zoo_cli::register_topic(char *topic){
	char p[50]="/topics/";
	strcat(p,topic);
	int rc = zoo_create(zh,p,"subscribe",9,&ZOO_OPEN_ACL_UNSAFE,0,
			                     0, 0);
	if(rc){
		std::cout<<"Register topic error!";
	}
}
/*******Delete Topic
 * 删除指定的topic
 * 如topic下有consumer,级联删除
 * ********************/
void Zoo_cli::delete_topic(char *topic){
	char p[50]="/topics/";
	strcat(p,topic);
	vector<string> result;
	result = this->get_topic_cosumers(topic);
	if(!result.empty()){
		cout<<"!empty"<<endl;
		for(vector<string>::iterator siter = result.begin();
								siter!=result.end();siter++){
			string s = *siter;
			char* c =new char[s.length()];
			strcpy(c,s.c_str());
//			cout<<"c "<<c<<endl;
			this->delete_topic_consumer(c,topic);
			delete c;
	   	}
	}
	int rc = zoo_delete(zh,p, -1);
	if(rc){
		std::cout<<"Delete topic error!"<<endl;
	}
}
/***********Get Topic
 * 获取当前所有topic
 * 结果放在返回的vector类型中
 * *************/
vector<string> Zoo_cli::get_topics(){
	vector<string> topics;
	struct String_vector strings;
    struct Stat stat;
    int flag = zoo_wget_children2(zh,"/topics",
                                  watcher,"topics getChildren",
                                  &strings,&stat);
	//cout<<"rc= "<<rc<<"count "<<strings<<endl;
	if (flag==ZOK){
		int32_t i=0;
		for (;i<strings.count;++i){
			string s(strings.data[i]);
//			cout<<s<<endl;
			topics.push_back(s);
		}
	}
	return topics;
}
/***************Get Topic consumers
 * 获取指定topic下的所有consumers
 * 结果放在返回的vector类型中
 * **************/
vector<string> Zoo_cli::get_topic_cosumers(char *topic){
	vector<string> consumers;
	char p[50]="/topics/";
	strcat(p,topic);
	struct String_vector strings;
	struct Stat stat;
	int rc = zoo_wget_children2(zh,p,
	                                  watcher,"topic_consumers getChildren",
	                                  &strings,&stat);
	if (rc==ZOK){
		int32_t i=0;
		for (;i<strings.count;++i){
			string s(strings.data[i]);
//			cout<<s<<endl;
			consumers.push_back(s);
		}
	}
	return consumers;
}
/***************Delete Topic consumer
 * 删除指定topic下的consumer节点
 * **************/
void Zoo_cli::delete_topic_consumer(char * consumer,char * topic){
	vector<string> consumers;
	char p[50]="/topics/";
	strcat(p,topic);
	strcat(p,"/");
	strcat(p,consumer);
	int rc = zoo_delete(zh,p,-1);
	if(rc){
		std::cout<<"Delete topic consumer error!";
	}
}
/*************consumer***************/
void Zoo_cli::register_consumer(char * con_name){
	//ZOO_EPHEMERAL 临时节点 ZOO_SEQUENCE 编号递增
	//char *p ="/"+this->IP;
	char p[30]="/consumers/";
	strcat(p,con_name);
	int rc = zoo_create(zh,p,"",0 ,&ZOO_OPEN_ACL_UNSAFE,0,
	                     0, 0);
	if(rc){
		std::cout<<"Register consumer error!";
	}
//	else{
//		std::cout<<"consumer_id :"<<consumer_id;
//	}
}
void Zoo_cli::delete_consumer(char *consumer){
	char p[50]="/consumers/";
	strcat(p,consumer);
	int rc = zoo_delete(zh,p,-1);
	if(rc){
		std::cout<<"Delete consumer error!";
	}
}

vector<string> Zoo_cli::get_consumers(){
	vector<string> consumers;
	struct String_vector strings;
	struct Stat stat;
	int rc = zoo_wget_children2(zh,"/consumers",
	                                  watcher,"consumers getChildren",
	                                  &strings,&stat);
	if (rc==ZOK){
		int32_t i=0;
		for (;i<strings.count;++i){
			string s(strings.data[i]);
//			cout<<s<<endl;
			consumers.push_back(s);
		}
	}
	return consumers;
}
/***************Sub
 * 在指定topic节点下创建consumer节点
 * 即某个consumer订阅某个topic
 * ************/
void Zoo_cli::sub_(char * consumer,char* topic){
	char p[50]="/topics/";
	strcat(p,topic);
	strcat(p,"/");
	strcat(p,consumer);
	int rc = zoo_create(zh,p,"",0,&ZOO_OPEN_ACL_UNSAFE,0,
				                     0, 0);
	if(rc){
			std::cout<<"sub_ error! rc = "<<rc<<endl;
		}
}



