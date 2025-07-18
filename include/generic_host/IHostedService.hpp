#pragma once
#include <boost/asio.hpp>

namespace generic_host {

    struct IHostedService {
        virtual void Start(boost::asio::io_context& io) = 0;
        virtual void Stop() = 0;
        virtual ~IHostedService() = default;
    };

}
