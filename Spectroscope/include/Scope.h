#ifndef SCOPE_H
#define SCOPE_H
#include "Configuration.h"
#include "fftStorage.h"
#include "Sfgr.h"
#include "SFML/Audio.hpp"


class Scope : public sf::SoundRecorder
{
    Configuration*  c;
    fftStorage      stor;
    Sfgr            graph;
    Sfgr::Frame*    f = nullptr;
    Sfgr::Frame*    p = nullptr;
    sf::Color       GraphColor;
    unsigned        mProcIntv = 0;
    bool            PeakHoldEnable = false;
    bool            GetStatusStringIfChanged(sf::String& text);
    bool            onProcessSamples(const sf::Int16* samples, size_t SampleCount);
    float           AmpTodB(float Amplitude);
    void            AdjustProcInterval(size_t SampleCount);
    public:
    std::thread*    t;
    std::string ErrorDesc="";
    Scope(Configuration &cfg);
    void    Start();
    void    Stop();
    virtual ~Scope();
};

#endif // SCOPE_H
