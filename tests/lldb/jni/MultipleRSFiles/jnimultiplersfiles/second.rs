#pragma version(1)
#pragma rs java_package_name(com.android.rs.jnimultiplersfiles)

/* RenderScript kernel that just returns the swizzled input. */
uchar4 __attribute__((kernel)) second_kernel(uchar4 in)
{
    uchar4 result = in.wzyx;
    return result;
}
