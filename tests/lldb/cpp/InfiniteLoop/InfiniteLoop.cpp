#include <thread>
#include <chrono>

#include <RenderScript.h>

#include "ScriptC_infiniteloop.h"

int main()
{
    static const int size = 64;
    sp<RS> rs = new RS();

    rs->init("/data/rscache", RS_INIT_LOW_LATENCY);

    auto e = Element::RGBA_8888(rs);
    Type::Builder tb(rs, e);
    tb.setX(size);
    tb.setY(size);
    auto t = tb.create();

    auto a = Allocation::createTyped(rs, t);
    auto b = Allocation::createTyped(rs, t);

    sp<ScriptC_infiniteloop> s = new ScriptC_infiniteloop(rs);

    // Test is designed to loop forever, waits for two seconds
    // between each invocation of the kernel
    bool forever = true;
    while(forever)
    {
        s->forEach_simple_kernel(a, b);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    uint32_t * output = new uint32_t[size*size];
    b->copy2DRangeTo(0, 0, size, size, output);
    delete [] output;

    return 0;
}

