package com.android.rs.branchingfuncalls;

import android.app.Activity;
import android.os.Bundle;
import android.renderscript.*;

public class MainActivity extends Activity {
    private RenderScript mRS;
    private Allocation mInAllocation;
    private Allocation mOutAllocation;
    private ScriptC_scalars mScript;
    private int mAllocSize = 256;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main_layout);
        createScript();
        runScript();
    }

    private void createScript() {
        mRS = RenderScript.create(this,
            RenderScript.ContextType.NORMAL,
            RenderScript.CREATE_FLAG_LOW_LATENCY |
            RenderScript.CREATE_FLAG_WAIT_FOR_ATTACH);

        Element e = Element.I32(mRS);
        mInAllocation = Allocation.createSized(mRS, e, mAllocSize);
        mOutAllocation = Allocation.createSized(mRS, e, mAllocSize);

        mScript = new ScriptC_scalars(mRS);
    }

    private void runScript() {
        mScript.invoke_addToGlobal(234);

        int[] init = new int[mAllocSize];
        for(int i = 0; i < mAllocSize; ++i) {
            init[i] = i - (mAllocSize / 2);
        }
        mInAllocation.copy1DRangeFrom(0, mAllocSize, init);
        mScript.forEach_simple_kernel(mInAllocation, mOutAllocation);
    }
}

