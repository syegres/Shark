#define BOOST_TEST_MODULE ML_Download
#include <boost/test/unit_test.hpp>

#include <shark/Data/Download.h>
#include <iostream>

using namespace shark;


bool verifyConnection()
{
	detail::Socket socket("mldata.org", 80);
	return socket.connected();
}


BOOST_AUTO_TEST_SUITE(Data_Download)

BOOST_AUTO_TEST_CASE(Data_Download_URL)
{
	if (! verifyConnection())
	{
		std::cout << "cannot reach mldata.org server; skipping data download test" << std::endl;
		return;
	}

	// test the download of a data file from a given URL
	LabeledData<RealVector, unsigned int> dataset;
	try
	{
		downloadSparseData(dataset, "http://mldata.org/repository/data/download/libsvm/iris/");
		BOOST_CHECK_EQUAL(dataset.numberOfElements(), 150);
		BOOST_CHECK_EQUAL(inputDimension(dataset), 4);
		BOOST_CHECK_EQUAL(numberOfClasses(dataset), 3);
	}
	catch(std::runtime_error err)
	{
		// Don't count the HTTP 500 error as an actual error...
		std::string msg500("[download] failed with HTTP status 500 Internal Server Error");
		BOOST_CHECK_MESSAGE(err.what() == msg500,
		                    "Got exception " + std::string(err.what()));
	}
}

BOOST_AUTO_TEST_CASE(Data_Download_MLData)
{
	if (! verifyConnection())
	{
		std::cout << "cannot reach mldata.org server; skipping data download test" << std::endl;
		return;
	}

	// test the download of a data file from openml.org given a data set name
	LabeledData<RealVector, unsigned int> dataset;
	try
	{
		downloadFromMLData(dataset, "iris");
		BOOST_CHECK_EQUAL(dataset.numberOfElements(), 150);
		BOOST_CHECK_EQUAL(inputDimension(dataset), 4);
		BOOST_CHECK_EQUAL(numberOfClasses(dataset), 3);
	}
	catch(std::runtime_error err)
	{
		// Don't count the HTTP 500 error as an actual error...
		std::string msg500("[download] failed with HTTP status 500 Internal Server Error");
		BOOST_CHECK_MESSAGE(err.what() == msg500,
		                    "Got exception " + std::string(err.what()));
	}
}

BOOST_AUTO_TEST_CASE(Data_Download_Url_splitter)
{
	using std::make_tuple;
	std::vector<std::tuple<std::string, std::string, std::string>> data{
		make_tuple("http://mldata.org/repository/data/download/libsvm/iris/",
		           "mldata.org", 
		           "/repository/data/download/libsvm/iris/"),
			make_tuple("http://dr.dk/nyhederne", 
			           "dr.dk", 
			           "/nyhederne"),
			make_tuple("google.com/en?sdfsdfsfs", 
			           "google.com", 
			           "/en?sdfsdfsfs"),
			make_tuple("https://secret.website.com/noaccess", 
			           "secret.website.com", 
			           "/noaccess"),
			make_tuple("http://alexandra.dk", 
			           "alexandra.dk", 
			           "/"),
			make_tuple("alexandra.dk", 
			           "alexandra.dk", 
			           "/"),
			make_tuple("alexandra.dk/about/hello",
			           "alexandra.dk", 
			           "/about/hello"),
			make_tuple("alexandra.dk/", 
			           "alexandra.dk",
			           "/"),
			make_tuple("http://alexandra.dk/",
			           "alexandra.dk",
			           "/")
			};
	for(auto & tc : data)
	{
		std::string d, r;
		std::tie(d, r) = splitUrl(std::get<0>(tc));
		BOOST_CHECK_EQUAL(d, std::get<1>(tc));
		BOOST_CHECK_EQUAL(r, std::get<2>(tc));
	}
}

BOOST_AUTO_TEST_SUITE_END()
