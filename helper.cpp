#pragma once
#include "helper.hpp"


void ErrorLogger::log(const std::string& info)
{
    fstreamBuffer.open("log.txt", std::ios::out | std::ios::app);
    fstreamBuffer << info;
    fstreamBuffer.flush();
    fstreamBuffer.close();
}

void ErrorLogger::popMessageBox(const std::string& popupInfo)
{
    log(popupInfo);
    std::wstring wstr(popupInfo.begin(), popupInfo.end());
    MessageBox(NULL, wstr.c_str(), NULL, MB_ABORTRETRYIGNORE);
}

void Timer::start()
{
    m_start = std::chrono::high_resolution_clock::now();
}

std::chrono::duration<double, std::milli> Timer::getDuration()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> milliDuration = now - m_start;
    return milliDuration;
}