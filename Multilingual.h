#pragma once
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
#include <map>
#include <vector>
#include <string>

class Multilingual
{
private:
	typedef char**	StringList;
    std::string m_curLanguage;
    std::map<std::string, StringList> m_lists;
    std::map<std::string, short> m_count;
    std::map<std::string, std::map<std::string, size_t>> m_indexs;
public:
	Multilingual();
	~Multilingual();
	static Multilingual * getInstance();
public:
	void unloadPack();
	bool preLoadPacksIndex(const std::string & sheet);
	bool switchPack(const std::string & language, const std::vector<std::string> & sheets);
    const char* getTxt(const std::string & name, const std::string & sheet) const;
    const char* getText(const char* uri) const;
    bool preloadPack(const std::string & language, const std::vector<std::string> & sheets);
    bool preloadPackSheet(const std::string & language, const std::string & sheet);
    
};

#define GetStringMgr() Multilingual::getInstance()