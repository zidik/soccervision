#include <boost/test/unit_test.hpp>
#include "Configuration.h"

BOOST_AUTO_TEST_SUITE(COnfigurationTests)
BOOST_AUTO_TEST_CASE(Configuration_PropertyTreeMerging)
{
	boost::property_tree::ptree tree1;
	tree1.put("one.val1", 1);
	tree1.put("one.val2", 2);
	tree1.put("other.val1", 3);
	boost::property_tree::ptree tree2;
	tree1.put("one.val2", 4);
	tree1.put("one.val3", 5);

	PropertyTreeMerger merger;
	merger.set_ptree(tree1);
	merger.update_ptree(tree2);
	BOOST_TEST(merger.get_ptree().get<int>("one.val1") == 1);
	BOOST_TEST(merger.get_ptree().get<int>("one.val2") == 4); // This one should be overwritten
	BOOST_TEST(merger.get_ptree().get<int>("one.val3") == 5);
	BOOST_TEST(merger.get_ptree().get<int>("other.val1") == 3);
}
BOOST_AUTO_TEST_SUITE_END()