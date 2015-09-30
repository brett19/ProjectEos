#pragma once

#include "AvatarModel.h"
#include "RZsc.h"
#include "RZscHelper.h"
#include "RSkelLoader.h"
#include "RMeshLoader.h"

using namespace Ogre;

class AvatarModelManager
{
private:
	class SkeletonLoader : public ManualResourceLoader
	{
	private:
		AvatarModelManager &mCreator;
		ushort mGenderId;
		Ogre::MemoryDataStreamPtr mSkelStream;
		std::vector<Ogre::MemoryDataStreamPtr> mAnimStreams;

		virtual void prepareResource(Ogre::Resource* resource) override
		{
			if( mGenderId != 1 ) {
				mSkelStream = MemoryDataStreamPtr(new Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource( "3DDATA\\avatar\\male.zmd" )));
			} else {
				mSkelStream = MemoryDataStreamPtr(new Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource( "3DDATA\\avatar\\female.zmd" )));
			}

		}

		virtual void loadResource(Ogre::Resource* resource) override
		{
			Ogre::Skeleton *skel = (Ogre::Skeleton*)resource;

			// Move the MemoryDataStream's to local scope so they gets destroyed
			MemoryDataStreamPtr localStream = mSkelStream;
			mSkelStream.setNull();

			// Read the skeleton
			RSkelLoader::readSkel( skel, localStream.get() );
		}

	public:
		SkeletonLoader( AvatarModelManager &creator, ushort genderId )
			: mCreator(creator), mGenderId(genderId)
		{
		}

	};

	class MeshLoader : public ManualResourceLoader
	{
	private:
		Ogre::MemoryDataStreamPtr mMeshStream;
		AvatarModelManager &mCreator;
		ushort mDataId;
		ushort mModelId;
		ushort mPartId;

		virtual void prepareResource(Ogre::Resource* resource) override
		{
			const RZscChar& zdata = mCreator.mModelData[mDataId];
			const RZscChar::Model& zmdl = zdata.getModel( mModelId );
			const RZscChar::Model::Part& zprt = zmdl.getPart( mPartId );
			const RZscChar::Mesh& zmsh = zdata.getMesh( zprt.getMeshId() );
			mMeshStream = MemoryDataStreamPtr(new Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource( zmsh.getMeshPath() )));
		}

		virtual void loadResource(Ogre::Resource* resource) override
		{
			Ogre::Mesh *mesh = (Ogre::Mesh*)resource;
			const RZscChar& zdata = mCreator.mModelData[mDataId];
			const RZscChar::Model& zmdl = zdata.getModel( mModelId );
			const RZscChar::Model::Part& zprt = zmdl.getPart( mPartId );

			// Set the mesh bounds to all 0, it will get updated by readMesh
			mesh->_setBoundingSphereRadius(0);
			mesh->_setBounds(AxisAlignedBox(0,0,0,0,0,0));

			// Set the skeleton!
			int genderId = mCreator.getSkelGenderId( mDataId );
			if( genderId >= 0 ) {
				mesh->setSkeletonName( mCreator.getSkeletonName(genderId) );
			}

			// Move the MemoryDataStream to local scope so it gets destroyed
			MemoryDataStreamPtr localStream = mMeshStream;
			mMeshStream.setNull();

			// Read the mesh, and set its material!
			SubMesh *pSubmesh = RMeshLoader::readMesh( mesh, localStream.get() );
			pSubmesh->setMaterialName( mCreator.getMaterialName( mDataId, zprt.getMatId() ) );
		}

	public:
		MeshLoader( AvatarModelManager &creator, ushort dataId, ushort modelId, ushort partId )
			: mCreator(creator), mDataId(dataId), mModelId(modelId), mPartId(partId) 
		{
		}

	};
public:
	void prepare( )
	{
		mModelData[ 0].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_mFACE.ZSC")));
		mModelData[ 1].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_wFACE.ZSC")));
		mModelData[ 2].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_mHAIR.ZSC")));
		mModelData[ 3].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_wHAIR.ZSC")));
		mModelData[ 4].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_mCAP.ZSC")));
		mModelData[ 5].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_wCAP.ZSC")));
		mModelData[ 6].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_mBODY.ZSC")));
		mModelData[ 7].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_wBODY.ZSC")));
		mModelData[ 8].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_mARMS.ZSC")));
		mModelData[ 9].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_wARMS.ZSC")));
		mModelData[10].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_mFOOT.ZSC")));
		mModelData[11].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_wFOOT.ZSC")));
		mModelData[12].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_FACEIEM.ZSC")));
		mModelData[13].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\avatar\\LIST_BACK.ZSC")));
		mModelData[14].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\weapon\\LIST_weapon.ZSC")));
		mModelData[15].read(Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource("3Ddata\\weapon\\LIST_subwpn.ZSC")));

		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_FACE     ] = 0;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_FACE     ] = 1;
		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_HAIR     ] = 2;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_HAIR     ] = 3;
		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_CAP      ] = 4;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_CAP      ] = 5;
		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_BODY     ] = 6;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_BODY     ] = 7;
		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_ARMS     ] = 8;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_ARMS     ] = 9;
		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_FOOT     ] = 10;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_FOOT     ] = 11;
		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_FACEITEM ] = 12;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_FACEITEM ] = 12;
		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_BACK     ] = 13;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_BACK     ] = 13;
		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_LWEAPON  ] = 14;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_LWEAPON  ] = 14;
		mModelDataLookup[ AvatarModel::AG_MALE   ][ AvatarModel::IS_RWEAPON  ] = 15;
		mModelDataLookup[ AvatarModel::AG_FEMALE ][ AvatarModel::IS_RWEAPON  ] = 15;
	}

	void load( )
	{
		{
			SkeletonLoader *pLoader = new SkeletonLoader( *this, 0 );
			SkeletonManager::getSingleton().create( getSkeletonName(0), "General", true, pLoader );
		}
		{
			SkeletonLoader *pLoader = new SkeletonLoader( *this, 1 );
			SkeletonManager::getSingleton().create( getSkeletonName(1), "General", true, pLoader );
		}

		for( int dataId = 0; dataId < 16; ++dataId )
		{
			RZscChar& modelData = mModelData[dataId];

			for( int materialId = 0; materialId < modelData.getNumMaterials(); ++materialId ) 
			{
				const RZscChar::Material& zmat = modelData.getMaterial( materialId );

				Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create( getMaterialName(dataId,materialId), "General" );
				ROGRE::ZSC::BuildMaterial( mat.get(), zmat );
			}

			for( int modelId = 0; modelId < modelData.getNumModels(); ++modelId )
			{
				const RZscChar::Model& zmdl = modelData.getModel( modelId );

				if( !zmdl.isEnabled() ) continue;

				for( int partId = 0; partId < zmdl.getNumParts(); ++partId ) 
				{
					const RZscChar::Model::Part& zprt = zmdl.getPart( partId );

					MeshLoader *pLoader = new MeshLoader( *this, dataId, modelId, partId );
					MeshManager::getSingleton().create( getMeshName(dataId,modelId,partId), "General", true, pLoader );
				}

			}
		}
	}

	void unload( )
	{
	}

	void unprepare( )
	{
	}

	int getSkelGenderId( int dataId ) const
	{
		if( dataId >= 6 && dataId < 12 ) {
			return dataId % 2;
		}
		return -1;
	}

	int getDataGender( int dataId ) const
	{
		if( dataId >= 0 && dataId <= 11 ) {
			return dataId % 2;
		}
		return -1;
	}

	String getSkeletonName( int genderId ) const
	{
		std::stringstream stream;
		stream << "RAVT_" << genderId << "_SKEL";
		return stream.str();
	}

	String getAnimationName( int animId ) const
	{
		std::stringstream stream;
		stream << "RANIM_" << animId;
		return stream.str();
	}

	String getMaterialName( int dataId, int materialId ) const
	{
		std::stringstream stream;
		stream << "RAVT_" << dataId << "_" << materialId << "_MAT";
		return stream.str();
	}

	String getMeshName( int dataId, int modelId, int partId ) const
	{
		std::stringstream stream;
		stream << "RAVT_" << dataId << "_" << modelId << "_" << partId << "_MESH";
		return stream.str();
	}

	int getDataId( int genderId, AvatarModel::ItemSlot slot ) const 
	{
		return mModelDataLookup[ genderId ][ slot ];
	}

	void removeAllParts( AvatarModel *model ) const
	{
		for( auto i = model->mSections.begin(); i != model->mSections.end(); ++i ) {
			for( auto j = i->mParts.begin(); j != i->mParts.end(); ++j ) {
				model->mScene->destroySceneNode( j->getSceneNode() );
				model->mScene->destroyEntity( j->getEntity() );
			}
			i->mParts.clear( );
		}
		model->mSections.clear( );
	}

	AvatarModel::Section createSection( AvatarModel *model, AvatarModel::ItemSlot slot, ItemId itemId ) const
	{
		AvatarModel::Section section;
		int dataId = getDataId( model->mGender, slot );
		const RZscChar& modelData = mModelData[ dataId ];
		const RZscChar::Model& itemMdl = modelData.getModel( itemId );
		SceneManager *scene = model->mScene;

		for( auto i = 0; i < itemMdl.getNumParts( ); ++i ) {
			const RZscChar::Model::Part& itemPart = itemMdl.getPart( i );

			Ogre::SceneNode *pnode = nullptr;
			Ogre::Entity *pent = scene->createEntity( getMeshName(dataId, itemId, i) );

			int boneIdx = itemPart.getBoneIdx( );
			int dummyIdx = itemPart.getDummyIdx( );

			if( boneIdx == -1 ) {
				if( slot == AvatarModel::IS_FACE ) {
					boneIdx = 4; //BONE_IDX_HEAD
				} else if( slot == AvatarModel::IS_HAIR ) {
					boneIdx = 4; //BONE_IDX_HEAD
				}
			}
			if( dummyIdx == -1 ) {
				if( slot == AvatarModel::IS_CAP ) {
					dummyIdx = 6; //DUMMY_IDX_CAP
				} else if( slot == AvatarModel::IS_BACK ) {
					dummyIdx = 3; //DUMMY_IDX_BACK
				} else if( slot == AvatarModel::IS_FACEITEM ) {
					dummyIdx = 4; //DUMMY_IDX_MOUSE
				}
			}

			if( boneIdx >= 0 ) {
				// Attached directly to Bone

				if( model->mRootEntity ) {
					model->mRootEntity->attachObjectToBone( RSkelLoader::getBoneName(boneIdx), pent );
				} else {
					throw std::exception( "No Root Entity" );
				}

			} else if( dummyIdx >= 0 ) {
				// Attached directly to Dummy

				if( model->mRootEntity ) {
					model->mRootEntity->attachObjectToBone( RSkelLoader::getDummyName(dummyIdx), pent );
				} else {
					throw std::exception( "No Root Entity" );
				}

			} else {
				// Not Attached
				pnode = scene->createSceneNode();
				pnode->attachObject( pent );
				model->mRootNode->addChild( pnode );

				if( model->mRootEntity ) {
					pent->shareSkeletonInstanceWith( model->mRootEntity );
				} else {
					model->mRootEntity = pent;
				}
			}

			section.mParts.push_back(AvatarModel::Section::Part(pnode,pent));
		}

		return section;
	}

	void updateAllParts( AvatarModel *model ) const
	{
		removeAllParts( model );

		AvatarModel::ItemSlot loadOrder[] = {
			AvatarModel::IS_BODY,
			AvatarModel::IS_ARMS,
			AvatarModel::IS_FOOT,
			AvatarModel::IS_FACEITEM,
			AvatarModel::IS_BACK,
			AvatarModel::IS_LWEAPON,
			AvatarModel::IS_RWEAPON,
			AvatarModel::IS_CAP,
			AvatarModel::IS_FACE,
			AvatarModel::IS_HAIR
		};
		const int numLoadOrder = 10;

		for( int i = 0; i < numLoadOrder; ++i ) {
			AvatarModel::ItemSlot slotId = loadOrder[ i ];
			model->mSections.push_back( createSection( model, slotId, model->mItemIds[slotId] ) );
		}
	}

	void updatePart( AvatarModel *model )
	{

	}


	AvatarModel* create( SceneManager *scene )
	{
		AvatarModel *model = new AvatarModel( this, scene );
		model->mRootNode = scene->createSceneNode( );
		updateAllParts( model );
		return model;
	}

protected:
	RZscChar mModelData[16];
	int mModelDataLookup[2][10];

};