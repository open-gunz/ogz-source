#ifndef _MCSVPARSER_H
#define _MCSVPARSER_H


#include <string.h>
#include <stdio.h>


class MCSVReader 
{
private :
	char*		m_pBuffer;
	int*		m_pOffset;
	int			m_nLine;

	int CountLine(const char* buffer, int* offset);
	int PassToken(const char* str);
	char* ReadFile(const char* fname);
public :
	MCSVReader();
	~MCSVReader();

	bool Load(const char* fname);
	int GetData(int col, int row, char* outptr, int outlen);
};



/////////////////////////////////////////////////////
// sample
//	FILE* fp = fopen("c:\\output.txt", "wt");
//
//	CCSVReader Parser;
//	Parser.Load("c:\\±ÝÄ¢¾î.csv");
//
//	const int LEN = 256;
//	char text[LEN] = "";
//
//	for (int i = 0; i < 1000; i++)
//	{
//		for (int j = 0; j < 1000; j++)
//		{
//          Parser.GetData(j, i, text, 256);
//			if ((text[0] != '\0') && (text[0] != 13))
//			{
//				fprintf(fp, "%s\n", text);
//			}
//		}
//	}
//
//	fclose(fp);
//	printf("OK!!\n");
//	return 0;
//






#endif