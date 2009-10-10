/*
	ViRo HUD
	by Bruno Fanini ( http://www.brunofanini.tk )

========================================================================*/

// Header
#include <CommonCore/Manipulators/ViroHud.h>

#include <osg/Notify>
#include <osg/Matrixd>
#include <osg/Transform>
#include <osg/CullStack>
#include <osgViewer/Viewer>
#include <osg/PositionAttitudeTransform>

using namespace osg;
using namespace Manipulators;

//////////////////////////////////////////////////////////////////////////

ViroHud::ViroHud(){
	_currentLoad = 0;
	_vp_height = 700; //768;
	_vp_width  = 1400; //1024;
}

ViroHud::ViroHud( osgViewer::Viewer* v ){
	ViroHud();
	_viewer = v;
}
ViroHud::ViroHud( ViroManipulator* vm ){
	ViroHud();
	this->setViroManipulator( vm );
}

void ViroHud::CreateLoadingBar(){
	if (_LoadingBarGeom.get()) return;

	_LoadingBarGeom = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> Lvertex = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> Lcolor  = new osg::Vec4Array;

	float d = -0.1f;
	float barsize = 8.0f;
	//LoadingBarSize = new osg::Vec3f(_vp_width*0.4,40.0, 0.0);

	Lvertex->push_back(osg::Vec3(0.0,_vp_height-barsize, d));
	Lvertex->push_back(osg::Vec3(0.0,_vp_height, d));
	Lvertex->push_back(osg::Vec3(_vp_width*0.4,_vp_height, d));
	Lvertex->push_back(osg::Vec3(_vp_width*0.4,_vp_height-barsize, d));

	Lcolor->push_back(Vec4(VIROHUD_SIDECOLOR));//Vec4(0,0,0, 0.1));
	Lcolor->push_back(Vec4(VIROHUD_SIDECOLOR));//Vec4(0,0,0, 0.4));
	Lcolor->push_back(Vec4(1,0,0, 1.0));
	Lcolor->push_back(Vec4(1,0,0, 0.1));

	_LoadingBarGeom->setVertexArray( Lvertex.get() );
	_LoadingBarGeom->setColorArray( Lcolor.get() );
	_LoadingBarGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	_LoadingBarGeom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
}

void ViroHud::UpdateLoadingBar(){
	osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>( _LoadingBarGeom->getVertexArray() );
	if (verts && _viewer.get()){
		_currentLoad = _viewer->getDatabasePager()->getFileRequestListSize();

		float S = _vp_width * 0.001;
		(*verts)[2][0] = _currentLoad * S;
		(*verts)[3][0] = _currentLoad * S;

		_LoadingBarGeom->getVertexArray()->dirty();
		_LoadingBarGeom->dirtyDisplayList();
		}
}

void ViroHud::Realize(){
	if ( _HUD.get() ) return;

	osg::ref_ptr<Geode> geodeHUD = new osg::Geode();
	osg::ref_ptr<StateSet> hudSS = new StateSet;
	osg::ref_ptr<osgText::Font> hudFont = osgText::readFontFile("arial.ttf");
	_HUDlabel = new osgText::Text;

	osg::Vec3 position(5.0f,5.0f,0.0f);

	// hud ss
	hudSS->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
	hudSS->setMode(GL_FOG,osg::StateAttribute::OFF);
	hudSS->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);

	geodeHUD->setStateSet( hudSS.get() );

	// hud label
	_HUDlabel->setPosition( position );
	_HUDlabel->setFont( hudFont.get() );
	_HUDlabel->setColor( Vec4f(0.0,0.0,0.0, 1.0) );
	_HUDlabel->setText("ViRo HUD");
	_HUDlabel->setFontResolution(15,15);
	_HUDlabel->setCharacterSize(15, 1.1);
	//_HUDlabel->setBackdropType(osgText::Text::BackdropType::OUTLINE);
	//_HUDlabel->setBackdropColor( Vec4f(1.0,1.0,1.0, 0.7) );

	geodeHUD->addDrawable( _HUDlabel.get() );
	geodeHUD->setName("NOPICK");

	_HUD = new osg::Projection(osg::Matrix::ortho2D(0,_vp_width, 0,_vp_height));
	_HUD->setCullingActive(false);
	
	osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform(osg::Matrix::identity());
	xform->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	_HUD->addChild(xform.get());

	osg::StateSet *ss = xform->getOrCreateStateSet();
	ss->setRenderBinDetails(100, "RenderBin");
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	ss->setMode(GL_FOG,osg::StateAttribute::OFF);

	osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc;
	ss->setAttributeAndModes(bf.get());

	// HUD bkg
	Vec4f bkColor( VIROHUD_HUDCOLOR );
	float x0,y0,x1,y1, d;
	x0 = 0.0;
	y0 = 0.0;
	x1 = _vp_width;
	y1 = 20.0;
	d = -0.1f;

	osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
	osg::ref_ptr<osg::Geometry> gsmooth = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> vx = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> vxsmooth = new osg::Vec3Array;

	vx->push_back(osg::Vec3(x0,y0, d));
	vx->push_back(osg::Vec3(x1,y0, d));
	vx->push_back(osg::Vec3(x1,y1, d));
	vx->push_back(osg::Vec3(x0,y1, d));

	//vxsmooth->push_back(osg::Vec3(0.0,0.0, d));
	//vxsmooth->push_back(osg::Vec3(20.0,0.0, d));
	//vxsmooth->push_back(osg::Vec3(20.0,_vp_width, d));
	//vxsmooth->push_back(osg::Vec3(0.0,_vp_width, d));

	vxsmooth->push_back(osg::Vec3(0.0,0.0, d));
	vxsmooth->push_back(osg::Vec3(_vp_width,0.0, d));
	vxsmooth->push_back(osg::Vec3(_vp_width,40.0, d));
	vxsmooth->push_back(osg::Vec3(0.0,40.0, d));

	geo->setVertexArray(vx.get());
	gsmooth->setVertexArray(vxsmooth.get());

	osg::ref_ptr<osg::Vec4Array> clr = new osg::Vec4Array;
	clr->push_back(bkColor);
	clr->push_back(bkColor);
	clr->push_back(bkColor + Vec4(0,0,1, 0.5)); //- Vec4(0,0,0, 0.6)); //+ Vec4(0,0,1, 0.5));
	clr->push_back(bkColor + Vec4(0,0,1, 0.5)); //- Vec4(0,0,0, 0.6)); //+ Vec4(0,0,1, 0.5));

	osg::ref_ptr<osg::Vec4Array> clrsmooth = new osg::Vec4Array;
	Vec4f sidecol( VIROHUD_SIDECOLOR );
	//clrsmooth->push_back(sidecol);
	//clrsmooth->push_back(sidecol - Vec4(0,0,0, 1));
	//clrsmooth->push_back(sidecol - Vec4(0,0,0, 1));
	//clrsmooth->push_back(sidecol);

	clrsmooth->push_back(sidecol);
	clrsmooth->push_back(sidecol);
	clrsmooth->push_back(sidecol - Vec4(0,0,0, 1));
	clrsmooth->push_back(sidecol - Vec4(0,0,0, 1));

	geo->setColorArray(clr.get());
	gsmooth->setColorArray(clrsmooth.get());

	geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	geo->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	gsmooth->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	gsmooth->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	geodeHUD->addDrawable( geo.get() );
	geodeHUD->addDrawable( gsmooth.get() );

	xform->addChild( geodeHUD.get() );
	_CompassPAT = new osg::PositionAttitudeTransform;
	xform->addChild( _CompassPAT.get() );

	/*
	osg::ref_ptr<osg::Node> loadedCompass = osgDB::readNodeFile( VIROHUD_COMPASSNODE );
	if ( loadedCompass.get() ){
		//_CompassPAT->setPivotPoint( Vec3(_vp_width-50,40,d) );
		_CompassPAT->setPosition( Vec3(_vp_width-50,40,d) );
		_CompassPAT->addChild( loadedCompass.get() );
		}
	*/

//	else {
		CreateCompass(_vp_width-50,40, 70);
		_CompassPAT->addChild( _Compass.get() );
//		}

	CreateLoadingBar();
	geodeHUD->addDrawable( _LoadingBarGeom.get() );
}

void ViroHud::CreateCompass(float px,float py, float size){
	if ( _Compass.get() ) return;

	_Compass = new osg::Geode();

	float z = -0.1f;
	float halfsize = size*0.5f;
	float xsize = halfsize*0.7f;
	float quartersize = size*0.25f;
	float shortsize = size*0.15f;
	float octavesize = quartersize*0.5f;

	osg::ref_ptr<osg::Geometry> NS = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> NSvertex = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> NScolor = new osg::Vec4Array;

	osg::ref_ptr<osg::Geometry> WE = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> WEvertex = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> WEcolor = new osg::Vec4Array;

	osg::ref_ptr<osg::Geometry> BK = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> BKvertex = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> BKcolor = new osg::Vec4Array;

	NSvertex->push_back(osg::Vec3(0.0,-halfsize, z));
	NSvertex->push_back(osg::Vec3(shortsize,0.0, z));
	NSvertex->push_back(osg::Vec3(0.0,halfsize, z));
	NSvertex->push_back(osg::Vec3(-shortsize,0.0, z));

	NScolor->push_back(Vec4(0,0,0, 1.0));
	NScolor->push_back(Vec4(0,0,0, 0.5));
	NScolor->push_back(Vec4(VIROHUD_COMPASSNORTHCOLOR));
	NScolor->push_back(Vec4(0,0,0, 0.5));

	NS->setVertexArray( NSvertex.get() );
	NS->setColorArray( NScolor.get() );
	NS->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	NS->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	WEvertex->push_back(osg::Vec3(-halfsize,0.0, z));
	WEvertex->push_back(osg::Vec3(0.0,-shortsize, z));
	WEvertex->push_back(osg::Vec3(halfsize,0.0, z));
	WEvertex->push_back(osg::Vec3(0.0,shortsize, z));

	WEcolor->push_back(Vec4(0,0,0, 1.0));
	WEcolor->push_back(Vec4(0,0,0, 0.5));
	WEcolor->push_back(Vec4(0,0,0, 1.0));
	WEcolor->push_back(Vec4(VIROHUD_COMPASSNORTHCOLOR));

	WE->setVertexArray( WEvertex.get() );
	WE->setColorArray( WEcolor.get() );
	WE->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	WE->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	BKvertex->push_back(osg::Vec3(0.0,-octavesize, z));
	BKvertex->push_back(osg::Vec3(octavesize,0.0, z));
	BKvertex->push_back(osg::Vec3(0.0,octavesize, z));
	BKvertex->push_back(osg::Vec3(-octavesize,0.0, z));

	BKcolor->push_back(Vec4(VIROHUD_HUDCOLOR));
	BKcolor->push_back(Vec4(VIROHUD_HUDCOLOR));
	BKcolor->push_back(Vec4(VIROHUD_HUDCOLOR));
	BKcolor->push_back(Vec4(VIROHUD_HUDCOLOR));

	BK->setVertexArray( BKvertex.get() );
	BK->setColorArray( BKcolor.get() );
	BK->setColorBinding(osg::Geometry::BIND_OVERALL);
	BK->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	// North Label
	osg::ref_ptr<osgText::Font> northFont = osgText::readFontFile("arial.ttf");
	osg::ref_ptr<osgText::Text> northLabel = new osgText::Text;
	northLabel->setPosition( Vec3(0,halfsize,z) );
	northLabel->setAlignment(osgText::Text::CENTER_CENTER);
	northLabel->setFont( northFont.get() );
	northLabel->setColor( Vec4f(0.0,0.0,0.0, 1.0) );
	northLabel->setText("- N -");
	northLabel->setFontResolution(20,20);
	northLabel->setCharacterSize(20, 1.1);
	northLabel->setBackdropType(osgText::Text::OUTLINE);
	northLabel->setBackdropColor( Vec4(VIROHUD_HUDCOLOR) );


	_Compass->addDrawable( WE.get() );
	_Compass->addDrawable( NS.get() );
	_Compass->addDrawable( BK.get() );
	_Compass->addDrawable( northLabel.get() );

	_Compass->setName("NOPICK");

	// Use Image
	/*
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
    texture->setImage(osgDB::readImageFile(filename));
    
    osg::StateSet* stateset = geom->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
*/

	_CompassPAT->setPosition( Vec3(px,py,z) );

}


void ViroHud::Update(){
	if ( !_HUD.get() || !_vm.get()) return;

/*
	if (_viewer.get() && _viewer.get()->getCamera() && _viewer.get()->getCamera()->getViewport()){
		_vp_height = _viewer->getCamera()->getViewport()->height();
		_vp_width  = _viewer->getCamera()->getViewport()->width();
		}
*/
		
		std::ostringstream HUDstring;
		
		/*
		sprintf(HUDstring,"Position:( %.2f, %.2f )  Altitude: %.2f  Speed: %.1f  Pivot:( %.2f, %.2f, %.2f)",
			_vm->getPosition().x(),
			_vm->getPosition().y(),
			_vm->getPosition().z(),
			_vm->getSpeed(),
			_vm->getLastPickedPoint().x(),
			_vm->getLastPickedPoint().y(),
			_vm->getLastPickedPoint().z()
			);
		
		sprintf(_HUDstring,"Altitude: %.2f - Speed: %.1f",
			_vm->getPosition().z(),
			_vm->getSpeed(),
			);
*/
		//if ( _vm->isEnabled(_vm->GRAVITY) ) sprintf(_HUDstring );
		

		//_currentLoad = osgDB::Registry::instance()->getOrCreateDatabasePager()->getAverageTimeToMergeTiles();

		//HUDstring << _serverPrefix;
		//HUDstring << _viewer->getDatabasePager()->getAverageTimeToMergeTiles();
		//HUDstring << std::string(" ::: ");
		UpdateLoadingBar();

		if ( _vm->getHoldLock() ){
			if (_vm->getTumble()) HUDstring << std::string("<TUMBLE> | ");
			else                  HUDstring << std::string("<PAN> | ");
			}
		if ( _vm->isEnabled(_vm->Z_LOCK) )		HUDstring << std::string("HEIGHT LOCK | ");
		if ( _vm->isEnabled(_vm->GRAVITY) )		HUDstring << std::string("WALK | ");
		if ( _vm->isEnabled(_vm->COLLISIONS) )	HUDstring << std::string("COLLISIONS | ");
		HUDstring << "Altitude: " << _vm->getPosition().z();
		if ( _vm->getSpeed() != 0.0) HUDstring << " Speed: " << _vm->getSpeed();
		
		//HUDstring << std::string(" POS: ") << _vm->getPosition().x() << std::string(" - ") << _vm->getPosition().y();

		if ( _vm->getSoftImpact() ) HUDstring << std::string(" - Impact!!");

		_HUDlabel->setText( HUDstring.str() );

		// Update Compass Attitude
		osg::Quat R;
		osg::Vec3d L = _vm->getLookVec();
		L.normalize();
		L[2] = 0.0;
		R.makeRotate( L, Vec3d(0,1,0) );
		_CompassPAT->setAttitude( R );

		//double s = _currentLoad * 0.01; //_vm->getSpeed() * 0.01;
		//_CompassPAT->setScale( Vec3d(1+s,1+s,1) );
}