#include "handle_ini.h"

Handle_ini::SplitLine::SplitLine(string Line)
{
    IsComment   =   false;
    IsSection   =   false;
    IsKeyValue  =   false;
    IsEmpty     =   false;
    Name        =   "";
    Value       =   "";
    size_t Pos  =   0;
    if(Line.find("#")==0){
        IsComment = true;
        return;
    }
    Pos = Line.find("]");
    if(Pos!=string::npos && Line.find("[")==0){
        Name = Line.substr(1, Pos-1);
        IsSection = true;
        return;
    }
    Pos = Line.find("=");
    if(Pos!=string::npos){
        Name  = Line.substr(0, Pos);
        Value = Line.substr(Pos +1);
        IsKeyValue = true;
        return;
    }
    IsEmpty=true;
}

Handle_ini::Handle_ini()
{
    iniContent.clear();
    IsLoaded=false;
}

Handle_ini::Handle_ini(string Filename)
{
    LoadFile(Filename);
}

void    Handle_ini::AddComment(string Comment)
{
    string AddString = "# " + Comment;
    if(SectionEndIter!=iniContent.end()-1) iniContent.insert(SectionEndIter, AddString);
    else iniContent.push_back(AddString);
    SetSection(CurrentSection);
}

void    Handle_ini::AddSection(string SectionName)
{
    bool SectionNotExist = true;
    string FindLine = "[" + SectionName + "]";
    for(string Line : iniContent) if(Line==FindLine) { SectionNotExist=false; break; }
    if(SectionNotExist) iniContent.push_back(FindLine);
}

void    Handle_ini::SetSectionNone()
{
    Section="";
    SectionBegIter=iniContent.begin();
    unsigned int SecEndPos = 0;
    for(string Line : iniContent)
    {
        SplitLine sline(Line);
        if(sline.IsSection)
        {
            SectionEndIter=SectionBegIter+SecEndPos;
            return;
        }
        ++SecEndPos;
    }
    SectionEndIter=iniContent.end();
}

void    Handle_ini::SetSection(string SectionName)
{
    if(SectionName=="")
    {
        SetSectionNone();
        return;
    }
    bool SectionFound = false;
    SectionBegIter=iniContent.begin();
    SectionEndIter=iniContent.end()-1;
    unsigned int Pos=0;
    for(string Line : iniContent)
    {
        SplitLine sline(Line);
        if(!SectionFound && sline.IsSection && sline.Name==SectionName)
        {
            SectionFound=true;
            Section=SectionName;
            SectionBegIter+=Pos;
        }
        if(SectionFound && sline.IsSection && sline.Name!=SectionName)
        {
            SectionEndIter=iniContent.begin()+Pos;
            return;
        }
        ++Pos;
    }
    if(!SectionFound) { SetSectionNone(); return; }
}

void    Handle_ini::LoadFile(string Filename)
{
    IsLoaded=false;
    iniContent.clear();
    std::ifstream inifile(Filename);
    if(!inifile.is_open()) return;
    string line;
    while(std::getline(inifile, line))
    {
        SplitLine sline(line);
        if(!sline.IsEmpty) iniContent.push_back(line);
    }
    inifile.close();
    SetSection("");
    IsLoaded=true;
}

string  Handle_ini::GetStringValue(string Keyname)
{
    if(Keyname=="") return "";
    vector<string>::iterator KeyPos = SectionBegIter;
    while(KeyPos!=SectionEndIter)
    {
        ++KeyPos;
        string Line = *KeyPos;
        SplitLine sline(Line);
        if(sline.IsKeyValue && sline.Name==Keyname) return sline.Value;
    }
    return "";
}

int     Handle_ini::GetIntValue(string Keyname)
{
    string Keyvalue=GetStringValue(Keyname);
    if(Keyvalue!="") return stoi(GetStringValue(Keyname));
    return 0;
}

float   Handle_ini::GetFloatValue(string Keyname)
{
    string Keyvalue=GetStringValue(Keyname);
    if(Keyvalue!="") return stof(GetStringValue(Keyname));
    return 0.f;
}

bool    Handle_ini::GetBoolValue(string Keyname)
{
    if(GetStringValue(Keyname)=="yes") return true;
    return false;
}

void    Handle_ini::SetStringValue(string Keyname, string Keyvalue)
{
    if(Keyname=="") return;
    string NewLine = Keyname + "=" + Keyvalue;
    vector<string>::iterator KeyPos = SectionBegIter;
    while(KeyPos!=SectionEndIter)
    {
        ++KeyPos;
        string Line = *KeyPos;
        SplitLine sline(Line);
        if(sline.IsKeyValue && sline.Name==Keyname)
        {
            *KeyPos = NewLine;
            return;
        }
    }
    iniContent.insert(SectionBegIter + 1, NewLine);
    SetSection(CurrentSection);
}

void    Handle_ini::SetIntValue(string Keyname, int Keyvalue)
{
    SetStringValue(Keyname, std::to_string(Keyvalue));
}

void    Handle_ini::SetFloatValue(string Keyname, float Keyvalue)
{
    SetStringValue(Keyname, std::to_string(Keyvalue));
}

void    Handle_ini::SetBoolValue(string Keyname, bool Keyvalue)
{
    string ValStr;
    if(Keyvalue) ValStr="yes";
    else ValStr="no";
    SetStringValue(Keyname, ValStr);
}

void    Handle_ini::DeleteKey(string Keyname)
{
    if(Keyname=="") return;
    vector<string>::iterator KeyPos = SectionBegIter;
    while(KeyPos!=SectionEndIter)
    {
        ++KeyPos;
        string Line = *KeyPos;
        SplitLine sline(Line);
        if (sline.IsKeyValue && sline.Name==Keyname)
        {
            iniContent.erase(KeyPos);
            SetSection(CurrentSection);
            return;
        }
    }
}

void    Handle_ini::DeleteSection()
{
    string LastStr = *SectionEndIter;
    SplitLine sline(LastStr);
    int AndLast = 1;
    if(sline.IsSection) AndLast = 0;
    iniContent.erase(SectionBegIter, SectionEndIter+AndLast);
    SetSection("");
}

void    Handle_ini::WriteFile(string Filename)
{
    using std::endl;
    std::ofstream WFile;
    WFile.open(Filename, std::ios::trunc);
    for(string line : iniContent)
    {
        SplitLine sline(line);
        if(sline.IsSection) WFile << endl;
        WFile << line << endl;
    }
}
