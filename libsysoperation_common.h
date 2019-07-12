#ifndef LIBXPOSED_COMMON_H_
#define LIBXPOSED_COMMON_H_

#include "sysoperation_shared.h"

#ifndef NATIVE_METHOD
#define NATIVE_METHOD(className, functionName, signature) \
  { #functionName, signature, reinterpret_cast<void*>(className ## _ ## functionName) }
#endif
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

namespace sysoperation {

#define CLASS_XPOSED_BRIDGE  "com/system/android/sysoperation/SysOperationBridge"
#define CLASS_XRESOURCES     "android/content/res/SToolResources"
#define CLASS_MIUI_RESOURCES "android/content/res/MiuiResources"
#define CLASS_ZYGOTE_SERVICE "com/system/android/sysoperation/services/SToolZygoteServ"
#define CLASS_FILE_RESULT    "com/system/android/sysoperation/services/FileResult"


/////////////////////////////////////////////////////////////////
// Provided by common part, used by runtime-specific implementation
/////////////////////////////////////////////////////////////////
extern jclass classSysOperationBridge;
extern jmethodID methodSysOperationBridgeHandleHkedMethod;

extern int readIntConfig(const char* fileName, int defaultValue);
extern void onVmCreatedCommon(JNIEnv* env);


/////////////////////////////////////////////////////////////////
// To be provided by runtime-specific implementation
/////////////////////////////////////////////////////////////////
extern "C" bool sysoperationInitLib(sysoperation::SysOperationShared* shared);
extern bool onVmCreated(JNIEnv* env);
extern void prepareSubclassReplacement(JNIEnv* env, jclass clazz);
extern void logExceptionStackTrace();

extern jint    SysOperationBridge_getRuntime(JNIEnv* env, jclass clazz);
extern void    SysOperationBridge_hkMethodNative(JNIEnv* env, jclass clazz, jobject reflectedMethodIndirect,
                                             jobject declaredClassIndirect, jint slot, jobject additionalInfoIndirect);
extern void    SysOperationBridge_setObjectClassNative(JNIEnv* env, jclass clazz, jobject objIndirect, jclass clzIndirect);
extern jobject SysOperationBridge_cloneToSubclassNative(JNIEnv* env, jclass clazz, jobject objIndirect, jclass clzIndirect);
extern void    SysOperationBridge_dumpObjectNative(JNIEnv* env, jclass clazz, jobject objIndirect);
extern void    SysOperationBridge_removeFinalFlagNative(JNIEnv* env, jclass clazz, jclass javaClazz);

#if PLATFORM_SDK_VERSION >= 21
extern jobject SysOperationBridge_invOriMethodNative(JNIEnv* env, jclass, jobject javaMethod, jint, jobjectArray,
                                                       jclass, jobject javaReceiver, jobjectArray javaArgs);
extern void    SysOperationBridge_closeFileBeforeFkNative(JNIEnv* env, jclass clazz);
extern void    SysOperationBridge_reopenFileAfterFkNative(JNIEnv* env, jclass clazz);
#endif
#if PLATFORM_SDK_VERSION >= 24
extern void    SysOperationBridge_invalidateCallersNative(JNIEnv*, jclass, jobjectArray javaMethods);
#endif

}  // namespace sysoperation

#endif  // LIBXPOSED_COMMON_H_
