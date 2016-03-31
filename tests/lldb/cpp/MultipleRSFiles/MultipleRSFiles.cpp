#include <RenderScript.h>

#include "ScriptC_first.h"
#include "ScriptC_second.h"

using namespace android;
using namespace RSC;

int main()
{
    static const int size = 64;
    sp<RS> rs = new RS();

    rs->init("/data/rscache", RS_INIT_LOW_LATENCY | RS_INIT_WAIT_FOR_ATTACH);

    auto e = Element::RGBA_8888(rs);
    Type::Builder tb(rs, e);
    tb.setX(size);
    tb.setY(size);
    auto t = tb.create();

    auto a = Allocation::createTyped(rs, t);
    auto b = Allocation::createTyped(rs, t);

    // Script is executed once, then the data is copied back when finished
    sp<ScriptC_first> s1 = new ScriptC_first(rs);
    sp<ScriptC_second> s2 = new ScriptC_second(rs);

    s1->forEach_first_kernel(a, b);
    uint32_t * output = new uint32_t[size*size];
    b->copy2DRangeTo(0, 0, size, size, output);
    delete [] output;

    s2->forEach_second_kernel(a, b);

    rs->finish();
    return 0;
}
