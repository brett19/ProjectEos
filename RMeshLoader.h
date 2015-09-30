#ifndef __RMeshLoader_h_
#define __RMeshLoader_h_

#include "RFS.h"

using namespace Ogre;

#define ZZ_VF_NONE         (1 << 0)
#define ZZ_VF_POSITION     (1 << 1)
#define ZZ_VF_NORMAL       (1 << 2)
#define ZZ_VF_COLOR        (1 << 3)
#define ZZ_VF_BLEND_WEIGHT (1 << 4)
#define ZZ_VF_BLEND_INDEX  (1 << 5)
#define ZZ_VF_TANGENT      (1 << 6)
#define ZZ_VF_UV0          (1 << 7)
#define ZZ_VF_UV1          (1 << 8)
#define ZZ_VF_UV2          (1 << 9)
#define ZZ_VF_UV3          (1 << 10)

class RMeshLoader : public Ogre::ManualResourceLoader
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

		pMesh->_setBoundingSphereRadius(0);
		pMesh->_setBounds(AxisAlignedBox(0,0,0,0,0,0));


		pMesh->setSkeletonName( "CHIEF_BONE.ZMD" );

		SubMesh *pSubmesh = readMesh( pMesh, &fs );
		pSubmesh->setMaterialName( "TestMat1" );
		
	}

	static SubMesh* readMesh( Mesh *mesh, DataStream *stream )
	{
		SubMesh *subMesh = mesh->createSubMesh( );
		if( !subMesh ) return nullptr;

		DataStream& fs = *stream;

		// Skip Version ID
		String verid = RFS::readString(fs);

		if( verid != "ZMS0007" && verid != "ZMS0008" ) {
			__asm{ int 3 };
		}

		uint vertex_format;
		fs.read( &vertex_format, sizeof(uint) );

		Vector3 bb_min, bb_max;
		fs.read( &bb_min,sizeof(Vector3) );
		fs.read( &bb_max,sizeof(Vector3) );
		bb_min *= 100.0f;
		bb_max *= 100.0f;

		AxisAlignedBox aabb(bb_min,bb_max);
		mesh->_setBounds(aabb);
		mesh->_setBoundingSphereRadius(sqrt(aabb.getSize().x*aabb.getSize().y*aabb.getSize().z));

		std::vector<ushort> bone_lookup;
		ushort num_bones;
		fs.read( &num_bones, sizeof(ushort) );
		bone_lookup.resize( num_bones );

		for( ushort i = 0; i < num_bones; ++i ) {
			ushort bone_index;
			fs.read( &bone_index, sizeof(ushort) );
			bone_lookup[ i ] = bone_index;
		}


		ushort num_verts;
		fs.read( &num_verts, sizeof(ushort) );

		Ogre::VertexData* data = new Ogre::VertexData();
		subMesh->vertexData = data;
		subMesh->useSharedVertices = false;

		data->vertexStart = 0;
		data->vertexCount = num_verts;

		Ogre::VertexDeclaration* decl = data->vertexDeclaration;

		size_t offset = 0;
		if( vertex_format & ZZ_VF_POSITION ) {
			decl->addElement( 0, offset, VET_FLOAT3, VES_POSITION, 0 );
			offset += VertexElement::getTypeSize( VET_FLOAT3 );
		}
		if( vertex_format & ZZ_VF_NORMAL ) {
			decl->addElement( 0, offset, VET_FLOAT3, VES_NORMAL, 0 );
			offset += VertexElement::getTypeSize( VET_FLOAT3 );
		}

		offset = 0;
		if( vertex_format & ZZ_VF_COLOR ) {
			decl->addElement( 1, offset, VET_COLOUR_ARGB, VES_DIFFUSE, 0 );
			offset += VertexElement::getTypeSize( VET_COLOUR_ARGB );
		}
		if( vertex_format & ZZ_VF_TANGENT ) {
			decl->addElement( 1, offset, VET_FLOAT3, VES_TANGENT, 0 );
			offset += VertexElement::getTypeSize( VET_FLOAT3 );
		}
		if( vertex_format & ZZ_VF_UV0 ) {
			decl->addElement( 1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0 );
			offset += VertexElement::getTypeSize( VET_FLOAT2 );
		}
		if( vertex_format & ZZ_VF_UV1 ) {
			decl->addElement( 1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1 );
			offset += VertexElement::getTypeSize( VET_FLOAT2 );
		}
		if( vertex_format & ZZ_VF_UV2 ) {
			decl->addElement( 1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 2 );
			offset += VertexElement::getTypeSize( VET_FLOAT2 );
		}
		if( vertex_format & ZZ_VF_UV3 ) {
			decl->addElement( 1, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 3 );
			offset += VertexElement::getTypeSize( VET_FLOAT2 );
		}

		uint vsize1 = decl->getVertexSize(0);
		Ogre::HardwareVertexBufferSharedPtr vbuf1 = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
			vsize1,                     // This value is the size of a vertex in memory
			num_verts,                                 // The number of vertices you'll put into this buffer
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY // Properties
			);

		char* vptr1 = (char*)vbuf1->lock( HardwareBuffer::HBL_NO_OVERWRITE );

		for( uint i = 0; i < num_verts; ++i ) {
			const VertexElement *elPosition = decl->findElementBySemantic( VES_POSITION, 0 );
			Vector3* pData = (Vector3*)&vptr1[ vsize1*i + elPosition->getOffset() ];
			fs.read( pData, sizeof(Vector3) );
			(*pData) *= 100.0f;
		}

		if( vertex_format & ZZ_VF_NORMAL ) {
			for( uint i = 0; i < num_verts; ++i ) {
				const VertexElement *elNormal = decl->findElementBySemantic( VES_NORMAL, 0 );
				Vector3* pData = (Vector3*)&vptr1[ vsize1*i + elNormal->getOffset() ];
				fs.read( pData, sizeof(Vector3) );

			}
		}

		vbuf1->unlock( );

		uint vsize2 = decl->getVertexSize(1);
		Ogre::HardwareVertexBufferSharedPtr vbuf2 = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
			vsize2,                     // This value is the size of a vertex in memory
			num_verts,                                 // The number of vertices you'll put into this buffer
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY // Properties
			);

		char* vptr2 = (char*)vbuf2->lock( HardwareBuffer::HBL_NO_OVERWRITE );


		if( vertex_format & ZZ_VF_COLOR ) {
			Vector4 color;
			for( uint i = 0; i < num_verts; ++i ) {
				const VertexElement *elColor = decl->findElementBySemantic( VES_DIFFUSE, 0 );
				uint* pData = (uint*)&vptr1[ vsize1*i + elColor->getOffset() ];
				fs.read( &color, sizeof(Vector4) );
				//*pData = ((uint)(color.w*255.0f)<<24) | ((uint)(color.x*255.0f)<<16) | ((uint)(color.y*255.0f)<<8) | (uint)(color.z*255.0f);
			}
		}

		bool bones_enabled = vertex_format & ZZ_VF_BLEND_INDEX && vertex_format & ZZ_VF_BLEND_WEIGHT;
		if( bones_enabled )
		{
			Ogre::VertexBoneAssignment vba;

			for( uint i = 0; i < num_verts; ++i )
			{
				float blend[4];
				ushort bone[4];

				fs.read( blend, sizeof(float) * 4 );
				fs.read( bone, sizeof(ushort) * 4 );

				vba.vertexIndex = i;
				for( uint j = 0; j < 4; ++j ) {
					vba.boneIndex = bone_lookup[ bone[j] ];
					vba.weight = blend[j];

					subMesh->addBoneAssignment( vba );
				}
			}
		}

		if( vertex_format & ZZ_VF_TANGENT ) {
			for( uint i = 0; i < num_verts; ++i ) {
				const VertexElement *elTangent = decl->findElementBySemantic( VES_TANGENT, 0 );
				Vector3* pData = (Vector3*)&vptr1[ vsize1*i + elTangent->getOffset() ];
				fs.read( pData, sizeof(Vector3) );
			}
		}
		if( vertex_format & ZZ_VF_UV0 ) {
			for( uint i = 0; i < num_verts; ++i ) {
				const VertexElement *elUV = decl->findElementBySemantic( VES_TEXTURE_COORDINATES, 0 );
				Vector2* pData = (Vector2*)&vptr2[ vsize2*i + elUV->getOffset() ];
				fs.read( pData, sizeof(Vector2) );
			}
		}
		if( vertex_format & ZZ_VF_UV1 ) {
			for( uint i = 0; i < num_verts; ++i ) {
				const VertexElement *elUV = decl->findElementBySemantic( VES_TEXTURE_COORDINATES, 1 );
				Vector2* pData = (Vector2*)&vptr2[ vsize2*i + elUV->getOffset() ];
				fs.read( pData, sizeof(Vector2) );
			}
		}
		if( vertex_format & ZZ_VF_UV2 ) {
			for( uint i = 0; i < num_verts; ++i ) {
				const VertexElement *elUV = decl->findElementBySemantic( VES_TEXTURE_COORDINATES, 2 );
				Vector2* pData = (Vector2*)&vptr2[ vsize2*i + elUV->getOffset() ];
				fs.read( pData, sizeof(Vector2) );
			}
		}
		if( vertex_format & ZZ_VF_UV3 ) {
			for( uint i = 0; i < num_verts; ++i ) {
				const VertexElement *elUV = decl->findElementBySemantic( VES_TEXTURE_COORDINATES, 3 );
				Vector2* pData = (Vector2*)&vptr2[ vsize2*i + elUV->getOffset() ];
				fs.read( pData, sizeof(Vector2) );
			}
		}

		vbuf2->unlock( );



		ushort num_faces;
		fs.read( &num_faces, sizeof(ushort) );
		uint num_indices = num_faces * 3;

		Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
			Ogre::HardwareIndexBuffer::IT_16BIT,        // You can use several different value types here
			num_indices,                                  // The number of indices you'll put in that buffer
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY // Properties
			);

		ushort* iptr = (ushort*)ibuf->lock( HardwareBuffer::HBL_NO_OVERWRITE );

		fs.read( iptr, sizeof(ushort) * num_indices );

		/*
		for( uint i = 0 i; < num_indices; ++i ) {
			fs.read( iptr, sizeof(ushort) );
		}
		*/

		ibuf->unlock( );

		Ogre::VertexBufferBinding* bind = data->vertexBufferBinding;
		bind->setBinding( 0, vbuf1 );
		bind->setBinding( 1, vbuf2 );

		Ogre::IndexData* idata = subMesh->indexData;
		idata->indexStart = 0;
		idata->indexCount = num_indices;
		idata->indexBuffer = ibuf;

		if( bones_enabled ) {
			subMesh->_compileBoneAssignments();
		}

		return subMesh;
	}

};

#endif // __RMeshLoader_h_