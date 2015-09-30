#include "StdAfx.h"
#include "MinimalOgre.h"
#include "NpcModelManager.h"
#include "AvatarModelManager.h"

//-------------------------------------------------------------------------------------
MinimalOgre::MinimalOgre(void)
	: mRoot(0),
	mCamera(0),
	mSceneMgr(0),
	mWindow(0),
	mResourcesCfg(Ogre::StringUtil::BLANK),
	mPluginsCfg(Ogre::StringUtil::BLANK),
	mTrayMgr(0),
	mCameraMan(0),
	mDetailsPanel(0),
	mCursorWasVisible(false),
	mShutDown(false),
	mInputManager(0),
	mMouse(0),
	mKeyboard(0)
{
}
AnimationState *pAS = nullptr;

//-------------------------------------------------------------------------------------
MinimalOgre::~MinimalOgre(void)
{
	if (mTrayMgr) delete mTrayMgr;
	if (mCameraMan) delete mCameraMan;

	//Remove ourself as a Window listener
	Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
	delete mRoot;
}

bool MinimalOgre::go(void)
{
#ifdef _DEBUG
	mResourcesCfg = "resources_d.cfg";
	mPluginsCfg = "plugins_d.cfg";
#else
	mResourcesCfg = "resources.cfg";
	mPluginsCfg = "plugins.cfg";
#endif

	// construct Ogre::Root
	mRoot = new Ogre::Root(mPluginsCfg);

	//-------------------------------------------------------------------------------------
	// setup resources
	// Load resource paths from config file
	Ogre::ConfigFile cf;
	cf.load(mResourcesCfg);

	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}
	//-------------------------------------------------------------------------------------
	// configure
	// Show the configuration dialog and initialise the system
	// You can skip this and use root.restoreConfig() to load configuration
	// settings if you were sure there are valid ones saved in ogre.cfg
	if(mRoot->restoreConfig() || mRoot->showConfigDialog())
	{
		// If returned true, user clicked OK so initialise
		// Here we choose to let the system create a default rendering window by passing 'true'
		mWindow = mRoot->initialise(true, "Project Eos");
	}
	else
	{
		return false;
	}
	//-------------------------------------------------------------------------------------
	// choose scenemanager
	// Get the SceneManager, in this case a generic one
	mSceneMgr = mRoot->createSceneManager("OctreeSceneManager");
	//-------------------------------------------------------------------------------------
	// create camera
	// Create the camera
	mCamera = mSceneMgr->createCamera("PlayerCam");
	mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);

	Ogre::SceneNode *camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	camNode->pitch(Degree(90));
	camNode->attachObject( mCamera );



	// Position it at 500 in Z direction
	mCamera->setPosition(Ogre::Vector3(563080, 537.219f, -506411));
	// Look back along -Z
	mCamera->lookAt(Ogre::Vector3(563450, 507418, 30.2188f));
	mCamera->setNearClipDistance(20);

	mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
	mCameraMan->setTopSpeed( mCameraMan->getTopSpeed() * 50.0f );

	//-------------------------------------------------------------------------------------
	// create viewports
	// Create one viewport, entire window
	Ogre::Viewport* vp = mWindow->addViewport(mCamera);
	vp->setBackgroundColour(Ogre::ColourValue(0.3f,0.0f,0.0f));

	// Alter the camera aspect ratio to match the viewport
	mCamera->setAspectRatio(
		Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
	//-------------------------------------------------------------------------------------
	// Set default mipmap level (NB some APIs ignore this)
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	//-------------------------------------------------------------------------------------
	// Create any resource listeners (for loading screens)
	//createResourceListener();
	//-------------------------------------------------------------------------------------

	Ogre::ResourceGroupManager::getSingleton().declareResource( "BODY01.ZMS", "Mesh", "General", &mRMeshLoader );
	Ogre::ResourceGroupManager::getSingleton().declareResource( "BODY02.ZMS", "Mesh", "General", &mRMeshLoader );
	Ogre::ResourceGroupManager::getSingleton().declareResource( "HEAD01.ZMS", "Mesh", "General", &mRMeshLoader );
	Ogre::ResourceGroupManager::getSingleton().declareResource( "JELLYBEAN2_BONE.ZMD", "Skeleton", "General", &mRSkelLoader );
	Ogre::ResourceGroupManager::getSingleton().declareResource( "CHIEF_BONE.ZMD", "Skeleton", "General", &mRSkelLoader );

	mNpcMgr.load( );

	// load resources
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	//-------------------------------------------------------------------------------------
	// Create the scene
	Ogre::SceneNode *pFlipNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	//pFlipNode->rotate( Ogre::Vector3(1,0,0), Ogre::Radian(Ogre::Degree(-90.0f)) );

	//RNpcMgr::getSingleton().createModel( 1001, mSceneMgr, pFlipNode );


	AvatarModelManager* avtMgr = new AvatarModelManager( );
	avtMgr->prepare( );
	avtMgr->load( );

	AvatarModel *avtMdl = avtMgr->create( mSceneMgr );
	SceneNode *npcnode = avtMdl->getSceneNode();
	npcnode->setPosition( 563432.0f, 507362.0f, 12.0f );
	npcnode->setScale( 1.2f, 1.2f, 1.2f );
	pFlipNode->addChild( npcnode );


	/*
	NpcModelManager* npcMgr = new NpcModelManager( );
	npcMgr->prepare( );
	npcMgr->load( );

	NpcModel *npcMdl = npcMgr->create( mSceneMgr, 1001 );
	SceneNode *npcnode = npcMdl->getSceneNode();
	npcnode->setPosition( 563432.0f, 507362.0f, 12.0f );
	npcnode->setScale( 1.2f, 1.2f, 1.2f );
	pFlipNode->addChild( npcnode );
	*/

	/*
	RMap *pMap = new RMap( 2 );
	RPage *pPage = new RPage( pMap, 35, 30 );

	TerLoader *pLoader = new TerLoader( pPage, 0, 0 );
	*/

	///*
	RMap *pMap = new RMap( 2 );
	pMap->load( );
	pMap->declare( );

	Ogre::SceneNode *mapNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

	//*
	int minPageX = 35;
	int minPageY = 33;
	int maxPageX = 35;
	int maxPageY = 33;
	//*/
	/*
	int minPageX = 30;
	int minPageY = 30;
	int maxPageX = 37;
	int maxPageY = 35;
	//*/

	for( int ix = minPageX; ix <= maxPageX; ++ix ) {
		for( int iy = minPageY; iy <= maxPageY; ++iy ) {
			RMap::Page *pPage = pMap->getPage( ix, iy );
			pPage->load( );
			pPage->create( mSceneMgr, mapNode );
		}
	}
	//*/


	RZscFixed zsc;

	Ogre::DataStreamPtr fs = Ogre::ResourceGroupManager::getSingleton().openResource("3DDATA\\JUNON\\LIST_CNST_JPT.ZSC");
	zsc.read( *fs );


	/*
	Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "HEAD01.ZMS");
	Ogre::SceneNode* headNode = pFlipNode->createChildSceneNode();
	headNode->attachObject(ogreHead);

	Ogre::Entity* ogreHead2 = mSceneMgr->createEntity("Body", "BODY01.ZMS");
	Ogre::SceneNode* headNode2 = pFlipNode->createChildSceneNode();
	headNode2->attachObject(ogreHead2);

	ogreHead2->shareSkeletonInstanceWith( ogreHead );
	ogreHead->setDisplaySkeleton( true );
	///*
	pAS = ogreHead->getAnimationState("Run");
	pAS->setLoop( true );
	pAS->setEnabled( true );
	//*/

	mCameraMan->setStyle(OgreBites::CS_MANUAL);

	// Set ambient light

	/*
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);
	mSceneMgr->setShadowTextureSelfShadow( true );
	mSceneMgr->setShadowTextureSize( 2048 );
	*/

	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.86f , 0.83f , 0.8f));

	Ogre::Light* directionalLight = mSceneMgr->createLight("directionalLight");
	directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
	directionalLight->setDiffuseColour(Ogre::ColourValue(1.0f,1.0f,1.0f));
	directionalLight->setSpecularColour(Ogre::ColourValue(0,0,0));

	directionalLight->setDirection(Ogre::Vector3( 0.5f, 0.5f, 0.5f ));



	/*
	Ogre::PageManager *pPager = new Ogre::PageManager( );
	Ogre::PagedWorld *pWorld = pPager->createWorld( "MAP_1" );

	Ogre::Grid2DPageStrategy *pPageS = new Ogre::Grid2DPageStrategy( pPager );
	

	Ogre::PagedWorldSection *pSec1 = pWorld->createSection( pPageS, mSceneMgr );

	Ogre::Grid2DPageStrategyData *pSData = (Ogre::Grid2DPageStrategyData*)pSec1->getStrategyData();
	pSData->setCellRange( 0, 0, 65, 65 );
	pSData->setMode( G2D_X_Y );
	pSData->setLoadRadius( 1000.0f );
	pSData->setHoldRadius( 1500.0f );
	pSData->setCellSize( 1000.0f );
	*/


	//-------------------------------------------------------------------------------------
	//create FrameListener
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
	pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));

	mInputManager = OIS::InputManager::createInputSystem( pl );

	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);

	//Set initial mouse clipping size
	windowResized(mWindow);

	//Register as a Window listener
	Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

	mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
	mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
	mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
	mTrayMgr->hideCursor();

	// create a params panel for displaying sample details
	Ogre::StringVector items;
	items.push_back("cam.pX");
	items.push_back("cam.pY");
	items.push_back("cam.pZ");
	items.push_back("");
	items.push_back("cam.oW");
	items.push_back("cam.oX");
	items.push_back("cam.oY");
	items.push_back("cam.oZ");
	items.push_back("");
	items.push_back("Filtering");
	items.push_back("Poly Mode");

	mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
	mDetailsPanel->setParamValue(9, "Bilinear");
	mDetailsPanel->setParamValue(10, "Solid");
	//mDetailsPanel->hide();
	mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);

	mRoot->addFrameListener(this);
	//-------------------------------------------------------------------------------------
	mRoot->startRendering();

	return true;
}

bool MinimalOgre::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	if(mWindow->isClosed())
		return false;

	if(mShutDown)
		return false;

	//Need to capture/update each device
	mKeyboard->capture();
	mMouse->capture();

	if( pAS ) pAS->addTime( evt.timeSinceLastFrame );

	mTrayMgr->frameRenderingQueued(evt);

	if (!mTrayMgr->isDialogVisible())
	{
		mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
		if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
		{
			mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
			mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
			mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
			mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
			mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
			mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
			mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
		}
	}

	return true;
}
//-------------------------------------------------------------------------------------
bool MinimalOgre::keyPressed( const OIS::KeyEvent &arg )
{
	if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

	if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
	{
		mTrayMgr->toggleAdvancedFrameStats();
	}
	else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
	{
		if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
		{
			mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
			mDetailsPanel->show();
		}
		else
		{
			mTrayMgr->removeWidgetFromTray(mDetailsPanel);
			mDetailsPanel->hide();
		}
	}
	else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
	{
		Ogre::String newVal;
		Ogre::TextureFilterOptions tfo;
		unsigned int aniso;

		switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
		{
		case 'B':
			newVal = "Trilinear";
			tfo = Ogre::TFO_TRILINEAR;
			aniso = 1;
			break;
		case 'T':
			newVal = "Anisotropic";
			tfo = Ogre::TFO_ANISOTROPIC;
			aniso = 8;
			break;
		case 'A':
			newVal = "None";
			tfo = Ogre::TFO_NONE;
			aniso = 1;
			break;
		default:
			newVal = "Bilinear";
			tfo = Ogre::TFO_BILINEAR;
			aniso = 1;
		}

		Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
		Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
		mDetailsPanel->setParamValue(9, newVal);
	}
	else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
	{
		Ogre::String newVal;
		Ogre::PolygonMode pm;

		switch (mCamera->getPolygonMode())
		{
		case Ogre::PM_SOLID:
			newVal = "Wireframe";
			pm = Ogre::PM_WIREFRAME;
			break;
		case Ogre::PM_WIREFRAME:
			newVal = "Points";
			pm = Ogre::PM_POINTS;
			break;
		default:
			newVal = "Solid";
			pm = Ogre::PM_SOLID;
		}

		mCamera->setPolygonMode(pm);
		mDetailsPanel->setParamValue(10, newVal);
	}
	else if(arg.key == OIS::KC_F5)   // refresh all textures
	{
		Ogre::TextureManager::getSingleton().reloadAll();
	}
	else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
	{
		mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
	}
	else if (arg.key == OIS::KC_ESCAPE)
	{
		mShutDown = true;
	}

	mCameraMan->injectKeyDown(arg);
	return true;
}

bool MinimalOgre::keyReleased( const OIS::KeyEvent &arg )
{
	mCameraMan->injectKeyUp(arg);
	return true;
}

bool MinimalOgre::mouseMoved( const OIS::MouseEvent &arg )
{
	if (mTrayMgr->injectMouseMove(arg)) return true;
	mCameraMan->injectMouseMove(arg);
	return true;
}

bool MinimalOgre::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseDown(arg, id)) return true;

	if( id == OIS::MB_Left) {
		mCameraMan->setStyle(OgreBites::CS_FREELOOK);
	}

	mCameraMan->injectMouseDown(arg, id);
	return true;
}

bool MinimalOgre::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseUp(arg, id)) return true;

	if( id == OIS::MB_Left) {
		mCameraMan->setStyle(OgreBites::CS_MANUAL);
	}

	mCameraMan->injectMouseUp(arg, id);
	return true;
}

//Adjust mouse clipping area
void MinimalOgre::windowResized(Ogre::RenderWindow* rw)
{
	unsigned int width, height, depth;
	int left, top;
	rw->getMetrics(width, height, depth, left, top);

	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void MinimalOgre::windowClosed(Ogre::RenderWindow* rw)
{
	//Only close for window that created OIS (the main window in these demos)
	if( rw == mWindow )
	{
		if( mInputManager )
		{
			mInputManager->destroyInputObject( mMouse );
			mInputManager->destroyInputObject( mKeyboard );

			OIS::InputManager::destroyInputSystem(mInputManager);
			mInputManager = 0;
		}
	}
}