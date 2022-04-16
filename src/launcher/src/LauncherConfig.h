#pragma once

#include "GlobalTypes.h"

namespace LauncherConfig
{

constexpr char ApplicationName[] = "Open GunZ Launcher";
constexpr char LauncherFilename[] = "launcher.exe";
constexpr char PatchDomain[] = "";
constexpr u16 PatchPort = 80; // 80 is the default http port.
constexpr size_t BlockSize = 32 * 1024; // 32 KiB

}