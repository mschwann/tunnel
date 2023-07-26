#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>

#include "tuntap_interface.h"

void tuntap_worker(TunTapInterface& tuntap, std::atomic_bool& isShutdown,
                   size_t bufSize,
                   std::function<void(std::vector<uint8_t>)> callback);