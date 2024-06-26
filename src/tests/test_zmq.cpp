#include <catch2/catch_test_macros.hpp>
#include <zmq.hpp>

#include <future>
#include <iostream>
#include <string>
#include <thread>

#include "zmq.hpp"
#include "zmq_addon.hpp"

void PublisherThread(zmq::context_t *ctx)
{
    //  Prepare publisher
    zmq::socket_t publisher(*ctx, zmq::socket_type::pub);
    publisher.bind("inproc://endpoint");

    // Give the subscribers a chance to connect, so they don't lose any messages
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    //  Write three messages, each with an envelope and content
    publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
    publisher.send(zmq::str_buffer("Message in A envelope"));
    publisher.send(zmq::str_buffer("B"), zmq::send_flags::sndmore);
    publisher.send(zmq::str_buffer("Message in B envelope"));
    publisher.send(zmq::str_buffer("C"), zmq::send_flags::sndmore);
    publisher.send(zmq::str_buffer("Message in C envelope"));

    // Send quit command
    publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
    publisher.send(zmq::str_buffer("quit"));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void SubscriberThread(zmq::context_t *ctx, const std::vector<std::string> &envelopes)
{
    //  Prepare subscriber
    zmq::socket_t subscriber(*ctx, zmq::socket_type::sub);
    subscriber.connect("inproc://endpoint");

    //  Thread2 opens "A" and "B" envelopes
    std::string labels;
    for (auto &e : envelopes)
    {
        subscriber.set(zmq::sockopt::subscribe, e);
        labels += e;
    }

    bool keep_running = true;
    while (keep_running)
    {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result =
            zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
        assert(result && "recv failed");
        assert(*result == 2);

        const std::string message = recv_msgs[1].to_string();

        if (message == "quit")
        {
            keep_running = false;
        }

        std::cout << "subscriber \"" << labels << "\": [" << recv_msgs[0].to_string() << "] "
                  << message << std::endl;
    }
}

TEST_CASE("zmq sends and receives messages")
{
    zmq::context_t ctx(0);

    std::thread thread1(PublisherThread, &ctx);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    const std::vector<std::string> e1 = {"A", "B"};
    const std::vector<std::string> e2 = {""};

    std::thread thread2(SubscriberThread, &ctx, e1);
    std::thread thread3(SubscriberThread, &ctx, e2);

    thread1.join();
    thread2.join();
    thread3.join();
}
