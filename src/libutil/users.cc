#include "nix/util/util.hh"
#include "nix/util/users.hh"
#include "nix/util/file-system.hh"

namespace nix {

std::filesystem::path createNixStateDir()
{
    std::filesystem::path dir = getStateDir();
    createDirs(dir);
    return dir;
}

std::string expandTilde(std::string_view path)
{
    // TODO: expand ~user ?
    auto tilde = path.substr(0, 2);
    if (tilde == "~/" || tilde == "~")
        return getHome().string() + std::string(path.substr(1));
    else
        return std::string(path);
}

} // namespace nix
