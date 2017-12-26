#include <iostream>
#include <cpp_redis/cpp_redis>

int main(void)
{
    cpp_redis::client client;
    client.connect();

    client.set("cpp_redis", "100");
    client.get("cpp_redis", [](cpp_redis::reply& reply) {
        std::cout << reply << std::endl;
    });
    //! also support std::future
    //! std::future<cpp_redis::reply> get_reply = client.get("hello");

    client.sync_commit();
    //! or client.commit(); for synchronous call
    return 0;
}
