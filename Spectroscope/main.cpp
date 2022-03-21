#include <iostream>
#include <windows.h>
#include "Configuration.h"
#include "Scope.h"

using namespace std;

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    Configuration cfg;
    if(!cfg.LoadConfig(argc, argv)) { cout << cfg.ErrorDesc << endl; return 1; }
    Scope scp(cfg);
    if(scp.ErrorDesc!="") { cout << scp.ErrorDesc << endl; return 1; }
    scp.Start();
    (*scp.t).join();
    scp.Stop();
    return 0;
}
