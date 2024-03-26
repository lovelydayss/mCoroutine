#include "coroutine/memory.h"
#include "coroutine/utils.h"

int main() {

    SETLOGLEVEL(fmtlog::LogLevel::ERR);
    // fmtlog::setLogLevel(fmtlog::LogLevel::OFF);
    SETLOGHEADER("[{l}] [{YmdHMSe}] [{t}] [{g}] ");

    DEBUGFMTLOG("SDASADASDASDASDAS");
    INFOFMTLOG("AAAAAAAAAAAAAAAAAAAAAAAAAA");
    ERRORFMTLOG("CCCCCCCCCCCCCCCCCCCCCCCCC");

}