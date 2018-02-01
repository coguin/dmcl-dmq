#include <cstdlib>
#include <stdlib.h>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <random>
#include <utility>
#include <boost/asio.hpp>
#include "chat_message.h"
#include "../zookeeper/zoo_cli.h"

using boost::asio::ip::tcp;



typedef std::deque<chat_message> chat_message_queue;



class chat_participant
{
public:
    virtual ~chat_participant() {}

};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

struct topics
{
    std::set<chat_participant_ptr> consumers_;
    std::set<chat_participant_ptr> producers_;

    enum { max_recent_msgs = 100 };
    chat_message_queue recent_msgs_;

    void join(bool type, chat_participant_ptr participant)
    {
        if(type)
        {
            producers_.insert(participant);
        }
        else
        {
            consumers_.insert(participant);
            for(auto msg: recent_msgs_)
                ;
        }
    }
    void leave(bool type, chat_participant_ptr participant)
    {
        if(type)
            producers_.erase(participant);
        else
            consumers_.erase(participant);
    }
    void deliver(const chat_message& msg)
    {
        recent_msgs_.push_back(msg);
        while(recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();

        for(auto participant: consumers_)
            ;
    }
};

class chat_room
{
public:
    void join(chat_participant_ptr participant)
    {
        participants_.insert(participant);
    }

    void leave(chat_participant_ptr participant)
    {
        participants_.erase(participant);
    }

    void deliver(const chat_message& msg)
    {

    }

public:
    Zoo_cli zoo_cli_;

private:
    std::set<chat_participant_ptr> participants_;
    enum { max_recent_msgs = 100 };
    chat_message_queue recent_msgs_;

    // Zoo_cli zoo_cli_;
};


class chat_session
        : public chat_participant,
          public std::enable_shared_from_this<chat_session>
{
public:
    chat_session(tcp::socket socket, chat_room& room)
            : socket_(std::move(socket)),
              room_(room)
              // topic_("")
    {
        
    }

    void start()
    {
        room_.join(shared_from_this());
        do_read_header();
    }

private:
    void do_read_header()
    {

        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.data(), chat_message::header_length),
            [this](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec && read_msg_.decode_header())
                {
                    std::cout << "start do_read_heder." << "\n";
                    do_read_body();
                }
                else
                {
                    room_.leave(shared_from_this());
                }
            });
    }

    void do_read_body()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    std::cout << "start do_read_body." << "\n";
                    char type = read_msg_.body()[0];
                    std::string recv_topic(read_msg_.body() + 1, read_msg_.body_length() - 1);
                    // topic_.append(read_msg_.body() + 1, read_msg_.body_length() - 1);
                    
                    std::cout << "recv_topic:" << recv_topic << "\n";
                    if(type == '0' || type == '1')  // consumer :0 producer :1
                    {
                        std::string broker_ip;
                        
                        std::vector<std::string> topic_list = room_.zoo_cli_.get_topics();
                        std::vector<std::string> broker_ip_list = room_.zoo_cli_.get_brokers();
                        std::cout << "获取topic_list:" << "\n";
                        for(auto iter= topic_list.cbegin(); iter != topic_list.cend(); ++iter)  
                        {
                            std::cout << *iter << "\t";
                        }
                        std::cout << "\n";
                        std::cout << "获取的broker_ip_list:" << "\n";
                        for(auto iter = broker_ip_list.cbegin(); iter != broker_ip_list.cend(); ++iter)
                        {
                            std::cout << *iter << "\t";
                        }
                        std::cout << "\n";

                        if( topic_list.empty() && type == '0')   // 如果topic为空，则对于consumer需要返回错误信息
                        {
                            std::string result_content = "topic不存在，无法消费";
                            write_msg_.body_length(result_content.size());
                            std::memcpy(write_msg_.body(), result_content.c_str(), write_msg_.body_length());
                            write_msg_.encode_header();

                            do_write();
                        }
                        else  // producer不管topic成功与否 与 consumer当topic不为空时，都直接返回broker_ip即可, 
                        {                                        
                            if(!broker_ip_list.empty())
                            {
                                std::random_device rd;
                                int index= rd() % broker_ip_list.size();
                                broker_ip = broker_ip_list[index];
                                std::cout << "router返回的ip地址：" << broker_ip << "\n";
                                // std::string ip = "192.168.1.1";
                                write_msg_.body_length(broker_ip.size());
                                std::memcpy(write_msg_.body(), broker_ip.c_str(), write_msg_.body_length());
                                write_msg_.encode_header();

                                do_write();
                            }
                            else
                            {
                                std::cout << "broker_ip_list is empty." << "\n";
                            }
                        }
                    }
                    else if(type == '2' || type == '3')
                    {
                        // get_brokers()返回为空(有时)
                        std::vector<std::string> broker_ip_list = room_.zoo_cli_.get_brokers();
                        for(auto iter = broker_ip_list.cbegin(); iter != broker_ip_list.cend(); ++iter)
                        {
                            std::cout << "broker_ip_list's content:" << *iter << "\n";
                        }
                        if(type == '2')     // create topic    需要在zookeeper上面设置topic和broker_ip的对应关系绑定
                        {
                            std::string broker_ip;
                            // std::vector<std::string> topic_list = room_.zoo_cli_.get_topics();
                            std::string r = room_.zoo_cli_.get_topic_data(const_cast<char*>(recv_topic.c_str()));
                            if(0 == r.size())   // topic 不存在
                            {
                                std::cout << "topic" << recv_topic<< "不存在." << "\n";

                                if(!broker_ip_list.empty())
                                {
                                    std::random_device rd;
                                    int index= rd() % broker_ip_list.size();
                                    broker_ip = broker_ip_list[index];
                                    std::cout << "获取的broker_ip:" << broker_ip << "\n";
                                    int result = room_.zoo_cli_.register_topic(const_cast<char*>(recv_topic.c_str()), 
                                        const_cast<char*>(broker_ip.c_str()));
                                    if(result == 0)  // 向客户端回复创建topic成功与否
                                    {
                                        std::cout << "create " << recv_topic << " ok." << "\n";
                                       
                                        std::string result_content = "create:" + recv_topic + " ok";
                                        write_msg_.body_length(result_content.size());
                                        std::memcpy(write_msg_.body(), result_content.c_str(), write_msg_.body_length());
                                        write_msg_.encode_header();

                                        do_write();
                                    }
                                    else
                                    {
                                        std::cerr << "create  "<< recv_topic << "error." << "\n";
                                    }
                                }
                            }
                            else    // topic已存在,返回topic已存在      
                            {
                                std::cout << "topic:" << recv_topic << "已存在." << "\n";

                                std::string result_content = "topic:" + recv_topic + " 已存在";
                                write_msg_.body_length(result_content.size());
                                std::memcpy(write_msg_.body(), result_content.c_str(), write_msg_.body_length());
                                write_msg_.encode_header();

                                do_write();
                            }

                        }
                        else if(type == '3')    // delete topic
                        {
                            std::string r = room_.zoo_cli_.get_topic_data(const_cast<char*>(recv_topic.c_str()));
                            if(0 == r.size())   // topic不存在
                            {
                                std::string result_content = "topic不存在";
                                write_msg_.body_length(result_content.size());
                                std::memcpy(write_msg_.body(), result_content.c_str(), write_msg_.body_length());
                                write_msg_.encode_header();

                                do_write();
                            }
                            else  // topic 已存在
                            {
                                int r = room_.zoo_cli_.delete_topic(const_cast<char*>(recv_topic.c_str()));
                                if(r == 0)  // 删除成功
                                {
                                    std::string result_content = "topic删除成功";
                                    write_msg_.body_length(result_content.size());
                                    std::memcpy(write_msg_.body(), result_content.c_str(), write_msg_.body_length());
                                    write_msg_.encode_header();

                                    do_write();
                                }
                                else    // 删除失败
                                {
                                    std::string result_content = "topic删除失败";
                                    write_msg_.body_length(result_content.size());
                                    std::memcpy(write_msg_.body(), result_content.c_str(), write_msg_.body_length());
                                    write_msg_.encode_header();

                                    do_write();
                                }
                            }
                        }
                    }

                   do_read_header();    
                } else{
                    room_.leave(shared_from_this());
                }
            });
    }

    void do_write()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_,
                boost::asio::buffer(write_msg_.data(),
                                    write_msg_.length()),
                [this, self](boost::system::error_code ec, std::size_t /*length*/)
                {
                    if (!ec)
                    {
                        room_.leave(shared_from_this());
                    }
                    else
                    {
                        room_.leave(shared_from_this());
                    }
                });
    }

    void close()
    {
        socket_.close();
    }

    tcp::socket socket_;
    chat_room& room_;
    chat_message read_msg_;
    chat_message write_msg_;
    char type_;

    // std::string topic_;
};



class chat_server
{
public:
    chat_server(boost::asio::io_service& io_service,
                const tcp::endpoint& endpoint)
            : acceptor_(io_service, endpoint),
              socket_(io_service)
    {
        room_.zoo_cli_.init();
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(socket_,
                               [this](boost::system::error_code ec)
                               {
                                   if (!ec)
                                   {
                                       std::make_shared<chat_session>(std::move(socket_), room_)->start();
                                   }

                                   do_accept();
                               });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    chat_room room_;
};



int main(int argc, char* argv[])
{
    try
    {
//        if (argc < 2)
//        {
//            std::cerr << "Usage: chat_server <port> [<port> ...]\n";
//            return 1;
//        }

        boost::asio::io_service io_service;

        chat_server server(io_service, tcp::endpoint(tcp::v4(), 20000));
//        std::list<chat_server> servers;
//        for (int i = 1; i < argc; ++i)
//        {
//            tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
//            servers.emplace_back(io_service, endpoint);
//        }

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
