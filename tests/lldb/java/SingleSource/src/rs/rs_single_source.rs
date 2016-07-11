#pragma version(1)
#pragma rs java_package_name(com.android.rs.singlesource)

float __attribute__((kernel)) square_kernel(float a)
{
    return a * a;
}

void script_invoke(rs_allocation out, rs_allocation in)
{
    rsForEach(square_kernel, out, in);
}
