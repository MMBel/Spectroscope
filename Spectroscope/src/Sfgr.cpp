#include "Sfgr.h"

Sfgr::Sfgr(){ cfg.Init(); }

Sfgr::~Sfgr()
{
    if(Window.isOpen()) Window.close();
    t.join();
}
float           Sfgr::YValToCoord(float YValue){
    float ymin = cfg.TMargin;
    float ymax = cfg.Vidmode.height - cfg.BMargin;
    static const float YRatio = float(ymax - cfg.TMargin)/float(cfg.YDimMax - cfg.YDimMin);
    float result = ymin - YValue*YRatio;
    if (result < ymin) result=ymin;
    if (result > ymax) result=ymax;
    return result;
}
float           Sfgr::XValToCoord(float XValue){
    // X0 = a + b * log10(xmin)
    // X1 = a + b * log10(xmax)
    // b = (X1 - X0) / log10 (xmax/xmin)
    // a = X0 - b * log10(xmin)
    float X0, X1, xmin, xmax;
    xmin = cfg.XDimMin;
    xmax = cfg.XDimMax;
    X0 = cfg.LMargin;
    X1 = cfg.Vidmode.width - cfg.RMargin;
    static const float b = (X1-X0)/log10(xmax/xmin);
    static const float a = X0 - b * log10(xmin);
    float result = a + b * log10(XValue);
    if (result < X0) result = X0;
    if (result > X1) result = X1;
    return result;
}
void            Sfgr::Configuration::Init(){
    using namespace std;
    WindowTitle = "Graphic";
    Title = L"Амплитудно-частотная характеристика";
    YTitle = L"Амплитуда, dBa";
    Xtitle = L"Частота, Гц";
    Vidmode = sf::VideoMode::getDesktopMode();
    // Vidmode.width-=Vidmode.width*0.25;
     Vidmode.height-=Vidmode.height*0.25;
    Font = getFontNamed("tahoma");
    ScrshotDir = "";
    BGCol=sf::Color(235,235,235);
    TitleCol=sf::Color::Black;
    AxisCol=sf::Color::Black;
    GridCol=sf::Color(200,200,200,128);
    TitleFontSize=22;
    AxisTitleFontSize=18;
    AxisMarksFontSize=16;
    AxisBold=2;
    GridBold=2;
    LMargin=RMargin=Vidmode.width*0.05;
    TMargin=BMargin=Vidmode.height*0.1;
    XGrid.clear();
    for(int i=20; i<100; i+=10) XGrid.push_back(i);
    for(int i=100; i<1000; i+=100) XGrid.push_back(i);
    for(int i=1000; i<=10000; i+=1000) XGrid.push_back(i);
    XGrid.push_back(20000.f);
    YGrid.clear();
    for(int i=-55; i<=0; i+=5) YGrid.push_back(i);
    YMarks.insert(make_pair(-55.f, L"-55"));
    YMarks.insert(make_pair(-50.f, L"-50"));
    YMarks.insert(make_pair(-45.f, L"-45"));
    YMarks.insert(make_pair(-40.f, L"-40"));
    YMarks.insert(make_pair(-35.f, L"-35"));
    YMarks.insert(make_pair(-30.f, L"-30"));
    YMarks.insert(make_pair(-25.f, L"-25"));
    YMarks.insert(make_pair(-20.f, L"-20"));
    YMarks.insert(make_pair(-15.f, L"-15"));
    YMarks.insert(make_pair(-10.f, L"-10"));
    YMarks.insert(make_pair(-5.f,  L"-5 "));
    YMarks.insert(make_pair(0,     L"0 "));
    XMarks.insert(make_pair(10.f,  L"10 Гц"));
    XMarks.insert(make_pair(100.f, L"100 Гц"));
    XMarks.insert(make_pair(1000.f, L"1 КГц"));
    XMarks.insert(make_pair(10000.f, L"10 КГц"));
    XMarks.insert(make_pair(20000.f, L"20 КГц"));
}
sf::Font        Sfgr::Configuration::getFontNamed(std::string FontName){
    TCHAR wdir[MAX_PATH];
    GetWindowsDirectory(wdir, MAX_PATH);
    std::string fontpath;
    #ifndef UNICODE
        fontpath=wdir;
    #else
        wstring wwdir = fontpath;
        wdir=string(wwdir.begin(), wwdir.end());
    #endif // UNICODE
    fontpath+="\\Fonts\\";
    fontpath+=FontName;
    fontpath+=".ttf";
    sf::Font fnt;
    fnt.loadFromFile(fontpath);
    return fnt;
}
Sfgr::Frame*    Sfgr::NewFrame(unsigned ArrayGraphSize){
    Sfgr::Frame* f = new Sfgr::Frame();
    f->mSerial = FramesCounter;
    ++FramesCounter;
    f->ArrayGraph.clear();
    f->ArrayGraph.reserve(ArrayGraphSize);
    return f;
}
void            Sfgr::winloop(){
    sf::VertexArray Graph;
    auto Draw{[](Sfgr::Frame* f, sf::RenderTarget &tgt)
    {
        /*
        for(sf::Text T : f->Texts) tgt.draw(T);
        for(sf::RectangleShape L : f->Lines) tgt.draw(L);
        for(sf::CircleShape C : f->Points) tgt.draw(C);
        */
        for(const auto& V : f->Texts)  tgt.draw(V);
        for(const auto& V : f->Lines)  tgt.draw(V);
        for(const auto& V : f->Points) tgt.draw(V);
    }
    };
    unsigned ScrshotsCount = 0;
    sf::ContextSettings settings;
    settings.antialiasingLevel = 5;
    Window.create(cfg.Vidmode, cfg.WindowTitle, sf::Style::Close, settings);
    Window.setFramerateLimit(cfg.FPS);
    Window.setVerticalSyncEnabled(true);
    sf::RenderTexture MTexture;
    MTexture.create(cfg.Vidmode.width, cfg.Vidmode.height);
    MTexture.clear(cfg.BGCol);
    Draw(Frames[0], MTexture);
    MTexture.display();
    sf::Sprite msp;
    msp.setTexture(MTexture.getTexture());
    while(Window.isOpen()){
        if(Frames.empty()) { Window.close(); return; }
        sf::Event event;
        while(Window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) { Window.close(); return; }
            if(event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Q) { Window.close(); return; }
                if(event.key.code == sf::Keyboard::PageDown){
                    sf::Texture sshot;
                    sshot.create(Window.getSize().x, Window.getSize().y);
                    sshot.update(Window);
                    if(cfg.ScrshotDir!="") sshot.copyToImage().saveToFile(cfg.ScrshotDir+"\\Image_"+std::to_string(ScrshotsCount++)+".jpg");
                }
            }
        }
    Window.draw(msp);
    LockFrames();
    for(unsigned f=1; f<Frames.size(); ++f) {
        LockFrame(Frames[f]);
        Graph.clear();
        Graph.setPrimitiveType(sf::LineStrip);
        Graph.resize(Frames[f]->ArrayGraph.size());
        for(size_t i=0; i<Frames[f]->ArrayGraph.size(); ++i){
            Graph[i].position = sf::Vector2f(XValToCoord(Frames[f]->ArrayGraph[i].XValue), YValToCoord(Frames[f]->ArrayGraph[i].YValue));
            Graph[i].color = Frames[f]->GraphColor;
        }
        if(!Window.isOpen()) return;
        Window.draw(Graph);
        Draw(Frames[f], Window);
        UnlockFrame(Frames[f]);
    }
    UnlockFrames();
    if (Window.isOpen()) Window.display();
    }
}
void            Sfgr::Start(){
    if(Window.isOpen()) return;
    Frames.clear();
    Frame* m = NewFrame(0);
    AddHText(m, cfg.Vidmode.width/2, cfg.TMargin/2, cfg.Title, cfg.TitleFontSize, cfg.TitleCol, sf::Text::Regular, TextPos::CENTER);
    AddVText(m, cfg.LMargin/3, cfg.Vidmode.height/2, cfg.YTitle, cfg.AxisTitleFontSize, cfg.AxisCol, sf::Text::Regular);
    AddHText(m, cfg.Vidmode.width/2, cfg.Vidmode.height-cfg.TMargin/2, cfg.Xtitle, cfg.AxisTitleFontSize, cfg.AxisCol, sf::Text::Regular, TextPos::CENTER);
    AddHLine(m, cfg.YDimMin, cfg.AxisBold, cfg.AxisCol, "", 0);
    AddVLine(m, 0, cfg.AxisBold, cfg.AxisCol, "", 0);
    for(float G : cfg.YGrid) {
        AddYMark(m, G, cfg.AxisBold, cfg.AxisCol);
        AddHLine(m, G, cfg.GridBold, cfg.GridCol, "", 0);
    }
        for(float G : cfg.XGrid) {
        AddXMark(m, G, cfg.AxisBold, cfg.AxisCol);
        AddVLine(m, G, cfg.GridBold, cfg.GridCol, "", 0);
    }
    std::map<float, sf::String>::iterator it = cfg.XMarks.begin();
    while(it!=cfg.XMarks.end()){
        AddHText(m, XValToCoord(it->first), YValToCoord(cfg.YDimMin), it->second, cfg.AxisMarksFontSize, cfg.AxisCol, sf::Text::Regular, TextPos::UNDER);
        it++;
    }
    it = cfg.YMarks.begin();
    while(it!=cfg.YMarks.end()){
        AddHText(m, XValToCoord(cfg.XDimMin), YValToCoord(it->first), it->second, cfg.AxisMarksFontSize, cfg.AxisCol, sf::Text::Regular, TextPos::LEFTER);
        it++;
    }
    ShowFrame(m);
    t = std::thread([this]() {winloop();});
}
void            Sfgr::Stop(){
    LockFrames();
    Frames.clear();
    UnlockFrames();
}
void            Sfgr::ShowFrame(Sfgr::Frame* f){
    LockFrames();
    for(Sfgr::Frame* F : Frames) if(F->Serial==f->Serial) { UnlockFrames(); return; }
    Frames.push_back(f);
    UnlockFrames();
}
void            Sfgr::HideFrame(Sfgr::Frame* f){
    LockFrames();
    std::vector<Sfgr::Frame*>::iterator it = Frames.begin();
    while(it!=Frames.end())
    {
        if((*it)->Serial==f->Serial) { Frames.erase(it); break; }
        it++;
    }
    UnlockFrames();
}
void            Sfgr::DeleteFrame(Frame* f){
    HideFrame(f);
    delete f;
}
void            Sfgr::AddPoint(Frame* f, float XValue, float YValue, unsigned Radius, sf::Color Col, sf::String txt, unsigned FontSize){
    if(XValue==0 || YValue==0) return;
    LockFrame(f);
    sf::Vector2f Position;
    Position.x = XValToCoord(XValue);
    Position.y = YValToCoord(YValue);
    sf::CircleShape* s = new sf::CircleShape;
    s->setRadius(float(Radius));
    s->setFillColor(Col);
    sf::FloatRect r = s->getLocalBounds();
    s->setOrigin(r.width/2, r.height/2);
    s->setPosition(Position);
    f->Points.push_back(*s);
    UnlockFrame(f);
    if(txt=="") return;
    AddHText(f, Position.x, Position.y, txt, FontSize, sf::Color::Black, sf::Text::Style::Regular, TextPos::RIGHTER);
}
void            Sfgr::AddHText(Frame* f, float XValue, float YValue, sf::String txt, unsigned FontSize, sf::Color Col, sf::Text::Style style, TextPos Pos){
    if(XValue==0 || YValue==0) return;
    LockFrame(f);
    sf::Vector2f Position = sf::Vector2f(XValue, YValue);
    sf::Text* text = new sf::Text;
    text->setFont(this->cfg.Font);
    text->setFillColor(Col);
    text->setCharacterSize(FontSize);
    text->setStyle(style);
    text->setString("X");
    sf::FloatRect r = text->getLocalBounds();
    Position.x -= r.width/2;
    Position.y -= r.height/2;
    text->setString(txt);
    r = text->getLocalBounds();
    switch (Pos)
    {
    case (TextPos::CENTER):  { text->setOrigin(r.width/2, r.height/2); } break;
    case (TextPos::LEFTER):  { text->setOrigin(r.width+r.width/2, r.top); } break;
    case (TextPos::RIGHTER): { txt="  "+txt; text->setString(txt); r=text->getLocalBounds(); text->setOrigin(r.left, r.top); } break;
    case (TextPos::UPPER):   { Position.y -= this->cfg.Font.getLineSpacing(FontSize); text->setOrigin(r.left, r.top); } break;
    case (TextPos::UNDER):   { text->setOrigin(r.width/2, r.top); Position.y +=this->cfg.Font.getLineSpacing(FontSize); } break;
    }
    text->setPosition(Position);
    f->Texts.push_back(*text);
    UnlockFrame(f);
}
void            Sfgr::AddVText(Frame* f, float XValue, float YValue, sf::String text, unsigned FontSize, sf::Color Col, sf::Text::Style style){
    if(XValue==0 || YValue==0) return;
    LockFrame(f);
    sf::Text* txt = new sf::Text;
    txt->setFont(this->cfg.Font);
    txt->setCharacterSize(FontSize);
    txt->setString(text);
    txt->setFillColor(Col);
    txt->setStyle(style);
    sf::FloatRect r = txt->getLocalBounds();
    txt->setOrigin(r.width/2, r.height/2);
    txt->setRotation(270.f);
    txt->setPosition(sf::Vector2f(XValue, YValue));
    f->Texts.push_back(*txt);
    UnlockFrame(f);
}
void            Sfgr::AddHLine(Frame* f, float YValue, unsigned Bold, sf::Color Col, sf::String txt, unsigned FontSize){
    LockFrame(f);
    txt+="  ";
    sf::Text t;
    t.setFont(this->cfg.Font);
    t.setCharacterSize(FontSize);
    t.setString(txt);
    sf::FloatRect rt = t.getLocalBounds();
    float Ypos = this->YValToCoord(YValue);
    float HLineLength = float(this->cfg.Vidmode.width - this->cfg.LMargin - this->cfg.RMargin - rt.width);
    float HLinePos = float(this->cfg.LMargin);
    sf::RectangleShape* s = new sf::RectangleShape(sf::Vector2f(HLineLength, float(Bold)));
    s->setFillColor(Col);
    sf::FloatRect r = s->getLocalBounds();
    s->setOrigin(0, r.height/2);
    s->setPosition(sf::Vector2f(HLinePos + rt.width, Ypos));
    f->Lines.push_back(*s);
    UnlockFrame(f);
    AddHText(f, HLinePos, Ypos, txt, FontSize, this->cfg.AxisCol, sf::Text::Regular, TextPos::RIGHTER);
}
void            Sfgr::AddVLine(Frame* f, float XValue, unsigned Bold, sf::Color Col, sf::String txt, unsigned FontSize){
    LockFrame(f);
    sf::Text t;
    t.setFont(this->cfg.Font);
    t.setCharacterSize(FontSize);
    t.setString(txt);
    sf::FloatRect rt = t.getLocalBounds();
    float Xcoord = this->XValToCoord(XValue);
    float VLineLength = float(this->cfg.Vidmode.height - this->cfg.TMargin - this->cfg.BMargin - rt.height*2);
    float VLinePos = float(VLineLength + this->cfg.TMargin);
    sf::RectangleShape* s = new sf::RectangleShape(sf::Vector2f(VLineLength, float(Bold)));
    s->setFillColor(Col);
    sf::FloatRect r = s->getLocalBounds();
    s->setOrigin(0, r.height/2);
    s->setRotation(270.f);
    s->setPosition(sf::Vector2f(Xcoord, VLinePos));
    f->Lines.push_back(*s);
    UnlockFrame(f);
    AddHText(f, Xcoord, VLinePos + rt.height, txt, FontSize, this->cfg.AxisCol, sf::Text::Regular, TextPos::CENTER);
}
void            Sfgr::AddYMark(Frame* f, float YValue, unsigned Bold, sf::Color Col){
    LockFrame(f);
    float Ypos = this->YValToCoord(YValue);
    static const float YMarkLength = float(this->cfg.AxisBold*3);
    static const float YmarkXpos = float(this->cfg.LMargin) - YMarkLength;
    sf::RectangleShape* s = new sf::RectangleShape(sf::Vector2f(YMarkLength, Bold));
    s->setFillColor(Col);
    sf::FloatRect r = s->getLocalBounds();
    s->setOrigin(0, r.height/2);
    s->setPosition(sf::Vector2f(YmarkXpos, Ypos));
    f->Lines.push_back(*s);
    UnlockFrame(f);
}
void            Sfgr::AddXMark(Frame* f, float XValue, unsigned Bold, sf::Color Col){
    LockFrame(f);
    float Xpos = this->XValToCoord(XValue);
    static const float XMarkLength = float(this->cfg.AxisBold*3);
    static const float XmarkYpos = float(this->cfg.Vidmode.height - this->cfg.BMargin) + XMarkLength;
    sf::RectangleShape* s = new sf::RectangleShape(sf::Vector2f(XMarkLength, Bold));
    s->setFillColor(Col);
    sf::FloatRect r = s->getLocalBounds();
    s->setOrigin(0, r.height/2);
    s->setRotation(270.f);
    s->setPosition(sf::Vector2f(Xpos, XmarkYpos));
    f->Lines.push_back(*s);
    UnlockFrame(f);
}
void            Sfgr::LockFrame(Frame* f){
    while(f->Locked) sf::sleep(sf::milliseconds(1));
    f->Locked = true;
}
void            Sfgr::UnlockFrame(Frame* f){
    f->Locked = false;
}
void            Sfgr::LockFrames(){
    if(FramesLocked) sf::sleep(sf::milliseconds(1));
    FramesLocked = true;
}
void            Sfgr::UnlockFrames(){
    FramesLocked = false;
}
