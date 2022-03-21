#include "Scope.h"

Scope::Scope(Configuration &cfg)
{
    struct Color{
    sf::Color Value;
    Color(std::string CName)
    {
        if(CName=="Blue") { Value=sf::Color::Blue; return; }
        if(CName=="Red") { Value=sf::Color::Red; return; }
        if(CName=="Cyan") { Value=sf::Color::Cyan; return; }
        if(CName=="Green") { Value=sf::Color::Green; return; }
        if(CName=="Magenta") { Value=sf::Color::Magenta; return; }
        if(CName=="Transparent") { Value=sf::Color::Transparent; return; }
        if(CName=="White") { Value=sf::Color::White; return; }
        if(CName=="Yellow") { Value=sf::Color::Yellow; return; }
        Value=sf::Color::Black;
    }
    };
    int s;
    sf::String sfs;
    c=&cfg;
    stor.setAveraging(c->Averaging);
    stor.setFrameSize(c->FFTFS);
    stor.setFreqLimit(20000);
    stor.setSampleRate(c->SampleRate);
    stor.Clear(c->FFTWindowing);
    cfg.cfile.SetSection("Graph");
    if(cfg.cfile.CurrentSection=="") { ErrorDesc="No [Graph] section in config"; return; }
    graph.cfg.Font=graph.cfg.getFontNamed(cfg.cfile.GetStringValue("FontName"));
    graph.cfg.WindowTitle=cfg.cfile.GetStringValue("WindowTitle");
    graph.cfg.ScrshotDir = c->ScrShotsDir;
    s = cfg.cfile.GetIntValue("XMin");
    if(s>0) graph.cfg.XDimMin=s;
    s = cfg.cfile.GetIntValue("XMax");
    if(s>0) graph.cfg.XDimMax=s;
    s = cfg.cfile.GetIntValue("YMin");
    if(s>0) graph.cfg.YDimMin=s;
    s = cfg.cfile.GetIntValue("YMax");
    if(s>0) graph.cfg.YDimMax=s;
    s = cfg.cfile.GetIntValue("TitleFontSize");
    if(s>0) graph.cfg.TitleFontSize=s;
    s = cfg.cfile.GetIntValue("AxisTitleFontSize");
    if(s>0) graph.cfg.AxisTitleFontSize=s;
    s = cfg.cfile.GetIntValue("AxisMarksFontSize");
    if(s>0) graph.cfg.AxisMarksFontSize=s;
    s = cfg.cfile.GetIntValue("AxisBold");
    if(s>0) graph.cfg.AxisBold=s;
    s = cfg.cfile.GetIntValue("GridBold");
    if(s>0) graph.cfg.GridBold=s;
    s = cfg.cfile.GetIntValue("FrameRate");
    if(s>0) graph.cfg.FPS=s;
    s = cfg.cfile.GetIntValue("LeftMargin");
    if(s>0) graph.cfg.LMargin=s;
    s = cfg.cfile.GetIntValue("RightMargin");
    if(s>0) graph.cfg.RMargin=s;
    s = cfg.cfile.GetIntValue("TopMargin");
    if(s>0) graph.cfg.TMargin=s;
    s = cfg.cfile.GetIntValue("BottomMargin");
    if(s>0) graph.cfg.BMargin=s;
    s = cfg.cfile.GetIntValue("WindowWidthPercent");
    if(s>0 && s<100) graph.cfg.WinWidthPercent = s;
    s = cfg.cfile.GetIntValue("WindowHeightPercent");
    if(s>0 && s<100) graph.cfg.WinHeightPercent = s;
    sfs = cfg.cfile.GetStringValue("Title");
    if(sfs!="") graph.cfg.Title = sfs.fromUtf8(sfs.begin(), sfs.end());
    sfs = cfg.cfile.GetStringValue("XTitle");
    if(sfs!="") graph.cfg.Xtitle = sfs.fromUtf8(sfs.begin(), sfs.end());
    sfs = cfg.cfile.GetStringValue("YTitle");
    if(sfs!="") graph.cfg.YTitle = sfs.fromUtf8(sfs.begin(), sfs.end());
    f = graph.NewFrame(stor.AmountPnt);
}

Scope::~Scope()
{
    //dtor
}

float   Scope::AmpTodB(float Amplitude)
{
    return 20*log10(1.f*Amplitude/32768.f);
}

void    Scope::AdjustProcInterval(size_t SampleCount)
{
    if(mProcIntv == 0)
    {
        mProcIntv = 100;
        setProcessingInterval(sf::milliseconds(mProcIntv));
        return;
    }
    if(SampleCount < c->FFTFS/4) mProcIntv+=10;
    if(SampleCount > 3*c->FFTFS/4 && mProcIntv > 25) mProcIntv-=5;
    setProcessingInterval(sf::milliseconds(mProcIntv));
}

bool    Scope::GetStatusStringIfChanged(sf::String& text)
{
    static unsigned Interval;
    if(Interval != mProcIntv)
    {
        static const std::string strSampleRate = "[" + std::to_string(c->SampleRate) + "]";
        static const std::string strFFTFrameSz = "[" + std::to_string(c->FFTFS) + "]";
        static const std::string strWindowFunc = "[" + c->FFTWindowing + "]";
        text = strSampleRate + "\t" + strFFTFrameSz + "\t[" + std::to_string(mProcIntv) + "ms]\t" + strWindowFunc;
        Interval = mProcIntv;
        return true;
    }
    return false;
}

bool    Scope::onProcessSamples(const sf::Int16* samples, size_t SampleCount)
{
    AdjustProcInterval(SampleCount);
    if(SampleCount==0) return true;

    for(size_t i=0; i<SampleCount; ++i)
    {
        float Amp = c->MicSigMult * samples[i];
        if (Amp>32767) Amp=32767;
        if (Amp<-32767) Amp=-32767;
        stor.AddSample(Amp);
    }
    stor.DoFFT();

    f->ArrayGraph.clear();
    fpoint fpnt;
    Sfgr::point spnt;
    for(size_t i=0; i<stor.AmountPnt; ++i)
    {
        fpnt = stor.GetPointOptimized(i);
        spnt.XValue = fpnt.Frequency;
        spnt.YValue = AmpTodB(fpnt.Amplitude);
        f->ArrayGraph.push_back(spnt);
    }

    sf::String txt;
    if(GetStatusStringIfChanged(txt))
    {
        f->Texts.clear();
        graph.AddHText(f, graph.cfg.Vidmode.width/2, graph.cfg.Vidmode.height-5, txt, 11, sf::Color::Black, sf::Text::Regular, Sfgr::TextPos::CENTER);
    }

    graph.ShowFrame(f);
    return true;
}

void    Scope::Start()
{
    graph.Start();
    start(c->SampleRate);
    t=&graph.t;
}

void    Scope::Stop()
{
    stop();
    graph.Stop();
}
