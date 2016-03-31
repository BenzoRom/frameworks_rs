#include <RenderScript.h>

#include "ScriptC_simple.h"

using namespace android;
using namespace RSC;

int main()
{
    static const int size = 64;
    sp<RS> rs = new RS();

    rs->init("/data/rscache", RS_INIT_LOW_LATENCY | RS_INIT_WAIT_FOR_ATTACH);

    auto e = Element::I32(rs);
    Type::Builder tb(rs, e);
    tb.setX(size);
    tb.setY(size);
    auto t = tb.create();

    auto a = Allocation::createTyped(rs, t);
    auto b = Allocation::createTyped(rs, t);

    int * input = new int[size*size];
    for(int i = 0; i < size*size; ++i) {
        input[i] = i - (size*size / 2);
    }
    a->copy2DRangeFrom(0, 0, size, size, input);
    delete [] input;

    // Script is executed once, then the data is copied back when finished
    sp<ScriptC_simple> s = new ScriptC_simple(rs);
    s->invoke_addToGlobal(234);
    s->forEach_simple_kernel(a, b);
    rs->finish();
    int32_t * output = new int32_t[size*size];
    b->copy2DRangeTo(0, 0, size, size, output);
    delete [] output;

    return 0;
}

