#include "ElmavCrashHandler.h"

#include <QApplication>
#include <QStandardPaths>

#ifdef linux
#include <client/linux/handler/exception_handler.h>
#endif


#ifdef Q_OS_WIN
#include <client/windows/handler/exception_handler.h>
#endif

static google_breakpad::ExceptionHandler* eh=0;

#ifdef Q_OS_LINUX
static google_breakpad::MinidumpDescriptor* md=0;

static bool startCrashReporter(const google_breakpad::MinidumpDescriptor& descriptor,void* context, bool succeeded)
{
    std::cerr << "creating crash dump " << descriptor.path();

    // start the crash reporter
    return succeeded;
}
#endif



#ifdef Q_OS_WIN
static bool startCrashReporter(const wchar_t* dump_path,const wchar_t* id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* ass, bool succeeded)
{
    std::wcerr << "dump path : " << dump_path << std::endl;
    std::wcerr << "dump path : " << *dump_path << std::endl;
    return succeeded;
}
#endif

ElmavCrashHandler::ElmavCrashHandler()
{
    // set up break pad
    QDir dir;
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QDir::separator() + \
                   qApp->organizationName() + QDir::separator() + qApp->applicationName() + QDir::separator() + "crash_dumps" \
                   + QDir::separator() ;
    dir.mkpath(path);

    #ifdef Q_OS_LINUX
        md = new google_breakpad::MinidumpDescriptor(path.toStdString());
        eh = new google_breakpad::ExceptionHandler(*md, 0, startCrashReporter,0, true, -1);
    #endif

    #ifdef Q_OS_WIN
        eh = new google_breakpad::ExceptionHandler(path.toStdWString(), 0, startCrashReporter, 0, google_breakpad::ExceptionHandler::HANDLER_ALL);
    #endif

}

ElmavCrashHandler::~ElmavCrashHandler()
{

    #ifdef Q_OS_LINUX
        delete md;
    #endif

    delete eh;
}

