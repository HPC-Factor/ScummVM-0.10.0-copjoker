#include <cxxtest/TestSuite.h>

#include "common/stdafx.h"
#include "common/str.h"

class StringTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear( void )
	{
		Common::String str("test");
		TS_ASSERT( !str.empty() );
		str.clear();
		TS_ASSERT( str.empty() );
	}

	void test_lastChar( void )
	{
		Common::String str;
		TS_ASSERT_EQUALS( str.lastChar(), '\0' );
		str = "test";
		TS_ASSERT_EQUALS( str.lastChar(), 't' );
		Common::String str2("bar");
		TS_ASSERT_EQUALS( str2.lastChar(), 'r' );
	}

	void test_concat1( void )
	{
		Common::String str("foo");
		Common::String str2("bar");
		str += str2;
		TS_ASSERT_EQUALS( str, "foobar" );
		TS_ASSERT_EQUALS( str2, "bar" );
	}

	void test_concat2( void )
	{
		Common::String str("foo");
		str += "bar";
		TS_ASSERT_EQUALS( str, "foobar" );
	}

	void test_concat3( void )
	{
		Common::String str("foo");
		str += 'X';
		TS_ASSERT_EQUALS( str, "fooX" );
	}

	void test_refCount( void )
	{
		Common::String foo1("foo");
		Common::String foo2("foo");
		Common::String foo3(foo2);
		foo3 += 'X';
		TS_ASSERT_EQUALS( foo2, foo1 );
		TS_ASSERT_EQUALS( foo2, "foo" );
		TS_ASSERT_EQUALS( foo3, "foo""X" );
	}

	void test_refCount2( void )
	{
		Common::String foo1("fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		Common::String foo2("fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		Common::String foo3(foo2);
		foo3 += 'X';
		TS_ASSERT_EQUALS( foo2, foo1 );
		TS_ASSERT_EQUALS( foo2, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd" );
		TS_ASSERT_EQUALS( foo3, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd""X" );
	}

	void test_refCount3( void )
	{
		Common::String foo1("0123456789abcdefghijk");
		Common::String foo2("0123456789abcdefghijk");
		Common::String foo3(foo2);
		foo3 += "0123456789abcdefghijk";
		TS_ASSERT_EQUALS( foo2, foo1 );
		TS_ASSERT_EQUALS( foo2, "0123456789abcdefghijk" );
		TS_ASSERT_EQUALS( foo3, "0123456789abcdefghijk""0123456789abcdefghijk" );
	}

	void test_refCount4( void )
	{
		Common::String foo1("fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		Common::String foo2("fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		Common::String foo3(foo2);
		foo3 += "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd";
		TS_ASSERT_EQUALS( foo2, foo1 );
		TS_ASSERT_EQUALS( foo2, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd" );
		TS_ASSERT_EQUALS( foo3, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd""fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd" );
	}

	void test_hasPrefix( void )
	{
		Common::String str("this/is/a/test, haha");
		TS_ASSERT_EQUALS( str.hasPrefix(""), true );
		TS_ASSERT_EQUALS( str.hasPrefix("this"), true );
		TS_ASSERT_EQUALS( str.hasPrefix("thit"), false );
		TS_ASSERT_EQUALS( str.hasPrefix("foo"), false );
	}

	void test_hasSuffix( void )
	{
		Common::String str("this/is/a/test, haha");
		TS_ASSERT_EQUALS( str.hasSuffix(""), true );
		TS_ASSERT_EQUALS( str.hasSuffix("haha"), true );
		TS_ASSERT_EQUALS( str.hasSuffix("hahb"), false );
		TS_ASSERT_EQUALS( str.hasSuffix("hahah"), false );
	}

	void test_contains( void )
	{
		Common::String str("this/is/a/test, haha");
		TS_ASSERT_EQUALS( str.contains(""), true );
		TS_ASSERT_EQUALS( str.contains("haha"), true );
		TS_ASSERT_EQUALS( str.contains("hahb"), false );
		TS_ASSERT_EQUALS( str.contains("test"), true );
	}

	void test_toLowercase( void )
	{
		Common::String str("Test it, NOW! 42");
		str.toLowercase();
		TS_ASSERT_EQUALS( str, "test it, now! 42" );
	}

	void test_toUppercase( void )
	{
		Common::String str("Test it, NOW! 42");
		str.toUppercase();
		TS_ASSERT_EQUALS( str, "TEST IT, NOW! 42" );
	}
};
