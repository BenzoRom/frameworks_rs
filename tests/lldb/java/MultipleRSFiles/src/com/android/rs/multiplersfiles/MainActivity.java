package com.android.rs.multiplersfiles;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.widget.ImageView;
import android.renderscript.*;

public class MainActivity extends Activity {
    private Bitmap mBitmapIn;
    private Bitmap mBitmapOut;
    private ImageView mImageView;

    private RenderScript mRS;
    private Allocation mInAllocation;
    private Allocation mOutAllocation;
    private ScriptC_first mFirstScript;
    private ScriptC_second mSecondScript;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main_layout);

        mBitmapIn = Bitmap.createBitmap(500, 500, Bitmap.Config.ARGB_8888);
        mBitmapOut = Bitmap.createBitmap(mBitmapIn.getWidth(),
                    mBitmapIn.getHeight(), mBitmapIn.getConfig());

        mImageView = (ImageView) findViewById(R.id.imageView);
        mImageView.setImageBitmap(mBitmapOut);

        createScript();
        updateImage(1.0f);
    }

    private void createScript() {
        mRS = RenderScript.create(this,
            RenderScript.ContextType.NORMAL,
            RenderScript.CREATE_FLAG_LOW_LATENCY |
            RenderScript.CREATE_FLAG_WAIT_FOR_ATTACH);

        mInAllocation = Allocation.createFromBitmap(mRS, mBitmapIn);
        mOutAllocation = Allocation.createFromBitmap(mRS, mBitmapOut);

        mFirstScript = new ScriptC_first(mRS);
        mSecondScript = new ScriptC_second(mRS);
    }


    private void updateImage(final float f) {
        mFirstScript.set_gColor(new Float4(0.9f, 0.8f, 0.5f, 1.0f));
        mFirstScript.forEach_first_kernel(mInAllocation, mOutAllocation);
        mOutAllocation.copyTo(mBitmapOut);
        mSecondScript.forEach_second_kernel(mInAllocation, mOutAllocation);
    }
}

