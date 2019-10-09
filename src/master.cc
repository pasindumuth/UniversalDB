#include <string>
#include <memory>

#include <boost/asio.hpp>

#include <logging/log.h>
#include <net/impl/ChannelImpl.h>
#include <proto/master.pb.h>
#include <proto/message.pb.h>
#include <utils.h>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  auto hostnames = parse_hostnames(argc, argv);
  auto const constants = initialize_constants();

  // To bootstrap the system, the first thing that the Master does is to send
  // all slaves a list of the hostnames of all the other slaves.
  auto slave_list = proto::master::SlaveList();
  for (auto const& hostname : hostnames) {
    slave_list.add_slave_hostnames(hostname.c_str());
  }

  auto channels = std::vector<std::shared_ptr<uni::net::ChannelImpl>>();
  auto io_context = boost::asio::io_context();
  for (auto const& hostname : hostnames) {
    auto resolver = tcp::resolver(io_context);
    auto endpoints = resolver.resolve(hostname, std::to_string(constants.master_port));
    auto socket = std::make_shared<tcp::socket>(io_context);
    // Connect to the slave
    boost::asio::async_connect(*socket, endpoints,
      [socket, &slave_list, &channels, &hostname](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
        auto channel = std::make_shared<uni::net::ChannelImpl>(std::move(*socket));
        auto message_wrapper = new proto::message::MessageWrapper();
        auto master_message = new proto::master::MasterMessage();
        auto master_request = new proto::master::MasterRequest();
        master_request->set_allocated_slave_list(new proto::master::SlaveList(slave_list));
        master_message->set_allocated_request(master_request);
        message_wrapper->set_allocated_master_message(master_message);
        // Send the slave the list contains the hostnames of all slaves.
        channel->queue_send(message_wrapper->SerializeAsString());
        channels.push_back(channel);
        LOG(uni::logging::Level::INFO, "Sent SlaveList to Slave: " + hostname)
    });
  }

  auto work_guard = boost::asio::make_work_guard(io_context);
  io_context.run();
}
