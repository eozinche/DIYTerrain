//
//  diyterrainApp.cpp w/ Cinder
//
//
//  Created by Eozin & MOQN on 10/19/14.
//


#include "Terrain.h"



class diyterrainApp : public AppNative {
public:
    void setup();
    void prepareSettings( Settings* settings);
    
    void keyDown( KeyEvent event );
    void mouseMove( MouseEvent event );
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void mouseUp( MouseEvent event );
    void mouseWheel( MouseEvent event );
    
    void update();
    void draw();
    
    void renewTerrain();
    void exportTerrain();
    
    void changeView_side();
    void changeView_top();
    
    Terrain theTerrain;
    params::InterfaceGlRef mGui;
    
};



void diyterrainApp::prepareSettings(cinder::app::AppBasic::Settings *settings){
    settings->setFullScreen();
    //settings->setWindowSize(1024, 768);
}



void diyterrainApp::setup()
{
    theTerrain.setup();
    
    // draw GUIs
    int guiSize = 220;
    int guiGap = 30;
    
    std::string tModes[] = { "Points", "Wireframe", "Shaded" };
    std::string tColors[] = { "Black", "Orange", "Green", "Navy", "Mint", "Violet", "Custom" };
    
    mGui = params::InterfaceGl::create( "Parameters" , Vec2f(guiSize,guiSize+200), ColorA( 0,0,0,.3 ) );
    mGui->setPosition( Vec2f( getWindowWidth()-guiSize-guiGap, guiGap ) );
    
    mGui->addSeparator();
    mGui->addButton("New", std::bind( &diyterrainApp::renewTerrain, this) );
    mGui->addButton("Export", std::bind( &diyterrainApp::exportTerrain, this) );
    
    mGui->addSeparator();
    mGui->addParam( "Render Mode", vector<string>(tModes,tModes+3), (int*) &theTerrain.mMode );
    //mGui->addParam( "View Mode", vector<string>(tViews,tViews+3), (int*) &theTerrain.mView );
    mGui->addButton("Side View", std::bind( &diyterrainApp::changeView_side, this) );
    mGui->addButton("Top View", std::bind( &diyterrainApp::changeView_top, this) );
    
    mGui->addParam( "Distance", &theTerrain.mCameraDistance,
                   "min=50.0 max=400.0 step=1.0" );
    
    mGui->addSeparator();
    mGui->addParam( "Height Scale", &theTerrain.mHeightScale,
                   "min=-10.0 max=20.0 step=0.1" );
    mGui->addParam( "Brush Size", &theTerrain.mBrushSize,
                   "min=3 max=15 step=1.0" );
    
    
    mGui->addSeparator();
    mGui->addParam( "Color Mode", vector<string>(tColors,tColors+7), (int*) &theTerrain.mColor);
    mGui->addParam( "Color", &theTerrain.mCustomColor );
    
    mGui->addSeparator();
    
    mGui->addSeparator();
    mGui->addParam( "Rotate Scene", &theTerrain.mRotation );
}



void diyterrainApp::update()
{
    theTerrain.update();
}



void diyterrainApp::draw()
{
    theTerrain.draw();
    mGui->draw();
}



void diyterrainApp::keyDown( KeyEvent event )
{
    switch( event.getCode() ) {
        case KeyEvent::KEY_p:
            theTerrain.mView = Terrain::PERSPECTIVE;
            break;
        case KeyEvent::KEY_t:
            theTerrain.mView = Terrain::TOP;
            break;
        case KeyEvent::KEY_s:
            theTerrain.mView = Terrain::SIDE;
            break;
        case KeyEvent::KEY_r:
            theTerrain.renderNoiseToFbo();
            break;
        case KeyEvent::KEY_m:
            if( theTerrain.mMode == Terrain::POINTS ){
                theTerrain.mMode = Terrain::WIREFRAME;
            }
            else if( theTerrain.mMode == Terrain::WIREFRAME ){
                theTerrain.mMode = Terrain::SHADED;
            }
            else if( theTerrain.mMode == Terrain::SHADED ){
                theTerrain.mMode = Terrain::POINTS;
            }
            break;
    }
}



void diyterrainApp::mouseDrag( MouseEvent event )
{
    //theTerrain.addDraggedPoints( event.getPos() );
    if ( event.isLeftDown() ) theTerrain.mBrushState = true;
    else if ( event.isRightDown() ) theTerrain.mBrushState = false;
    
    theTerrain.mMousePos = event.getPos();
    theTerrain.renderToFbo();
    theTerrain.mMouseDragged = true;
}



void diyterrainApp::mouseUp( MouseEvent event )
{
    theTerrain.mMouseDragged = false;
}



void diyterrainApp::mouseDown( MouseEvent event )
{
    if ( event.isLeftDown() ) theTerrain.mBrushState = true;
    else if ( event.isRightDown() ) theTerrain.mBrushState = false;
    
    theTerrain.renderToFbo();
    theTerrain.mMouseDragged = true;
}



void diyterrainApp::mouseMove( MouseEvent event )
{
//    if ( event.getPos().x < 10 ) {
//        theTerrain.mCam.lookAt( Vec3f(0,0, 0), Vec3f(getElapsedFrames(),0,0) );
//    }
}



void diyterrainApp::mouseWheel( MouseEvent event ){
    
    theTerrain.mCameraDistance += event.getWheelIncrement() * -1.5;
    
    if(theTerrain.mCameraDistance>=500) theTerrain.mCameraDistance = 500;
    else if(theTerrain.mCameraDistance<=50) theTerrain.mCameraDistance = 50;
    
    theTerrain.mCam.lookAt( Vec3f(0,25,theTerrain.mCameraDistance), Vec3f(0,60,0), Vec3f::yAxis() );
}



void diyterrainApp::renewTerrain()
{
    theTerrain.mHeightScale = INIT_HEIGHT_SCALE;
    theTerrain.mRotation.set(0.4, 0, 0);
    theTerrain.mCameraDistance = 250.f;
    theTerrain.mCustomColor = Color( 1.0,0.3,0.0 );;
    
    theTerrain.mMode = Terrain::POINTS;
    theTerrain.mView = Terrain::PERSPECTIVE;
    theTerrain.mColor = Terrain::CUSTOM;
    
    theTerrain.renderNoiseToFbo();
}



void diyterrainApp::exportTerrain()
{
    theTerrain.exportTerrain();
}



void diyterrainApp::changeView_side(){
    theTerrain.mCam.lookAt( Vec3f(0,-70,theTerrain.mCameraDistance), Vec3f(0,60,0), Vec3f::yAxis() );
    theTerrain.mRotation.set(0.45, 0, 0);
}



void diyterrainApp::changeView_top(){
    theTerrain.mCam.lookAt( Vec3f(0,200,50), Vec3f(0,60,0), Vec3f::yAxis() );
    theTerrain.mRotation.set(0.3, 0, 0);
}

CINDER_APP_NATIVE( diyterrainApp, RendererGl )

