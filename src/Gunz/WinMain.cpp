#include <WTypes.h>

int PASCAL GunzMain(HINSTANCE, HINSTANCE, LPSTR, int);

int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	return GunzMain(this_inst, prev_inst, cmdline, cmdshow);
}