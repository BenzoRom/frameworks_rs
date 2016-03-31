#pragma version(1)
#pragma rs java_package_name(com.android.rs.waitattachnodebug)

float4 gColor = {0.299f, 0.587f, 0.114f, 1.f};

/* RenderScript kernel that just sets the colour of the screen and does some
 * simple operations so it is not completely empty
 * (and can therefore be debugged).
 */
uchar4 __attribute__((kernel)) simple_kernel(uchar4 in)
{
    float4 temp = rsUnpackColor8888(in);
    temp = gColor;
    uchar4 result = rsPackColorTo8888(temp);
    return result;
}

