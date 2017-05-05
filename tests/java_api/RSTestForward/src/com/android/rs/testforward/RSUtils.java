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

package com.android.rs.testforward;

import com.android.rs.unittest.UnitTest;

import android.content.Context;
import android.support.test.InstrumentationRegistry;

import dalvik.system.DexFile;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Enumeration;

public class RSUtils {
    /** Returns a list of all proper subclasses of the input class */
    public static <T> Iterable<Class<? extends T>> getProperSubclasses(Class<T> klass)
            throws ClassNotFoundException, IOException {
        Context context = InstrumentationRegistry.getTargetContext();

        ArrayList<Class<? extends T>> ret = new ArrayList<>();
        DexFile df = new DexFile(context.getPackageCodePath());
        Enumeration<String> iter = df.entries();
        while (iter.hasMoreElements()) {
            String s = iter.nextElement();
            Class<?> cur = Class.forName(s);
            while (cur != null) {
                if (cur.getSuperclass() == klass) {
                    break;
                }
                cur = cur.getSuperclass();
            }
            if (cur != null) {
                ret.add((Class<? extends T>) cur);
            }
        }
        return ret;
    }
}
