#pragma once

using namespace Ogre;

class NpcModel
{
	friend class NpcModelManager;

protected:
	class Section
	{
		friend class NpcModelManager;

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

	NpcModel( )
		: mRootNode(nullptr), mRootEntity(nullptr)
	{
	}

public:
	~NpcModel( )
	{
	}

	SceneNode* getSceneNode( ) const { return mRootNode; }

protected:
	SceneNode *mRootNode;
	Entity *mRootEntity;
	std::vector<Section> mSections;

};