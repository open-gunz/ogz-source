#pragma once

#include "MXml.h"

class MQuestFormula
{
private:

public:
	static bool Create(void);
	static int CalcQL(int nMinPlayerLevel);
	static int CalcQLD(int nQuestLevel);
	static int CalcLMT(int nQuestLevel);
	static float CalcTC(int nQuestLevel);
	static void CalcRewardRate(float& foutXPRate, 
							   float& foutBPRate,
                               int nScenarioQL,
                               int nPlayerQL,
                               int nDeathCount,
                               int nUsedPageSacriItemCount,
                               int nUsedExtraSacriItemCount);

	static int CalcSectorXP(int nClearXP, int nSectorCount);
};