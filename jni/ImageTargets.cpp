/*==============================================================================
            Copyright (c) 2010-2011 QUALCOMM Incorporated.
            All Rights Reserved.
            Qualcomm Confidential and Proprietary
            
@file 
    ImageTargets.cpp

@brief
    Sample for ImageTargets

==============================================================================*/


#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef USE_OPENGL_ES_1_1
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/CameraCalibration.h>
#include <QCAR/UpdateCallback.h>
#include <QCAR/Area.h>
#include <QCAR/Rectangle.h>
#include <QCAR/ImageTarget.h>
#include <QCAR/VirtualButton.h>

#include "SampleUtils.h"
#include "CubeShaders.h"

#ifdef __cplusplus
extern "C"
{
#endif

bool isFree=true;
int idOfActiveWifi[10]={0};	//*******************************************************************  
int numOfActiveWifi =0;	
bool displayedMessage;

    
bool isActivityInPortraitMode   = false;

unsigned int screenWidth        = 0;
unsigned int screenHeight       = 0;

#ifdef USE_OPENGL_ES_2_0
unsigned int shaderProgramID    = 0;
GLint vertexHandle              = 0;
GLint normalHandle              = 0;
GLint textureCoordHandle        = 0;
GLint mvpMatrixHandle           = 0;

unsigned int vbShaderProgramID  = 0;
GLint vbVertexHandle            = 0;
#endif

QCAR::Matrix44F projectionMatrix;

int numActiveTrackables = 0;

class ActiveTrackableInfo
{
		public:
		int tIdx;	//可能需要修改成string name*******************************************************************************************
		float pixelsCoord[4];	
};

ActiveTrackableInfo trackableInfo[10];	//使用时注意修改数组的大小***********************************************************************


//用来做测试的**********************************************************************************************************************************
GLfloat vertexCoordinates[1][4]={-123.f,86.f,123.f, -86.f};
float num1=-100.f;
float num2=-200.f;
float num3=-100.0f;
float num4=-100.0f;

JNIEXPORT jfloat JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_getTryNative1(JNIEnv *, jobject)
{
		return trackableInfo[0].pixelsCoord[0];	//num1;
}
JNIEXPORT jfloat JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_getTryNative2(JNIEnv *, jobject)
{
		return trackableInfo[0].pixelsCoord[1];	//num2;
}
JNIEXPORT jfloat JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_getTryNative3(JNIEnv *, jobject)
{
		return trackableInfo[0].pixelsCoord[2];	//num3;
}
JNIEXPORT jfloat JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_getTryNative4(JNIEnv *, jobject)
{
		return trackableInfo[0].pixelsCoord[3];	//num4;
}
//**********************************************************************************************************************************



//***************************************************************************************************************************************
JNIEXPORT jboolean JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_isFreeNative(JNIEnv *, jobject)
{
			return isFree;
}

JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_setIsFreeNative(JNIEnv *, jobject)
{
			isFree = isFree? false:true;
}

JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_setNumOfActiveWifi(JNIEnv *, jobject , jint num)
{
			numOfActiveWifi = num;
}

JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_setIdOfAvtiveWifi(JNIEnv *, jobject , jint i,jint id)
{
			idOfActiveWifi[i] = id;
}




JNIEXPORT jint JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_getOpenGlEsVersionNative(JNIEnv *, jobject)
{
#ifdef USE_OPENGL_ES_1_1        
    return 1;
#else
    return 2;
#endif
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
{
    isActivityInPortraitMode = isPortrait;
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_initApplicationNative( JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_initApplicationNative");
    
    screenWidth = width;
    screenHeight = height;
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_initRendering( JNIEnv* env, jobject obj)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_initRendering");

    glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);

#ifndef USE_OPENGL_ES_1_1  
    shaderProgramID     = SampleUtils::createProgramFromBuffer(cubeMeshVertexShader, cubeFragmentShader);

    vertexHandle        = glGetAttribLocation(shaderProgramID, "vertexPosition");
    normalHandle        = glGetAttribLocation(shaderProgramID, "vertexNormal");
    textureCoordHandle  = glGetAttribLocation(shaderProgramID, "vertexTexCoord");
    mvpMatrixHandle     = glGetUniformLocation(shaderProgramID, "modelViewProjectionMatrix");

		vbShaderProgramID   = SampleUtils::createProgramFromBuffer(lineMeshVertexShader, lineFragmentShader);
                                                               
    vbVertexHandle      = glGetAttribLocation(vbShaderProgramID, "vertexPosition");   
#endif
}


JNIEXPORT int JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_renderFrame(JNIEnv *, jobject)
{
    //LOG("Java_com_qualcomm_QCARSamples_ImageTargets_GLRenderer_renderFrame");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QCAR::State state = QCAR::Renderer::getInstance().begin();
        
#ifdef USE_OPENGL_ES_1_1
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);       
#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    int trackingNumber = 0;

		if(state.getNumActiveTrackables())
		{
	    const QCAR::Trackable* trackable;
	    QCAR::Matrix44F modelViewMatrix;
	    QCAR::Matrix44F modelViewProjection;
	    const QCAR::ImageTarget* target;
	    	
	    GLfloat vbVertices[24];
	    unsigned char vbCounter;
      const QCAR::VirtualButton* button;
      const QCAR::Area* vbArea;
      const QCAR::Rectangle* vbRectangle;
      	
      // Did we find any trackables this frame?
	    for(int tIdx = 0; tIdx < state.getNumActiveTrackables(); tIdx++ )
	    {
	        trackable = state.getActiveTrackable(tIdx);
	        modelViewMatrix = QCAR::Tool::convertPose2GLMatrix(trackable->getPose());       
	        SampleUtils::multiplyMatrix(&projectionMatrix.data[0], &modelViewMatrix.data[0], &modelViewProjection.data[0]); 
	        	     	    			
	        assert(trackable->getType() == QCAR::Trackable::IMAGE_TARGET);
	        target = static_cast<const QCAR::ImageTarget*>(trackable);


	        trackingNumber = 0;

	        /*
	        if((!strcmp(trackable->getName(), "stones")))
	        {
	        	trackingNumber = 1;
	        }
	        if((!strcmp(trackable->getName(), "chips")))
	        {
	        	trackingNumber = 2;
	        }
	        */

	        if((!strcmp(trackable->getName(), "trackable1")))
	        	        {
	        	        	trackingNumber = 1;
	        	        }
	        	        if((!strcmp(trackable->getName(), "trackable2")))
	        	        {
	        	        	trackingNumber = 2;
	        	        }
	        	        if((!strcmp(trackable->getName(), "trackable3")))
	        	        	        	        {
	        	        	        	        	trackingNumber = 3;
	        	        	        	        }

	      	vbCounter=0;
	
	        if ( target->getNumVirtualButtons() )
	        {
	            button = target->getVirtualButton( 0 );
	            vbArea = &button->getArea();
	            assert(vbArea->getType() == QCAR::Area::RECTANGLE);
	            vbRectangle = static_cast<const QCAR::Rectangle*>(vbArea);
	
	            vbVertices[vbCounter+ 0]=vbRectangle->getLeftTopX();
	            vbVertices[vbCounter+ 1]=vbRectangle->getLeftTopY();
	            vbVertices[vbCounter+ 2]=0.0f;
	            vbVertices[vbCounter+ 3]=vbRectangle->getRightBottomX();
	            vbVertices[vbCounter+ 4]=vbRectangle->getLeftTopY();
	            vbVertices[vbCounter+ 5]=0.0f;
	            vbVertices[vbCounter+ 6]=vbRectangle->getRightBottomX();
	            vbVertices[vbCounter+ 7]=vbRectangle->getLeftTopY();
	            vbVertices[vbCounter+ 8]=0.0f;
	            vbVertices[vbCounter+ 9]=vbRectangle->getRightBottomX();
	            vbVertices[vbCounter+10]=vbRectangle->getRightBottomY();
	            vbVertices[vbCounter+11]=0.0f;
	            vbVertices[vbCounter+12]=vbRectangle->getRightBottomX();
	            vbVertices[vbCounter+13]=vbRectangle->getRightBottomY();
	            vbVertices[vbCounter+14]=0.0f;
	            vbVertices[vbCounter+15]=vbRectangle->getLeftTopX();
	            vbVertices[vbCounter+16]=vbRectangle->getRightBottomY();
	            vbVertices[vbCounter+17]=0.0f;
	            vbVertices[vbCounter+18]=vbRectangle->getLeftTopX();
	            vbVertices[vbCounter+19]=vbRectangle->getRightBottomY();
	            vbVertices[vbCounter+20]=0.0f;
	            vbVertices[vbCounter+21]=vbRectangle->getLeftTopX();
	            vbVertices[vbCounter+22]=vbRectangle->getLeftTopY();
	            vbVertices[vbCounter+23]=0.0f;
	
	            vbCounter+=24;       
	        }                        
	
	
	

/*另一种方法求点******************************************************************************************************************************
					for( int i = 0; i < 24; i++ )	
	      	{
	      			vbVertices[ i ] = 0.0f;	
	      	}
	      	vbCounter=0;
	      	
					vbVertices[vbCounter+ 0]=vertexCoordinates[0][0];
	        vbVertices[vbCounter+ 1]=vertexCoordinates[0][1];
	        vbVertices[vbCounter+ 2]=0.0f;
	        vbVertices[vbCounter+ 3]=vertexCoordinates[0][2];
	        vbVertices[vbCounter+ 4]=vertexCoordinates[0][1];
	        vbVertices[vbCounter+ 5]=0.0f;
	        vbVertices[vbCounter+ 6]=vertexCoordinates[0][2];
	        vbVertices[vbCounter+ 7]=vertexCoordinates[0][1];
	        vbVertices[vbCounter+ 8]=0.0f;
	        vbVertices[vbCounter+ 9]=vertexCoordinates[0][2];
	        vbVertices[vbCounter+10]=vertexCoordinates[0][3];
	        vbVertices[vbCounter+11]=0.0f;
	        vbVertices[vbCounter+12]=vertexCoordinates[0][2];
	        vbVertices[vbCounter+13]=vertexCoordinates[0][3];
	        vbVertices[vbCounter+14]=0.0f;
	        vbVertices[vbCounter+15]=vertexCoordinates[0][0];
	        vbVertices[vbCounter+16]=vertexCoordinates[0][3];
	        vbVertices[vbCounter+17]=0.0f;
	        vbVertices[vbCounter+18]=vertexCoordinates[0][0];
	        vbVertices[vbCounter+19]=vertexCoordinates[0][3];
	        vbVertices[vbCounter+20]=0.0f;
	        vbVertices[vbCounter+21]=vertexCoordinates[0][0];
	        vbVertices[vbCounter+22]=vertexCoordinates[0][1];
	        vbVertices[vbCounter+23]=0.0f;
	        
	        vbCounter+=24;     
*///*********************************************************************************************************************************            



	        if( vbCounter > 0 )
	        {
	            glUseProgram(vbShaderProgramID);

	            glVertexAttribPointer(vbVertexHandle, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)vbVertices);
	            glEnableVertexAttribArray(vbVertexHandle);

	            glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE, (GLfloat*)&modelViewProjection.data[0] );
	
	
	
							//试验直线的颜色和宽度*************************************************************************************************
//						glEnable(GL_LINE_SMOOTH);
//						glEnable(GL_BLEND);
//						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//						glHint(GL_LINE_SMOOTH_HINT,GL_DONT_CARE);
//						glDisable(GL_TEXTURE_2D);
//						glColor3f( 1.0 , 0.0 , 0.0 );
							glLineWidth( 10.0f );				
							//**********************************************************************************************************************
					
							
							
	            glDrawArrays(GL_LINES, 0, 8 );
	            SampleUtils::checkGlError("VirtualButtons drawButton");

	            glDisableVertexAttribArray(vbVertexHandle);
	        	}

	        /*
	        	if(trackingNumber == 1)
	        	{
	        	 	return trackingNumber;
	        	} */
	    	}
	    	
	    	if (!displayedMessage) 
	    	{
            //displayMessage("Touch and drag slowly to drop a row of dominoes.");
            //displayedMessage = true;
        }
		}

    glDisable(GL_DEPTH_TEST);

#ifdef USE_OPENGL_ES_1_1        
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); 
#endif

    return trackingNumber;
    QCAR::Renderer::getInstance().end();
}


void
configureVideoBackground()
{
    QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
    QCAR::VideoMode videoMode = cameraDevice.getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);


    QCAR::VideoBackgroundConfig config;
    config.mEnabled = true;
    config.mSynchronous = true;
    config.mPosition.data[0] = 0.0f;
    config.mPosition.data[1] = 0.0f;
    
    if (isActivityInPortraitMode)
    {
        //LOG("configureVideoBackground PORTRAIT");
        
        config.mSize.data[0] = videoMode.mHeight * (screenHeight / (float)videoMode.mWidth);
        config.mSize.data[1] = screenHeight;
    }
    else
    {
        //LOG("configureVideoBackground LANDSCAPE");
        
        config.mSize.data[0] = screenWidth;
        config.mSize.data[1] = videoMode.mHeight * (screenWidth / (float)videoMode.mWidth);
    }

    QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_updateRendering( JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_updateRendering");
    
    screenWidth = width;
    screenHeight = height;

    configureVideoBackground();
}

class VirtualButton_UpdateCallback : public QCAR::UpdateCallback
{    
    virtual void QCAR_onUpdate(QCAR::State& state )
    { 		
        if ( state.getNumActiveTrackables() > 0 )
        {
        		numActiveTrackables = state.getNumActiveTrackables();        		        		        		
        		
        		QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();	//简化成共有变量*********************************
        		QCAR::VideoMode videoMode = cameraDevice.getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);
        			
        		const QCAR::Tracker& tracker = QCAR::Tracker::getInstance();
				    const QCAR::CameraCalibration& cameraCalibration = tracker.getCameraCalibration();
				    	
				    int i , j;	
				    	
        		const QCAR::Trackable* trackable;
        		const QCAR::Trackable* trackable1;
        		const QCAR::ImageTarget* target;        		
        		const QCAR::VirtualButton* button;
        		const QCAR::Area* vbArea;
        		const QCAR::Rectangle* vbRectangle;


        		QCAR::Vec3F vecCoords3F;
        		QCAR::Vec2F vecCoords2F;    
        		        		
        		for( i = 0 ; i < numActiveTrackables; i ++ )
        		{
        				trackable = state.getActiveTrackable( i );  
 				        assert(trackable->getType() == QCAR::Trackable::IMAGE_TARGET);
				        
								for( j = 0; j < tracker.getNumTrackables(); j ++ )
								{														
										if( strcmp( trackable->getName(), tracker.getTrackable( j )->getName() ) == 0 )
												break;
								}				
								trackableInfo[i].tIdx = j;
													
								trackable1 = tracker.getTrackable(j);
								target = static_cast<const QCAR::ImageTarget*>(trackable1);				        
				        button = target->getVirtualButton( 0 );                   
				        vbArea = &button->getArea();
				        assert(vbArea->getType() == QCAR::Area::RECTANGLE);
				        vbRectangle = static_cast<const QCAR::Rectangle*>(vbArea);
            	
            		vecCoords3F.data[0] = vbRectangle->getLeftTopX();
				    		vecCoords3F.data[1] = vbRectangle->getLeftTopY();
				    		vecCoords3F.data[2] = 0.0f;				    			
				    		vecCoords2F = QCAR::Tool::projectPoint(cameraCalibration, trackable->getPose() , vecCoords3F );
				    		trackableInfo[i].pixelsCoord[0] = vecCoords2F.data[0] *(screenWidth/(float)videoMode.mWidth);//优化啊，把它们变成常量***************
				    		trackableInfo[i].pixelsCoord[1] = vecCoords2F.data[1] *(screenWidth/(float)videoMode.mWidth);
				    			
				        vecCoords3F.data[0] = vbRectangle->getRightBottomX();
				    		vecCoords3F.data[1] = vbRectangle->getRightBottomY();
				        vecCoords2F = QCAR::Tool::projectPoint(cameraCalibration, trackable->getPose() , vecCoords3F );
				        trackableInfo[i].pixelsCoord[2] = vecCoords2F.data[0] *(screenWidth/(float)videoMode.mWidth);
				    		trackableInfo[i].pixelsCoord[3] = vecCoords2F.data[1] *(screenWidth/(float)videoMode.mWidth);
		    		}
        }
        else
        {
        		numActiveTrackables = 0;   
        }
    }
} qcarUpdate;


JNIEXPORT void JNICALL	//不懂什么意思***********************************************************************************************
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_onQCARInitializedNative(JNIEnv *, jobject)
{
	QCAR::setHint(QCAR::HINT_MAX_SIMULTANEOUS_IMAGE_TARGETS, 3);
		//QCAR::setHint(QCAR::HINT_MAX_SIMULTANEOUS_IMAGE_TARGETS, 2);
//		QCAR::setHint(QCAR::HINT_IMAGE_TARGET_MULTI_FRAME_ENABLED, 1);
    
    QCAR::registerCallback( &qcarUpdate );
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_startCamera(JNIEnv *, jobject)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_startCamera");

    if (!QCAR::CameraDevice::getInstance().init())
        return;

    configureVideoBackground();

    if (!QCAR::CameraDevice::getInstance().selectVideoMode( QCAR::CameraDevice::MODE_DEFAULT))
        return;

    if (!QCAR::CameraDevice::getInstance().start())
        return;

    //if(QCAR::CameraDevice::getInstance().setFlashTorchMode(true))
    //LOG("IMAGE TARGETS : enabled torch");

    //if(QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_INFINITY))
    //LOG("IMAGE TARGETS : enabled infinity focus");

    QCAR::Tracker::getInstance().start();
 
    const QCAR::Tracker& tracker = QCAR::Tracker::getInstance();
    const QCAR::CameraCalibration& cameraCalibration = tracker.getCameraCalibration();
    projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f, 2000.0f);
}


JNIEXPORT jboolean JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_toggleFlash(JNIEnv*, jobject, jboolean flash)
{
    return QCAR::CameraDevice::getInstance().setFlashTorchMode((flash==JNI_TRUE)) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT jboolean JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_autofocus(JNIEnv*, jobject)
{
    return QCAR::CameraDevice::getInstance().startAutoFocus()?JNI_TRUE:JNI_FALSE;
}


JNIEXPORT jboolean JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_setFocusMode(JNIEnv*, jobject, jint mode)
{
    return QCAR::CameraDevice::getInstance().setFocusMode(mode)?JNI_TRUE:JNI_FALSE;
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_stopCamera(JNIEnv *, jobject)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_stopCamera");

    QCAR::Tracker::getInstance().stop();
    QCAR::CameraDevice::getInstance().stop();
    QCAR::CameraDevice::getInstance().deinit();
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_deinitApplicationNative( JNIEnv* env, jobject obj)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_deinitApplicationNative");
}


JNIEXPORT int JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_onTouchEventNative(JNIEnv *, jobject , jfloat X , jfloat Y )
{
		if( numActiveTrackables > 0 )
		{
				float xx;
				QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();	//简化成共有变量*********************************
        QCAR::VideoMode videoMode = cameraDevice.getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);
        xx=((float)videoMode.mHeight*screenWidth/videoMode.mWidth-screenHeight)/2.0f;
				for( int i= 0; i < numActiveTrackables ; i ++ )
				{
						if( ( X >= trackableInfo[ i ].pixelsCoord[0] && X <= trackableInfo[ i ].pixelsCoord[2] )||
								( X >= trackableInfo[ i ].pixelsCoord[2] && X <= trackableInfo[ i ].pixelsCoord[0] ) )
								if( ( Y >= ( trackableInfo[ i ].pixelsCoord[1] - xx )&& Y <= (trackableInfo[ i ].pixelsCoord[3]-xx) )||
										( Y >= ( trackableInfo[ i ].pixelsCoord[3] - xx )&& Y <= (trackableInfo[ i ].pixelsCoord[1]-xx) ) )
										return trackableInfo[ i ].tIdx;
				}
							
				return -1;
		}
		else
		{
			return -1;
		}
}


#ifdef __cplusplus
}
#endif
