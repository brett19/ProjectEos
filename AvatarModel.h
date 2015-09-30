#pragma once

using namespace Ogre;

class AvatarModelManager;

typedef int ItemId;

class AvatarModel
{
	friend class AvatarModelManager;

protected:
	class Section
	{
		friend class AvatarModelManager;

	public:
		Section( )
		{
		}

		~Section( )
		{
		}

	protected:
		class Part
		{
		public:
			Part( SceneNode *node, Entity *entity )
				: mNode(node), mEntity(entity)
			{
			}

			~Part( )
			{
			}

			SceneNode* getSceneNode( ) const { return mNode; }
			Entity* getEntity( ) const { return mEntity; }

		protected:
			SceneNode *mNode;
			Entity *mEntity;
		};

		std::vector<Part> mParts;
	};

public:
	enum ItemSlot : ushort
	{
		IS_FACE = 0,
		IS_HAIR = 1,
		IS_CAP = 2,
		IS_BODY = 3,
		IS_ARMS = 4,
		IS_FOOT = 5,
		IS_FACEITEM = 6,
		IS_BACK = 7,
		IS_LWEAPON = 8,
		IS_RWEAPON = 9,

		IS_MAX = 10
	};

	enum AvatarGender : ushort
	{
		AG_MALE = 0,
		AG_FEMALE = 1
	};

	AvatarModel( AvatarModelManager *parent, SceneManager *scene );
	~AvatarModel( );

	void setGender( AvatarGender gender );
	void setItem( ItemSlot itemSlot, ItemId itemId );

	SceneNode* getSceneNode( ) const { return mRootNode; }

protected:
	SceneNode *mRootNode;
	AvatarGender mGender;
	AvatarModelManager *mParent;
	SceneManager *mScene;
	Entity *mRootEntity;
	ItemId mItemIds[IS_MAX];
	std::vector<Section> mSections;

};