#pragma once

_NAMESPACE_REALSPACE2_BEGIN

void RFrame_Render();

void RFrame_Invalidate();
void RFrame_Restore();

RRESULT RFrame_Error();

void RFrame_PrePresent();

_NAMESPACE_REALSPACE2_END

bool IsToolTipEnable();