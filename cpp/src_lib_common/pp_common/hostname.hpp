#pragma once
#include "./base.hpp"

uint32_t HashHostname(const char * Hostname);
uint32_t HashHostname(const char * Hostname, size_t Len);
uint32_t HashHostname(const std::string & Hostname);