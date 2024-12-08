#pragma once
#include <string>

std::wstring StringToWStringOneByte(const std::string& s)
{
    return std::wstring(s.begin(), s.end());
}