#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE asio_buffer_test
#include <boost/test/unit_test.hpp>

#include <boost/asio/streambuf.hpp>
#include <ostream>
#include <sstream>
#include <string>


BOOST_AUTO_TEST_CASE(consumption_test)
{
  boost::asio::streambuf b;
  std::ostream os(&b);
  os << "Hello, World!\nThis is me.";
  b.consume(7);
  std::string str;
  std::istream is(&b);
  is >> str;
  BOOST_TEST(str == "World!");
  std::stringbuf sb;
  is >> std::skipws >> &sb;
  BOOST_TEST(sb.str() == "This is me.");
}
