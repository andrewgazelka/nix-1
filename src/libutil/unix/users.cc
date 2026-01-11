#include "nix/util/util.hh"
#include "nix/util/users.hh"
#include "nix/util/environment-variables.hh"
#include "nix/util/file-system.hh"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

namespace nix {

std::string getUserName()
{
    auto pw = getpwuid(geteuid());
    std::string name = pw ? pw->pw_name : getEnv("USER").value_or("");
    if (name.empty())
        throw Error("cannot figure out user name");
    return name;
}

std::filesystem::path getHomeOf(uid_t userId)
{
    std::vector<char> buf(16384);
    struct passwd pwbuf;
    struct passwd * pw;
    if (getpwuid_r(userId, &pwbuf, buf.data(), buf.size(), &pw) != 0 || !pw || !pw->pw_dir || !pw->pw_dir[0])
        throw Error("cannot determine user's home directory");
    return pw->pw_dir;
}

std::filesystem::path getHome()
{
    static std::filesystem::path homeDir = []() {
        std::optional<std::string> unownedUserHomeDir = {};
        auto homeDir = getEnv("HOME");
        if (homeDir) {
            // Only use $HOME if doesn't exist or is owned by the current user.
            struct stat st;
            int result = stat(homeDir->c_str(), &st);
            if (result != 0) {
                if (errno != ENOENT) {
                    warn(
                        "couldn't stat $HOME ('%s') for reason other than not existing ('%d'), falling back to the one defined in the 'passwd' file",
                        *homeDir,
                        errno);
                    homeDir.reset();
                }
            } else if (st.st_uid != geteuid()) {
                unownedUserHomeDir.swap(homeDir);
            }
        }
        if (!homeDir) {
            homeDir = getHomeOf(geteuid());
            if (unownedUserHomeDir.has_value() && unownedUserHomeDir != homeDir) {
                warn(
                    "$HOME ('%s') is not owned by you, falling back to the one defined in the 'passwd' file ('%s')",
                    *unownedUserHomeDir,
                    *homeDir);
            }
        }
        return *homeDir;
    }();
    return homeDir;
}

bool isRootUser()
{
    return getuid() == 0;
}

std::filesystem::path getCacheDir()
{
    auto dir = getEnv("NIX_CACHE_HOME");
    if (dir) {
        return *dir;
    } else {
        auto xdgDir = getEnv("XDG_CACHE_HOME");
        if (xdgDir) {
            return std::filesystem::path{*xdgDir} / "nix";
        } else {
            return getHome() / ".cache" / "nix";
        }
    }
}

std::filesystem::path getConfigDir()
{
    auto dir = getEnv("NIX_CONFIG_HOME");
    if (dir) {
        return *dir;
    } else {
        auto xdgDir = getEnv("XDG_CONFIG_HOME");
        if (xdgDir) {
            return std::filesystem::path{*xdgDir} / "nix";
        } else {
            return getHome() / ".config" / "nix";
        }
    }
}

std::vector<std::filesystem::path> getConfigDirs()
{
    std::filesystem::path configHome = getConfigDir();
    auto configDirs = getEnv("XDG_CONFIG_DIRS").value_or("/etc/xdg");
    auto tokens = tokenizeString<std::vector<std::string>>(configDirs, ":");
    std::vector<std::filesystem::path> result;
    result.push_back(configHome);
    for (auto & token : tokens) {
        result.push_back(std::filesystem::path{token} / "nix");
    }
    return result;
}

std::filesystem::path getDataDir()
{
    auto dir = getEnv("NIX_DATA_HOME");
    if (dir) {
        return *dir;
    } else {
        auto xdgDir = getEnv("XDG_DATA_HOME");
        if (xdgDir) {
            return std::filesystem::path{*xdgDir} / "nix";
        } else {
            return getHome() / ".local" / "share" / "nix";
        }
    }
}

std::filesystem::path getStateDir()
{
    auto dir = getEnv("NIX_STATE_HOME");
    if (dir) {
        return *dir;
    } else {
        auto xdgDir = getEnv("XDG_STATE_HOME");
        if (xdgDir) {
            return std::filesystem::path{*xdgDir} / "nix";
        } else {
            return getHome() / ".local" / "state" / "nix";
        }
    }
}

} // namespace nix
