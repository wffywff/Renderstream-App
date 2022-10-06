#pragma once
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <Windows.h>
#include <fstream>
#include <libloaderapi.h>
#include <filesystem>
#include <iostream>
#include <Shlwapi.h>

class ErrorLogger
{
public:
    void log(const std::string& info);
    void popMessageBox(const std::string& popupinfo);
    std::fstream fstreamBuffer;
};

class Timer
{
public:
    void start();
    std::chrono::duration<double, std::milli> getDuration();
    int frameCount;
    
private:
    std::chrono::high_resolution_clock::time_point m_start;
};