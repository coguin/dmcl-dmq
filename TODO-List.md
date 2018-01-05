# DMCL-DMQ TODO-List

## 2018.01.02-2018.01.06
#### 1. 完善消息传输内容，消息传输中区分出Producer和Consumer [**chenzl**]
- 仿照Kafka提供的操作，根据现有代码提供Producer，Consumer，Topic的创建
- Kafka提供的操作：
	- kafka-topic.sh  --create  --zookeeper zkhost:port  --topic  topicname 
	- kafka-producer.sh  --broker-list brokerhost:port  --topic topicname
	- kafka-consumer.sh --broker-list brokerhost:port --topic topicname
####涉及内容：
- Broker将Producer发送的Message，按照topic管理，Broker设计class用于管理topic中的Message，暂时考虑仿照LevelDB的数据管理方式， Message的内容直接使用string存储，同时配合vector，用于记录存储在Topic中的每条message的offset.
- Consumer订阅Topic时向Broker发送订阅请求，Broker将该Topic下的Message返回至Consumer, Consumer的消费记录由Consumer保存
 
####2.集成zookeeper服务 [**zhaoj**]

####3. 消息存储支持Leveldb [**zhoulk**]
