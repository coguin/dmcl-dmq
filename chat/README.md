# chat程序说明

- chat基于boost.asio网络库，是boost.asio的作者在boost的文档中给出的例子，用以说明boost.asio的用法， 具体的链件http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/examples/cpp11_examples.html
g++ -c zoo_cli.cpp -o zoo_cli.o -fpermissive
g++ -o test zoo_test.cpp ./zoo_cli.o /usr/local/lib/libzookeeper_mt.so  

