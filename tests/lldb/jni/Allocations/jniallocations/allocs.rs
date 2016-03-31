#pragma version(1)
#pragma rs java_package_name(com.android.rs.jniallocations)

// Kernel performs basic vector swizzle
uchar4 __attribute__((kernel)) swizzle_kernel(uchar4 in)
{
    return in.wzyx;
}

// Kernel squares every element in allocation
uint __attribute__((kernel)) square_kernel(ushort in)
{
    uint result = (uint)(in) * (uint)in;
    return result;
}

// Helper function adding 1/2 to passed in double
static double half_helper(double in)
{
    return (in + 0.5);
}

// Kernel returns first 3 elements of a double4 plus 1/2
double3 __attribute__((kernel)) add_half_kernel(double4 in)
{
    double3 result;
    result.x = half_helper(in.x);
    result.y = half_helper(in.y);
    result.z = half_helper(in.z);
    return result;
}
