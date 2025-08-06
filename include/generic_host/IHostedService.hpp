#pragma once
#include <boost/asio/io_context.hpp>

namespace gh {

    struct IHostedService {
        virtual void Start(const boost::asio::io_context& io) {}
        virtual void Stop() {}
        virtual ~IHostedService() = default;
    };

}
