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

package com.android.rs.unittest;

import android.content.Context;
import android.renderscript.RenderScript.RSMessageHandler;
import android.util.Log;

public abstract class UnitTest {
    public enum UnitTestResult {
        UT_NOT_STARTED,
        UT_RUNNING,
        UT_SUCCESS,
        UT_FAIL,
    }

    private final static String TAG = "RSUnitTest";

    public String name;
    private UnitTestResult result;
    protected Context mCtx;

    /* These constants must match those in shared.rsh */
    public static final int RS_MSG_TEST_PASSED = 100;
    public static final int RS_MSG_TEST_FAILED = 101;

    public UnitTest(String n, Context ctx) {
        name = n;
        mCtx = ctx;
        result = UnitTestResult.UT_NOT_STARTED;
    }

    protected void _RS_ASSERT(String message, boolean b) {
        if (!b) {
            Log.e(TAG, message + " FAILED");
            failTest();
        }
    }

    protected RSMessageHandler mRsMessage =
            new RSMessageHandler() {
                public void run() {
                    if (result == UnitTestResult.UT_RUNNING) {
                        switch (mID) {
                            case RS_MSG_TEST_PASSED:
                                result = UnitTestResult.UT_SUCCESS;
                                break;
                            case RS_MSG_TEST_FAILED:
                                result = UnitTestResult.UT_FAIL;
                                break;
                            default:
                                Log.v(TAG, "Unit test got unexpected message");
                                break;
                        }
                    }
                }
            };

    protected void failTest() {
        result = UnitTestResult.UT_FAIL;
    }

    protected void passTest() {
        if (result != UnitTestResult.UT_FAIL) {
            result = UnitTestResult.UT_SUCCESS;
        }
    }

    public String toString() {
        String out = name;
        if (result == UnitTestResult.UT_SUCCESS) {
            out += " - PASSED";
        } else if (result == UnitTestResult.UT_FAIL) {
            out += " - FAILED";
        }
        return out;
    }

    public UnitTestResult getResult() {
        return result;
    }

    public boolean getSuccess() {
        return result == UnitTestResult.UT_SUCCESS;
    }

    public void runTest() {
        result = UnitTestResult.UT_RUNNING;
        run();
    }

    abstract protected void run();
}
