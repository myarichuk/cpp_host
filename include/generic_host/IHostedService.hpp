#pragma once

namespace generic_host {

    struct IHostedService {
        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual ~IHostedService() = default;
    };

}
