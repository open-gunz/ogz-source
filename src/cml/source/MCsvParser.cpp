#include "stdafx.h"
#include "MCsvParser.h"


MCSVReader::MCSVReader()
{
	m_pBuffer = NULL;
	m_pOffset = NULL;
	m_nLine = 0;
}

MCSVReader::~MCSVReader()
{
	if (m_pBuffer)
		delete [] m_pBuffer;
	if (m_pOffset)
		delete [] m_pOffset;
}


char* MCSVReader::ReadFile(const char* fname)
{
	FILE *fp;
	char * ptr;
	int len;

	fp = fopen(fname, "rb");
	if (!fp) return nullptr;

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	ptr = new char [len+1];
	fread(ptr, 1, len, fp);
	ptr[len] = '\0';
	fclose(fp);

	return ptr;
}

bool MCSVReader::Load(const char* fname)
{
//	파일 전체를 읽는다
//
	m_pBuffer = ReadFile(fname);

//	라인수를 센 후 각 오프셋을 구한다
//
	m_nLine = CountLine(m_pBuffer, NULL);
	m_pOffset = new int[m_nLine];
	CountLine(m_pBuffer, m_pOffset);

	return true;
}

int MCSVReader::CountLine(const char* buffer, int* offset)
{
	const char* p = buffer;
	int line;

	for(line=0; p; line++) 
	{
		if (line > 0) p++;

		if (offset)
			offset[line] = (int)(p - buffer);

		p = strchr(p, '\n');
	}

	return line;
}

int MCSVReader::PassToken(const char* str)
{
	int i = 0;

	if (str[0] == '\"') 
	{
		for(i=1; ;) 
		{
			if (str[i] == '\"') 
			{
				if (str[i+1] == '\"') i += 2;
				else break;
			}	
			else
			{
				i += 1;
			}
		}
		return i+1;
	}	
	else 
	{
		for(i=0; !strchr(",\n", str[i]); i++) ;
		return i;
	}
}

int MCSVReader::GetData(int col, int row, char* outptr, int outlen)
{
	int i, off;

	if (row >= m_nLine)
		return 0;

	for(i=0, off=m_pOffset[row]; i<col; off++, i++) 
	{
		off += PassToken(m_pBuffer + off);
		if (strchr("\n", m_pBuffer[off])) 
		{
			outptr[0] = '\0';
			return 0;
		}
	}

	if (m_pBuffer[off] == '\"') 
	{
		for(i=0, off+=1; ; i++) 
		{
			if (m_pBuffer[off] == '\"' && m_pBuffer[off+1] == '\"') 
			{
				if (i < outlen-1)
					outptr[i] = m_pBuffer[off];
				off += 2;
			}
			else
			{
				if (m_pBuffer[off] != '\"') 
				{
					outptr[i] = m_pBuffer[off];
					off += 1;
                }	
				else
				{
					break;
				}
			}
		}
	}	
	else 
	{
		for(i=0; !strchr(",\n", m_pBuffer[off]); i++, off++) 
		{
			if (i < outlen-1)
				outptr[i] = m_pBuffer[off];
		}
	}

	if (i >= outlen-1) outptr[outlen-1] = '\0';
	else outptr[i] = '\0';


	return i;
}
