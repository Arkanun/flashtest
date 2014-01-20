/*==============================================================================
            Copyright (c) 2010-2011 QUALCOMM Incorporated.
            All Rights Reserved.
            Qualcomm Confidential and Proprietary
            
@file 
    ImageTargetsRenderer.java

@brief
    Sample for ImageTargets

==============================================================================*/


package com.qualcomm.QCARSamples.ImageTargets;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;

import com.qualcomm.QCAR.QCAR;


public class ImageTargetsRenderer implements GLSurfaceView.Renderer
{
	int testFlag = 0;
    public boolean mIsActive = false;
    
  
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        DebugLog.LOGD("GLRenderer::onSurfaceCreated");

        initRendering();      
        QCAR.onSurfaceCreated();
    }
    
    public native void initRendering();
    
    
    public void onDrawFrame(GL10 gl)
    {
    	testFlag = 0;
        if ( !mIsActive )
            return;

        testFlag = renderFrame();  
        //System.err.println("Test Flag is: " + testFlag);
    }
    
    public native int renderFrame();
    
    
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
        DebugLog.LOGD("GLRenderer::onSurfaceChanged");
        
        updateRendering(width, height);
        QCAR.onSurfaceChanged(width, height);
    }    
    
    public native void updateRendering(int width, int height);
}
