#pragma version(1)
#pragma rs java_package_name(com.android.rs.branchingfuncalls)

static bool is_neg(int a)
{
    if(a < 0)
        return true;
    else
        return false;
}

static bool is_pos(int a)
{
    if(a > 0)
        return true;
    else
        return false;
}

static void set_i(int * a, int b)
{
    int tmp = b;
    *a = tmp;
}

static void modify_f(float * f)
{
    *f *= 0.5f;
}

static void modify_i(int * i)
{
    int j = *i;
    int cutoff = 2 << 6;
    if(j > cutoff)
        j = cutoff;
    if(is_neg(j))
        set_i(i, 0);
    else if(is_pos(j))
        set_i(i, j);
    else
        set_i(i, cutoff);
}

int __attribute__((kernel)) simple_kernel(int in)
{
    int i = in;
    float f = (float) i;
    modify_f(&f);
    modify_i(&i);
    int ret = (int) f;
    return in * ret;
}

int glob = 123;

void addToGlobal(int arg)
{
    glob += arg;
}
