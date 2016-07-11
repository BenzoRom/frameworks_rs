package com.android.rs.singlesource;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.widget.ImageView;
import android.renderscript.*;

public class MainActivity extends Activity {

    private RenderScript mRS;
    private Allocation mAllocIn;
    private Allocation mAllocOut;
    private ScriptC_rs_single_source mScript;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        setContentView(R.layout.main_layout);

        mRS = RenderScript.create(
              this,
              RenderScript.ContextType.NORMAL,
              RenderScript.CREATE_FLAG_LOW_LATENCY |
              RenderScript.CREATE_FLAG_WAIT_FOR_ATTACH);

        mScript = new ScriptC_rs_single_source(mRS);

        float [] input = new float[]{ 1.f, 2.f, 3.f, 4.f };
        float [] output = new float[4];

        mAllocIn  = Allocation.createSized(mRS, Element.F32(mRS), 4);
        mAllocOut = Allocation.createSized(mRS, Element.F32(mRS), 4);

        mAllocIn.copyFrom(input);

        mScript.invoke_script_invoke(mAllocOut, mAllocIn);
    }
}
