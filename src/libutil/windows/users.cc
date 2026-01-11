#include "nix/util/util.hh"
#include "nix/util/users.hh"
#include "nix/util/environment-variables.hh"
#include "nix/util/file-system.hh"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>

namespace nix {

using namespace nix::windows;

std::string getUserName()
{
    // Get the required buffer size
    DWORD size = 0;
    if (!GetUserNameA(nullptr, &size)) {
        auto lastError = GetLastError();
        if (lastError != ERROR_INSUFFICIENT_BUFFER)
            throw WinError(lastError, "cannot figure out size of user name");
    }

    std::string name;
    // Allocate a buffer of sufficient size
    //
    // - 1 because no need for null byte
    name.resize(size - 1);

    // Retrieve the username
    if (!GetUserNameA(&name[0], &size))
        throw WinError("cannot figure out user name");

    return name;
}

std::filesystem::path getHome()
{
    static std::filesystem::path homeDir = []() {
        std::filesystem::path homeDir = getEnv("USERPROFILE").value_or("C:\\Users\\Default");
        assert(!homeDir.empty());
        return canonPath(homeDir.string());
    }();
    return homeDir;
}

bool isRootUser()
{
    return false;
}

static std::filesystem::path getKnownFolder(REFKNOWNFOLDERID rfid)
{
    PWSTR str = nullptr;
    auto res = SHGetKnownFolderPath(rfid, /*dwFlags=*/0, /*hToken=*/nullptr, &str);
    Finally cleanup([&]() { CoTaskMemFree(str); });
    if (SUCCEEDED(res))
        return std::filesystem::path(str);
    throw WinError(static_cast<DWORD>(res), "failed to get known folder path");
}

static const std::filesystem::path & getLocalAppDataFolder()
{
    static const auto path = getKnownFolder(FOLDERID_LocalAppData);
    return path;
}

static const std::filesystem::path & getRoamingAppDataFolder()
{
    static const auto path = getKnownFolder(FOLDERID_RoamingAppData);
    return path;
}

std::filesystem::path getCacheDir()
{
    auto dir = getEnv("NIX_CACHE_HOME");
    if (dir)
        return *dir;
    return getLocalAppDataFolder() / "nix" / "cache";
}

std::filesystem::path getConfigDir()
{
    auto dir = getEnv("NIX_CONFIG_HOME");
    if (dir)
        return *dir;
    return getRoamingAppDataFolder() / "nix" / "config";
}

std::vector<std::filesystem::path> getConfigDirs()
{
    return {getConfigDir()};
}

std::filesystem::path getDataDir()
{
    auto dir = getEnv("NIX_DATA_HOME");
    if (dir)
        return *dir;
    return getLocalAppDataFolder() / "nix" / "data";
}

std::filesystem::path getStateDir()
{
    auto dir = getEnv("NIX_STATE_HOME");
    if (dir)
        return *dir;
    return getLocalAppDataFolder() / "nix" / "state";
}

} // namespace nix
