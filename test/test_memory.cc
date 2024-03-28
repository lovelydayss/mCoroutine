#include "coroutine/memory.h"
#include "coroutine/utils.h"
// #include "gtest/gtest.h"

int main() {

    SETLOGLEVEL(fmtlog::LogLevel::DBG);
    // fmtlog::setLogLevel(fmtlog::LogLevel::OFF);
    SETLOGHEADER("[{l}] [{YmdHMSe}] [{t}] [{g}] ");

    DEBUGFMTLOG("SDASADASDASDASDAS   {}", 11111);
    INFOFMTLOG("AAAAAAAAAAAAAAAAAAAAAAAAAA");
    ERRORFMTLOG("CCCCCCCCCCCCCCCCCCCCCCCCC");

}