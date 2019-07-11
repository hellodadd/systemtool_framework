#ifndef XPOSED_H_
#define XPOSED_H_

#include "systemtool_shared.h"

#define XPOSED_PROP_FILE "/system/systemtool.prop"

#if defined(__LP64__)
  #define XPOSED_LIB_DIR "/system/lib64/"
#else
  #define XPOSED_LIB_DIR "/system/lib/"
#endif
#define XPOSED_LIB_DALVIK        XPOSED_LIB_DIR "libsystemtool_dalvik.so"
#define XPOSED_LIB_ART           XPOSED_LIB_DIR "libsystemtool_art.so"
#define XPOSED_JAR               "/system/framework/SystemToolBridge.jar"
#define XPOSED_JAR_NEWVERSION    XPOSED_DIR "bin/SystemToolBridge.jar.newversion"
#define XPOSED_LOAD_BLOCKER      XPOSED_DIR "conf/disabled"
#define XPOSED_SAFEMODE_NODELAY  XPOSED_DIR "conf/safemode_nodelay"
#define XPOSED_SAFEMODE_DISABLE  XPOSED_DIR "conf/safemode_disable"

#define XPOSED_CLASS_DOTS_ZYGOTE "com.system.android.systemtool.SystemToolBridge"
#define XPOSED_CLASS_DOTS_TOOLS  "com.system.android.systemtool.SystemToolBridge$ToolEntryPoint"

#if XPOSED_WITH_SELINUX
#include <selinux/selinux.h>
#define ctx_system ((security_context_t) "u:r:system_server:s0")
#if PLATFORM_SDK_VERSION >= 23
#define ctx_app    ((security_context_t) "u:r:untrusted_app:s0:c512,c768")
#else
#define ctx_app    ((security_context_t) "u:r:untrusted_app:s0")
#endif  // PLATFORM_SDK_VERSION >= 23
#endif  // XPOSED_WITH_SELINUX

namespace systemtool {

    bool handleOptions(int argc, char* const argv[]);
    bool initialize(bool zygote, bool startSystemServer, const char* className, int argc, char* const argv[]);
    void printRomInfo();
    void parseSystemToolProp();
    int getSdkVersion();
    bool isDisabled();
    void disableSystemTool();
    bool isSafemodeDisabled();
    bool shouldSkipSafemodeDelay();
    bool shouldIgnoreCommand(int argc, const char* const argv[]);
    bool addJarToClasspath();
    void onVmCreated(JNIEnv* env);
    void setProcessName(const char* name);
    bool determineSystemToolInstallerUidGid();
    bool switchToSystemToolInstallerUidGid();
    void dropCapabilities(int8_t keep[] = NULL);
    bool isMinimalFramework();

}  // namespace systemtool

#endif  // XPOSED_H_
