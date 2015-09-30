#ifndef __RSkelLoader_h_
#define __RSkelLoader_h_

#include "RFS.h"

using namespace Ogre;

#define ZZ_CTYPE_NONE        (1 << 0)
#define ZZ_CTYPE_POSITION    (1 << 1)
#define ZZ_CTYPE_ROTATION    (1 << 2)
#define ZZ_CTYPE_NORMAL      (1 << 3)
#define ZZ_CTYPE_ALPHA       (1 << 4)
#define ZZ_CTYPE_UV0         (1 << 5)
#define ZZ_CTYPE_UV1         (1 << 6)
#define ZZ_CTYPE_UV2         (1 << 7)
#define ZZ_CTYPE_UV3         (1 << 8)
#define ZZ_CTYPE_TEXTUREANIM (1 << 9)
#define ZZ_CTYPE_SCALE       (1 << 10)


class RSkelLoader : public Ogre::ManualResourceLoader
{
public:
	virtual void prepareResource(Ogre::Resource* resource) override
	{

	}

	static String getBoneName( int boneIdx )
	{
		std::stringstream stream;
		stream << "BONE_" << boneIdx;
		return stream.str();
	}
	
	static String getDummyName( int boneIdx )
	{
		std::stringstream stream;
		stream << "DUMMY_" << boneIdx;
		return stream.str();
	}

	static void readSkel( Skeleton* skel, DataStream *stream )
	{
		Ogre::DataStream& xMDS = *stream;

		xMDS.skip( 7 );

		uint num_bones;
		xMDS.read( &num_bones, sizeof(uint) );

		for( uint i = 0; i < num_bones; ++i )
		{
			uint parent_id;
			xMDS.read( &parent_id, sizeof(uint) );

			String bone_name = RFS::readString( xMDS );

			Vector3 pos;
			xMDS.read( &pos, sizeof(Vector3) );

			Quaternion rot;
			xMDS.read( &rot, sizeof(Quaternion) );

			Bone* pBone = skel->createBone( getBoneName(i), i );
			pBone->setPosition( pos );
			pBone->setOrientation( rot );

			if( parent_id != i ) {
				Bone *pParent = skel->getBone( parent_id );
				pParent->addChild( pBone );
			}

			pBone->setBindingPose();
			pBone->setInitialState();
		}

		uint num_dummies;
		xMDS.read( &num_dummies, sizeof(uint) );

		for( uint i = 0; i < num_dummies; ++i )
		{
			String dummy_name = RFS::readString( xMDS );

			uint parent_id;
			xMDS.read( &parent_id, sizeof(uint) );

			Vector3 pos;
			xMDS.read( &pos, sizeof(Vector3) );

			Quaternion rot;
			xMDS.read( &rot, sizeof(Quaternion) );

			Bone* pBone = skel->createBone( getDummyName(i), num_bones + i );
			pBone->setPosition( pos );
			pBone->setOrientation( rot );

			if( parent_id != i ) {
				Bone *pParent = skel->getBone( parent_id );
				pParent->addChild( pBone );
			}

			pBone->setBindingPose();
			pBone->setInitialState();
		}
	}

	static void readAnim( Skeleton* skel, String animName, DataStream *stream )
	{
		DataStream& xMS = *stream;

		String motionVer = RFS::readString( xMS );

		uint fps, frames, channels;
		xMS.read( &fps, sizeof(uint) );
		xMS.read( &frames, sizeof(uint) );
		xMS.read( &channels, sizeof(uint) );

		float ftime = (float)frames/(float)fps;
		float frame_time = 1.0f / (float)fps;

		Animation* pAnim = skel->createAnimation( animName, ftime );

		std::map<uint,NodeAnimationTrack*> tdata;

		typedef std::pair<uint,uint> TrackPair;
		std::vector<TrackPair> tracks;
		tracks.resize( channels );

		for( uint i = 0; i < channels; ++i ) {
			TrackPair x;
			xMS.read( &x.first, sizeof(uint) );
			xMS.read( &x.second, sizeof(uint) );
			tracks[i] = x;

			if( tdata[ tracks[i].second ] == nullptr ) {
				NodeAnimationTrack* track = pAnim->createNodeTrack( x.second, skel->getBone(x.second) );
				tdata[ tracks[i].second ] = track;

				for( uint t = 0; t < frames; ++t ) {
					TransformKeyFrame* pKey = track->createNodeKeyFrame( frame_time * t );
				}
			}
		}

		for( uint t = 0; t < frames; ++t ) {
			for( uint i = 0; i < channels; ++i ) {
				const TrackPair& x = tracks[i];
				NodeAnimationTrack* track = tdata[ x.second ];
				TransformKeyFrame *pKey = track->getNodeKeyFrame( t );
				Bone *pBone = skel->getBone( x.second );

				if( x.first == ZZ_CTYPE_POSITION ) {
					Vector3 pos;
					xMS.read( &pos, sizeof(Vector3) );
					pos -= pBone->getInitialPosition( );
					pKey->setTranslate( pos );
				} else if( x.first == ZZ_CTYPE_ROTATION ) {
					Quaternion rot;
					xMS.read( &rot, sizeof(Quaternion) );
					rot = pBone->getInitialOrientation( ).Inverse( ) * rot;
					pKey->setRotation( rot );
				} else {
					__asm{ int 3 };
				}
			}			
		}

	}

	virtual void loadResource(Ogre::Resource* resource) override
	{
		Skeleton* pSkel = (Skeleton*)resource;

		Ogre::MemoryDataStream xMS(Ogre::ResourceGroupManager::getSingleton().openResource( resource->getName() ));
		readSkel( pSkel, &xMS );

		Ogre::MemoryDataStream xMS2(Ogre::ResourceGroupManager::getSingleton().openResource( "3DDATA\\MOTION\\AVATAR\\EMPTY_STOP1_M1.ZMO" ));
		readAnim( pSkel, "Run", &xMS2 );
	}
};

#endif // __RSkelLoader_h_