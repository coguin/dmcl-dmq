all: chat_client.cpp chat_producer.cpp chat_server.cpp chat_consumer.cpp chat_router.cpp chat_message.h
	g++ -g -o client chat_client.cpp -lboost_system -lpthread -std=c++11
	g++ -g -o consumer chat_consumer.cpp -lboost_system -lpthread -std=c++11
	g++ -g -o producer chat_producer.cpp -lboost_system -lpthread -std=c++11
	g++ -g -o server chat_server.cpp -lboost_system -lpthread -std=c++11
	g++ -g -o topic chat_topic.cpp -lboost_system -lpthread -std=c++11
	g++ -g -w -o router ../zookeeper/zoo_cli.cpp chat_router.cpp -fpermissive -lboost_system -lpthread -std=c++11 /usr/local/lib/libzookeeper_mt.so
