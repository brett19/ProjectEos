#ifndef __RZsc_h_
#define __RZsc_h_

#include "RFS.h"

using namespace Ogre;

namespace ROSE
{
	namespace ZSC
	{
		class _Mesh
		{
		public:
			_Mesh( DataStream &stream )
			{
				mMeshPath = RFS::readString( stream );
			}

			String getMeshPath( ) const { return mMeshPath; }

		protected:
			String mMeshPath;

		};

		class _Material
		{
		public:
			enum BlendType : ushort
			{
				BT_NORMAL = 0,
				BT_LIGHTEN = 1,
				BT_CUSTOM = 3,
				BT_NONE = 255
			};

			enum GlowType : ushort
			{
				GT_NONE = 0,
				GT_NOTSET = 1,
				GT_SIMPLE = 2,
				GT_LIGHT = 3,
				GT_TEXTURE = 4,
				GT_TEXTURE_LIGHT = 5,
				GT_ALPHA = 6
			};

			String getTexPath( ) const { return mTexPath; }
			bool isSkin( ) const { return mIsSkin; }
			bool isAlpha( ) const { return mIsAlpha != 0; }
			bool isTwoSided( ) const { return mIsTwoSided != 0; }
			bool isAlphaTest( ) const { return mIsAlphaTest != 0; }
			ushort getAlphaRef( ) const { return mAlphaRef; }
			bool isZWrite( ) const { return mZWrite != 0; }
			bool isZTest( ) const { return mZTest != 0; }
			BlendType getBlendingMode( ) const { return mBlendingMode; }
			bool isSpecular( ) const { return mIsSpecular != 0; }
			float getAlpha( ) const { return mAlpha; }
			GlowType getGlowType( ) const { return mGlowType; }
			const ColourValue& getGlowColour( ) const { return mGlowColor; }

			_Material( DataStream &stream )
				: mGlowColor(1,1,1,1)
			{
				mTexPath = RFS::readString( stream );
				mIsSkin = RFS::readBool<ushort>( stream );
				mIsAlpha = RFS::readBool<ushort>( stream );
				mIsTwoSided = RFS::readBool<ushort>( stream );
				mIsAlphaTest = RFS::readBool<ushort>( stream );
				stream.read( &mAlphaRef, sizeof(ushort) );
				mZTest = RFS::readBool<ushort>( stream );
				mZWrite = RFS::readBool<ushort>( stream );
				stream.read( &mBlendingMode, sizeof(ushort) );
				mIsSpecular = RFS::readBool<ushort>( stream );
				stream.read( &mAlpha, sizeof(float) );
				stream.read( &mGlowType, sizeof(ushort) );
				stream.read( &mGlowColor.r, sizeof(float) );
				stream.read( &mGlowColor.g, sizeof(float) );
				stream.read( &mGlowColor.b, sizeof(float) );
			}

		protected:
			String mTexPath;
			bool mIsSkin;
			bool mIsAlpha;
			bool mIsTwoSided;
			bool mIsAlphaTest;
			bool mAlphaRef;
			bool mZTest;
			bool mZWrite;
			BlendType mBlendingMode;
			bool mIsSpecular;
			float mAlpha;
			GlowType mGlowType;
			ColourValue mGlowColor;

		};

		class _Effect
		{
		public:
			_Effect( DataStream &stream )
			{
				mEftPath = RFS::readString( stream );
			}

			String getEftPath( ) const { return mEftPath; }

		protected:
			String mEftPath;

		};

		class _BasicPart abstract
		{
		protected:
			void readFlags( DataStream &stream )
			{
				uchar blkFlag;
				uchar blkLen;

				while( true ) {
					stream.read( &blkFlag, sizeof(uchar) );
					if( blkFlag == 0 ) break;

					stream.read( &blkLen, sizeof(uchar) );

					handleFlag( stream, blkFlag, blkLen );
				}
			}

			virtual void handleFlag( DataStream &stream, uchar flag, uchar len )
			{
				stream.skip( len );
			}
		};

		class _CharPart : public _BasicPart
		{
		public:
			_CharPart( )
			{
			}

			_CharPart( DataStream &stream )
			{
				read( stream );
			}

			void read( DataStream &stream )
			{
				mBoneIdx = -1;
				mDummyIdx = -1;

				stream.read( &mMeshId, sizeof(ushort) );
				stream.read( &mMatId, sizeof(ushort) );

				readFlags( stream );
			}

			ushort getMeshId( ) const { return mMeshId; }
			ushort getMatId( ) const { return mMatId; }
			short getBoneIdx( ) const { return mBoneIdx; }
			short getDummyIdx( ) const { return mDummyIdx; }

		protected:
			virtual void handleFlag( DataStream &stream, uchar flag, uchar len ) override
			{
				if( flag == 5 ) {
					// Bone IDX
					stream.read( &mBoneIdx, sizeof(short) );
				} else if( flag == 6 ) {
					// Dummy IDX
					stream.read( &mDummyIdx, sizeof(short) );
				} else {
					_BasicPart::handleFlag( stream, flag, len );
				}
			}

			ushort mMeshId;
			ushort mMatId;
			short mBoneIdx;
			short mDummyIdx;

		};

		class _FixedPart : public _BasicPart
		{
		public:
			_FixedPart( DataStream &stream ) 
				: mParentId( -1 )
			{
				stream.read( &mMeshId, sizeof(ushort) );
				stream.read( &mMatId, sizeof(ushort) );

				readFlags( stream );
			}

			ushort getMeshId( ) const { return mMeshId; }
			ushort getMatId( ) const { return mMatId; }
			const Vector3& getPosition( ) const { return mPosition; }
			const Quaternion& getRotation( ) const { return mRotation; }
			const Vector3& getScale( ) const { return mScale; }
			int getParentId( ) const { return mParentId; }

		protected:
			virtual void handleFlag( DataStream &stream, uchar flag, uchar len ) override
			{
				if( flag == 1 ) {
					// Position
					stream.read( &mPosition, sizeof(Vector3) );
				} else if( flag == 2 ) {
					// Rotation
					stream.read( &mRotation, sizeof(Quaternion) );
				} else if( flag == 3 ) {
					// Scale
					stream.read( &mScale, sizeof(Vector3) );
				} else if( flag == 7 ) {
					// Parent
					ushort parent_id;
					stream.read( &parent_id, sizeof(ushort) );
					mParentId = (int)parent_id - 1;
				} else if( flag == 29 ) {
					// Collision Type
					stream.skip( sizeof(ushort) );
				} else {
					_BasicPart::handleFlag( stream, flag, len );
				}
			}

			ushort mMeshId;
			ushort mMatId;
			int mParentId;
			Vector3 mPosition;
			Quaternion mRotation;
			Vector3 mScale;

		};

		class _PointPart : public _BasicPart
		{
		public:
			_PointPart( DataStream &stream ) 
			{
				stream.read( &mEftId, sizeof(ushort) );
				stream.read( &mEftType, sizeof(ushort) );

				readFlags( stream );
			}

			ushort getEftId( ) const { return mEftId; }
			ushort getEftType( ) const { return mEftType; }

		protected:
			ushort mEftId;
			ushort mEftType;

		};

		template<typename PartType>
		class _Model
		{
		public:
			typedef PartType Part;
			typedef _PointPart Dummy;
			typedef std::vector<Part> PartList;
			typedef std::vector<Dummy> DummyList;

			bool isEnabled( ) const { return mEnabled; }

			const Part& getPart( int id ) const { return mParts[id]; }
			int getNumParts( ) const { return mParts.size(); }
			const PartList& getParts( ) const { return mParts; }

			const Dummy& getDummy( int id ) const { return mDummies[id]; }
			int getNumDummies( ) const { return mDummies.size(); }
			const DummyList& getDummies( ) const { return mDummies; }

			_Model( )
			{
			}

			_Model( DataStream &stream )
			{
				read( stream );
			}

			void read( DataStream &stream )
			{
				stream.skip( sizeof(uint) + sizeof(uint) + sizeof(uint) );

				ushort num_parts;
				stream.read( &num_parts, sizeof(ushort) );

				if( num_parts > 0 ) {
					mEnabled = true;
				} else {
					mEnabled = false;
					return;
				}

				mParts.reserve( num_parts );
				for( ushort i = 0; i < num_parts; ++i ) {
					mParts.push_back(Part( stream ));
				}
				
				ushort num_dummies;
				stream.read( &num_dummies, sizeof(ushort) );
				mDummies.reserve( num_dummies );
				for( ushort i = 0; i < num_dummies; ++i ) {
					mDummies.push_back(Dummy( stream ));
				}

				stream.skip( sizeof(Vector3) + sizeof(Vector3) );
			}

		protected:
			bool mEnabled;
			PartList mParts;
			DummyList mDummies;

		};

		template<typename PartType>
		class File
		{
		public:
			typedef _Mesh Mesh;
			typedef _Material Material;
			typedef _Effect Effect;
			typedef _Model<PartType> Model;

			typedef std::vector<Mesh> MeshList;
			typedef std::vector<Material> MaterialList;
			typedef std::vector<Effect> EffectList;
			typedef std::vector<Model> ModelList;

			File( )
			{
			}

			~File( )
			{
			}

			bool read( DataStream& stream )
			{
				ushort num_meshs;
				stream.read( &num_meshs, sizeof(ushort) );
				mMeshs.reserve( num_meshs );
				for( ushort i = 0; i < num_meshs; ++i ) {
					mMeshs.push_back(Mesh( stream ));
				}

				ushort num_mats;
				stream.read( &num_mats, sizeof(ushort) );
				mMaterials.reserve( num_mats );
				for( ushort i = 0; i < num_mats; ++i ) {
					mMaterials.push_back(Material( stream ));
				}

				ushort num_efts;
				stream.read( &num_efts, sizeof(ushort) );
				mEffects.reserve( num_efts );
				for( ushort i = 0; i < num_efts; ++i ) {
					mEffects.push_back(Effect( stream ));
				}

				ushort num_models;
				stream.read( &num_models, sizeof(ushort) );
				mModels.reserve( num_models );
				for( ushort i = 0; i < num_models; ++i ) {
					mModels.push_back(std::move(Model( stream )));
				}

				return true;
			}

			int getNumModels( ) const { return mModels.size(); }
			const Model& getModel( int id ) const { return mModels[id]; }
			const ModelList& getModels( ) const { return mModels; }

			int getNumMeshs( ) const { return mMeshs.size(); }
			const Mesh& getMesh( int id ) const { return mMeshs[id]; }
			const MeshList& getMeshs( ) const { return mMeshs; }

			int getNumMaterials( ) const { return mMaterials.size(); }
			const Material& getMaterial( int id ) const { return mMaterials[id]; }
			const MaterialList& getMaterials( ) const { return mMaterials; }

			int getNumEffects( ) const { return mEffects.size(); }
			const Effect& getEffect( int id ) const { return mEffects[id]; }
			const EffectList& getEffects( ) const { return mEffects; }

		protected:
			MeshList mMeshs;
			MaterialList mMaterials;
			EffectList mEffects;
			ModelList mModels;

		};

		typedef ROSE::ZSC::File<ROSE::ZSC::_CharPart> FileChar;
		typedef ROSE::ZSC::File<ROSE::ZSC::_FixedPart> FileFixed;
	}
}

typedef ROSE::ZSC::FileChar RZscChar;
typedef ROSE::ZSC::FileFixed RZscFixed;

#endif // __RZsc_h_