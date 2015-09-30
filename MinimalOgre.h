#ifndef __MinimalOgre_h_
#define __MinimalOgre_h_

#include "RFS.h"
#include "RMeshLoader.h"
#include "RSkelLoader.h"
#include "RTerLoader.h"
#include "RNpcMgr.h"

class MinimalOgre : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
	MinimalOgre(void);
	virtual ~MinimalOgre(void);
	bool go(void);
protected:

	// ROSE
	RNpcMgr	mNpcMgr;
	RMeshLoader mRMeshLoader;
	RSkelLoader mRSkelLoader;

	// OGRE
	Ogre::Root *mRoot;
	Ogre::Camera* mCamera;
	Ogre::SceneManager* mSceneMgr;
	Ogre::RenderWindow* mWindow;
	Ogre::String mResourcesCfg;
	Ogre::String mPluginsCfg;

	// OgreBites
	OgreBites::SdkTrayManager* mTrayMgr;
	OgreBites::SdkCameraMan* mCameraMan;      // basic camera controller
	OgreBites::ParamsPanel* mDetailsPanel;    // sample details panel
	bool mCursorWasVisible;                   // was cursor visible before dialog appeared
	bool mShutDown;

	// OIS Input devices
	OIS::InputManager* mInputManager;
	OIS::Mouse*    mMouse;
	OIS::Keyboard* mKeyboard;

	// Ogre::FrameListener
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	// OIS::KeyListener
	virtual bool keyPressed( const OIS::KeyEvent &arg );
	virtual bool keyReleased( const OIS::KeyEvent &arg );
	// OIS::MouseListener
	virtual bool mouseMoved( const OIS::MouseEvent &arg );
	virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

	// Ogre::WindowEventListener
	virtual void windowResized(Ogre::RenderWindow* rw);
	virtual void windowClosed(Ogre::RenderWindow* rw);
};

#endif // #ifndef __MinimalOgre_h_