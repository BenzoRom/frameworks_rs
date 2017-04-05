/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.rs.testbackward;

import com.android.rs.unittest.*;

import android.content.Context;
import android.support.test.InstrumentationRegistry;
import android.support.test.filters.MediumTest;
import android.util.Log;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameter;
import org.junit.runners.Parameterized.Parameters;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;


/**
 * RSTestBackward, functional test for platform RenderScript APIs.
 * To run the test, please use command
 *
 * adb shell am instrument -w com.android.rs.testbackward/android.support.test.runner.AndroidJUnitRunner
 *
 */
@RunWith(Parameterized.class)
public class RSBackwardCompatibilityTests {
    private static final String TAG = "RSBackwardCompatibilityTests";

    /**
      * Returns the list of subclasses of UnitTest to run.
      *
      * Filters out any tests with API version greater than current API version.
      */
    @Parameters(name = "{0}")
    public static Iterable<?> getParams() throws Exception {
        Context ctx = InstrumentationRegistry.getTargetContext();

        int thisApiVersion = android.os.Build.VERSION.SDK_INT;

        ArrayList<UnitTest> validUnitTests = new ArrayList<>();

        if (thisApiVersion >= 19) {
            validUnitTests.add(new UT_alloc(ctx));
            validUnitTests.add(new UT_array_alloc(ctx));
            validUnitTests.add(new UT_array_init(ctx));
            validUnitTests.add(new UT_atomic(ctx));
            validUnitTests.add(new UT_bug_char(ctx));
            validUnitTests.add(new UT_check_dims(ctx));
            validUnitTests.add(new UT_clamp(ctx));
            validUnitTests.add(new UT_clamp_relaxed(ctx));
            validUnitTests.add(new UT_constant(ctx));
            validUnitTests.add(new UT_convert(ctx));
            validUnitTests.add(new UT_convert_relaxed(ctx));
            validUnitTests.add(new UT_copy_test(ctx));
            validUnitTests.add(new UT_element(ctx));
            validUnitTests.add(new UT_foreach_bounds(ctx));
            validUnitTests.add(new UT_foreach(ctx));
            validUnitTests.add(new UT_fp_mad(ctx));
            validUnitTests.add(new UT_int4(ctx));
            validUnitTests.add(new UT_kernel(ctx));
            validUnitTests.add(new UT_kernel_struct(ctx));
            validUnitTests.add(new UT_math_agree(ctx));
            validUnitTests.add(new UT_math_conformance(ctx));
            validUnitTests.add(new UT_math(ctx));
            // validUnitTests.add(new UT_mesh(ctx)); // removed in 21
            validUnitTests.add(new UT_min(ctx));
            validUnitTests.add(new UT_noroot(ctx));
            validUnitTests.add(new UT_primitives(ctx));
            // validUnitTests.add(new UT_program_raster(ctx)); // removed in 21
            // validUnitTests.add(new UT_program_store(ctx)); // removed in 21
            validUnitTests.add(new UT_refcount(ctx));
            validUnitTests.add(new UT_rsdebug(ctx));
            validUnitTests.add(new UT_rstime(ctx));
            validUnitTests.add(new UT_rstypes(ctx));
            validUnitTests.add(new UT_sampler(ctx));
            validUnitTests.add(new UT_static_globals(ctx));
            validUnitTests.add(new UT_struct(ctx));
            validUnitTests.add(new UT_unsigned(ctx));
            validUnitTests.add(new UT_vector(ctx));
        }

        if (thisApiVersion >= 23) {
            validUnitTests.add(new UT_ctxt_default(ctx));
            validUnitTests.add(new UT_foreach_multi(ctx));
            validUnitTests.add(new UT_kernel2d(ctx));
            validUnitTests.add(new UT_kernel2d_oldstyle(ctx));
            validUnitTests.add(new UT_kernel3d(ctx));
            validUnitTests.add(new UT_script_group2_gatherscatter(ctx));
            validUnitTests.add(new UT_script_group2_nochain(ctx));
            validUnitTests.add(new UT_script_group2_pointwise(ctx));
        }

        if (thisApiVersion >= 24) {
            validUnitTests.add(new UT_fp16_globals(ctx));
            validUnitTests.add(new UT_fp16(ctx));
            validUnitTests.add(new UT_math_24(ctx));
            validUnitTests.add(new UT_math_fp16(ctx));
            validUnitTests.add(new UT_reduce_backward(ctx));
            validUnitTests.add(new UT_reduce(ctx));
            validUnitTests.add(new UT_rsdebug_24(ctx));
            validUnitTests.add(new UT_script_group2_float(ctx));
            validUnitTests.add(new UT_single_source_alloc(ctx));
            validUnitTests.add(new UT_single_source_ref_count(ctx));
            validUnitTests.add(new UT_single_source_script(ctx));
            validUnitTests.add(new UT_small_struct(ctx));
        }

        if (thisApiVersion >= 25) {
            validUnitTests.add(new UT_bitfield(ctx));
            validUnitTests.add(new UT_small_struct_2(ctx));
            validUnitTests.add(new UT_struct_field(ctx));
            validUnitTests.add(new UT_struct_field_simple(ctx));
        }

        return validUnitTests;
    }

    @Parameter(0)
    public UnitTest mTest;

    @Test
    @MediumTest
    public void testRSUnitTest() throws Exception {
        mTest.runTest();
        switch (mTest.getResult()) {
            case UT_NOT_STARTED:
            case UT_RUNNING:
                Log.w(TAG, "unexpected unit test result: " + mTest.getResult().toString());
                break;
        }
        Assert.assertTrue(mTest.getSuccess());
    }
}
