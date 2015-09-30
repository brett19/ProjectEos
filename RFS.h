#ifndef __RFS_H_
#define __RFS_H_

namespace RFS
{
	static char gTmpString[ 2048 ];

	static inline Ogre::String readString( Ogre::DataStream& stream )
	{
		Ogre::String output;
		char this_char;
		
		char *str_iter = gTmpString;
		while( true ) {
			stream.read( &this_char, 1 );
			*str_iter++ = this_char;
			if( !this_char ) break;
		}
		output.assign( gTmpString );
		return output;
	}

	static inline Ogre::String readString( Ogre::DataStream& stream, size_t strLength )
	{
		Ogre::String output;
		output.reserve( strLength );
		char this_char;
		for( size_t i = 0; i < strLength; ++i ) {
			stream.read( &this_char, 1 );
			output.push_back( this_char );
		}
		return output;
	}

	template<typename T>
	static inline Ogre::String readString( Ogre::DataStream& stream )
	{
		Ogre::String output;
		char this_char;
		T inlen;
		stream.read( &inlen, sizeof(T) );
		output.reserve( inlen );
		for( T i = 0; i < inlen; ++i ) {
			stream.read( &this_char, 1 );
			if( this_char == 0 ) break;
			output.push_back( this_char );
		}
		return output;
	}

	template<typename T>
	static inline bool readBool( Ogre::DataStream& stream )
	{
		T value;
		stream.read( &value, sizeof(T) );
		return ( value != 0 );
	}

};

#endif // __RFS_H_