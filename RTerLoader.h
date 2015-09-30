#ifndef __RTerLoader_h_
#define __RTerLoader_h_

#include <memory>
#include "RFS.h"
#include "StaticModelManager.h"

using namespace Ogre;

class RMap
{
public:
	class Page {
	private:
		class PageData
		{
		public:
			PageData( RMap *parent, int pageX, int pageY)
				: mParent(parent), mPageX(pageX), mPageY(pageY), mHeightData(new float[65*65]), mTileData(new int[16*16])
			{
			}

			~PageData(  )
			{
				if( mHeightData ) {
					delete[] mHeightData;
					mHeightData = nullptr;
				}
				if( mTileData ) {
					delete[] mTileData;
					mTileData = nullptr;
				}
			}

			RMap* getParent( ) const { return mParent; }
			int getPageX( ) const { return mPageX; }
			int getPageY( ) const { return mPageY; }
			float* getHeightData( ) const { return mHeightData; }
			int* getTileData( ) const { return mTileData; }

		private:
			RMap *mParent;
			int mPageX;
			int mPageY;
			float *mHeightData;
			int *mTileData;

		};
		typedef std::shared_ptr<PageData> PageDataPtr;

		class TerLoader : public ManualResourceLoader
		{
		public:
			TerLoader( PageDataPtr data, int patchX, int patchY ) 
				: mData(data), mPatchX(patchX), mPatchY(patchY)
			{
			}

			virtual void prepareResource(Ogre::Resource* resource) override
			{
			}

			virtual void loadResource(Ogre::Resource* resource) override
			{
				Ogre::Mesh* pMesh = (Ogre::Mesh*)resource;

				SubMesh *subMesh = pMesh->createSubMesh( );

				int num_verts = 5 * 5;

				Ogre::VertexData* data = new Ogre::VertexData();
				subMesh->vertexData = data;
				subMesh->useSharedVertices = false;

				data->vertexStart = 0;
				data->vertexCount = num_verts;

				Ogre::VertexDeclaration* decl = data->vertexDeclaration;

				size_t offset = 0;
				decl->addElement( 0, offset, VET_FLOAT3, VES_POSITION, 0 );
				offset += VertexElement::getTypeSize( VET_FLOAT3 );
				decl->addElement( 0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0 );
				offset += VertexElement::getTypeSize( VET_FLOAT2 );

				uint vsize1 = decl->getVertexSize(0);
				Ogre::HardwareVertexBufferSharedPtr vbuf1 = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
					vsize1,                     // This value is the size of a vertex in memory
					num_verts,                                 // The number of vertices you'll put into this buffer
					Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY // Properties
					);

				char* vptr1 = (char*)vbuf1->lock( HardwareBuffer::HBL_NO_OVERWRITE );

				float *hd = mData->getHeightData();

				const VertexElement *elPosition = decl->findElementBySemantic( VES_POSITION, 0 );
				const VertexElement *elUV = decl->findElementBySemantic( VES_TEXTURE_COORDINATES, 0 );

				float minHeight = hd[0];
				float maxHeight = hd[0];

				char *vptr = (char*)vptr1;
				for( int iy = 0; iy < 5; ++iy ) {
					for( int ix = 0; ix < 5; ++ix ) {
						int irx = mPatchX*4 + ix;
						int iry = mPatchY*4 + iy;
						float& h = hd[ iry*65 + irx];

						*(((float*)vptr)+0) = ix * 250.0f;
						*(((float*)vptr)+1) = -iy * 250.0f;
						*(((float*)vptr)+2) = h;
						*(((float*)vptr)+3) = (float)ix / 4.0f;
						*(((float*)vptr)+4) = (float)iy / 4.0f;

						if( h < minHeight ) minHeight = h;
						if( h > maxHeight ) maxHeight = h;

						vptr += vsize1;
					}
				}

				vbuf1->unlock( );

				int num_indices = 4*4*6;

				Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
					Ogre::HardwareIndexBuffer::IT_16BIT,        // You can use several different value types here
					num_indices,                                  // The number of indices you'll put in that buffer
					Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY // Properties
					);

				ushort* iptr = (ushort*)ibuf->lock( HardwareBuffer::HBL_NO_OVERWRITE );

				ushort* i = iptr;
				for( int iy = 0; iy < 4; ++iy ) {
					for( int ix = 0; ix < 4; ++ix ) {
						if( true ) {
							*i++ = (ix+1) + (iy+1)*5;
							*i++ = ix + iy*5;
							*i++ = ix + (iy+1)*5;

							*i++ = ix + iy*5;
							*i++ = (ix+1) + (iy+1)*5;
							*i++ = (ix+1) + iy*5;
						} else {
							*i++ = ix + (iy+1)*5;
							*i++ = ix + iy*5;
							*i++ = (ix+1) + (iy+1)*5;

							*i++ = (ix+1) + iy*5;
							*i++ = (ix+1) + (iy+1)*5;
							*i++ = ix + iy*5;
						}
						

					}
				}

				ibuf->unlock( );

				Ogre::VertexBufferBinding* bind = data->vertexBufferBinding;
				bind->setBinding( 0, vbuf1 );

				Ogre::IndexData* idata = subMesh->indexData;
				idata->indexStart = 0;
				idata->indexCount = num_indices;
				idata->indexBuffer = ibuf;

				int *td = mData->getTileData();
				subMesh->setMaterialName( mData->getParent()->getTileName(td[mPatchY*16+mPatchX]) );
				//subMesh->setMaterialName( mData->getParent()->getTileName(0) );

				float br1 = maxHeight-minHeight;
				float br2 = br1 < 1000.0f ? 1000.0f : br1;
				float br3 = sqrt(br2*br2);

				pMesh->_setBoundingSphereRadius(br3/2.0f);
				pMesh->_setBounds(AxisAlignedBox(0.0f,minHeight,0.0f,1000.0f,maxHeight,1000.0f));
			}

		private:
			int mPatchX;
			int mPatchY;
			PageDataPtr mData;

		};

	public:
		Page( RMap *parent, int pageX, int pageY )
			: mData(new PageData(parent,pageX,pageY))
		{
		}

		bool _loadHeightData( Ogre::DataStream *stream )
		{
			DataStream& fs = *stream;

			uint width;
			fs.read( &width, sizeof(uint) );

			uint height;
			fs.read( &height, sizeof(uint) );

			uint grid_count;
			fs.read( &grid_count, sizeof(uint) );

			float grid_size;
			fs.read( &grid_size, sizeof(float) );

			if( width != 65 || height != 65 || grid_count != 4 || abs(grid_size-250.0f) > 0.0001f ) {
				return false;
			}

			fs.read( mData->getHeightData(), sizeof(float)*65*65 );

			return true;
		}

		bool _loadTileData( Ogre::DataStream *stream )
		{
			DataStream& fs = *stream;

			uint width;
			fs.read( &width, sizeof(uint) );

			uint height;
			fs.read( &height, sizeof(uint) );

			if( width != 16 || height != 16 ) {
				return false;
			}

			int* td = mData->getTileData();
			for( int ix = 0; ix < 16; ++ix ) {
				for( int iy = 0; iy < 16; ++iy ) {
					fs.skip( 3 );
					fs.read( td++, sizeof(int) );
				}
			}

			return true;
		}

		class BaseObject
		{
		public:
			BaseObject( )
			{
			}

			BaseObject( DataStream &stream )
			{
				readBase( stream );
			}

			void readBase( DataStream &stream )
			{
				mName = RFS::readString<uchar>( stream );
				stream.skip( sizeof(ushort) + sizeof(ushort) + sizeof(uint) );
				stream.read( &mObjId, sizeof(uint) );
				stream.skip( sizeof(uint) + sizeof(uint) );

				stream.read( &mRotation.x, sizeof(float) );
				stream.read( &mRotation.y, sizeof(float) );
				stream.read( &mRotation.z, sizeof(float) );
				stream.read( &mRotation.w, sizeof(float) );

				stream.read( &mPosition, sizeof(Vector3) );
				stream.read( &mScale, sizeof(Vector3) );

				mPosition += Vector3( 520000.0f, 520000.0f, 0.0f );
			}
			
			String getName( ) const { return mName; }
			uint getObjId( ) const { return mObjId; }
			const Vector3& getPosition( ) const { return mPosition; }
			const Quaternion& getRotation( ) const { return mRotation; }
			const Vector3& getScale( ) const { return mScale; }

		protected:
			String mName;
			uint mObjId;
			Vector3 mPosition;
			Quaternion mRotation;
			Vector3 mScale;

		};

		typedef BaseObject DecoObject;
		typedef BaseObject CnstObject;

		std::vector<CnstObject> mCnsts;
		std::vector<CnstObject> mDecos;

		bool _loadPageData( Ogre::DataStream *stream )
		{
			DataStream& fs = *stream;

			uint num_blocks;
			fs.read( &num_blocks, sizeof(uint) );

			struct block_info {
				int type;
				int offset;
			} *blocks = new block_info[ num_blocks ];

			fs.read( blocks, sizeof(block_info)*num_blocks );

			for( uint i = 0; i < num_blocks; ++i )
			{
				const block_info& block = blocks[i];
				fs.seek( block.offset );

				if( block.type == 1 ) { // DECO List
					uint num_objs;
					fs.read( &num_objs, sizeof(uint) );
					for( uint i = 0; i < num_objs; ++i ) {
						mDecos.push_back(DecoObject( fs ));
					}
				} else if( block.type == 3 ) { // CNST List
					uint num_objs;
					fs.read( &num_objs, sizeof(uint) );
					for( uint i = 0; i < num_objs; ++i ) {
						mCnsts.push_back(CnstObject( fs ));
					}
				}
			}

			delete[] blocks;

			return true;
		}

		bool prepare( )
		{
			char file_path[ 256 ];
			{
				sprintf( file_path, "3DDATA\\MAPS\\JUNON\\JPT01\\%d_%d.HIM", mData->getPageX(), mData->getPageY() );
				Ogre::MemoryDataStream hfs(Ogre::ResourceGroupManager::getSingleton().openResource( file_path ));
				_loadHeightData( &hfs );
			}
			{
				sprintf( file_path, "3DDATA\\MAPS\\JUNON\\JPT01\\%d_%d.TIL", mData->getPageX(), mData->getPageY() );
				Ogre::MemoryDataStream tfs(Ogre::ResourceGroupManager::getSingleton().openResource( file_path ));
				_loadTileData( &tfs );
			}
			{
				sprintf( file_path, "3DDATA\\MAPS\\JUNON\\JPT01\\%d_%d.IFO", mData->getPageX(), mData->getPageY() );
				Ogre::MemoryDataStream tfs(Ogre::ResourceGroupManager::getSingleton().openResource( file_path ));
				_loadPageData( &tfs );
			}

			mCnstModelData = new StaticModelManager( "3DDATA\\JUNON\\LIST_CNST_JPT.ZSC" );
			mCnstModelData->prepare( );
			
			mDecoModelData = new StaticModelManager( "3DDATA\\JUNON\\LIST_DECO_JPT.ZSC" );
			mDecoModelData->prepare( );

			return true;
		}

		bool load( )
		{
			char virt_name[65];

			for( int ix = 0; ix < 16; ++ix ) {
				for( int iy = 0; iy < 16; ++iy ) {

					sprintf( virt_name, "ROSE_MAP_%d_%d_%d_%d_%d_MESH", mData->getParent()->getMapId(), mData->getPageX(), mData->getPageY(), ix, iy );

					TerLoader *mPatchLdr = new TerLoader( mData, ix, iy );
					Ogre::MeshManager::getSingleton().create( virt_name, "General", true, mPatchLdr );

				}
			}

			mCnstModelData->load( );
			mDecoModelData->load( );

			return true;
		}

		bool create( SceneManager *scene, SceneNode *rootnode )
		{
			char virt_name[ 64 ];

			float pageWorldX = (mData->getPageX()) * 16000.0f;
			float pageWorldY = (65-mData->getPageY()) * 16000.0f;

			Ogre::SceneNode *pagenode = rootnode->createChildSceneNode( );

			sprintf( virt_name, "ROSE_MAP_%d_%d_%d_STATIC", mData->getParent()->getMapId(), mData->getPageX(), mData->getPageY() );
			Ogre::StaticGeometry *sg = scene->createStaticGeometry( virt_name );

			for( int ix = 0; ix < 16; ++ix ) {
				for( int iy = 0; iy < 16; ++iy ) {
					sprintf( virt_name, "ROSE_MAP_%d_%d_%d_%d_%d_MESH", mData->getParent()->getMapId(), mData->getPageX(), mData->getPageY(), ix, iy );

					float tileX = ix*1000.0f;
					float tileY = -iy*1000.0f;

					Ogre::Entity* entity = scene->createEntity(virt_name);
					entity->setCastShadows( false );

					//*
					sg->addEntity( entity, Vector3( pageWorldX+tileX, pageWorldY+tileY, 0.0f ) );
					//*/
					/*
					Ogre::SceneNode* node = pagenode->createChildSceneNode();
					node->attachObject(entity);
					node->setPosition( tileX, 0.0f, tileY );
					//*/
				}
			}

			float regionSize = 160000.0f;
			sg->setRegionDimensions(Vector3(regionSize));
			sg->build( );
			pagenode->setPosition( pageWorldX, pageWorldY, 0.0f );




			for( auto i = mCnsts.begin(); i != mCnsts.end(); ++i ) 
			{
				StaticModel *model = mCnstModelData->create( scene, i->getObjId() );
				Ogre::SceneNode *node = model->getSceneNode( );
				node->setPosition( i->getPosition() );
				node->setOrientation( i->getRotation() );
				node->setScale( i->getScale() );
				rootnode->addChild( node );
				mCnstModels.push_back( model );
			}

			for( auto i = mDecos.begin(); i != mDecos.end(); ++i ) 
			{
				StaticModel *model = mDecoModelData->create( scene, i->getObjId() );
				Ogre::SceneNode *node = model->getSceneNode( );
				node->setPosition( i->getPosition() );
				node->setOrientation( i->getRotation() );
				node->setScale( i->getScale() );
				rootnode->addChild( node );
				mDecoModels.push_back( model );
			}

			return true;
		}

	private:
		PageDataPtr mData;

		StaticModelManager *mCnstModelData;
		StaticModelManager *mDecoModelData;
		std::vector<StaticModel*> mCnstModels;
		std::vector<StaticModel*> mDecoModels;
	};

	RMap( int mapId )
		: mMapId(mapId)
	{
		for( int ix = 0; ix < 65; ++ix ) {
			for( int iy = 0; iy < 65; ++iy ) {
				mPages[ix][iy] = nullptr;
			}
		}
	}

	~RMap( )
	{
		for( int ix = 0; ix < 65; ++ix ) {
			for( int iy = 0; iy < 65; ++iy ) {
				if( mPages[ix][iy] ) {
					delete mPages[ix][iy];
					mPages[ix][iy] = nullptr;
				}
			}
		}
	}
	
	Page* getPage( int pageX, int pageY )
	{
		Page*& page = mPages[pageX][pageY];

		if( !page ) {
			page = new Page( this, pageX, pageY );
			page->prepare( );
		}

		return page;
	}

	struct TileInfo {
		uint base1;
		uint base2;
		uint offset1;
		uint offset2;
		uint is_blending;
		uint orientation;
		uint tile_type;
	};

	bool _loadMapData( DataStream *stream )
	{
		DataStream& fs = *stream;

		uint num_blocks;
		fs.read( &num_blocks, sizeof(uint) );

		struct block_info {
			int type;
			int offset;
		} *blocks = new block_info[ num_blocks ];

		fs.read( blocks, sizeof(block_info)*num_blocks );

		for( uint i = 0; i < num_blocks; ++i )
		{
			const block_info& block = blocks[i];
			fs.seek( block.offset );

			if( block.type == 2 ) { // Texture List
				uint num_textures;
				fs.read( &num_textures, sizeof(uint) );
				for( uint j = 0; j < num_textures; ++j ) {
					mTextures.push_back( RFS::readString<uchar>(fs) );
				}
			} else if( block.type == 3 ) { // Tile List
				uint num_textures;
				fs.read( &num_textures, sizeof(uint) );
				for( uint j = 0; j < num_textures; ++j ) {
					TileInfo ti;
					fs.read( &ti, sizeof(TileInfo) );
					mTiles.push_back(ti);
				}
			}
		}

		delete[] blocks;

		return true;
	}

	String getTileName( int tileId )
	{
		char virt_name[ 65 ];
		sprintf( virt_name, "ROSE_TILE_%d_%d_MAT", mMapId, tileId );
		return virt_name;
	}

	bool declare( )
	{
		for( size_t i = 0; i < mTiles.size(); ++i )
		{
			TileInfo& tile = mTiles[i];

			Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create( getTileName(i), "General" );
			Technique *tech = mat->getTechnique(0);
			Pass *pass = tech->getPass( 0 );

			//pass->setAmbient( 1.0f, 1.0f, 1.0f );

			pass->setDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );

			pass->setSceneBlending( SBT_REPLACE );
			pass->setDepthCheckEnabled( true );
			pass->setDepthWriteEnabled( true );
			
			//pass->setLightingEnabled( false );
			mat->setReceiveShadows( true );

			String& texName1 = mTextures[tile.base1+tile.offset1];
			String& texName2 = mTextures[tile.base2+tile.offset2];

			if( tile.is_blending == 0 ) {
				TextureUnitState *unit1 = pass->createTextureUnitState( );
				unit1->setTextureName( texName1 );
				unit1->setColourOperation( LBO_REPLACE );
				unit1->setTextureAddressingMode( TextureUnitState::TAM_CLAMP );
			} else {
				TextureUnitState *unit1 = pass->createTextureUnitState( );
				unit1->setTextureName( texName1 );
				unit1->setColourOperation( LBO_REPLACE );
				unit1->setTextureAddressingMode( TextureUnitState::TAM_CLAMP );

				TextureUnitState *unit2 = pass->createTextureUnitState( );
				unit2->setTextureName( texName2 );
				unit2->setColourOperation( LBO_ALPHA_BLEND );
				unit2->setTextureAddressingMode( TextureUnitState::TAM_CLAMP );

				if( tile.orientation == 2 ) {
					unit2->setTextureScale( -1.0f, +1.0f );
				} else if( tile.orientation == 3 ) {
					unit2->setTextureScale( +1.0f, -1.0f );
				} else if( tile.orientation == 4 ) {
					unit2->setTextureScale( -1.0f, -1.0f );
				} else if( tile.orientation == 5 ) {
					pass->setDiffuse( 1.0f, 0.0f, 0.0f, 1.0f );
				} else if( tile.orientation == 6 ) {
					pass->setDiffuse( 0.0f, 1.0f, 0.0f, 1.0f );
				}
			}
		}

		return true;
	}

	bool load( )
	{
		{
			Ogre::MemoryDataStream zfs(Ogre::ResourceGroupManager::getSingleton().openResource( "3DDATA\\MAPS\\JUNON\\JPT01\\JPT01.ZON" ));
			_loadMapData( &zfs );
		}

		return true;
	}

	int getMapId( ) const { return mMapId; }

private:
	int mMapId;
	std::vector<String> mTextures;
	std::vector<TileInfo> mTiles;
	Page* mPages[65][65];

};

class RTerLoader : public Ogre::ManualResourceLoader
{
public:
	virtual void prepareResource(Ogre::Resource* resource) override
	{
	}

	virtual void loadResource(Ogre::Resource* resource) override
	{
		Ogre::Mesh* pMesh = (Ogre::Mesh*)resource;

		Ogre::MemoryDataStream fs(Ogre::ResourceGroupManager::getSingleton().openResource( resource->getName() ));
		if( !fs.isReadable() ) return;

		
	}


};

#endif // __RTerLoader_h_