#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "handle_ini.h"
#include "SFML/Audio.hpp"
typedef unsigned UINT;

class Configuration
{
    public:
    Handle_ini  cfile;
    string      ConfigFilename,
                ProgramName,
                ProgramPath,
                MicDevName,
                ErrorDesc="",
                FFTWindowing,
                ResultsDir,
                ScrShotsDir,
                CapSection="RecordingDevices";
    UINT        SampleRate,
                Averaging,
                FFTFS;
    float       MicSigMult;

    Configuration();
    virtual ~Configuration();

    bool    LoadConfig(int argc, char* argv[]);
    bool    RefreshConfig();
    void    PrintUsage();

    private:
    UINT        RecDevNum;
};

#endif // CONFIGURATION_H
