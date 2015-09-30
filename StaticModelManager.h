#pragma once

#include "StaticModel.h"
#include "RZsc.h"
#include "RZscHelper.h"
#include "RMeshLoader.h"

using namespace Ogre;

// NOT EVEN REMOTELY THREAD SAFE
class StaticModelManager
{
private:
	class MeshLoader : public ManualResourceLoader
	{
	private:
		Ogre::MemoryDataStreamPtr mMeshStream;
		StaticModelManager &mCreator;
		ushort mModelId;
		ushort mPartId;

		virtual void prepareResource(Ogre::Resource* resource) override
		{
			const RZscFixed::Model& zmdl = mCreator.mModelData.getModel( mModelId );
			const RZscFixed::Model::Part& zprt = zmdl.getPart( mPartId );
			const RZscFixed::Mesh& zmsh = mCreator.mModelData.getMesh( zprt.getMeshId() );
			mMeshStream = MemoryDataStreamPtr(new Ogre::MemoryDataStream(Ogre::ResourceGroupManager::getSingleton().openResource( zmsh.getMeshPath() )));
		}

		virtual void loadResource(Ogre::Resource* resource) override
		{
			Ogre::Mesh *mesh = (Ogre::Mesh*)resource;
			const RZscFixed::Model& zmdl = mCreator.mModelData.getModel( mModelId );
			const RZscFixed::Model::Part& zprt = zmdl.getPart( mPartId );

			// Set the mesh bounds to all 0, it will get updated by readMesh
			mesh->_setBoundingSphereRadius(0);
			mesh->_setBounds(AxisAlignedBox(0,0,0,0,0,0));

			// Move the MemoryDataStream to local scope so it gets destroyed
			MemoryDataStreamPtr localStream = mMeshStream;
			mMeshStream.setNull();

			// Read the mesh, and set its material!
			SubMesh *pSubmesh = RMeshLoader::readMesh( mesh, localStream.get() );
			pSubmesh->setMaterialName( mCreator.getMaterialName( zprt.getMatId() ) );
		}

	public:
		MeshLoader( StaticModelManager &creator, ushort modelId, ushort partId )
			: mCreator(creator), mModelId(modelId), mPartId(partId) 
		{
		}

	};

public:
	StaticModelManager( String filePath )
		: mFilePath(filePath)
	{
		static int zscId = 0;
		mZscId = ++zscId;
	}

	~StaticModelManager( )
	{
		unload( );
		unprepare( );
	}

	void prepare( )
	{
		Ogre::MemoryDataStream fs(Ogre::ResourceGroupManager::getSingleton().openResource(mFilePath));
		mModelData.read(fs);
	}

	void load( )
	{
		for( int modelId = 0; modelId < mModelData.getNumMaterials(); ++modelId ) 
		{
			const RZscFixed::Material& zmat = mModelData.getMaterial( modelId );

			Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create( getMaterialName(modelId), "General" );
			ROGRE::ZSC::BuildMaterial( mat.get(), zmat );
		}

		for( int modelId = 0; modelId < mModelData.getNumModels(); ++modelId )
		{
			const RZscFixed::Model& zmdl = mModelData.getModel( modelId );

			if( !zmdl.isEnabled() ) continue;

			for( int partId = 0; partId < zmdl.getNumParts(); ++partId ) 
			{
				const RZscFixed::Model::Part& zprt = zmdl.getPart( partId );

				MeshLoader *pLoader = new MeshLoader( *this, modelId, partId );
				MeshManager::getSingleton().create( getMeshName(modelId,partId), "General", true, pLoader );
			}

		}
	}

	void unload( )
	{
	}

	void unprepare( )
	{
	}

	String getMaterialName( int materialId ) const
	{
		char virt_name[65];
		sprintf( virt_name, "RZSC_%d_%d_MAT", mZscId, materialId );
		return virt_name;
	}

	String getMeshName( int modelId, int partId ) const
	{
		char virt_name[65];
		sprintf( virt_name, "RZSC_%d_%d_%d_MESH", mZscId, modelId, partId );
		return virt_name;
	}

	StaticModel* create( SceneManager *scene, int modelId )
	{
		const RZscFixed::Model& zmdl = mModelData.getModel( modelId );

		if( !zmdl.isEnabled() ) {
			throw std::exception( "Instantiating a disabled model!" );
		}

		StaticModel *model = new StaticModel( );

		// Create all the parts
		std::vector<SceneNode*> nodes;
		nodes.reserve( zmdl.getNumParts() );

		for( int partId = 0; partId < zmdl.getNumParts(); ++partId )
		{
			const RZscFixed::Model::Part& zprt = zmdl.getPart( partId );

			Ogre::SceneNode *pnode = scene->createSceneNode();
			Ogre::Entity *pent = scene->createEntity( getMeshName(modelId,partId) );
			pnode->attachObject( pent );

			pnode->setPosition( zprt.getPosition() );
			pnode->setOrientation( zprt.getRotation() );
			pnode->setScale( zprt.getScale() );

			nodes.push_back(pnode);
			model->mParts.push_back(StaticModel::Part(pnode,pent));
		}

		// Link all the nodes, and locate the root!
		for( int partId = 0; partId < zmdl.getNumParts(); ++partId )
		{
			const RZscFixed::Model::Part& zprt = zmdl.getPart( partId );

			int parent_id = zprt.getParentId();
			if( parent_id >= 0 ) {

				if( parent_id > (int)nodes.size() ) {
					__asm{ int 3 };
				}

				nodes[parent_id]->addChild( nodes[partId] );
			} else {
				model->mRootNode = nodes[partId];
			}
		}

		return model;
	}

protected:
	String mFilePath;
	int mZscId;
	RZscFixed mModelData;

};
