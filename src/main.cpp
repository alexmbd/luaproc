#include "core/app.hpp"
#include "core/msghandler.hpp"

int main(int argc, char **argv)
{
    if (argc == 1) { LuaProc::conditionalExit(LuaProc::MessageType::CPP_ERROR, LuaProc::Message::GENERIC, "no lua file provided"); }
    else if (argc > 2)
    {
        LuaProc::conditionalExit(LuaProc::MessageType::CPP_ERROR, LuaProc::Message::GENERIC, "more than one lua file was provided");
    }
    LuaProc::Application app(argv[1]);
    app.run();

    return 0;
}