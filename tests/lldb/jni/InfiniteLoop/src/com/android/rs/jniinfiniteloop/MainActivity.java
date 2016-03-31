package com.android.rs.jniinfiniteloop;

import android.app.Activity;
import android.os.Bundle;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.widget.ImageView;

public class MainActivity extends Activity {
    private Bitmap mBitmapIn;
    private Bitmap mBitmapOut;

    static {
        System.loadLibrary("RS");
        System.loadLibrary("jniinfiniteloop");
    }

    native void nativeRS(String cacheDir);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_layout);
        nativeRS(this.getCacheDir().toString());
    }
}

