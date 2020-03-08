#ifndef MCHUNKREWRITER_H
#define MCHUNKREWRITER_H

#include <stdio.h>
#include "CMList.h"

#define MCHUNKLISTINFOID	0x12341234
struct MCHUNKLISTINFO{
	int nID;
	long int nChunkListPos;
	int nChunkListCount;
};
struct MCHUNKHEADER{
	long int nChunkID;
	long int nChunkPos;
	long int nChunkSize;
};

class MChunkWriter{
	FILE*							m_fp;
	long							m_nInitialPos;
	MCHUNKHEADER*					m_pCurrChunkHeader;
	long							m_nCurrChunkPos;
	CMLinkedList<MCHUNKHEADER>		m_ChunkHeaders;
public:
	MChunkWriter(void);
	void Start(FILE* fp);

	void Begin(int nChunkID);
	void End(void);

	void Finish(void);
};

class MChunkReader{
	MCHUNKLISTINFO					m_ChunkListInfo;
	CMLinkedList<MCHUNKHEADER>		m_ChunkHeaders;
	FILE*							m_fp;
public:
	MChunkReader(void);
	bool Start(FILE* fp);
	bool Seek(int nChunkID);
	void Finish(void);

	MCHUNKHEADER* GetChunkHeader(int nChunkID);
};

#endif
