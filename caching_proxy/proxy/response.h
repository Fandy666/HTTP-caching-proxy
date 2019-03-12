#ifndef _RESPONSE_H_
#define _RESPONSE_H_
#include <string>
#include <vector>
#include <ctime>
using namespace std;

class RESPONSE{
public:
	vector<char> content;
	string cache_control;
	time_t expire_time ;
  string status;
  string response_mess;
	int content_len;
  string date;
  string Etag;
  string if_modified;
  string expire;
};

#endif