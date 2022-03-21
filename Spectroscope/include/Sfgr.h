#ifndef SFGR_H
#define SFGR_H
#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <windows.h>
#include <math.h>
#include "SFML/Graphics.hpp"

class Sfgr
{
public:
    enum class TextPos { LEFTER, RIGHTER, UPPER, UNDER, CENTER };
    struct point{ float XValue = 0; float YValue = 0; };
    struct Configuration{
    float                       XDimMin = 10,
                                XDimMax = 20000,
                                YDimMin = -60,
                                YDimMax = 0;
    sf::VideoMode               Vidmode;
    sf::Font                    Font;
    sf::Color                   BGCol, TitleCol, AxisCol, GridCol;
    unsigned                    TitleFontSize, AxisTitleFontSize, AxisMarksFontSize, AxisBold, GridBold;
    unsigned                    LMargin, RMargin, TMargin, BMargin;
    unsigned                    FPS = 30;
    std::string                 WindowTitle, ScrshotDir;
    sf::String                  Title, Xtitle, YTitle;
    std::vector<float>          XGrid, YGrid;
    std::map<float, sf::String> XMarks, YMarks;
    void                        Init();
    sf::Font                    getFontNamed(std::string FontName);
    };
    struct Frame{
        bool                            Locked = false;
        const unsigned long long        &Serial = mSerial;
        sf::Color                       GraphColor = sf::Color::Blue;
        std::vector<point>              ArrayGraph;
        std::vector<sf::CircleShape>    Points;
        std::vector<sf::Text>           Texts;
        std::vector<sf::RectangleShape> Lines;
        void                            ClearGraph();
        friend class Sfgr;
    private:
        unsigned long long              mSerial = 0;
    };
private:
    sf::RenderWindow    Window;
    void                winloop();
    unsigned long long  FramesCounter = 0;
    bool                FramesLocked = false;
    std::vector<Frame*> Frames;
    void                LockFrame(Frame* f);
    void                UnlockFrame(Frame* f);
    void                LockFrames();
    void                UnlockFrames();
public:
    std::thread         t;
    Configuration cfg;
    Sfgr();
    virtual ~Sfgr();
    virtual float   XValToCoord(float XValue);
    virtual float   YValToCoord(float YValue);
    void    Start();
    void    Stop();
    Frame*  NewFrame(unsigned ArrayGraphSize);
    void    DeleteFrame(Frame *f);
    void    ShowFrame(Frame* f);
    void    HideFrame(Frame* f);
    void    AddPoint(Frame* f, float XValue, float YValue, unsigned Radius, sf::Color Col, sf::String txt, unsigned FontSize);
    void    AddHText(Frame* f, float XValue, float YValue, sf::String txt, unsigned FontSize, sf::Color Col, sf::Text::Style style, TextPos Pos);
    void    AddVText(Frame* f, float XValue, float YValue, sf::String txt, unsigned FontSize, sf::Color Col, sf::Text::Style style);
    void    AddVLine(Frame* f, float XValue, unsigned Bold, sf::Color Col, sf::String txt, unsigned FontSize);
    void    AddHLine(Frame* f, float YValue, unsigned Bold, sf::Color Col, sf::String txt, unsigned FontSize);
    void    AddXMark(Frame* f, float XValue, unsigned Bold, sf::Color Col);
    void    AddYMark(Frame* f, float YValue, unsigned Bold, sf::Color Col);
};

#endif // SFGR_H
