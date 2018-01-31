# chat程序说明

- chat基于boost.asio网络库，是boost.asio的作者在boost的文档中给出的例子，用以说明boost.asio的用法， 具体的链件http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/examples/cpp11_examples.html
一共有三个文件
zoo_cli.h 为头文件
zoo_cli.cpp为源文件
zoo_test.cpp 为测试文件
编译方法如下：以编译那个test文档为例
首先将cpp文件生成.o文件
g++ -c zoo_cli.cpp -o zoo_cli.o -fpermissive
之后生成目标程序
注意：编译加上libzookeeper_mt.so 多线程库
g++ -o test zoo_test.cpp ./zoo_cli.o /usr/local/lib/libzookeeper_mt.so 

