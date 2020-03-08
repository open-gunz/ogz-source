#ifndef MSTRINGTABLE_H
#define MSTRINGTABLE_H

#define MSID_MINT		0
#define MSID_OK			1
#define MSID_CANCEL		2
#define MSID_YES		3
#define MSID_NO			4
#define MSID_MESSAGE	5
#define MSID_OVERWRITE	6

const char* MGetString(int nID);
void MSetString(int nID, const char* szString);

#endif