package com.qualcomm.QCARSamples.ImageTargets;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnKeyListener;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

public class FlashActivity extends Activity {
	
	private int switchFlag = 0;
	private String url = null;
	private WebView wv;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.flash);
	
		Bundle extras = getIntent().getExtras();
        switchFlag = extras.getInt("com.qualcomm.QCARSamples.ImageTargets.flagTest");
		
		System.err.println("Switch Flag is: " + switchFlag);
		
		// For testing purposes only
		//url ="http://www.flashflashrevolution.com/ffr.swf";
		//url ="http://farm.stickpage.com/xiao4.swf";
		
		if(switchFlag == 0)
        {
			url ="http://www.flashflashrevolution.com/ffr.swf";
        }
        else if(switchFlag == 1)
        {
        	url ="http://farm.stickpage.com/xiao4.swf";
        }
		
		// Target URL
		//String url ="https://cn.mahogarden.com/login.aspx?n=demoboy&p=20090319&auto=true";
		//String url ="https://cn.mahogarden.com/cdn/page_child/giCenter/main.swf";

		wv=(WebView) findViewById(R.id.webViewTest);
		WebSettings ws = wv.getSettings();
		
		// How come I still hear flash music when I press the home button?
		wv.setOnKeyListener(new OnKeyListener()
	    {                           
			@Override
			public boolean onKey(View arg0, int arg1, KeyEvent arg2) {
				if (arg1 == KeyEvent.KEYCODE_BACK) {
					System.err.println("Back Button pressed");
					wv.loadUrl("about:blank");
		            finish();
		            return true;
		        }
				return false;
			}
	    });
		
	    //wv.getSettings().setPluginsEnabled(true);
	    //wv.setWebViewClient(new FlashWebViewClient());
		ws.setPluginsEnabled(true);
		ws.setJavaScriptEnabled(true);
	    wv.loadUrl(url);
	    
	   
	}
	
	/* not needed
	 @Override
     protected void onPause(){
         super.onPause();

         wv.pauseTimers();
         if(isFinishing()){
             wv.loadUrl("about:blank");
             //setContentView(new FrameLayout(this));
         }
     }

     @Override
     protected void onResume(){
         super.onResume();
         wv.resumeTimers();
     }
     */
	
	@Override
	protected void onPause() {
		callHiddenWebViewMethod("onPause");
		wv.pauseTimers();
	super.onPause();
	}

	@Override
	protected void onResume() {
		callHiddenWebViewMethod("onResume");
		wv.resumeTimers();
	super.onResume();
	}


	// Prevent flash player from going on after user presses home or back button
	private void callHiddenWebViewMethod(String name){
	    if( wv != null ){
	        try {
	            Method method = WebView.class.getMethod(name);
	            method.invoke(wv);
	        } catch (NoSuchMethodException e) {
	            //Log.error("No such method: " + name, e);
	        } catch (IllegalAccessException e) {
	            //Log.error("Illegal Access: " + name, e);
	        } catch (InvocationTargetException e) {
	            //Log.error("Invocation Target Exception: " + name, e);
	        }
	    }
	}

	
	// Do we want to override webview client?
	private class FlashWebViewClient extends WebViewClient {
	    @Override
	    public boolean shouldOverrideUrlLoading(WebView view, String url) {
	        view.loadUrl(url);
	        return true;
	    }
	}
}