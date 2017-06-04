package com.android.rs.test;

import com.android.rs.unittest.RSListActivity;
import com.android.rs.unittest.UnitTest;

import android.app.ListActivity;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.support.test.rule.ActivityTestRule;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

public class RSTestListActivity extends RSListActivity {
    private static final String TAG = RSTestListActivity.class.getSimpleName();

    protected Iterable<Class<? extends UnitTest>> getUnitTests() throws Exception {
        return UnitTest.getProperSubclasses(this);
    }

    protected void logStartUnitTest(UnitTest test) {
        String thisDeviceName = android.os.Build.DEVICE;
        int thisApiVersion = android.os.Build.VERSION.SDK_INT;
        Log.i(TAG, String.format("RenderScript testing (%s) "
                + "on device %s, API version %d",
                test.toString(), thisDeviceName, thisApiVersion));
    }
}
