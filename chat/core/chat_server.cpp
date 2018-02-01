#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "chat_message.h"

using boost::asio::ip::tcp;



typedef std::deque<chat_message> chat_message_queue;



class chat_participant
{
public:
    virtual ~chat_participant() {}
    virtual void deliver(const chat_message& msg) = 0;
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
                participant->deliver(msg);
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
            participant->deliver(msg);
    }
};

class chat_room
{
public:
    void join(bool type, std::string topic, chat_participant_ptr participant)
    {
        iter_ = topics_.find(topic);
        if(iter_ != topics_.end())
        {
            iter_->second.join(type, participant);
        } else{
            std::cerr << "sorry, no such topic exist" << std::endl;
        }
    }

    void leave(bool type, std::string topic, chat_participant_ptr participant)
    {
        iter_ = topics_.find(topic);
        if(iter_ != topics_.end())
            iter_->second.leave(type, participant);
    }

    void deliver(std::string topic, const chat_message &msg)
    {
        //TODO msg是该topic下新收到的消息，可以考虑在这里加上落盘
        iter_ = topics_.find(topic);
        if(iter_ != topics_.end())
            //TODO 此处是将收到的消息分发给订阅了该topic的消费者
            iter_->second.deliver(msg);
    }

    void createTopic(std::string topic)
    {
        topics_.insert(std::pair<std::string, topics>(topic, topics()));
    }

    void delTopic(std::string topic)
    {
        iter_ = topics_.find(topic);

        if(iter_ != topics_.end())
            topics_.erase(iter_);
    }

private:
    std::map<std::string, struct topics> topics_;
    std::map<std::string, struct topics>::iterator iter_;
};


class chat_session
        : public chat_participant,
          public std::enable_shared_from_this<chat_session>
{
public:
    chat_session(tcp::socket socket, chat_room& room)
            : socket_(std::move(socket)),
              room_(room),
              topic_("")
    {
    }

    void start()
    {
        do_read_header();
    }

    void deliver(const chat_message& msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    }

private:
    void do_read_header()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
            boost::asio::buffer(read_msg_.data(), chat_message::header_length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec && read_msg_.decode_header())
                {
                    //std::cout << read_msg_.body_length() << std::endl;
                    do_read_body();
                }
                else
                {
                    room_.leave(type_, topic_, shared_from_this());
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
                    if(topic_.empty())
                    {
                        char type = read_msg_.body()[0];
                        topic_.append(read_msg_.body() + 1, read_msg_.body_length() - 1);
                        //std::cout << type_ << "topic: " << topic_ << std::endl;

                        if(type == '0' || type == '1')
                        {
                            type_ = (type == '1');
                            if(type_)
                                std::cout << "producer:topic: " << topic_ << std::endl;
                            else
                                std::cout << "consumer:topic: " << topic_ << std::endl;
                            room_.join(type_, topic_, shared_from_this());
                        }
                        else
                        {
                            if(type == '2')
                            {
                                std::cout << "create topic: " << topic_ << std::endl;
                                room_.createTopic(topic_);
                                topic_.clear();
                            }
                            else if(type == '3')
                            {
                                std::cout << "delete topic: " << topic_ << std::endl;
                                room_.delTopic(topic_);
                                topic_.clear();
                            }
                        }
                    }
                    else{
                        // 当创建topic之后，正常接收producer的消息，发送给consumer
                        room_.deliver(topic_, read_msg_);
                    }
                    do_read_header();
                }
                else
                {
                    room_.leave(type_, topic_, shared_from_this());
                }
            });
    }

    void do_write()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_,
            boost::asio::buffer(write_msgs_.front().data(),
                                write_msgs_.front().length()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    write_msgs_.pop_front();
                    if (!write_msgs_.empty())
                    {
                        do_write();
                    }
                }
                else
                {
                    room_.leave(type_, topic_, shared_from_this());
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
    chat_message_queue write_msgs_;
    char type_;
    std::string topic_;
};



class chat_server
{
public:
    chat_server(boost::asio::io_service& io_service,
                const tcp::endpoint& endpoint)
            : acceptor_(io_service, endpoint),
              socket_(io_service)
    {
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

/**
 * chat_server启动时需要从zookeeper获取topic信息
 */ 

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

        chat_server server(io_service, tcp::endpoint(tcp::v4(), 30000));

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
