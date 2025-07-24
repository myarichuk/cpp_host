#pragma once
#include <boost/asio/io_context.hpp>

namespace gh {

    struct IHostedService {
        virtual void Start(const boost::asio::io_context& io) = 0;
        virtual void Stop() = 0;
        virtual ~IHostedService() = default;
    };

}
