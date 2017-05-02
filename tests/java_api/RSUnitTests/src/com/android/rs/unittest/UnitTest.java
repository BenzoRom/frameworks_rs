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
import android.renderscript.RenderScript;
import android.renderscript.RenderScript.RSMessageHandler;
import android.util.Log;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public abstract class UnitTest {
    public enum UnitTestResult {
        UT_NOT_STARTED,
        UT_RUNNING,
        UT_SUCCESS,
        UT_FAIL,
    }

    private final static String TAG = "RSUnitTest";

    private String mName;
    private UnitTestResult mResult;
    private Context mCtx;
    /* Necessary to avoid race condition on pass/fail message. */
    private CountDownLatch mCountDownLatch;

    /* These constants must match those in shared.rsh */
    public static final int RS_MSG_TEST_PASSED = 100;
    public static final int RS_MSG_TEST_FAILED = 101;

    public UnitTest(String n, Context ctx) {
        mName = n;
        mCtx = ctx;
        mResult = UnitTestResult.UT_NOT_STARTED;
        mCountDownLatch = null;
    }

    protected void _RS_ASSERT(String message, boolean b) {
        if (!b) {
            Log.e(TAG, message + " FAILED");
            failTest();
        }
    }

    /**
     * Returns a RenderScript instance created from mCtx.
     *
     * @param enableMessages
     * true if expecting exactly one pass/fail message from the RenderScript instance.
     * false if no messages expected.
     * Any other messages are not supported.
     */
    protected RenderScript createRenderScript(boolean enableMessages) {
        RenderScript rs = RenderScript.create(mCtx);
        if (enableMessages) {
            RSMessageHandler handler = new RSMessageHandler() {
                public void run() {
                    switch (mID) {
                        case RS_MSG_TEST_PASSED:
                            passTest();
                            break;
                        case RS_MSG_TEST_FAILED:
                            failTest();
                            break;
                        default:
                            Log.w(TAG, String.format("Unit test %s got unexpected message %d",
                                    UnitTest.this.toString(), mID));
                            break;
                    }
                    mCountDownLatch.countDown();
                }
            };
            rs.setMessageHandler(handler);
            mCountDownLatch = new CountDownLatch(1);
        }
        return rs;
    }

    protected synchronized void failTest() {
        mResult = UnitTestResult.UT_FAIL;
    }

    protected synchronized void passTest() {
        if (mResult != UnitTestResult.UT_FAIL) {
            mResult = UnitTestResult.UT_SUCCESS;
        }
    }

    public UnitTestResult getResult() {
        return mResult;
    }

    public boolean getSuccess() {
        return mResult == UnitTestResult.UT_SUCCESS;
    }

    public void runTest() {
        mResult = UnitTestResult.UT_RUNNING;
        run();
        if (mCountDownLatch != null) {
            try {
                boolean success = mCountDownLatch.await(5 * 60, TimeUnit.SECONDS);
                if (!success) {
                    failTest();
                    Log.e(TAG, String.format("Unit test %s waited too long for pass/fail message",
                          toString()));
                }
            } catch (InterruptedException e) {
                failTest();
                Log.e(TAG, String.format("Unit test %s raised InterruptedException when " +
                        "listening for pass/fail message", toString()));
            }
        }
    }

    abstract protected void run();

    @Override
    public String toString() {
        return mName;
    }
}

