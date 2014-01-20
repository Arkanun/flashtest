/*==============================================================================
            Copyright (c) 2010-2011 QUALCOMM Incorporated.
            All Rights Reserved.
            Qualcomm Confidential and Proprietary
            
@file 
    ImageTargets.java

@brief
    Sample for ImageTargets
==============================================================================*/


package com.qualcomm.QCARSamples.ImageTargets;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SubMenu;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.view.ViewGroup.LayoutParams;
import android.widget.ImageView;

import com.qualcomm.QCAR.QCAR;


public class ImageTargets extends Activity
{
	private static final String NATIVE_LIB_QCAR = "QCAR"; 
	private static final String NATIVE_LIB_SAMPLE = "ImageTargets";    

	private int mSplashScreenImageResource = 0;
	 
	private int mQCARFlags = 0;
    
    private static final int APPSTATUS_UNINITED         = -1;
    private static final int APPSTATUS_INIT_APP         = 0;
    private static final int APPSTATUS_INIT_QCAR        = 1;
    private static final int APPSTATUS_INIT_APP_AR      = 2;
    private static final int APPSTATUS_INIT_TRACKER     = 3;
    private static final int APPSTATUS_INITED           = 4;
    private static final int APPSTATUS_CAMERA_STOPPED   = 5;
    private static final int APPSTATUS_CAMERA_RUNNING   = 6;
    
    private int mAppStatus = APPSTATUS_UNINITED;
    
    private int mScreenWidth = 0;
    private int mScreenHeight = 0;
    
    private ImageView mSplashScreenView;
    
    private long mSplashScreenStartTime = 0;
    
    private InitQCARTask mInitQCARTask;
    
    private QCARSampleGLView mGlView;
    
    private ImageTargetsRenderer mRenderer;
    
    private LoadTrackerTask mLoadTrackerTask;
    
    private boolean mFlash = false;
    
    private MenuItem checked;
    
    int tIdx=-1;
    
    private int[] mIdOfActiveWifi = new int[10];	//*******************************************************************
    
    private int mNumOfActiveWifi=1;	//***********************************************************************************
    
    private boolean setActiveWifi()
    {
    	if(isFreeNative()==true)
    	{
    		setIsFreeNative();
    		setNumOfActiveWifi(mNumOfActiveWifi);
    		for(int i=0;i<mNumOfActiveWifi;i++)
    		{
    			setIdOfAvtiveWifi(i,mIdOfActiveWifi[i]);
    		}
    		setIsFreeNative();
    		return true;
    	}
    	else
    		return false;
    }
    public native boolean isFreeNative();
    public native void setIsFreeNative();
    public native void setNumOfActiveWifi( int NumOfActiveWifi);
    public native void setIdOfAvtiveWifi( int i,int IdOfActiveWifi);
    
    
    static
    {
        loadLibrary(NATIVE_LIB_QCAR);
        loadLibrary(NATIVE_LIB_SAMPLE);
    }
    
    
    public static boolean loadLibrary(String nLibName)
    {
        try
        {
            System.loadLibrary(nLibName);
            DebugLog.LOGI("Native library lib" + nLibName + ".so loaded");
            return true;
        }
        catch (UnsatisfiedLinkError ulee)
        {
            DebugLog.LOGE("The library lib" + nLibName + ".so could not be loaded");
        }
        catch (SecurityException se)
        {
            DebugLog.LOGE("The library lib" + nLibName + ".so was not allowed to be loaded");
        }
        
        return false;
    }
    
    
    protected void onCreate(Bundle savedInstanceState)
    {
        DebugLog.LOGD("ImageTargets::onCreate");
        super.onCreate(savedInstanceState);
        
        mSplashScreenImageResource = R.drawable.splash_screen_image_targets;            
        mQCARFlags = getInitializationFlags();
        
        updateApplicationStatus(APPSTATUS_INIT_APP);    
    }   
       
    
    private int getInitializationFlags()
    {
        int flags = 0;
        
        if ( getOpenGlEsVersionNative() == 1 )
        {
            flags = QCAR.GL_11;
        }
        else
        {
            flags = QCAR.GL_20;
        }
        
        return flags;
    }
    
    public native int getOpenGlEsVersionNative();
    
    
    private synchronized void updateApplicationStatus(int appStatus)
    {
        if (mAppStatus == appStatus)
            return;
      
        mAppStatus = appStatus;

        switch (mAppStatus)
        {
            case APPSTATUS_INIT_APP:	initApplication();
            							updateApplicationStatus(APPSTATUS_INIT_QCAR);
            							break;

            case APPSTATUS_INIT_QCAR:	try
						                {
						                    mInitQCARTask = new InitQCARTask();
						                    mInitQCARTask.execute();
						                }
						                catch (Exception e)
						                {
						                    DebugLog.LOGE("Initializing QCAR SDK failed");
						                }
						                break;
                
            case APPSTATUS_INIT_APP_AR: initApplicationAR();               
						                updateApplicationStatus(APPSTATUS_INIT_TRACKER);
						                break;
                
            case APPSTATUS_INIT_TRACKER:	try
							                {
							                    mLoadTrackerTask = new LoadTrackerTask();
							                    mLoadTrackerTask.execute();
							                }
							                catch (Exception e)
							                {
							                    DebugLog.LOGE("Loading tracking data set failed");
							                }
							                break;
                
            case APPSTATUS_INITED:  System.gc();
					
					                onQCARInitializedNative();
					                
					                long splashScreenTime = System.currentTimeMillis() - mSplashScreenStartTime;					                
					                DebugLog.LOGI("splashScreenTime is " + splashScreenTime );
					                
					                Handler handler = new Handler();
					                handler.postDelayed(
					                    new Runnable() 
					                    {
					                        public void run()
					                        {
					                            mSplashScreenView.setVisibility(View.INVISIBLE);
					                            
					                            mRenderer.mIsActive = true;
					
					                            addContentView( mGlView, 
					                            				new LayoutParams( LayoutParams.FILL_PARENT,
					                                            				  LayoutParams.FILL_PARENT)
					                            			  );
					                            
					                            updateApplicationStatus(APPSTATUS_CAMERA_RUNNING);
					                        }	
					                    }, 0 
					                );                
					        		break;
                
            case APPSTATUS_CAMERA_RUNNING: 	startCamera(); 
											break;
											
            case APPSTATUS_CAMERA_STOPPED:	stopCamera(); 
							                break;
                          
            default:	throw new RuntimeException("Invalid application state");
        }
    }
       
      
    private void initApplication()
    {
        int screenOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
        
        setRequestedOrientation(screenOrientation);       
        setActivityPortraitMode(screenOrientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        mScreenWidth = metrics.widthPixels;
        mScreenHeight = metrics.heightPixels;

        getWindow().setFlags( WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
        					  WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );
              
        mSplashScreenView = new ImageView(this);
        mSplashScreenView.setImageResource(mSplashScreenImageResource);
        addContentView(mSplashScreenView, new LayoutParams( LayoutParams.FILL_PARENT, 
        													LayoutParams.FILL_PARENT));
        
        mSplashScreenStartTime = System.currentTimeMillis();
    }
    
    private native void setActivityPortraitMode(boolean isPortrait);
    
    
    private class InitQCARTask extends AsyncTask<Void, Integer, Boolean>
    {   
        private int mProgressValue = -1;
        
        
        protected Boolean doInBackground(Void... params)
        {
            QCAR.setInitParameters(ImageTargets.this, mQCARFlags);
            
            do
            {
                mProgressValue = QCAR.init();               
                publishProgress(mProgressValue);
            } 
            while (!isCancelled() && mProgressValue >= 0 && mProgressValue < 100);
            
            return (mProgressValue > 0);
        }

        
        protected void onProgressUpdate(Integer... values)
        {
        }

        
        protected void onPostExecute(Boolean result)
        {
            if (result)
            {
                DebugLog.LOGD("InitQCARTask::onPostExecute: QCAR initialization" + " successful");

                updateApplicationStatus(APPSTATUS_INIT_APP_AR);
            }
            else
            {
                AlertDialog dialogError = new AlertDialog.Builder(ImageTargets.this).create();
                dialogError.setButton( "Close",
					                    new DialogInterface.OnClickListener()
					                    {
					                        public void onClick(DialogInterface dialog, int which)
					                        {
					                            System.exit(1);
					                        }
					                    }
                ); 
                
                String logMessage;

                if (mProgressValue == QCAR.INIT_DEVICE_NOT_SUPPORTED)
                {
                    logMessage = "Failed to initialize QCAR because this device is not supported.";
                }
                else 
                	if (mProgressValue == QCAR.INIT_CANNOT_DOWNLOAD_DEVICE_SETTINGS)
	                {
	                    logMessage = "Network connection required to initialize camera settings. " + 
	                    			 "Please check your connection and restart the application. " + 
	                    			 "If you are still experiencing problems, " + 
	                    			 "then your device may not be currently supported.";
	                }
                	else
		                {
		                    logMessage = "Failed to initialize QCAR.";
		                }
                
                DebugLog.LOGE("InitQCARTask::onPostExecute: " + logMessage + " Exiting.");
                
                dialogError.setMessage(logMessage);  
                dialogError.show();
            }
        }
    }
    
    
    private void initApplicationAR()
    {        
        initApplicationNative(mScreenWidth, mScreenHeight);

        int depthSize = 16;
        int stencilSize = 0;
        boolean translucent = QCAR.requiresAlpha();
        
        mGlView = new QCARSampleGLView(this);
        mGlView.init(mQCARFlags, translucent, depthSize, stencilSize);
        // Initialize App here
        mGlView.setOnTouchListener( new OnTouchListener()
        {
        	@Override
        	public boolean onTouch( View arg0 , MotionEvent event )
        	{
        		tIdx = -1;
            	
            	tIdx = onTouchEventNative( event.getX() , event.getY() );
            	if( tIdx == -1 )
            		DebugLog.LOGI( "num is : " + tIdx +" ; "+ event.getX()+" ; " + event.getY() 
            				+" ; "+getTryNative1()+" ; " +(getTryNative2()-26.f) );
            	else
            	{
            		DebugLog.LOGI( "num is : " + tIdx +" ; "+ event.getX()+" ; " + event.getY() +" ; "+
            	getTryNative1()+" ; " +(getTryNative2()-26.f));
            		
            		/* dialog not working
            		AlertDialog.Builder builder = new AlertDialog.Builder(ImageTargets.this);
            		builder.setMessage("The flash game will now load. Press \"Begin!\" to start.")
            		.setNegativeButton("Begin!", new DialogInterface.OnClickListener() {
            			public void onClick(DialogInterface dialog, int id) {

            				//Log.d("Error Thread","Error Thread: " + "User Pushed OK");
            				dialog.dismiss();
            				Intent myIntent = new Intent(ImageTargets.this, FlashActivity.class);
                    		myIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    		startActivity(myIntent);

            			}
            		});
            		AlertDialog alert = builder.create();
            		alert.show(); */
            		
            		
            		Intent myIntent = new Intent(ImageTargets.this, FlashActivity.class);
            		myIntent.putExtra("com.qualcomm.QCARSamples.ImageTargets.flagTest", tIdx);
            		myIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            		startActivity(myIntent); 
            	}
            	
            	return true;
        	}
        }
        							);
        
        mRenderer = new ImageTargetsRenderer();
        mGlView.setRenderer(mRenderer);
    }
    
    private native void initApplicationNative(int width, int height);
   
   
    private class LoadTrackerTask extends AsyncTask<Void, Integer, Boolean>
    {
        protected Boolean doInBackground(Void... params)
        {
            int progressValue = -1;

            do
            {
                progressValue = QCAR.load();
                publishProgress(progressValue);
                
            } while (!isCancelled() && progressValue >= 0 && progressValue < 100);
            
            return (progressValue > 0);
        }
        
        
        protected void onProgressUpdate(Integer... values)
        {
        }
        
        
        protected void onPostExecute(Boolean result)
        {
            DebugLog.LOGD( "LoadTrackerTask::onPostExecute: execution " + 
            			   (result ? "successful" : "failed"));
            updateApplicationStatus(APPSTATUS_INITED);
        }
    }
    
    
    public native void onQCARInitializedNative();
    
    
    private native void startCamera();
    
    
    private native void stopCamera();
    
/*    
    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
    	int tIdx = -1;
    	
    	tIdx = onTouchEventNative( event.getX() , event.getY() );
    	if( tIdx == -1 )
    		DebugLog.LOGI( "num is : " + tIdx +" ; "+ event.getX()+" ; " + event.getY() );
    	else
    	{
    		DebugLog.LOGI( "num is : " + tIdx +" ; "+ " ;num1 is : "+getTryNative1()+ " ;num2 is : "+
    				getTryNative2()+ " ;num3 is : "+event.getX()+
    				 " ;num4 is : "+event.getY());
    		//发送序号信息给Alljoyn，然后退出程序	
    	}
    	
    	return true;
    }
*/    
    private native int onTouchEventNative( float X , float Y );
    
    
    public boolean onCreateOptionsMenu(Menu menu)
    {
        super.onCreateOptionsMenu(menu);
        
        menu.add("Toggle flash");
        menu.add("Autofocus");
        
        SubMenu focusModes = menu.addSubMenu("Focus Modes");
        focusModes.add("Auto Focus").setCheckable(true);
        focusModes.add("Fixed Focus").setCheckable(true);
        focusModes.add("Infinity").setCheckable(true);
        focusModes.add("Macro Mode").setCheckable(true);
        
        return true;
    }
    
    
    public boolean onOptionsItemSelected(MenuItem item)
    {
        if(item.getTitle().equals("Toggle flash"))
        {
            mFlash = !mFlash;
            boolean result = toggleFlash(mFlash);
            DebugLog.LOGI("Toggle flash "+(mFlash?"ON":"OFF")+" "+(result?"WORKED":"FAILED")+"!!");
        }
        else 
        	if(item.getTitle().equals("Autofocus"))
	        {
	            boolean result = autofocus();
	            DebugLog.LOGI( "Autofocus requested" +
	            			   ( result?" successfully.":
	            					   	".  Not supported in current mode or on this device." )
	            			 );
	        }
	        else 
	        {
	            int arg = -1;
	            if(item.getTitle().equals("Auto Focus"))
	                arg = 0;
	            if(item.getTitle().equals("Fixed Focus"))
	                arg = 1;
	            if(item.getTitle().equals("Infinity"))
	                arg = 2;
	            if(item.getTitle().equals("Macro Mode"))
	                arg = 3;
	            
	            if(arg != -1)
	            {
	                item.setChecked(true);
	                if(checked!= null)
	                    checked.setChecked(false);
	                checked = item;
	                
	                boolean result = setFocusMode(arg);	                
	                DebugLog.LOGI("Requested Focus mode "+item.getTitle()+(result?" successfully.":".  Not supported on this device."));
	            }
	        }
        
        return true;
    }
    
    private native boolean toggleFlash(boolean flash);
    
    private native boolean autofocus();
    
    private native boolean setFocusMode(int mode); 
    
    
    protected void onPause()
    {
        DebugLog.LOGD("ImageTargets::onPause");
        super.onPause();
        
        if (mGlView != null)
        {
            mGlView.setVisibility(View.INVISIBLE);
            mGlView.onPause();
        }
        
        QCAR.onPause();
        
        if (mAppStatus == APPSTATUS_CAMERA_RUNNING)
        {
            updateApplicationStatus(APPSTATUS_CAMERA_STOPPED);
        }
    }
    
    
    protected void onResume()
    {
        DebugLog.LOGD("ImageTargets::onResume");
        super.onResume();
        
        QCAR.onResume();
        
        if (mAppStatus == APPSTATUS_CAMERA_STOPPED)
            updateApplicationStatus(APPSTATUS_CAMERA_RUNNING);
        
        if (mGlView != null)
        {
            mGlView.setVisibility(View.VISIBLE);
            mGlView.onResume();
        }        
    }
    
   
    protected void onDestroy()
    { 	
        DebugLog.LOGD("ImageTargets::onDestroy");
        super.onDestroy();
        
        if (mInitQCARTask != null && mInitQCARTask.getStatus() != InitQCARTask.Status.FINISHED)
        {
            mInitQCARTask.cancel(true);
            mInitQCARTask = null;
        }
        
        if (mLoadTrackerTask != null && mLoadTrackerTask.getStatus() != LoadTrackerTask.Status.FINISHED)
        {
            mLoadTrackerTask.cancel(true);
            mLoadTrackerTask = null;
        }
        
        deinitApplicationNative();    
        QCAR.deinit();
        
        System.gc();
    }

    private native void deinitApplicationNative();    
    
    
    
    
      
//测试用的************************************************************************************************************      
   public native float getTryNative1();
   public native float getTryNative2();
   public native float getTryNative3();
   public native float getTryNative4();
//      DebugLog.LOGI("num1 is : " + getTryNative1() + " ,num2 is : " + getTryNative2() +
//         			  " ,num3 is : " + getTryNative3() + " ,num4 is : " + getTryNative4() );
//************************************************************************************************************    
}