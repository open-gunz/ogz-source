# Launcher

A basic command-line launcher application for automatic updating, using an rsync-like algorithm for partial updating of files, a file cache for fast loading when there's no update, and a simple GUI using ImGui on D3D9 to display the status. Patch files are created with the `../PatchCreator` program. The source server and other options can be configured in `src/LauncherConfig.h`.