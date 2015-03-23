//
//  Terrain.cpp
//  DIYTerrain
//
//  Created by Eozin & MOQN on 10/19/14.
//


#include "Terrain.h"



void Terrain::setup()
{
    // fbo setting
    gl::Fbo::Format format;
    //format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
    format.enableDepthBuffer( false );
    format.setColorInternalFormat( GL_RGBA32F_ARB );
    mFbo = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
    
    renderNoiseToFbo();
    
    // texture setting
    mNormalMap = gl::Texture::create( loadImage( loadAsset("noiseField_NRM.png") ) );
    mHeightMap = gl::Texture::create( loadImage( loadAsset("noiseField_height.jpg") ) );
    
    // shader setting
    mTerrainShader = gl::GlslProg::create( loadAsset("vertex_blinPhong_height.glsl"),
                                          loadAsset("fragment_blinPhong_height.glsl")
                                          );
    
    // cam setting
    mCam.setPerspective(50, getWindowAspectRatio(), .1, 100000);
    mCam.lookAt( Vec3f(0,25,mCameraDistance), Vec3f(0,60,0), Vec3f::yAxis() );
    
    // light setting
    mLight = LightRef( new gl::Light( gl::Light::Type::POINT, 0 ) );
    mLight->setPosition(Vec3f(5,5,5));
    mLight->setAmbient(Color(.1,.1,.1));
    mLight->setDiffuse(Color(1.,.7,.1));
    //mLight->setSpecular(Color( 1.,1.,1. ));
    mLight->setSpecular(Color( 0.8, 0.8, 0.8 ));
    mLight->setAttenuation(1.,1., 1.);
    
    // build mesh
    buildMesh();
    
    // depth r & w
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
}



void Terrain::update()
{
    
    // rebuild mesh when mode changed
    if ( mPrevMode != mMode ) {
        buildMesh();
        mPrevMode = mMode;
    }
    
    // set Custom Mode when custom color is chosen
    if ( mPrevColor != mCustomColor ) {
        mColor = CUSTOM;
        mPrevColor = mCustomColor;
    }
    
    
    switch (mColor) {
            
        default:
            mMaterial = gl::Material(ColorA(.1,.1,.1,1.),
                                     ColorA ( 1.0,0.3,0.0 ),
                                     ColorA(1.,1.,1.,1.), 50. );
        case BLACK:
            mMaterial = gl::Material(ColorA(.1,.1,.1,1.),
                                     ColorA ( 0.0,0.0,0.0 ),
                                     ColorA(1.,1.,1.,1.), 50. );
            break;
            
            
        case ORANGE:
            mMaterial = gl::Material(ColorA(.1,.1,.1,1.),
                                     ColorA ( 1.0,0.3,0.0 ),
                                     ColorA(1.,1.,1.,1.), 50. );
            break;
            
        case GREEN:
            mMaterial = gl::Material( ColorA(.1,.1,.1,1.),
                                     ColorA ( 0.4, 0.6, 0.2, .5),
                                     ColorA(1.,1.,1.,1.), 50. );
            break;
            
            
        case NAVY:
            mMaterial = gl::Material(ColorA(.1,.1,.1,1.),
                                     ColorA ( 0.0,0.0,0.9 ),
                                     ColorA(1.,1.,1.,1.), 50. );
            break;
            
        case MINT:
            mMaterial = gl::Material( ColorA(.1,.1,.1,1.),
                                     ColorA ( 0.1,0.2,0.8 ),
                                     ColorA(1.,1.,1.,1.), 50. );
            break;
            
        case VIOLET:
            mMaterial = gl::Material( ColorA(.1,.1,.1,1.),
                                     ColorA ( 0.2,0.0,0.8 ),
                                     ColorA(1.,1.,1.,1.), 50. );
            break;
            
        case CUSTOM:
            mMaterial = gl::Material( ColorA(.1,.1,.1,1.),
                                     mCustomColor,
                                     ColorA(1.,1.,1.,1.), 50. );
            break;
    }
    
    mLight->setPosition(Vec3f(120*cos(getElapsedSeconds()*LIGHT_ROTATION_SPEED),
                              600,
                              80+120*sin(getElapsedSeconds()*LIGHT_ROTATION_SPEED)));
}



void Terrain::draw()
{
    // clear
    gl::clear( Color( 0.3, 0.3, 0.3 ) );
    
    // fbo
    // renderAnimatedNoiseToFbo();
    gl::setViewport( getWindowBounds() );
    gl::setMatricesWindow( getWindowSize() );
    
    gl::Texture mFboTextrue = mFbo.getTexture();
    //mFboTextrue.setFlipped();
    if ( mMouseDragged ) {
        gl::color( 0.4f,0.4f,0.4f );
        gl::drawStrokedRect( Rectf( FBO_DISPLAY_GAP-3, FBO_DISPLAY_GAP-3,
                                   FBO_DISPLAY_GAP+FBO_DISPLAY_SIZE+3,
                                   FBO_DISPLAY_GAP+FBO_DISPLAY_SIZE+3 ) );
        gl::color( Color::white() );
        gl::draw( mFboTextrue, Rectf( FBO_DISPLAY_GAP, FBO_DISPLAY_GAP,
                                     FBO_DISPLAY_GAP+FBO_DISPLAY_SIZE,
                                     FBO_DISPLAY_GAP+FBO_DISPLAY_SIZE ) );
    }
    
    // set cam
    gl::setMatrices( mCam );
    gl::rotate(mRotation);
    
    mMaterial.apply();
    
    //mNormalMap->bind(0);
    mHeightMap->bind(0);
    mFboTextrue.bind(1);
    //mFbo.bindTexture(1);
    
    
    mTerrainShader->bind();
    mTerrainShader->uniform("normalMap", 0);
    mTerrainShader->uniform("heightMap", 1);
    mTerrainShader->uniform("heightScale", mHeightScale * HEIGHT_ADJUSTMENT );
    
    gl::color( Color::white() );
    if( mMode == WIREFRAME ) {
        gl::enableWireframe();
        gl::draw(mMesh);
        gl::disableWireframe();
    } else {
        gl::draw(mMesh);
    }
    
    //gl::drawSphere(mLight->getPosition(), .1);
    
    mTerrainShader->unbind();
    
    mNormalMap->unbind();
    mHeightMap->unbind();
    mFbo.unbindTexture();
    mFboTextrue.unbind();
}


void Terrain::buildMesh(){
    //build a high-res mesh to draw the texture to, later we can manipulate these vertices with the shader
    vector<Vec3f> positions;
    vector<Vec2f> texCoords;
    vector<Vec3f> normals;
    vector<uint32_t> indices;
    
    int width = MESH_WIDTH;
    int height = MESH_WIDTH;
    
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            
            //position these to make a flat plane that is centered around 0
            positions.push_back( Vec3f( x - (width / 2) , 0, y - (height / 2 ) ) );
            //Texcoords are normalized from 0-1, so divide the positions by the max. Cast the ints as floats to get a more precise division.
            texCoords.push_back( Vec2f( (float)x / ( (float)width-1.), (float)y / ( (float)height-1. ) ) );
            //add the triangles, except for edge cases
            if(x < width-1 && y < height - 1){
                indices.push_back(  x    +  y    * width );
                indices.push_back(  x    + (y+1) * width );
                indices.push_back( (x+1) + (y+1) * width );
                indices.push_back(  x    +  y    * width );
                indices.push_back( (x+1) + (y+1) * width );
                indices.push_back( (x+1) +  y    * width );
            }
            
            Vec3f normal = cross(Vec3f(x, y+1,0.f)-Vec3f(x, y,0.f), Vec3f(x+1, y,0.f)-Vec3f(x, y,0.f)).normalized();
            normals.push_back(normal);
            
        }
    }
    
    //create a layout that will include the types of data we are sending to the GPU
    
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    
    //create the mesh with the right number of verticies and indicies, set the layout and tell it how th draw
    
    
    if (mMode == POINTS ) mMesh = gl::VboMesh::create( positions.size(), indices.size(), layout, GL_POINTS );
    else mMesh = gl::VboMesh::create( positions.size(), indices.size(), layout, GL_TRIANGLES );
    
    //add the data to the mesh
    mMesh->bufferPositions( positions );
    // mMesh->bufferNormals( normals );
    mMesh->bufferIndices( indices );
    mMesh->bufferTexCoords2d( 0, texCoords );
    
    //unbind the mesh to clean up
    mMesh->unbindBuffers();
    
}



void Terrain::renderToFbo()
{
    float posX = lmap( (float)mMousePos.x,
                      0.f+100, (float)getWindowWidth()-100,
                      0.f, (float)FBO_WIDTH );
    float posY = lmap( (float)mMousePos.y,
                      (float)getWindowHeight()-100, 0.f+100,
                      0.f, (float)FBO_HEIGHT );
    
    gl::setViewport( Area( Vec2i::zero(), Vec2i(FBO_WIDTH,FBO_HEIGHT) ) );
    gl::setMatricesWindow( FBO_WIDTH,FBO_HEIGHT );
    
    gl::enableAlphaBlending();
    
    if ( mBrushState ) gl::color( 1.0f, 1.0f, 1.0f, 0.001f );
    else gl::color( 0.0f, 0.0f, 0.0f, 0.001f );
    
    mFbo.bindFramebuffer();
    for ( int i=0; i<mBrushSize; i++) {
        gl::drawSolidCircle( Vec2f(posX, posY), 1 + i*2 );
    }
    mFbo.unbindFramebuffer();
    
    gl::disableAlphaBlending();
}



void Terrain::renderNoiseToFbo()
{
    
    gl::setViewport( Area( Vec2i::zero(), Vec2i(FBO_WIDTH,FBO_HEIGHT) ) );
    gl::setMatricesWindow( FBO_WIDTH,FBO_HEIGHT );
    
    
    mFbo.bindFramebuffer();
    gl::clear( Color::black() );
    
    float xRand = randFloat(0.03f, 0.05f);
    float yRand = randFloat(0.03f, 0.05f);
    float noizeRand = randFloat(0.1f, 0.20f);
    
    for( int y=0; y<FBO_HEIGHT; y++ ){
        for( int x=0; x<FBO_WIDTH; x++ ){
            float c = mPerlin.noise(x * xRand,
                                    y * yRand ) * 85.0f;
            float n = mPerlin.noise(x * noizeRand,
                                    y * noizeRand ) * 15.0f;
            float colorVal = lmap( c+n, -100.0f, 100.0f, 0.35f, 0.65f );
            gl::color( colorVal,colorVal,colorVal );
            gl::drawSolidCircle( Vec2f(x,y), 1.0f);
        }
    }
    
    mFbo.unbindFramebuffer();
    
    
    gl::setViewport( getWindowBounds() );
    gl::setMatricesWindow( getWindowSize() );
}



void Terrain::renderAnimatedNoiseToFbo()
{
    
    gl::setViewport( Area( Vec2i::zero(), Vec2i(FBO_WIDTH,FBO_HEIGHT) ) );
    gl::setMatricesWindow( FBO_WIDTH,FBO_HEIGHT );
    
    
    mFbo.bindFramebuffer();
    gl::clear( Color::black() );
    
    float noizeRand = randFloat(0.1f, 0.20f);
    
    for( int y=0; y<FBO_HEIGHT; y++ ){
        for( int x=0; x<FBO_WIDTH; x++ ){
            float c = mPerlin.noise(x * (0.1 + sin(getElapsedSeconds()*0.1)*0.08),
                                    y * (0.1 + cos(getElapsedSeconds()*0.1)*0.05) ) * 85.0f;
            float n = mPerlin.noise(x * noizeRand,
                                    y * noizeRand ) * 15.0f;
            float colorVal = lmap( c+n, -100.0f, 100.0f, 0.45f, 0.55f );
            gl::color( colorVal,colorVal,colorVal );
            gl::drawSolidCircle( Vec2f(x,y), 1.0f);
        }
    }
    
    mFbo.unbindFramebuffer();
    
    
    gl::setViewport( getWindowBounds() );
    gl::setMatricesWindow( getWindowSize() );
}



void Terrain::working() {
    cout << "Ok. this is working" << endl;
}



void Terrain::exportTerrain() {
    
    // clear fbo Value Vector
    fboVals.clear();
    
    // create a surface from the fbo
    Surface32f mSur( mFbo.getTexture() );
    
    for ( int y=0; y<mSur.getHeight(); y++) {
        for ( int x=0; x<mSur.getWidth(); x++) {
            fboVals.push_back( mSur.getPixel(Vec2i(x,y))[0] );
        }
    }
    
    // clear the previous trimesh daya
    mTriMesh.clear();
    
    for(int y = 0; y < MESH_HEIGHT; y++){
        for(int x = 0; x < MESH_WIDTH; x++){
            
            int index = y*MESH_WIDTH + x;
            
            mTriMesh.appendVertex(Vec3f( x - (MESH_WIDTH / 2) , fboVals[index]*mHeightScale*HEIGHT_ADJUSTMENT , y - (MESH_HEIGHT / 2 ) ) );
            mTriMesh.appendTexCoord(Vec2f( (float)x / ( (float)MESH_WIDTH-1.), (float)y / ( (float)MESH_HEIGHT-1. ) ) );
            
            if(x < MESH_WIDTH-1 && y < MESH_HEIGHT - 1){
                mTriMesh.appendTriangle( x + y * MESH_WIDTH, x + (y+1) * MESH_WIDTH, (x+1) + (y+1) * MESH_WIDTH );
                mTriMesh.appendTriangle( x + y * MESH_WIDTH, (x+1) + (y+1) * MESH_WIDTH , (x+1) + y * MESH_WIDTH );
            }
            
            Vec3f normal = cross(Vec3f(x, y+1,0.f)-Vec3f(x, y,0.f), Vec3f(x+1, y,0.f)-Vec3f(x, y,0.f)).normalized();
            mTriMesh.appendNormal(normal);
            
        }
    }
    
    // save the obj
    fs::path path = getSaveFilePath( "myTerrain.obj" );
    
    if( ! path.empty() ) {
        console() << "Saving to " << path;
        ObjLoader::write( writeFile( path ), mTriMesh );
    }
    
}




