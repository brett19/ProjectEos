#pragma once

#include "NpcModel.h"
#include "RZsc.h"
#include "RChr.h"
#include "RZscHelper.h"
#include "RSkelLoader.h"
#include "RMeshLoader.h"

using namespace Ogre;

class NpcModelManager
{
private:
	class SkeletonLoader : public ManualResourceLoader
	{
	private:
		NpcModelManager &mCreator;
		ushort mNpcId;
		Ogre::MemoryDataStreamPtr mSkelStream;
		std::vector<Ogre::MemoryDataStreamPtr> mAnimStreams;

		virtual void prepareResource(Ogre::Resource* resource) override
		{
			const RChr::Character& cchar = mCreator.mNpcData.getCharacter( mNpcId );
			const RChr::Skeleton& cskel = mCreator.mNpcData.getSkeleton( cchar.getSkeletonId() );
			mSkelStream = MemoryDataStreamPtr(new Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource( cskel.getSkelPath() )));

			for( int animId = 0; animId < cchar.getNumAnimations( ); ++animId ) 
			{
				const RChr::Character::Animation& canim = cchar.getAnimation( animId );
				const RChr::Motion& cmotion = mCreator.mNpcData.getMotion( canim.getMotionId() );
				mAnimStreams.push_back(MemoryDataStreamPtr(new Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource( cmotion.getMotionPath() ))));
			}
		}

		virtual void loadResource(Ogre::Resource* resource) override
		{
			Ogre::Skeleton *skel = (Ogre::Skeleton*)resource;

			// Move the MemoryDataStream's to local scope so they gets destroyed
			MemoryDataStreamPtr localStream = mSkelStream;
			mSkelStream.setNull();
			std::vector<MemoryDataStreamPtr> localMotionStreams;
			localMotionStreams.swap( mAnimStreams );

			// Read the skeleton
			RSkelLoader::readSkel( skel, localStream.get() );
			
			const RChr::Character& cchar = mCreator.mNpcData.getCharacter( mNpcId );
			for( int animId = 0; animId < cchar.getNumAnimations( ); ++animId ) 
			{
				const RChr::Character::Animation& canim = cchar.getAnimation( animId );
				const RChr::Motion& cmotion = mCreator.mNpcData.getMotion( canim.getMotionId() );
				RSkelLoader::readAnim( skel, mCreator.getAnimationName(animId), localMotionStreams[animId].get() );
			}
		}

	public:
		SkeletonLoader( NpcModelManager &creator, ushort npcId )
			: mCreator(creator), mNpcId(npcId)
		{
		}

	};

	class MeshLoader : public ManualResourceLoader
	{
	private:
		NpcModelManager &mCreator;
		ushort mNpcId;
		ushort mSectionId;
		ushort mPartId;
		Ogre::MemoryDataStreamPtr mMeshStream;

		virtual void prepareResource(Ogre::Resource* resource) override
		{
			const RChr::Character& cchar = mCreator.mNpcData.getCharacter( mNpcId );
			const RChr::Character::Part& cpart = cchar.getPart( mSectionId );
			const RZscChar::Model& zmdl = mCreator.mModelData.getModel( cpart.getMeshId() );
			const RZscChar::Model::Part& zprt = zmdl.getPart( mPartId );
			const RZscChar::Mesh& zmsh = mCreator.mModelData.getMesh( zprt.getMeshId() );
			mMeshStream = MemoryDataStreamPtr(new Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource( zmsh.getMeshPath() )));
		}

		virtual void loadResource(Ogre::Resource* resource) override
		{
			Ogre::Mesh *mesh = (Ogre::Mesh*)resource;
			const RChr::Character& cchar = mCreator.mNpcData.getCharacter( mNpcId );
			const RChr::Character::Part& cpart = cchar.getPart( mSectionId );
			const RZscChar::Model& zmdl = mCreator.mModelData.getModel( cpart.getMeshId() );
			const RZscChar::Model::Part& zprt = zmdl.getPart( mPartId );

			// Set the mesh bounds to all 0, it will get updated by readMesh
			mesh->_setBoundingSphereRadius(0);
			mesh->_setBounds(AxisAlignedBox(0,0,0,0,0,0));

			// Set the skeleton!
			mesh->setSkeletonName( mCreator.getSkeletonName(mNpcId) );

			// Move the MemoryDataStream to local scope so it gets destroyed
			MemoryDataStreamPtr localStream = mMeshStream;
			mMeshStream.setNull();

			// Read the mesh, and set its material!
			SubMesh *pSubmesh = RMeshLoader::readMesh( mesh, localStream.get() );
			pSubmesh->setMaterialName( mCreator.getMaterialName( zprt.getMatId() ) );
		}

	public:
		MeshLoader( NpcModelManager &creator, ushort npcId, ushort sectionId, ushort partId )
			: mCreator(creator), mNpcId(npcId), mSectionId(sectionId), mPartId(partId) 
		{
		}

	};

public:
	NpcModelManager( )
	{
	}

	~NpcModelManager( )
	{
		unload( );
		unprepare( );
	}

	void prepare( )
	{
		{
			Ogre::MemoryDataStream fs(Ogre::ResourceGroupManager::getSingleton().openResource("3DDATA\\NPC\\PART_NPC.ZSC"));
			mModelData.read(fs);
		}
		{
			Ogre::MemoryDataStream fs(Ogre::ResourceGroupManager::getSingleton().openResource("3DDATA\\NPC\\LIST_NPC.CHR"));
			mNpcData.read(fs);
		}
	}

	void load( )
	{
		for( int npcId = 0; npcId < mNpcData.getNumCharacters(); ++npcId )
		{
			const RChr::Character& cchar = mNpcData.getCharacter( npcId );
			const RChr::Skeleton& cskel = mNpcData.getSkeleton( cchar.getSkeletonId( ) );

			SkeletonLoader *pLoader = new SkeletonLoader( *this, npcId );
			SkeletonManager::getSingleton().create( getSkeletonName(npcId), "General", true, pLoader );
		}

		for( int materialId = 0; materialId < mModelData.getNumMaterials(); ++materialId ) 
		{
			const RZscChar::Material& zmat = mModelData.getMaterial( materialId );

			Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create( getMaterialName(materialId), "General" );
			ROGRE::ZSC::BuildMaterial( mat.get(), zmat );
		}

		for( int npcId = 0; npcId < mNpcData.getNumCharacters(); ++npcId )
		{
			const RChr::Character& cchar = mNpcData.getCharacter( npcId );

			if( !cchar.isEnabled() ) continue;

			for( int sectionId = 0; sectionId < cchar.getNumParts(); ++sectionId )
			{
				const RChr::Character::Part& cpart = cchar.getPart( sectionId );
				const RZscChar::Model& zmdl = mModelData.getModel( cpart.getMeshId() );

				for( int partId = 0; partId < zmdl.getNumParts(); ++partId ) 
				{
					const RZscChar::Model::Part& zprt = zmdl.getPart( partId );

					MeshLoader *pLoader = new MeshLoader( *this, npcId, sectionId, partId );
					MeshManager::getSingleton().create( getMeshName(npcId,sectionId,partId), "General", true, pLoader );
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

	String getSkeletonName( int npcId ) const
	{
		char virt_name[65];
		sprintf( virt_name, "RNPC_%d_SKEL", npcId );
		return virt_name;
	}

	String getAnimationName( int animId ) const
	{
		char virt_name[65];
		sprintf( virt_name, "RANIM_%d", animId );
		return virt_name;
	}

	String getMaterialName( int materialId ) const
	{
		char virt_name[65];
		sprintf( virt_name, "RNPC_%d_MAT", materialId );
		return virt_name;
	}

	String getMeshName( int npcId, int sectionId, int partId ) const
	{
		char virt_name[65];
		sprintf( virt_name, "RNPC_%d_%d_%d_MESH", npcId, sectionId, partId );
		return virt_name;
	}

	NpcModel* create( SceneManager *scene, int npcId )
	{
		const RChr::Character& cchar = mNpcData.getCharacter( npcId );

		if( !cchar.isEnabled() ) {
			throw std::exception( "Instantiating disabled NPC!" );
		}

		NpcModel *model = new NpcModel( );

		SceneNode *root_node = scene->createSceneNode( );

		for( int sectionId = 0; sectionId < cchar.getNumParts(); ++sectionId )
		{
			const RChr::Character::Part& cpart = cchar.getPart( sectionId );
			const RZscChar::Model& zmdl = mModelData.getModel( cpart.getMeshId() );

			NpcModel::Section section;
			
			for( int partId = 0; partId < zmdl.getNumParts(); ++partId ) 
			{
				const RZscChar::Model::Part& zprt = zmdl.getPart( partId );

				Ogre::SceneNode *pnode = scene->createSceneNode();
				Ogre::Entity *pent = scene->createEntity( getMeshName(npcId,sectionId,partId) );
				pnode->attachObject( pent );

				if( model->mRootEntity ) {
					pent->shareSkeletonInstanceWith( model->mRootEntity );
				} else {
					model->mRootEntity = pent;
				}

				root_node->addChild( pnode );

				section.mParts.push_back(NpcModel::Section::Part(pnode,pent));
			}

			model->mSections.push_back( section );
		}

		model->mRootNode = root_node;

		return model;
	}

protected:
	RZscChar mModelData;
	RChr mNpcData;

};