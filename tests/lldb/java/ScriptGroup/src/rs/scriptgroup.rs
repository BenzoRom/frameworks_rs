#pragma version(1)
#pragma rs java_package_name(com.android.rs.scriptgroup)
#pragma rs_fp_full

int __attribute__((kernel)) foo(int a) {
    return a * a;
}

int __attribute__((kernel)) goo(int a) {
    return a + a;
}
