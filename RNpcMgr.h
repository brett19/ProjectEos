#ifndef __RNPCMGR_H_
#define __RNPCMGR_H_

#include "RFS.h"
#include "RMeshLoader.h"
#include "RSkelLoader.h"

#include "RZsc.h"
#include "RChr.h"

using namespace Ogre;

class RNpcMgr : public Ogre::Singleton<RNpcMgr>
{
private:
	struct ZscMat
	{
		String filePath;
		bool isSkin;
		bool isAlpha;
		bool isTwoSide;
		bool isAlphaTest;
		ushort alphaRef;
		bool isZTest;
		bool isZWrite;
	};

	struct ZscModel
	{
		struct Part
		{
			ushort mesh_id;
			ushort mat_id;
		};

		std::vector<Part> parts;
	};

	std::vector<String> mMeshs;
	std::vector<ZscMat> mMats;
	std::vector<ZscModel> mModels;

	class MeshLoader : public ManualResourceLoader
	{
	private:
		RNpcMgr *mCreator;
		ushort mNpcId;
		ushort mMdlId;
		ushort mPartId;

	public:
		MeshLoader( RNpcMgr *creator, ushort npcId, ushort mdlId, ushort partId )
			: mCreator(creator), mNpcId(npcId), mMdlId(mdlId), mPartId(partId) 
		{
		}

		virtual void prepareResource(Ogre::Resource* resource) override {}
		virtual void loadResource(Ogre::Resource* resource) override {
			mCreator->loadMesh( (Ogre::Mesh*)resource, mNpcId, mMdlId, mPartId );
		}
	};

	class SkelLoader : public ManualResourceLoader
	{
	private:
		RNpcMgr *mCreator;
		ushort mNpcId;

	public:
		SkelLoader( RNpcMgr *creator, ushort npcId )
			: mCreator(creator), mNpcId(npcId)
		{
		}

		virtual void prepareResource(Ogre::Resource* resource) override {}
		virtual void loadResource(Ogre::Resource* resource) override {
			mCreator->loadSkel( (Ogre::Skeleton*)resource, mNpcId );
		}
	};

public:

	void loadMesh( Mesh *mesh, ushort npcId, ushort mdlId, ushort partId )
	{
		const NpcCache* npcCache = mNpcs[npcId];
		const NpcCache::ModelCache& mdlCache = npcCache->models[mdlId];
		const NpcCache::ModelCache::PartCache& partCache = mdlCache.parts[partId];

		const ZscModel& zscmdl = mModels[mdlCache.mdlId];
		const ZscModel::Part& zscpart = zscmdl.parts[partId];

		mesh->_setBoundingSphereRadius(0);
		mesh->_setBounds(AxisAlignedBox(0,0,0,0,0,0));

		mesh->setSkeletonName( npcCache->skelName );

		Ogre::MemoryDataStream dsMesh(Ogre::ResourceGroupManager::getSingleton().openResource( mMeshs[zscpart.mesh_id] ));

		SubMesh *pSubmesh = RMeshLoader::readMesh( mesh, &dsMesh );
		pSubmesh->setMaterialName( partCache.matName );
	}

	void loadSkel( Skeleton *skel, ushort npcId )
	{
		const NpcCache* npcCache = mNpcs[npcId];
		
		Ogre::MemoryDataStream dsSkel(Ogre::ResourceGroupManager::getSingleton().openResource( mChrSkels[npcCache->base->skel_idx] ));
		RSkelLoader::readSkel( skel, &dsSkel );
		
		for( auto i = npcCache->anims.begin(); i != npcCache->anims.end(); ++i ) {
			const NpcCache::AnimCache& animCache = i->second;

			Ogre::MemoryDataStream dsMotion(Ogre::ResourceGroupManager::getSingleton().openResource( mChrMotions[animCache.motionId] ));
			RSkelLoader::readAnim( skel, animCache.animName, &dsMotion );

		}
	}

	struct ChrNpc;

	struct NpcCache
	{
		struct ModelCache
		{
			struct PartCache
			{
				const ZscModel::Part *base;
				String meshName;
				String matName;
			};

			const ZscModel *base;
			ushort mdlId;
			std::vector<PartCache> parts;
		};

		struct AnimCache
		{
			ushort motionId;
			String animName;
		};

		const ChrNpc *base;
		String skelName;
		std::vector<ModelCache> models;
		std::map<ushort,AnimCache> anims;
	};
	std::map<ushort,NpcCache*> mNpcs;



	void _declareNpc( ushort npcId )
	{
		if( mNpcs[npcId] ) return;

		char virt_name[ 32 ];

		const ChrNpc& npc = mChrNpcs[npcId];
		NpcCache *pNpc = new NpcCache( );

		sprintf( virt_name, "ROSE_NPC_%d_SKEL", npcId );
		pNpc->skelName = virt_name;

		SkelLoader *pLoader = new SkelLoader( this, npcId );
		SkeletonManager::getSingleton().create( pNpc->skelName, "General", true, pLoader );
		//Ogre::ResourceGroupManager::getSingleton().declareResource( pNpc->skelName, "Skeleton", "General", pLoader );
		

		pNpc->base = &npc;

		for( size_t i = 0; i < npc.mdls.size(); ++i ) {
			NpcCache::ModelCache mdl;

			mdl.mdlId = npc.mdls[i];
			const ZscModel& zscmdl = mModels[mdl.mdlId];

			mdl.base = &zscmdl;

			for( size_t j = 0; j < zscmdl.parts.size(); ++j ) {
				NpcCache::ModelCache::PartCache part;

				const ZscModel::Part& zscpart = zscmdl.parts[j];

				part.base = &zscpart;

				{
					const ZscMat& zscmat = mMats[zscpart.mat_id];

					sprintf( virt_name, "ROSE_NPC_%d_%d_%d_MAT", npcId, i, j );
					part.matName = virt_name;

					Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create( part.matName, "General" );

					Technique *tech = mat->getTechnique(0);
					Pass *pass = tech->getPass( 0 );

					pass->setAmbient( 0.3f, 0.3f, 0.3f );

					if( zscmat.isAlpha ) {
						pass->setSceneBlending( SBT_TRANSPARENT_ALPHA );

						if( zscmat.isAlphaTest ) {
							pass->setAlphaRejectSettings( CMPF_GREATER, zscmat.alphaRef );
						}
					}

					if( zscmat.isTwoSide ) {
						pass->setCullingMode( CULL_NONE );
					}
					pass->setDepthCheckEnabled( zscmat.isZTest );
					pass->setDepthWriteEnabled( zscmat.isZWrite );

					TextureUnitState *unit = pass->createTextureUnitState( );
					unit->setTextureName( zscmat.filePath );

				}
				{
					sprintf( virt_name, "ROSE_NPC_%d_%d_%d_MESH", npcId, i, j );
					part.meshName = virt_name;

					MeshLoader *pLoader = new MeshLoader( this, npcId, i, j );
					MeshManager::getSingleton().create( part.meshName, "General", true, pLoader );
					//Ogre::ResourceGroupManager::getSingleton().declareResource( part.meshName, "Mesh", "General", pLoader );
				}

				mdl.parts.push_back( part );
			}

			pNpc->models.push_back( mdl );
		}

		for( auto i = npc.anims.begin(); i != npc.anims.end(); ++i ) {
			NpcCache::AnimCache anim;

			anim.motionId = i->second;

			sprintf( virt_name, "ROSE_NPC_%d_%d_ANIM", npcId, i->first );
			anim.animName = virt_name;

			pNpc->anims.insert(std::pair<ushort,NpcCache::AnimCache>(i->first, anim));
		}

		mNpcs[npcId] = pNpc;
	}

	void createModel( ushort id, SceneManager *scene, SceneNode *root )
	{
		_declareNpc( id );

		std::vector<SceneNode*> nodes;
		std::vector<Entity*> entities;

		const NpcCache* npc = mNpcs[ id ];

		for( size_t i = 0; i < npc->models.size(); ++i ) {

			const NpcCache::ModelCache& mdl = npc->models[i];

			for( size_t j = 0; j < mdl.parts.size(); ++j ) {

				const NpcCache::ModelCache::PartCache& part = mdl.parts[j];

				Entity *ent = scene->createEntity( part.meshName );
				SceneNode *node = scene->createSceneNode( );
				node->attachObject( ent );

				if( entities.size() > 0 ) {
					ent->shareSkeletonInstanceWith( entities.front() );
				}

				entities.push_back( ent );
				nodes.push_back( node );

				root->addChild( node );
			}

		}

		if( entities.size() > 0 ) {

			Entity *rootent = entities.front();

			const NpcCache::AnimCache& anim = npc->anims.at(0);

			AnimationState *as = rootent->getAnimationState( anim.animName );
			as->setEnabled( true );

		}

	}

	void getNpc( ushort id )
	{


	}


	// ROSE_AVT_MALE_SKEL
	// ROSE_AVT_FEMALE_SKEL


	bool loadZsc( )
	{
		Ogre::MemoryDataStream fs(Ogre::ResourceGroupManager::getSingleton().openResource("3DDATA\\NPC\\PART_NPC.ZSC"));

		ushort num_meshs;
		fs.read( &num_meshs, sizeof(ushort) );
		mMeshs.reserve(num_meshs);

		for( ushort i = 0; i < num_meshs; ++i ) {
			mMeshs.push_back( RFS::readString(fs) );
		}

		ushort num_mats;
		fs.read( &num_mats, sizeof(ushort) );
		mMats.reserve(num_mats);
		
		for( ushort i = 0; i < num_mats; ++i ) {
			ZscMat mat;
			ushort buf;

			mat.filePath = RFS::readString( fs );

			fs.read( &buf, sizeof(ushort) );
			mat.isSkin = buf != 0;
			fs.read( &buf, sizeof(ushort) );
			mat.isAlpha = buf != 0;
			fs.read( &buf, sizeof(ushort) );
			mat.isTwoSide = buf != 0;
			fs.read( &buf, sizeof(ushort) );
			mat.isAlphaTest = buf != 0;
			fs.read( &mat.alphaRef, sizeof(ushort) );
			fs.read( &buf, sizeof(ushort) );
			mat.isZTest = buf != 0;
			fs.read( &buf, sizeof(ushort) );
			mat.isZWrite = buf != 0;
			
			fs.skip( sizeof(ushort) + sizeof(ushort) + sizeof(float) + sizeof(ushort) + sizeof(Vector3) );

			mMats.push_back( mat );
		}

		ushort num_efts;
		fs.read( &num_efts, sizeof(ushort) );

		for( ushort i = 0; i < num_efts; ++i ) {
			RFS::readString( fs );
		}

		ushort num_models;
		fs.read( &num_models, sizeof(ushort) );
		mModels.reserve( num_models );

		for( ushort i = 0; i < num_models; ++i ) {
			ZscModel mdl;

			fs.skip( sizeof(uint) + sizeof(uint) + sizeof(uint) );

			ushort num_parts;
			fs.read( &num_parts, sizeof(ushort) );

			if( num_parts == 0 ) {
				mModels.push_back( mdl );
				continue;
			}
			
			for( ushort j = 0; j < num_parts; ++j ) {
				ZscModel::Part prt;

				fs.read( &prt.mesh_id, sizeof(ushort) );
				fs.read( &prt.mat_id, sizeof(ushort) );


				while( true ) {
					uchar tag;
					fs.read( &tag, sizeof(uchar) );
					if( tag == 0 ) break;

					uchar len;
					fs.read( &len, sizeof(uchar) );

					fs.skip( len );
				}

				mdl.parts.push_back( prt );
			}

			ushort num_dummies;
			fs.read( &num_dummies, sizeof(ushort) );

			for( ushort j = 0; j < num_dummies; ++j ) {
				ushort eft_id;
				fs.read( &eft_id, sizeof(ushort) );
				ushort eft_type;
				fs.read( &eft_type, sizeof(ushort) );

				while( true ) {
					uchar tag;
					fs.read( &tag, sizeof(uchar) );
					if( tag == 0 ) break;

					uchar len;
					fs.read( &len, sizeof(uchar) );

					fs.skip( len );
				}
			}

			fs.skip( sizeof(Vector3) + sizeof(Vector3) );

			mModels.push_back( mdl );
		}

		return true;
	}

	std::vector<String> mChrSkels;
	std::vector<String> mChrMotions;
	std::vector<String> mChrEffects;

	struct ChrNpc
	{
		bool isActive;
		String name;
		ushort skel_idx;
		std::vector<uint> mdls;
		std::map<ushort,ushort> anims;
	};
	std::vector<ChrNpc> mChrNpcs;

	bool loadChr()
	{
		Ogre::MemoryDataStream fs(Ogre::ResourceGroupManager::getSingleton().openResource("3DDATA\\NPC\\LIST_NPC.CHR"));

		ushort num_skels;
		fs.read( &num_skels, sizeof(ushort) );
		mChrSkels.reserve(num_skels);

		for( ushort i = 0; i < num_skels; ++i ) {
			mChrSkels.push_back( RFS::readString(fs) );
		}

		ushort num_motions;
		fs.read( &num_motions, sizeof(ushort) );
		mChrMotions.reserve(num_motions);

		for( ushort i = 0; i < num_motions; ++i ) {
			mChrMotions.push_back( RFS::readString(fs) );
		}

		ushort num_effects;
		fs.read( &num_effects, sizeof(ushort) );
		mChrEffects.reserve(num_effects);

		for( ushort i = 0; i < num_effects; ++i ) {
			mChrEffects.push_back( RFS::readString(fs) );
		}

		ushort num_npcs;
		fs.read( &num_npcs, sizeof(ushort) );
		mChrNpcs.reserve( num_npcs );

		for( ushort i = 0; i < num_npcs; ++i ) {
			ChrNpc npc;

			uchar isValid;
			fs.read( &isValid, sizeof(uchar) );
			if( !isValid ) {
				npc.isActive = false;
				mChrNpcs.push_back( npc );
				continue;
			}
			npc.isActive = true;
			
			fs.read( &npc.skel_idx, sizeof(ushort) );

			npc.name = RFS::readString( fs );

			ushort num_models;
			fs.read( &num_models, sizeof(ushort) );
			npc.mdls.reserve( num_models );

			for( ushort j = 0; j < num_models; ++j ) {
				ushort mdl_idx;
				fs.read( &mdl_idx, sizeof(ushort) );

				npc.mdls.push_back( mdl_idx );
			}


			ushort num_motions;
			fs.read( &num_motions, sizeof(ushort) );

			for( ushort j = 0; j < num_motions; ++j ) {
				ushort anim_idx;
				fs.read( &anim_idx, sizeof(ushort) );

				ushort motion_idx;
				fs.read( &motion_idx, sizeof(ushort) );

				if( motion_idx < 0 || motion_idx >= mChrMotions.size() ) {
					continue;
				}

				npc.anims.insert(std::pair<ushort,ushort>( anim_idx, motion_idx ));
			}

			ushort num_boneefts;
			fs.read( &num_boneefts, sizeof(ushort) );

			fs.skip( num_boneefts * (sizeof(ushort)+sizeof(ushort)) );

			mChrNpcs.push_back( npc );
		}

		return true;
	}

	bool load( )
	{
		loadZsc( );
		loadChr( );
		return true;
	}
};

#endif // __RNPCMGR_H_