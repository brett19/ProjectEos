#ifndef __RChr_h_
#define __RChr_h_

#include "RFS.h"

using namespace Ogre;

namespace ROSE
{
	namespace Chr
	{
		class _Skeleton
		{
		public:
			_Skeleton( )
			{
			}

			_Skeleton( DataStream &stream )
			{
				read( stream );
			}

			void read( DataStream &stream )
			{
				mSkelPath = RFS::readString( stream );
			}

			String getSkelPath( ) const { return mSkelPath; }

		protected:
			String mSkelPath;

		};

		class _Motion
		{
		public:
			_Motion( )
			{
			}

			_Motion( DataStream &stream )
			{
				read( stream );
			}

			void read( DataStream &stream )
			{
				mMotionPath = RFS::readString( stream );
			}

			String getMotionPath( ) const { return mMotionPath; }

		protected:
			String mMotionPath;

		};

		class _Effect
		{
		public:
			_Effect( )
			{
			}

			_Effect( DataStream &stream )
			{
				read( stream );
			}

			void read( DataStream &stream )
			{
				mEffectPath = RFS::readString( stream );
			}

			String getEffectPath( ) const { return mEffectPath; }

		protected:
			String mEffectPath;

		};

		class _Part
		{
		public:
			_Part( )
			{
			}

			_Part( DataStream &stream )
			{
				read( stream );
			}

			void read( DataStream &stream )
			{
				stream.read( &mMeshId, sizeof(ushort) );
			}

			ushort getMeshId( ) const { return mMeshId; }

		protected:
			ushort mMeshId;

		};

		class _Animation
		{
		public:
			_Animation( )
			{
			}

			_Animation( DataStream &stream )
			{
				read( stream );
			}

			void read( DataStream &stream )
			{
				stream.read( &mAnimType, sizeof(ushort) );
				stream.read( &mMotionId, sizeof(ushort) );
			}

			ushort getAnimType( ) const { return mAnimType; }
			ushort getMotionId( ) const { return mMotionId; }
			
		protected:
			ushort mAnimType;
			ushort mMotionId;

		};

		class _BoneEffect
		{
		public:
			_BoneEffect( )
			{
			}

			_BoneEffect( DataStream &stream )
			{
				read( stream );
			}

			void read( DataStream &stream )
			{
				stream.read( &mBoneIdx, sizeof(ushort) );
				stream.read( &mEffectId, sizeof(ushort) );
			}

			ushort getBoneIdx( ) const { return mBoneIdx; }
			ushort getEffectId( ) const { return mEffectId; }

		protected:
			ushort mBoneIdx;
			ushort mEffectId;

		};

		class _Character
		{
		public:
			typedef _Part Part;
			typedef _Animation Animation;
			typedef _BoneEffect BoneEffect;

			typedef std::vector<Part> PartList;
			typedef std::vector<Animation> AnimationList;
			typedef std::vector<BoneEffect> BoneEffectList;

			_Character( )
			{
			}

			_Character( DataStream &stream )
			{
				read( stream );
			}

			void read( DataStream &stream )
			{
				uchar is_valid;
				stream.read( &is_valid, sizeof(uchar) );
				if( is_valid == 0 ) { 
					mEnabled = false;
					return;
				} else {
					mEnabled = true;
				}

				stream.read( &mSkelId, sizeof(ushort) );

				mName = RFS::readString( stream );

				ushort num_parts;
				stream.read( &num_parts, sizeof(ushort) );
				mParts.reserve( num_parts );
				for( ushort i = 0; i < num_parts; ++i ) {
					mParts.push_back(Part( stream ));
				}

				ushort num_anims;
				stream.read( &num_anims, sizeof(ushort) );
				mAnimations.reserve( num_anims );
				for( ushort i = 0; i < num_anims; ++i ) {
					mAnimations.push_back(Animation( stream ));
				}

				ushort num_befts;
				stream.read( &num_befts, sizeof(ushort) );
				mBoneEffects.reserve( num_befts );
				for( ushort i = 0; i < num_befts; ++i ) {
					mBoneEffects.push_back(BoneEffect( stream ));
				}
			}	

			bool isEnabled( ) const { return mEnabled; }
			ushort getSkeletonId( ) const { return mSkelId; }

			int getNumParts( ) const { return mParts.size(); }
			const Part& getPart( int id ) const { return mParts[id]; }
			const PartList& getParts( ) const { return mParts; }

			int getNumAnimations( ) const { return mAnimations.size(); }
			const Animation& getAnimation( int id ) const { return mAnimations[id]; }
			const AnimationList& getAnimations( ) const { return mAnimations; }

			int getNumBoneEffect( ) const { return mBoneEffects.size(); }
			const BoneEffect& getBoneEffect( int id ) const { return mBoneEffects[id]; }
			const BoneEffectList& getBoneEffects( ) const { return mBoneEffects; }

		protected:
			bool mEnabled;
			ushort mSkelId;
			String mName;
			PartList mParts;
			AnimationList mAnimations;
			BoneEffectList mBoneEffects;

		};

		class File
		{
		public:
			typedef _Skeleton Skeleton;
			typedef _Motion Motion;
			typedef _Effect Effect;
			typedef _Character Character;

			typedef std::vector<Skeleton> SkeletonList;
			typedef std::vector<Motion> MotionList;
			typedef std::vector<Effect> EffectList;
			typedef std::vector<Character> CharacterList;

			File( )
			{
			}

			File( DataStream &stream )
			{
				read( stream );
			}

			~File( )
			{
			}

			bool read( DataStream &stream )
			{
				ushort num_skels;
				stream.read( &num_skels, sizeof(ushort) );
				mSkeletons.reserve( num_skels );
				for( ushort i = 0; i < num_skels; ++i ) {
					mSkeletons.push_back(Skeleton( stream ));
				}

				ushort num_motions;
				stream.read( &num_motions, sizeof(ushort) );
				mMotions.reserve( num_motions );
				for( ushort i = 0; i < num_motions; ++i ) {
					mMotions.push_back(Motion( stream ));
				}

				ushort num_efts;
				stream.read( &num_efts, sizeof(ushort) );
				mEffects.reserve( num_efts );
				for( ushort i = 0; i < num_efts; ++i ) {
					mEffects.push_back(Effect( stream ));
				}

				ushort num_chars;
				stream.read( &num_chars, sizeof(ushort) );
				mCharacters.reserve( num_chars );
				for( ushort i = 0; i < num_chars; ++i ) {
					mCharacters.push_back(Character( stream ));
				}

				return true;
			}

			int getNumSkeletons( ) const { return mSkeletons.size(); }
			const Skeleton& getSkeleton( int id ) const { return mSkeletons[id]; }
			const SkeletonList& getSkeletons( ) const { return mSkeletons; }

			int getNumMotions( ) const { return mMotions.size(); }
			const Motion& getMotion( int id ) const { return mMotions[id]; }
			const MotionList& getMotions( ) const { return mMotions; }

			int getNumEffects( ) const { return mEffects.size(); }
			const Effect& getEffect( int id ) const { return mEffects[id]; }
			const EffectList& getEffects( ) const { return mEffects; }

			int getNumCharacters( ) const { return mCharacters.size(); }
			const Character& getCharacter( int id ) const { return mCharacters[id]; }
			const CharacterList& getCharacterList( ) const { return mCharacters; }

		protected:
			SkeletonList mSkeletons;
			MotionList mMotions;
			EffectList mEffects;
			CharacterList mCharacters;

		};
	}
}
typedef ROSE::Chr::File RChr;

#endif // __RChr_h_