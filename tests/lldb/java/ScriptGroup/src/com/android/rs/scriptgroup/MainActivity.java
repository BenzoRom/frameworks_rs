package com.android.rs.scriptgroup;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.widget.ImageView;
import android.renderscript.*;

public class MainActivity extends Activity {
    private static final int ARRAY_SIZE = 8;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_layout);

        // create renderscript context
        RenderScript pRS = RenderScript.create(this, RenderScript.ContextType.NORMAL,
            RenderScript.CREATE_FLAG_WAIT_FOR_ATTACH | RenderScript.CREATE_FLAG_LOW_LATENCY);

        ScriptC_scriptgroup script = new ScriptC_scriptgroup(pRS);

        // create and initalize a simple input allocation
        int[] array = new int[ARRAY_SIZE];
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array[i] = i;
        }
        Allocation input = Allocation.createSized(pRS, Element.I32(pRS), ARRAY_SIZE);
        input.copyFrom(array);

        ScriptGroup.Builder2 builder = new ScriptGroup.Builder2(pRS);

        ScriptGroup.Input unbound = builder.addInput();

        ScriptGroup.Closure c0 = builder.addKernel(
            script.getKernelID_foo(), Type.createX(pRS, Element.I32(pRS), ARRAY_SIZE), unbound);

        ScriptGroup.Closure c1 = builder.addKernel(script.getKernelID_goo(),
            Type.createX(pRS, Element.I32(pRS), ARRAY_SIZE), c0.getReturn());

        ScriptGroup group = builder.create("scriptgroup_test", c1.getReturn());

        int[] a = new int[ARRAY_SIZE];
        ((Allocation) group.execute(input)[0]).copyTo(a);

        pRS.finish();
        pRS.destroy();
    }
}
