#ifndef HANDLE_INI_H_INCLUDED
#define HANDLE_INI_H_INCLUDED
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

/*

    Min.M.Belovolov

                            NOTES
    Загружаем ini-файл: Handle_ini cfile("C:\\test\\test.ini");
        В случае успеха будет cfile.IsLoaded=true;

    Указываем имя текущей секции: cfile.SetSection("[section]")
        имя текущей секции будет в cfile.CurrentSection

        Если имя ключа не существует:
        GetBoolValue("wrongkey") = false;
        GetStringValue("wrongkey")="";
        GetIntValue("wrongkey")=0;
        GetFloatValue("wrongkey")=0;

    По-умолчанию геттеры и сеттеры действуют в секции "" (без [имени])
        она автоматически устанавливается при:
           - действии конструктора,
           - загрузке файла,
           - удалении текущей секции
           - попытке перейти в не существующую секцию

    новые ключи добавляются в начало секции,
    новые комментарии добавляются в конец секции,

    Выгрузка файла: cfile.WriteFile(string NewFullpath) файл будет перезаписан!


*/

using std::string;
using std::vector;

class Handle_ini{
    struct SplitLine{
        bool    IsComment,
                IsKeyValue,
                IsSection,
                IsEmpty;
        string  Name,
                Value;
        SplitLine(string Line);
    };
    vector<string>  iniContent;
    vector<string>::iterator SectionBegIter;
    vector<string>::iterator SectionEndIter;
    string  Section = "";
    void    SetSectionNone();
    public:
    Handle_ini();
    Handle_ini(string Filename);
    bool    IsLoaded;
    const   string &CurrentSection = Section;
    void    AddComment(string Comment);
    void    AddSection(string SectionName);
    void    SetSection(string SectionName);
    string  GetStringValue(string Keyname);
    int     GetIntValue(string Keyname);
    float   GetFloatValue(string Keyname);
    bool    GetBoolValue(string Keyname);
    void    SetStringValue(string Keyname, string Keyvalue);
    void    SetIntValue(string Keyname, int Keyvalue);
    void    SetFloatValue(string Keyname, float Keyvalue);
    void    SetBoolValue(string Keyname, bool Keyvalue);
    void    DeleteKey(string Keyname);
    void    DeleteSection();
    void    LoadFile(string Filename);
    void    WriteFile(string Filename);
};


#endif // HANDLE_INI_H_INCLUDED
