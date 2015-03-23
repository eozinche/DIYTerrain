//
//  Terrain.h
//  DIYTerrain
//
//  Created by Eozin & MOQN on 10/19/14.
//

#pragma once

#define WINDOW_WIDTH                1024
#define WINDOW_HEIGHT               768

#define MESH_WIDTH                  200
#define MESH_HEIGHT                 200

#define INIT_HEIGHT_SCALE           10.f
#define HEIGHT_ADJUSTMENT           10
#define LIGHT_ROTATION_SPEED        0.3

#define FBO_DISPLAY_SIZE            100.f
#define FBO_DISPLAY_GAP             30.f



#include "cinder/app/AppNative.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/Material.h"
#include "cinder/gl/Texture.h"

#include "cinder/ObjLoader.h"
#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"

#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"




using namespace ci;
using namespace ci::app;
using namespace std;


typedef std::shared_ptr<gl::Light> LightRef;


class Terrain {
public:
    gl::Fbo             mFbo;
    static const int    FBO_WIDTH = MESH_WIDTH,
                        FBO_HEIGHT = MESH_HEIGHT;
    
    gl::VboMeshRef      mMesh;
    gl::GlslProgRef     mTerrainShader;
    gl::TextureRef      mHeightMap, mNormalMap;
    
    TriMesh             mTriMesh;
    vector<float>       fboVals;
    
    CameraPersp         mCam;

    gl::Material        mMaterial;
    LightRef            mLight;

    float               mHeightScale;
    float               mCameraDistance;
    Quatf               mRotation;
    
    
    int                 mBrushSize;
    bool                mBrushState;
    
    Perlin              mPerlin;
    
    Vec2i               mMousePos;
    bool                mMouseDragged;
    
    typedef enum        { POINTS, WIREFRAME, SHADED } terrainModes;
    terrainModes        mMode;
    terrainModes        mPrevMode;

    typedef enum        { PERSPECTIVE, SIDE, TOP } terrainViews;
    terrainViews        mView;
    
    typedef enum        { BLACK, ORANGE, GREEN, NAVY, MINT, VIOLET, CUSTOM } terrainColors;
    terrainColors       mColor;
    Color               mCustomColor;
    Color               mPrevColor;
    
    Terrain()
    {
        mMode               = POINTS;
        mPrevMode           = mMode;
        mView               = PERSPECTIVE;
        mColor              = ORANGE;
        mCustomColor        = Color( 1.0,0.3,0.0 );
        mPrevColor          = mCustomColor;
        
        mHeightScale        = INIT_HEIGHT_SCALE;
        mCameraDistance     = 250.0;
        mRotation.set       (0.4, 0, 0);
        
        mBrushSize          = 10;
        mBrushState         = true;
        
        mMouseDragged       = false;
        
    }
    ~Terrain(){}
    
    void setup();
    void update();
    void draw();
    
    void renderToFbo();
    void renderNoiseToFbo();
    void renderAnimatedNoiseToFbo();
    
    void buildMesh();
    
    void exportTerrain();
    
    void working();
    
};






