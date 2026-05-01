//
// Created by Darek Rudiš on 03.05.2025.
//

#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <chrono>

inline auto file_path = "/Users/darek/Documents/vst3_debug_log.txt";

inline void log_debug(const std::string& msg)
{
    auto now = std::chrono::system_clock::now();
    std::string formatted_time = std::format("{0:%F_%T}", now);
    std::ofstream log(file_path, std::ios::app);
    log << formatted_time << ": "<< msg << std::endl;
}

#endif //UTILS_HPP
