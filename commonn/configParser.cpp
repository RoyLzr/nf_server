#include "configParser.h"

static const std::string empty_str = "";

const std::string & 
Section :: get(const std::string key) const
{
    std::map<std::string, std::string>::const_iterator it;
    it = items.find(key);
    if (it != items.end())
        return it->second;
    return empty_str;
} 


bool 
ConfigParser :: parser_file(const std::string &file)
{
    mSections.erase(mSections.begin(), mSections.end());

    std::ifstream instm(file.c_str());
    if (!instm)
        return false;
    std::string line;
    Section *curSection(NULL);
    
    while(instm.peek() != EOF)
    {
        std::getline(instm, line);
        //del last '\r' char
        
        if (line.empty() || line[0] == '#')
            continue;
        if (*(line.rbegin()) == '\n' || *(line.rbegin()) == '\r')
            line.erase(line.end() - 1);
        
        if ('[' == line[0])     //is scetion
        {
            if (']' != line[line.length()-1])
            {
                std::cerr << "parse line failed! --- " << line << std::endl;
                return false;
            }
            Section val;
            std::string section = line.substr(1,line.length()-2);
            mSections[section] = val;
            curSection = &mSections[section];
         }
         else    //is item
         {
            std::string::size_type pos = line.find('=');
            if (std::string::npos == pos)
            {
                std::cerr << "parse line failed! --- " << line << std::endl;
                return false;
            }
            std::string k,v;
            k = line.substr(0, pos);
            v = line.substr(pos + 1, line.length() - pos - 1);
            curSection->set(k,v);
        }
    }
}


const std::string& 
ConfigParser :: get(const std::string sectionName, const std::string keyName) const
{
    std::map<std::string, Section>::const_iterator itr;
    itr = mSections.find(sectionName);
    if (itr != mSections.end())
    {
        return itr->second.get(keyName);
    }
    return empty_str;
}

void 
Section :: scan() const
{
    std::map<std::string, std::string>::const_iterator it;
    for( it = items.begin(); it != items.end(); it++)
    {
        std::cout << it->first << " : " << it->second << std::endl;
    }
}

void 
ConfigParser :: scan() const
{
    std::map<std::string, Section>::const_iterator it_section;
    for( it_section = mSections.begin(); it_section != mSections.end(); it_section++)
    {
        std::cout << "[" << it_section->first << "] " << std::endl;
        Section temp = it_section->second;
        temp.scan();
    }
}


    //Singleton<ConfigParser>::instance()->parser_file("22.conf");
    //Singleton<ConfigParser>::instance()->scan();
    //Singleton<ConfigParser>::destroy();
