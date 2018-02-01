/*
 * zoo_test.cpp
 *
 *  Created on: Jan 24, 2018
 *      Author: silence
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zookeeper/zookeeper.h>
#include <zookeeper/zookeeper_log.h>
#include"zoo_cli.h"
#include <unistd.h>
int main(int argc, const char *argv[])
{
 
    //首先创建一个zookeeper的类，默认构造函数会以默认端口（2181）初始化连接zookeeper
// 页可以传入IP地址及端口号进行构造函数初始化
	Zoo_cli zoo;
   //系统第一次运行需要首先创建brokers，consumers和topics三个目录节点
// 系统再次运行时不需要再运行了
    zoo.init();
  //当初话完成后，就可以用zoo对象进行相关操作，包括创建topic，删除topic，
// 创建broker（不用删除，broker掉线自动删除），消费者订阅topic，获取订阅topic下的所有消费者等
//   创建topic 相关函数 data为想要在topic存储的数据
//    zoo.register_topic("1234","data1"); //创建名为“1234”的topic
    zoo.register_topic("1235","data2");//创建名为“1235”的topic
//    zoo.register_topic("1236","data3");//创建名为“1236”的topic
	//想要获取topic的数据,接口如下 传入的参数为topic的名字
	//返回为string类型的topic上的数据 即topic存储的broker的地址
	string s =zoo.get_topic_data("1235");
	cout<<"s ="<<s<<endl;
//   以下为创建消费者 即再consumers目录下创建一个节点，表示此节点是消费者
//    zoo.register_consumer("1234");//创建名为“1234”的consumer
//    zoo.register_consumer("1235");//创建名为“1235”的consumer
//    zoo.register_consumer("1236");//创建名为“1236”的consumer
//   以下为创建broker 即在broekrs目录下创建一个临时节点
//    zoo.register_broker("1234");//创建名为“1234”的brokers
//    zoo.register_broker("1235");//创建名为“1235”的brokers
// 如果想要获取所有的topic，可调用以下函数，返回为vector<string>类型，里面保存所有topic的名字
  	vector<string> result;
//    result =zoo.get_topics();//返回结果如：“1234”，"1235","1236"
//  如果某个消费者想要订阅某个topic，相关操作如下
//    zoo.sub_("9","1235");//消费者名为“9”订阅名为“1235”的topic
//    zoo.sub_("4","1235");//消费者名为“4”订阅名为“1235”的topic
//  想要获取订阅某个topic下的所有消费者信息，接口如下
   // result=zoo.get_topic_cosumers("1235");//获取名为“1235”的topic下的所有消费者信息
    //想要删除某个topic，接口：
//    zoo.delete_topic("1235");//删除名为“1235”的topic topic下的所有订阅者也将删除
 // 获取所有消费者信息，接口如下：
//    zoo.get_consumers();
// 获取所有brokers信息，接口如下：
//    zoo.get_brokers();
}
