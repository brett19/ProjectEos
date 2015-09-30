#include "StdAfx.h"
#include "RZscHelper.h"

namespace ROGRE {
	namespace ZSC
	{
		void BuildMaterial( Ogre::Material *mat, const ROSE::ZSC::_Material& zscmat )
		{
			Technique *tech = mat->getTechnique(0);
			Pass *pass = tech->getPass( 0 );

			if( zscmat.isAlpha() ) {
				pass->setSceneBlending( SBT_TRANSPARENT_ALPHA );
				
				if( zscmat.isAlphaTest() ) {
					pass->setAlphaRejectSettings( CMPF_GREATER, zscmat.getAlphaRef() );
				}
			} else {
				pass->setSceneBlending( SBT_REPLACE );
			}

			if( zscmat.isTwoSided() ) {
				pass->setCullingMode( CULL_NONE );
			}
			pass->setDepthCheckEnabled( zscmat.isZTest() );
			pass->setDepthWriteEnabled( zscmat.isZWrite() );

			mat->setReceiveShadows( false );

			//pass->setLightingEnabled( false );

			TextureUnitState *unit = pass->createTextureUnitState( );
			unit->setTextureName( zscmat.getTexPath() );
		}
	}
}