#include <memory>
#include <unistd.h>

#include <jni.h>
#include <RenderScript.h>

#include "ScriptC_infiniteLoop.h"

using namespace android;
using namespace RSC;

extern "C" void JNICALL
Java_com_android_rs_jniinfiniteloop_MainActivity_nativeRS(
    JNIEnv * env,
    jclass,
    jstring pathObj)
{
    static const int size = 64;
    sp<RS> rs = new RS();

    const char * path = env->GetStringUTFChars(pathObj, nullptr);
    rs->init(path, RS_INIT_LOW_LATENCY);
    env->ReleaseStringUTFChars(pathObj, path);

    auto e = Element::RGBA_8888(rs);
    Type::Builder tb(rs, e);
    tb.setX(size);
    tb.setY(size);
    auto t = tb.create();

    auto a = Allocation::createTyped(rs, t);
    auto b = Allocation::createTyped(rs, t);

    sp<ScriptC_infiniteLoop> s = new ScriptC_infiniteLoop(rs);

    // Test is designed to loop forever, waits for two seconds
    // between each invocation of the kernel
    bool forever = true;
    while(forever)
    {
        s->forEach_simple_kernel(a, b);
        sleep(2);
    }

    uint32_t * output = new uint32_t[size*size];
    b->copy2DRangeTo(0, 0, size, size, output);
    delete [] output;
}

