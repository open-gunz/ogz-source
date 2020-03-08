#pragma once

class MButton;

class MButtonGroup {
public:
	MButtonGroup();
	~MButtonGroup();

	int nCount;

	MButton *m_pPrevious;
};