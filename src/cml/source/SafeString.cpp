#include <cassert>

void(*SafeStringOnOverflowFunc)() = [] { assert(false); };