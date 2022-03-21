#include "Configuration.h"

Configuration::Configuration()
{
    //ctor
}

Configuration::~Configuration()
{
    //dtor
}

bool    Configuration::LoadConfig(int argc, char* argv[])
{
    using namespace std;
    ProgramPath = argv[0];
    for(int i=ProgramPath.size()-1; i>0; i--) if(ProgramPath[i]=='\\')
    {
        ProgramName=ProgramPath.substr(i+1, ProgramPath.size()-1);
        ProgramPath=ProgramPath.substr(0, i);
        break;
    }
    if(argc!=2) { PrintUsage(); ErrorDesc=" "; return false; }
    ConfigFilename = argv[1];
    cfile.LoadFile(ConfigFilename);
    if(!cfile.IsLoaded){ ErrorDesc="Can't load" + ConfigFilename; return false; }
    cfile.SetSection("General");
    if(cfile.CurrentSection=="") { ErrorDesc="Config: отсутствует секция [General] в файле конфигурации"; return false; }
    SampleRate = cfile.GetIntValue("SampleRate");
    switch(SampleRate)
    {
        case(22050): break;
        case(44100): break;
        case(48000): break;
        case(96000): break;
        case(192000): break;
        default: ErrorDesc="Config: недопустимое значение SampleRate (" + to_string(SampleRate) + ")"; return false;
    }
    FFTFS = cfile.GetIntValue("FFTFrameSize");
    switch(FFTFS)
    {
        case(1024): break;
        case(2048): break;
        case(4096): break;
        case(8192): break;
        case(16384): break;
        case(32768): break;
        default: ErrorDesc="Config: недопустимое значение FFTFrameSize (" + to_string(FFTFS) + ")\nДолжно быть одно из: 1024, 2048, 4096, 8192, 16384, 32768"; return false;
    }
    FFTWindowing= cfile.GetStringValue("FFTWindowing");
    ResultsDir  = cfile.GetStringValue("ResultsDir");
    ScrShotsDir = cfile.GetStringValue("ScreenShotsDir");
    RecDevNum   = cfile.GetIntValue("RecDevNumber");
    MicSigMult  = cfile.GetFloatValue("MicSignalMultiplier");
    if(MicSigMult<-100.f || MicSigMult==0 || MicSigMult>100.f) { ErrorDesc="Config: ошибочное значение MicSignalMultiplier, должно быть -100.0...+100.0 кроме 0"; return false; }
    Averaging   = cfile.GetIntValue("Averaging");
    if(Averaging<1 || Averaging>50) { ErrorDesc="Config: ошибочное значение Averaging (" + to_string(Averaging) + "), должно быть в пределах 1...50"; return false; }
    if(!RefreshConfig()) return false;
    cfile.SetSection(CapSection);
    MicDevName=cfile.GetStringValue(std::to_string(RecDevNum));
    if(cfile.CurrentSection=="") { ErrorDesc="Config: отсутствует раздел [" + CapSection + "] в файле конфигурации"; return false; }
    if(MicDevName=="") { ErrorDesc="Config: неверное значение RecDevNumber, проверьте и перезапустите программу"; return false; }
    return true;
}

bool    Configuration::RefreshConfig()
{
    if(!cfile.IsLoaded) { ErrorDesc="Конфиг не загружен"; return false; }
    cfile.SetSection(CapSection);
    if(cfile.CurrentSection=="") { cfile.AddSection(CapSection); cfile.SetSection(CapSection); }
    cfile.DeleteSection();
    cfile.AddSection(CapSection);
    cfile.SetSection(CapSection);
    vector<string> RDevList = sf::SoundRecorder::getAvailableDevices();
    if(RDevList.empty()) { ErrorDesc="Ошибка: устройства звукозаписи не найдены"; return false; }
    for(UINT i=0; i<RDevList.size(); ++i) cfile.SetStringValue(std::to_string(i), RDevList[i]);
    cfile.WriteFile(ConfigFilename);
    return true;
}

void    Configuration::PrintUsage()
{
    using namespace std;
    cout << "Usage:" << endl;
    cout << ProgramName << " config-file-fullpath\t\tЗапуск с указанным файлом конфигурации" << endl;
}
