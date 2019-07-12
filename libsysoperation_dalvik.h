#ifndef LIBXPOSED_DALVIK_H_
#define LIBXPOSED_DALVIK_H_

#define ANDROID_SMP 1
#include "Dalvik.h"

#include "libsysoperation_common.h"

namespace sysoperation {

#define XPOSED_OVERRIDE_JIT_RESET_OFFSET XPOSED_DIR "conf/jit_reset_offset"

struct SysOperationHookInfo {
    struct {
        Method originalMethod;
        // copy a few bytes more than defined for Method in AOSP
        // to accomodate for (rare) extensions by the target ROM
        int dummyForRomExtensions[4];
    } originalMethodStruct;

    Object* reflectedMethod;
    Object* additionalInfo;
};

}  // namespace sysoperation

#endif  // LIBXPOSED_DALVIK_H_
