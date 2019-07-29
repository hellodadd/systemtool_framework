#ifndef LIBXPOSED_COMMON_H_
#define LIBXPOSED_COMMON_H_

#include "systemtool_shared.h"

#ifndef NATIVE_METHOD
#define NATIVE_METHOD(className, functionName, signature) \
  { #functionName, signature, reinterpret_cast<void*>(className ## _ ## functionName) }
#endif
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

namespace systemtool {

#define CLASS_XPOSED_BRIDGE  "com/system/android/systemtool/SystemToolBridge"
#define CLASS_XRESOURCES     "android/content/res/SToolResources"
#define CLASS_MIUI_RESOURCES "android/content/res/MiuiResources"
#define CLASS_ZYGOTE_SERVICE "com/system/android/systemtool/services/ZygoteService"
#define CLASS_FILE_RESULT    "com/system/android/systemtool/services/FileResult"


/////////////////////////////////////////////////////////////////
// Provided by common part, used by runtime-specific implementation
/////////////////////////////////////////////////////////////////
extern jclass classSystemToolBridge;
extern jmethodID methodSystemToolBridgeHandleHookedMethod;

extern int readIntConfig(const char* fileName, int defaultValue);
extern void onVmCreatedCommon(JNIEnv* env);


/////////////////////////////////////////////////////////////////
// To be provided by runtime-specific implementation
/////////////////////////////////////////////////////////////////
extern "C" bool systemtoolInitLib(systemtool::SystemToolShared* shared);
extern bool onVmCreated(JNIEnv* env);
extern void prepareSubclassReplacement(JNIEnv* env, jclass clazz);
extern void logExceptionStackTrace();

extern jint    SystemToolBridge_getRuntime(JNIEnv* env, jclass clazz);
extern void    SystemToolBridge_hkMethodNative(JNIEnv* env, jclass clazz, jobject reflectedMethodIndirect,
                                             jobject declaredClassIndirect, jint slot, jobject additionalInfoIndirect);
extern void    SystemToolBridge_setObjectClassNative(JNIEnv* env, jclass clazz, jobject objIndirect, jclass clzIndirect);
extern jobject SystemToolBridge_cloneToSubclassNative(JNIEnv* env, jclass clazz, jobject objIndirect, jclass clzIndirect);
extern void    SystemToolBridge_dumpObjectNative(JNIEnv* env, jclass clazz, jobject objIndirect);
extern void    SystemToolBridge_removeFinalFlagNative(JNIEnv* env, jclass clazz, jclass javaClazz);

#if PLATFORM_SDK_VERSION >= 21
extern jobject SystemToolBridge_invokeOriMethodNative(JNIEnv* env, jclass, jobject javaMethod, jint, jobjectArray,
                                                       jclass, jobject javaReceiver, jobjectArray javaArgs);
extern void    SystemToolBridge_closeFilesBeforeForkNative(JNIEnv* env, jclass clazz);
extern void    SystemToolBridge_reopenFilesAfterForkNative(JNIEnv* env, jclass clazz);
#endif
#if PLATFORM_SDK_VERSION >= 24
extern void    SystemToolBridge_invalidateCallersNative(JNIEnv*, jclass, jobjectArray javaMethods);
#endif

}  // namespace systemtool

#endif  // LIBXPOSED_COMMON_H_
