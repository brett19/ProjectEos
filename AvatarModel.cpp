#include "StdAfx.h"
#include "AvatarModel.h"
#include "AvatarModelManager.h"

AvatarModel::AvatarModel( AvatarModelManager *parent, SceneManager *scene )
	: mParent(parent), mScene(scene), mRootNode(nullptr), mRootEntity(nullptr)
{
	mGender = AG_MALE;
	mItemIds[ IS_FACE ] = 1;
	mItemIds[ IS_HAIR ] = 0;
	mItemIds[ IS_CAP ] = 0;
	mItemIds[ IS_BODY ] = 0;
	mItemIds[ IS_ARMS ] = 0;
	mItemIds[ IS_FOOT ] = 0;
	mItemIds[ IS_FACEITEM ] = 0;
	mItemIds[ IS_BACK ] = 233;
	mItemIds[ IS_LWEAPON ] = 0;
	mItemIds[ IS_RWEAPON ] = 0;
}

AvatarModel::~AvatarModel( )
{
}

void AvatarModel::setGender( AvatarGender gender )
{
	if( gender == mGender ) return;
	mGender = gender;
	mParent->updateAllParts( this );
}

void AvatarModel::setItem( ItemSlot itemSlot, ItemId itemId )
{

}