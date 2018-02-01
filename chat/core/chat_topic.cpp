//
// Created by czlei on 18-1-24.
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "chat_message.h"

using boost::asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
    chat_client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
            : io_service_(io_service), socket_(io_service)
    {
        do_connect(endpoint_iterator);
    }

    void write(const chat_message& msg)
    {
        io_service_.post(
                [this, msg]()
                {
                    bool write_in_progress = !write_msgs_.empty();
                    write_msgs_.push_back(msg);
                    if(!write_in_progress)
                    {
                        do_write();
                    }
                });
    }

    void close()
    {
        io_service_.post([this]() { socket_.close(); });
    }

private:
    void do_connect(tcp::resolver::iterator endpoint_iterator)
    {
        boost::asio::async_connect(socket_, endpoint_iterator,
                                   [this](boost::system::error_code ec, tcp::resolver::iterator)
                                   {
                                       if(!ec)
                                       {
                                           do_read_header();
                                       }
                                   });
    }

    void do_read_header()
    {
        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.data(), chat_message::header_length),
            [this](boost::system::error_code ec, std::size_t)
            {
                if(!ec && read_msg_.decode_header())
                {
                    do_read_body();
                }
                else
                {
                    socket_.close();
                }
            });
    }

    void do_read_body()
    {
        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
            [this](boost::system::error_code ec, std::size_t)
            {
                if(!ec)
                {
                    std::cout.write(read_msg_.body(), read_msg_.body_length());
                    std::cout << "\n";
                    do_read_header();
                }
                else
                {
                    socket_.close();
                }
            });
    }

    void do_write()
    {
        boost::asio::async_write(socket_,
            boost::asio::buffer(write_msgs_.front().data(),
                                write_msgs_.front().length()),
            [this](boost::system::error_code ec, std::size_t)
            {
                if(!ec)
                {
                    write_msgs_.pop_front();
                    if(!write_msgs_.empty())
                    {
                        do_write();
                    }
                }
                else
                {
                    socket_.close();
                }
            });
    }

private:
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    chat_message read_msg_;
    chat_message_queue write_msgs_;
};

int main(int argc, char* argv[])
{
    try
    {
        if(argc != 2)
        {
            //std::cerr << "Usage: chat_client <host> <port>\n";
            std::cerr << "Usage: topic <host>\n";
            return 1;
        }

        // boost::asio::io_service io_service;

        // tcp::resolver resolver(io_service);
        // //auto endpoint_iterator = resolver.resolve({argv[1], argv[2]});
        // auto endpoint_iterator = resolver.resolve({argv[1], "20000"});
        // // chat_client c(io_service, endpoint_iterator);

        // // std::thread t([&io_service](){io_service.run();});

        while(1)
        {
             boost::asio::io_service io_service;

            tcp::resolver resolver(io_service);
            //auto endpoint_iterator = resolver.resolve({argv[1], argv[2]});
            auto endpoint_iterator = resolver.resolve({argv[1], "20000"});
            // chat_client c(io_service, endpoint_iterator);

            chat_client c(io_service, endpoint_iterator);

            std::thread t([&io_service](){io_service.run();});

            std::string command, topic;
            std::cin >> command >> topic;
            chat_message msg;
            // std::cout << command << ":" << topic << "\n";
            msg.body_length(topic.size() + 1);

            if(command.compare("new") == 0)
            {
                msg.body()[0] = '2';
            }
            else if(command.compare("del") == 0)
            {
                msg.body()[0] = '3';
            }
            else
            {
                std::cout << "sorry, no such commands, try commands like below:\n";
                std::cout << "new <topic> : create a topic\ndel <topic> : delete a topic\n";
            }

            std::memcpy(msg.body() + 1, topic.c_str(), msg.body_length() - 1);
            msg.encode_header();
            c.write(msg);

            
            t.join();
            c.close();
        }

        // c.close();
        // t.join();
    }
    catch(std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
