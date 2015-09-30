#pragma once

#include "RMeshLoader.h"

using namespace Ogre;

namespace ROGRE
{
	class ZmsLoader : public ManualResourceLoader
	{
	public:
		ZmsLoader( String name, String meshPath, String matName, String skelName = "" )
			: mName(name), mMeshPath(meshPath), mMatName(matName), mSkelName(skelName)
		{
		}

		virtual void prepareResource(Ogre::Resource* resource) override
		{

		}

		virtual void loadResource(Ogre::Resource* resource) override
		{
			Mesh *mesh = (Mesh*)resource;

			mesh->setSkeletonName( mSkelName );

			mesh->_setBoundingSphereRadius(0);
			mesh->_setBounds(AxisAlignedBox(0,0,0,0,0,0));

			Ogre::MemoryDataStream dsMesh(Ogre::ResourceGroupManager::getSingleton().openResource( mMeshPath ));

			SubMesh *pSubmesh = RMeshLoader::readMesh( mesh, &dsMesh );
			pSubmesh->setMaterialName( mMatName );
		}

		const String& getName( ) const { return mName; }

	protected:
		String mName;
		String mMeshPath;
		String mMatName;
		String mSkelName;

	};
};
typedef ROGRE::ZmsLoader RZmsLoader;