#ifndef _REQUEST_H_
#define _REQUEST_H_
#include <string>
#include <vector>

using namespace std;

class REQUEST{
public:
	char content[4096];
	string method;
	string part_url;
	string version;
	string port_num;
	string hostname;
	string cache_control;
	string max_age;
  int request_len;
};

#endif