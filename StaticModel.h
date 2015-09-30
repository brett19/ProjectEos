#pragma once

using namespace Ogre;

class StaticModel
{
	friend class StaticModelManager;

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

	StaticModel( )
		: mRootNode(nullptr)
	{
	}

public:
	~StaticModel( )
	{
	}

	SceneNode* getSceneNode( ) const { return mRootNode; }

protected:
	SceneNode *mRootNode;
	std::vector<Part> mParts;

};