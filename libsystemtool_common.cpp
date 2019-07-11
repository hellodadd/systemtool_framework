/**
 * This file includes functions shared by different runtimes.
 */

#define LOG_TAG "SystemTool"

#include "libsystemtool_common.h"
#include "JNIHelp.h"
#include <ScopedUtfChars.h>

#define private public
#if PLATFORM_SDK_VERSION == 15
#include <utils/ResourceTypes.h>
#else
#include <androidfw/ResourceTypes.h>
#endif
#undef private

namespace systemtool {

////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////

bool systemtoolLoadedSuccessfully = false;
systemtool::SystemToolShared* systemtool = NULL;
jclass classSystemToolBridge = NULL;
static jclass classXResources = NULL;
static jclass classFileResult = NULL;

jmethodID methodSystemToolBridgeHandleHkedMethod = NULL;
static jmethodID methodXResourcesTranslateResId = NULL;
static jmethodID methodXResourcesTranslateAttrId = NULL;
static jmethodID constructorFileResult = NULL;


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////

static int register_natives_SystemToolBridge(JNIEnv* env, jclass clazz);
static int register_natives_XResources(JNIEnv* env, jclass clazz);
static int register_natives_ZygoteService(JNIEnv* env, jclass clazz);


////////////////////////////////////////////////////////////
// Utility methods
////////////////////////////////////////////////////////////

/** Read an integer value from a configuration file. */
int readIntConfig(const char* fileName, int defaultValue) {
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
        return defaultValue;

    int result;
    int success = fscanf(fp, "%i", &result);
    fclose(fp);

    return (success >= 1) ? result : defaultValue;
}


////////////////////////////////////////////////////////////
// Library initialization
////////////////////////////////////////////////////////////

bool initSystemToolBridge(JNIEnv* env) {
    classSystemToolBridge = env->FindClass(CLASS_XPOSED_BRIDGE);
    if (classSystemToolBridge == NULL) {
        ALOGE("Error while loading SystemTool class '%s':", CLASS_XPOSED_BRIDGE);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }
    classSystemToolBridge = reinterpret_cast<jclass>(env->NewGlobalRef(classSystemToolBridge));

    ALOGI("Found SystemTool class '%s', now initializing", CLASS_XPOSED_BRIDGE);
    if (register_natives_SystemToolBridge(env, classSystemToolBridge) != JNI_OK) {
        ALOGE("Could not register natives for '%s'", CLASS_XPOSED_BRIDGE);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }

    methodSystemToolBridgeHandleHkedMethod = env->GetStaticMethodID(classSystemToolBridge, "handleHkedMethod",
        "(Ljava/lang/reflect/Member;ILjava/lang/Object;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    if (methodSystemToolBridgeHandleHkedMethod == NULL) {
        ALOGE("ERROR: could not find method %s.handleHkedMethod(Member, int, Object, Object, Object[])", CLASS_XPOSED_BRIDGE);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }

    return true;
}

bool initZygoteService(JNIEnv* env) {
    jclass zygoteServiceClass = env->FindClass(CLASS_ZYGOTE_SERVICE);
    if (zygoteServiceClass == NULL) {
        ALOGE("Error while loading ZygoteService class '%s':", CLASS_ZYGOTE_SERVICE);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }
    if (register_natives_ZygoteService(env, zygoteServiceClass) != JNI_OK) {
        ALOGE("Could not register natives for '%s'", CLASS_ZYGOTE_SERVICE);
        env->ExceptionClear();
        return false;
    }

    classFileResult = env->FindClass(CLASS_FILE_RESULT);
    if (classFileResult == NULL) {
        ALOGE("Error while loading FileResult class '%s':", CLASS_FILE_RESULT);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }
    classFileResult = reinterpret_cast<jclass>(env->NewGlobalRef(classFileResult));

    constructorFileResult = env->GetMethodID(classFileResult, "<init>", "(JJ)V");
    if (constructorFileResult == NULL) {
        ALOGE("ERROR: could not find constructor %s(long, long)", CLASS_FILE_RESULT);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }

    return true;
}

void onVmCreatedCommon(JNIEnv* env) {
    if (!initSystemToolBridge(env) || !initZygoteService(env)) {
        return;
    }

    if (!onVmCreated(env)) {
        return;
    }

    systemtoolLoadedSuccessfully = true;
    return;
}


////////////////////////////////////////////////////////////
// JNI methods
////////////////////////////////////////////////////////////

jboolean SystemToolBridge_hadInitErrors(JNIEnv*, jclass) {
    return !systemtoolLoadedSuccessfully;
}

jobject SystemToolBridge_getStartClassName(JNIEnv* env, jclass) {
    return env->NewStringUTF(systemtool->startClassName);
}

jboolean SystemToolBridge_startsSystemServer(JNIEnv*, jclass) {
    return systemtool->startSystemServer;
}

jint SystemToolBridge_getSystemToolVersion(JNIEnv*, jclass) {
    return systemtool->systemtoolVersionInt;
}

jboolean SystemToolBridge_initXSToolResourcesNative(JNIEnv* env, jclass) {
    classXResources = env->FindClass(CLASS_XRESOURCES);
    if (classXResources == NULL) {
        ALOGE("Error while loading Resources class '%s':", CLASS_XRESOURCES);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }
    classXResources = reinterpret_cast<jclass>(env->NewGlobalRef(classXResources));

    if (register_natives_XResources(env, classXResources) != JNI_OK) {
        ALOGE("Could not register natives for '%s'", CLASS_XRESOURCES);
        env->ExceptionClear();
        return false;
    }

    methodXResourcesTranslateResId = env->GetStaticMethodID(classXResources, "translateResId",
        "(ILandroid/content/res/SToolResources;Landroid/content/res/Resources;)I");
    if (methodXResourcesTranslateResId == NULL) {
        ALOGE("ERROR: could not find method %s.translateResId(int, SToolResources, Resources)", CLASS_XRESOURCES);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }

    methodXResourcesTranslateAttrId = env->GetStaticMethodID(classXResources, "translateAttrId",
        "(Ljava/lang/String;Landroid/content/res/SToolResources;)I");
    if (methodXResourcesTranslateAttrId == NULL) {
        ALOGE("ERROR: could not find method %s.findAttrId(String, XResources)", CLASS_XRESOURCES);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }

    return true;
}

void SToolResources_rewriteXmlReferencesNative(JNIEnv* env, jclass,
            jlong parserPtr, jobject origRes, jobject repRes) {

    using namespace android;

    ResXMLParser* parser = (ResXMLParser*)parserPtr;
    const ResXMLTree& mTree = parser->mTree;
    uint32_t* mResIds = (uint32_t*)mTree.mResIds;
    ResXMLTree_attrExt* tag;
    int attrCount;

    if (parser == NULL)
        return;

    do {
        switch (parser->next()) {
            case ResXMLParser::START_TAG:
                tag = (ResXMLTree_attrExt*)parser->mCurExt;
                attrCount = dtohs(tag->attributeCount);
                for (int idx = 0; idx < attrCount; idx++) {
                    ResXMLTree_attribute* attr = (ResXMLTree_attribute*)
                        (((const uint8_t*)tag)
                         + dtohs(tag->attributeStart)
                         + (dtohs(tag->attributeSize)*idx));

                    // find resource IDs for attribute names
                    int32_t attrNameID = parser->getAttributeNameID(idx);
                    // only replace attribute name IDs for app packages
                    if (attrNameID >= 0 && (size_t)attrNameID < mTree.mNumResIds && dtohl(mResIds[attrNameID]) >= 0x7f000000) {
                        size_t attNameLen;
                        const char16_t* attrName = mTree.mStrings.stringAt(attrNameID, &attNameLen);
                        jint attrResID = env->CallStaticIntMethod(classXResources, methodXResourcesTranslateAttrId,
                            env->NewString((const jchar*)attrName, attNameLen), origRes);
                        if (env->ExceptionCheck())
                            goto leave;

                        mResIds[attrNameID] = htodl(attrResID);
                    }

                    // find original resource IDs for reference values (app packages only)
                    if (attr->typedValue.dataType != Res_value::TYPE_REFERENCE)
                        continue;

                    jint oldValue = dtohl(attr->typedValue.data);
                    if (oldValue < 0x7f000000)
                        continue;

                    jint newValue = env->CallStaticIntMethod(classXResources, methodXResourcesTranslateResId,
                        oldValue, origRes, repRes);
                    if (env->ExceptionCheck())
                        goto leave;

                    if (newValue != oldValue)
                        attr->typedValue.data = htodl(newValue);
                }
                continue;
            case ResXMLParser::END_DOCUMENT:
            case ResXMLParser::BAD_DOCUMENT:
                goto leave;
            default:
                continue;
        }
    } while (true);

    leave:
    parser->restart();
}


jboolean SToolZygoteServ_checkFileAccess(JNIEnv* env, jclass, jstring filenameJ, jint mode) {
#if XPOSED_WITH_SELINUX
    ScopedUtfChars filename(env, filenameJ);
    return systemtool->zygoteservice_accessFile(filename.c_str(), mode) == 0;
#else  // XPOSED_WITH_SELINUX
    return false;
#endif  // XPOSED_WITH_SELINUX
}

jobject SToolZygoteServ_statFile(JNIEnv* env, jclass, jstring filenameJ) {
#if XPOSED_WITH_SELINUX
    ScopedUtfChars filename(env, filenameJ);

    struct stat st;
    int result = systemtool->zygoteservice_statFile(filename.c_str(), &st);
    if (result != 0) {
        if (errno == ENOENT) {
            jniThrowExceptionFmt(env, "java/io/FileNotFoundException", "No such file or directory: %s", filename.c_str());
        } else {
            jniThrowExceptionFmt(env, "java/io/IOException", "%s while reading %s", strerror(errno), filename.c_str());
        }
        return NULL;
    }

    return env->NewObject(classFileResult, constructorFileResult, (jlong) st.st_size, (jlong) st.st_mtime);
#else  // XPOSED_WITH_SELINUX
    return NULL;
#endif  // XPOSED_WITH_SELINUX
}

jbyteArray SToolZygoteServ_readFile(JNIEnv* env, jclass, jstring filenameJ) {
#if XPOSED_WITH_SELINUX
    ScopedUtfChars filename(env, filenameJ);

    int bytesRead = 0;
    char* content = systemtool->zygoteservice_readFile(filename.c_str(), &bytesRead);
    if (content == NULL) {
        if (errno == ENOENT) {
            jniThrowExceptionFmt(env, "java/io/FileNotFoundException", "No such file or directory: %s", filename.c_str());
        } else {
            jniThrowExceptionFmt(env, "java/io/IOException", "%s while reading %s", strerror(errno), filename.c_str());
        }
        return NULL;
    }

    jbyteArray ret = env->NewByteArray(bytesRead);
    if (ret != NULL) {
        jbyte* arrptr = (jbyte*)env->GetPrimitiveArrayCritical(ret, 0);
        if (arrptr) {
            memcpy(arrptr, content, bytesRead);
            env->ReleasePrimitiveArrayCritical(ret, arrptr, 0);
        }
    }

    free(content);
    return ret;
#else  // XPOSED_WITH_SELINUX
    return NULL;
#endif  // XPOSED_WITH_SELINUX
}

////////////////////////////////////////////////////////////
// JNI methods registrations
////////////////////////////////////////////////////////////

int register_natives_SystemToolBridge(JNIEnv* env, jclass clazz) {
    const JNINativeMethod methods[] = {
        NATIVE_METHOD(SystemToolBridge, hadInitErrors, "()Z"),
        NATIVE_METHOD(SystemToolBridge, getStartClassName, "()Ljava/lang/String;"),
        NATIVE_METHOD(SystemToolBridge, getRuntime, "()I"),
        NATIVE_METHOD(SystemToolBridge, startsSystemServer, "()Z"),
        NATIVE_METHOD(SystemToolBridge, getSystemToolVersion, "()I"),
        NATIVE_METHOD(SystemToolBridge, initXSToolResourcesNative, "()Z"),
        NATIVE_METHOD(SystemToolBridge, hkMethodNative, "(Ljava/lang/reflect/Member;Ljava/lang/Class;ILjava/lang/Object;)V"),
        NATIVE_METHOD(SystemToolBridge, setObjectClassNative, "(Ljava/lang/Object;Ljava/lang/Class;)V"),
        NATIVE_METHOD(SystemToolBridge, dumpObjectNative, "(Ljava/lang/Object;)V"),
        NATIVE_METHOD(SystemToolBridge, cloneToSubclassNative, "(Ljava/lang/Object;Ljava/lang/Class;)Ljava/lang/Object;"),
        NATIVE_METHOD(SystemToolBridge, removeFinalFlagNative, "(Ljava/lang/Class;)V"),
#if PLATFORM_SDK_VERSION >= 21
        NATIVE_METHOD(SystemToolBridge, invOriMethodNative,
            "!(Ljava/lang/reflect/Member;I[Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;"),
        NATIVE_METHOD(SystemToolBridge, closeFileBeforeFkNative, "()V"),
        NATIVE_METHOD(SystemToolBridge, reopenFileAfterFkNative, "()V"),
#endif
#if PLATFORM_SDK_VERSION >= 24
        NATIVE_METHOD(SystemToolBridge, invalidateCallersNative, "([Ljava/lang/reflect/Member;)V"),
#endif
    };
    return env->RegisterNatives(clazz, methods, NELEM(methods));
}

int register_natives_XResources(JNIEnv* env, jclass clazz) {
    const JNINativeMethod methods[] = {
        NATIVE_METHOD(SToolResources, rewriteXmlReferencesNative, "(JLandroid/content/res/SToolResources;Landroid/content/res/Resources;)V"),
    };
    return env->RegisterNatives(clazz, methods, NELEM(methods));
}

int register_natives_ZygoteService(JNIEnv* env, jclass clazz) {
    const JNINativeMethod methods[] = {
        NATIVE_METHOD(SToolZygoteServ, checkFileAccess, "(Ljava/lang/String;I)Z"),
        NATIVE_METHOD(SToolZygoteServ, statFile, "(Ljava/lang/String;)L" CLASS_FILE_RESULT ";"),
        NATIVE_METHOD(SToolZygoteServ, readFile, "(Ljava/lang/String;)[B"),
    };
    return env->RegisterNatives(clazz, methods, NELEM(methods));
}

}  // namespace systemtool
