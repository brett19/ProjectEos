#include "StdAfx.h"
#include "MinimalOgre.h"

int main( int argc, char* argv[] )
{
	// Create application object
	MinimalOgre app;

	try {
		app.go();
	} catch( Ogre::Exception& e ) {
		MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}
}