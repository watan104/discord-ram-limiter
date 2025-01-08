#include "discord-ram-limiter.hpp"

auto main() -> int {
    SetConsoleTitleA("DiscordRamLimiter https://github.com/steak137/");
    std::cout << "DiscordRamLimiter https://github.com/steak137/" << std::endl;
    std::thread m_thread(monitor_and_limit_mem);
    m_thread.join();
    return 0;
}
