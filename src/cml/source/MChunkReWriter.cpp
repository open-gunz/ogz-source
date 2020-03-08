#include "stdafx.h"
#include "MChunkReWriter.h"

MChunkWriter::MChunkWriter(void)
{
	m_fp = NULL;
	m_pCurrChunkHeader = NULL;
}

void MChunkWriter::Start(FILE* fp)
{
	m_fp = fp;
	m_pCurrChunkHeader = NULL;
	m_nInitialPos = ftell(fp);
	fseek(fp, sizeof(MCHUNKLISTINFO), SEEK_CUR);	// Chunk Header List의 위치를 저장할 공간 확보
}

void MChunkWriter::Begin(int nChunkID)
{
	_ASSERT(m_fp!=NULL);
	_ASSERT(m_pCurrChunkHeader==NULL);	// Begin은 초기에 불리거나, End()후에 불려야 한다.
	m_pCurrChunkHeader = new MCHUNKHEADER;
	m_pCurrChunkHeader->nChunkID = nChunkID;
	m_nCurrChunkPos = ftell(m_fp);
}

void MChunkWriter::End(void)
{
	_ASSERT(m_fp!=NULL);
	_ASSERT(m_pCurrChunkHeader!=NULL);	// End는 Begin후에 불려야 한다.
	long int nPos = ftell(m_fp);
	m_pCurrChunkHeader->nChunkPos = m_nCurrChunkPos;
	m_pCurrChunkHeader->nChunkSize = nPos - m_nCurrChunkPos;
	m_ChunkHeaders.Add(m_pCurrChunkHeader);
	m_pCurrChunkHeader = NULL;
}

void MChunkWriter::Finish(void)
{
	_ASSERT(m_fp!=NULL);
	long int nPos = ftell(m_fp);
	fseek(m_fp, m_nInitialPos, SEEK_SET);
	MCHUNKLISTINFO cli;
	cli.nID = MCHUNKLISTINFOID;
	cli.nChunkListPos = nPos;
	cli.nChunkListCount = m_ChunkHeaders.GetCount();
	fwrite(&cli, sizeof(MCHUNKLISTINFO), 1, m_fp);
	fseek(m_fp, nPos, SEEK_SET);

	for(int i=0; i<m_ChunkHeaders.GetCount(); i++){
		MCHUNKHEADER* pCH = m_ChunkHeaders.Get(i);
		fwrite(pCH, sizeof(MCHUNKHEADER), 1, m_fp);
	}
}

MChunkReader::MChunkReader(void)
{
	m_fp = NULL;
}


bool MChunkReader::Start(FILE* fp)
{
	fread(&m_ChunkListInfo, sizeof(MCHUNKLISTINFO), 1, fp);
	if(m_ChunkListInfo.nID!=MCHUNKLISTINFOID) return false;
	long int nPos = ftell(fp);
	fseek(fp, m_ChunkListInfo.nChunkListPos, SEEK_SET);
	for(int i=0; i<m_ChunkListInfo.nChunkListCount; i++){
		MCHUNKHEADER* pCH = new MCHUNKHEADER;
		fread(pCH, sizeof(MCHUNKHEADER), 1, fp);
		m_ChunkHeaders.Add(pCH);
	}
	fseek(fp, nPos, SEEK_SET);
	m_fp = fp;
	return true;
}

bool MChunkReader::Seek(int nChunkID)
{
	_ASSERT(m_fp!=NULL);
	for(int i=0; i<m_ChunkHeaders.GetCount(); i++){
		MCHUNKHEADER* pCH = m_ChunkHeaders.Get(i);
		if(pCH->nChunkID==nChunkID){
			fseek(m_fp, pCH->nChunkPos, SEEK_SET);
			return true;
		}
	}
	return false;
}

void MChunkReader::Finish(void)
{
	_ASSERT(m_fp!=NULL);
	fseek(m_fp, m_ChunkListInfo.nChunkListPos+m_ChunkListInfo.nChunkListCount*sizeof(MCHUNKHEADER), SEEK_SET);
}

MCHUNKHEADER* MChunkReader::GetChunkHeader(int nChunkID)
{
	for(int i=0; i<m_ChunkHeaders.GetCount(); i++){
		MCHUNKHEADER* pCH = m_ChunkHeaders.Get(i);
		if(pCH->nChunkID==nChunkID){
			return pCH;
		}
	}
	return NULL;
}
