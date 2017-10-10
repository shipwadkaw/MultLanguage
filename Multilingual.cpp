/*
Copyright(c) <2008-2010> shiplence.alpheilp
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
advertising materials, and other materials related to such
distribution and use acknowledge that the software was developed
by the <organization>.The name of the
<organization> may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
#include "Multilingual.h"

#include <stdlib.h>
#include <ctime>
using namespace std;

// ////////////////////////////////////////////////////////////////////////////////////////////
Multilingual::Multilingual()
{
	m_curLanguage = "english";
}

Multilingual::~Multilingual()
{
    unloadPack();
}

Multilingual* Multilingual::getInstance()
{
	static Multilingual* instance = NULL;
	if(!instance)
	{
		instance = new Multilingual();
	}
	return instance;
}


const char* Multilingual::getTxt(const std::string & name, const std::string & sheet) const
{
	std::map<std::string, std::map<std::string, size_t> >::const_iterator itSheet = m_indexs.find(sheet);

	if (itSheet == m_indexs.end())
	{
		printf("ERR: cannot find sheet %s\n", sheet.c_str());
		return 0;
	}

	std::map<std::string, size_t>::const_iterator it = itSheet->second.find(name);


	if (it == itSheet->second.end())
	{
		printf("ERR: cannot find symbol %s in sheet %s\n", name.c_str(), sheet.c_str());
		return 0;
	}

	size_t stringid = it->second;
 

	std::map<std::string, StringList>::const_iterator itList = m_lists.find(sheet);

	if (itList == m_lists.end())
	{
		printf("ERR: cannot find sheet %s 2\n", sheet.c_str());
		return 0;
	}

	return *(itList->second + stringid);
}


bool Multilingual::preloadPackSheet(const std::string & language, const std::string & sheet)
{
	if (!preLoadPacksIndex(sheet))
		return false;

	char			filename[100];
	sprintf(filename, "lc/%s_%s.bin", sheet.c_str(), language.c_str());

	FILE* fs = fopen(filename,"rb");
	if (fs != NULL)
	{
		StringList	slist;
		unsigned short	numStrings;
		unsigned short	strLength;

		fread(&numStrings, sizeof(numStrings), 1, fs);
		slist = new char*[numStrings + 1];

		for (unsigned short i = 0; i < numStrings; ++i)
		{	
			fread(&strLength,sizeof(strLength),1,fs);
			slist[i] = new char[strLength + 1];
			fread(slist[i], strLength,1,fs);			
			slist[i][strLength] = '\0';
		}

		slist[numStrings] = NULL;

		m_lists.insert(pair<std::string, StringList>(sheet, slist));
		m_count.insert(pair<std::string, short>(sheet, numStrings));

		return true;
	}
	else
	{
		printf("[Multilingual] : ERROR opening %s\n", filename);
		return false;
	}
}


bool Multilingual::preLoadPacksIndex(const std::string & sheet)
{
	std::map<std::string, size_t> map;

	m_indexs.insert(pair<std::string, std::map<std::string, size_t> >(sheet, map));

	char filename[128];
	sprintf(filename, "lc/%s.i", sheet.c_str());

	FILE* fs = fopen(filename,"rb");
	if (fs != NULL)
	{
		int index = 0;
		unsigned short numStrings;
		unsigned short strLength;
		
		fread(&numStrings,sizeof(numStrings),1,fs);
		char buf[512] = { 0 };
		for (size_t i = 0; i < numStrings; ++i)
		{	
			fread(&strLength,sizeof(strLength),1,fs);
			char* pChar = buf;			
			fread(pChar,strLength,1,fs);
			pChar[strLength] = '\0';
			m_indexs[sheet].insert(pair<std::string, size_t>(pChar, index++));
		}
		printf("Found %d text in sheet %s\n", numStrings, sheet.c_str());
		return true;
	}	
	return false;
}

bool Multilingual::preloadPack(const std::string & language, const std::vector<std::string> & sheets)
{
	unloadPack();
	bool ret = true;
	for (size_t index = 0; index < sheets.size(); ++index)
	{
		if (!preloadPackSheet(language, sheets[index]))
			ret = false;
	}
	return ret;
}

void Multilingual::unloadPack()
{
	std::map<std::string, StringList>::iterator itList;
	std::map<std::string, short>::iterator itCount = m_count.begin();
	while (itCount != m_count.end())
	{
		itList = m_lists.find(itCount->first);
		unsigned short numStrings = itCount->second;

		if (itList != m_lists.end())
		{
			StringList slist = itList->second;
			for (unsigned short i = 0; i < numStrings; ++i)
			{
				delete []slist[i];
			}
			delete[] slist;
		}

		itCount++;
	}

	m_lists.clear();
	m_count.clear();
	m_indexs.clear();
}

// ////////////////////////////////////////////////////////////////////////////////////////////
bool Multilingual::switchPack(const std::string & language, const std::vector<std::string> & sheets)
{
	m_curLanguage = language;
	printf("changePack %s", language.c_str());
	return preloadPack(language, sheets);
}


const char* Multilingual::getText(const char* uri) const
{
	const char* str = 0;
	if ((str = strstr(uri, "LC_")) != uri)
	{
		printf("ERR: %s is not a string uri.\n", uri);
		return 0;
	}
	std::string sheetstr(str);
	std::string::size_type pos = sheetstr.find_first_of('_');
	sheetstr = sheetstr.substr(pos + 1);
	pos = sheetstr.find_first_of('_');
	if (pos == std::string::npos)
	{
		printf("ERR: %s is not a string uri.\n", uri);
		return 0;
	}
	std::string s = sheetstr.substr(0, pos);

	char lower_sheetname[128] = { 0 };

	int nLoop;
	for (nLoop = 0; nLoop < s.size(); nLoop++)
	{
		lower_sheetname[nLoop] = tolower(s[nLoop]);
	}
	lower_sheetname[nLoop] = '\0';


	s = sheetstr.substr(pos + 1);
	const char* pSL = strstr(s.c_str(), "SLHM_");
	const char* pML = strstr(s.c_str(), "MLHM_");
	if (pSL == s.c_str() || pML == s.c_str())
	{
		s = s.substr(5);
	}

	return getTxt(s, lower_sheetname);
}


int main()
{
	std::vector<std::string> sheets;
	sheets.push_back("test1");
	sheets.push_back("test2");
	std::string language = "cn"; 
	GetStringMgr()->switchPack(language,sheets);

	const char* str = GetStringMgr()->getText("LC_TEST1_TEXT_1");

	printf("%s \n",str);
	return 0;
}