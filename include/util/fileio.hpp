#ifndef UTIL_FILEIO_HPP
#define UTIL_FILEIO_HPP

#include <filesystem>
#include <string>

// File functions

std::string getRandomLineFromFile(const std::filesystem::path& file);

#endif
