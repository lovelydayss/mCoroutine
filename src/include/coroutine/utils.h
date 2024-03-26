#ifndef MCOROUTINE_COMMON_UTILS_H
#define MCOROUTINE_COMMON_UTILS_H

#if __cplusplus >= 201703L

#define FMTLOG_HEADER_ONLY
#include "fmtlog/fmtlog.h"

#endif


#define MCOROUTINE_NAMESPACE_BEGIN namespace mcoroutine {

#define MCOROUTINE_NAMESPACE_END }      /* end of namespace mcoroutine */

MCOROUTINE_NAMESPACE_BEGIN

#if __cplusplus >= 201703L

#define FMTLOG_HEADER_ONLY
#include "fmtlog/fmtlog.h"

#define SETLOGLEVEL(level) fmtlog::setLogLevel(level);
#define SETLOGHEADER(header) fmtlog::setHeaderPattern(header)

#define DEBUGFMTLOG(str, ...) logd(str, ##__VA_ARGS__)
#define INFOFMTLOG(str, ...) logi(str, ##__VA_ARGS__)
#define WARNINGFMTLOG(str, ...) logw(str, ##__VA_ARGS__)
#define ERRORFMTLOG(str, ...) loge(str, ##__VA_ARGS__)

#define DEBUGFMTLOG_ONCE(str, ...) logdo(str, ##__VA_ARGS__)
#define INFOFMTLOG_ONCE(str, ...) logio(str, ##__VA_ARGS__)
#define WARNINGFMTLOG_ONCE(str, ...) logwo(str, ##__VA_ARGS__)
#define ERRORFMTLOG_ONCE(str, ...) logeo(str, ##__VA_ARGS__)

#else

#define SETLOGLEVEL(level) 
#define SETLOGHEADER(header) 

#define DEBUGFMTLOG(str, ...) 
#define INFOFMTLOG(str, ...) 
#define WARNINGFMTLOG(str, ...) 
#define ERRORFMTLOG(str, ...) 

#define DEBUGFMTLOG_ONCE(str, ...) 
#define INFOFMTLOG_ONCE(str, ...) 
#define WARNINGFMTLOG_ONCE(str, ...) 
#define ERRORFMTLOG_ONCE(str, ...)

#endif


MCOROUTINE_NAMESPACE_END

#endif  