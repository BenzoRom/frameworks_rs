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

package com.android.rs.testcompat;

import com.android.rs.unittest.RSListActivity;
import com.android.rs.unittest.UnitTest;

import android.util.Log;

public class RSSupportLibTestsListActivity extends RSListActivity {
    private static final String TAG = RSSupportLibTestsListActivity.class.getSimpleName();

    protected Iterable<Class<? extends UnitTest>> getUnitTests() throws Exception {
        return UnitTest.getProperSubclasses(this);
    }

    protected void logStartUnitTest(UnitTest test) {
        String thisDeviceName = android.os.Build.DEVICE;
        int thisApiVersion = android.os.Build.VERSION.SDK_INT;
        Log.i(TAG, String.format("RenderScript support lib testing (%s) "
                + "on device %s, API version %d",
                test.toString(), thisDeviceName, thisApiVersion));
    }
}
