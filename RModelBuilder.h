#pragma once

using namespace Ogre;

namespace ROGRE
{
	class _Part : public ManualResourceLoader
	{
	public:
		_Part( String name, String meshPath, String matName, String skelName = "" )
			: mName(name), mMeshPath(meshPath), mMatName(matName), mSkelName(skelName)
		{
		}

		virtual void prepareResource(Ogre::Resource* resource) override {}
		virtual void loadResource(Ogre::Resource* resource) override {
			Mesh *mesh = (Mesh*)resource;


		}

		const String& getName( ) const { return mName; }

	protected:
		String mName;
		String mMeshPath;
		String mMatName;
		String mSkelName;

	};
	
	class ModelBuilder
	{
	public:
		typedef _Part Part;
		typedef std::vector<Part> PartList;

		int getNumParts( ) const { return mParts.size(); }
		const Part& getPart( int id ) const { return mParts[id]; }
		const PartList& getParts( ) const { return mParts; }

		void addPart( const Part& part ) { mParts.push_back(part); }

	protected:
		PartList mParts;

	};
};

typedef ROGRE::ModelBuilder RModelBuilder;