#pragma once
#include "IPlatformHost.h"

using namespace generic_host;
class ConsoleHost final : public IPlatformHost {
public:
    explicit ConsoleHost(std::shared_ptr<ILogger> logger);

    int Run(std::function<void()> startApp) override;
};
