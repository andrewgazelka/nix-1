#pragma once
///@file
// Private header - not installed

#include <filesystem>

namespace nix::windows::known_folders {

/**
 * Get the Windows LocalAppData known folder.
 */
std::filesystem::path getLocalAppData();

/**
 * Get the Windows RoamingAppData known folder.
 */
std::filesystem::path getRoamingAppData();

} // namespace nix::windows::known_folders
