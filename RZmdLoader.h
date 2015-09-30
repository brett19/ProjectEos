#pragma once

#include "RSkelLoader.h"

using namespace Ogre;

namespace ROGRE
{
	class ZmdLoader : public ManualResourceLoader
	{
	public:
		typedef std::pair<String,String> Animation;
		typedef std::vector<Animation> AnimationList;

		ZmdLoader( const String& name, const String& skelPath, const AnimationList& anims )
			: mName(name), mSkelPath(skelPath), mAnimations(anims)
		{
		}

		virtual void prepareResource(Ogre::Resource* resource) override
		{

		}

		virtual void loadResource(Ogre::Resource* resource) override
		{
			Skeleton *skel = (Skeleton*)resource;

			Ogre::MemoryDataStream dsSkel(Ogre::ResourceGroupManager::getSingleton().openResource( mSkelPath ));
			RSkelLoader::readSkel( skel, &dsSkel );

			for( auto i = mAnimations.begin(); i != mAnimations.end(); ++i ) {
				Ogre::MemoryDataStream dsMotion(Ogre::ResourceGroupManager::getSingleton().openResource( i->second ));
				RSkelLoader::readAnim( skel, i->first, &dsMotion );
			}
		}

		const String& getName( ) const { return mName; }

	protected:
		String mName;
		String mSkelPath;
		AnimationList mAnimations;

	};
};
typedef ROGRE::ZmdLoader RZmdLoader;