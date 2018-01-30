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
	vector<string> result;
    Zoo_cli zoo;
   // zoo = new Zoo_cli();
    //zoo->init();
//    zoo.init();
//    zoo.register_topic("1234");
//    zoo.register_topic("1235");
//    zoo.register_topic("1236");
//    zoo.register_consumer("1234");
//    zoo.register_consumer("1235");
//    zoo.register_consumer("1236");
//    zoo.register_broker("1234");
//    zoo.register_broker("1235");
//    sleep(5);
//    zoo.register_consumer("1237");
//    result =zoo.get_topics();
//    zoo.sub_("9","1235");
//    zoo.sub_("4","1235");
    result=zoo.get_topic_cosumers("1235");
    for(vector<string>::iterator siter = result.begin();
    		siter!=result.end();siter++){
    		cout<<*siter<<endl;
    	}
    zoo.delete_topic("1235");
//    zoo.get_consumers();
//    zoo.get_brokers();
//    zoo.sub_("1234","1235");
//    zoo.get_topic_cosumers("1235");
}



