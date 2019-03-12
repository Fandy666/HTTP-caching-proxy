#ifndef __LOG_H_
#define __LOG_H_
#include <fstream>
#include <iostream>
#include <mutex>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>
using namespace std;

class Log{
  ofstream ss;
  string filename;
  struct sockaddr_in host;
  mutex log_mutex;
  mutex host_mutex;
  
public:
//initial
 Log(string FN):filename(FN){
   ss.open(filename.c_str(), ofstream::trunc);
   if(!ss.is_open()){
     perror("can not open file");
   }
 }
 void print_infile(string content, int ID){
   log_mutex.lock();
   ss<<ID<<": "<<content<<endl;
   log_mutex.unlock();
 }
 void get_ip(struct sockaddr_in socket_addr){
   host_mutex.lock();
   host = socket_addr;
   host_mutex.unlock();
 }
 void new_req(REQUEST &req, int ID){
   host_mutex.lock();
   char* ip_addr = inet_ntoa(host.sin_addr);
   host_mutex.unlock();
   time_t current_time;
   struct tm* tt;
   log_mutex.lock();
   time(&current_time);
   tt = localtime(&current_time);
   ss<<ID<<": \""<<req.method<<" "<<req.part_url<<" "<<req.version<<"\" from "<<ip_addr<<" @ "<<asctime(tt);
   log_mutex.unlock();
}
 

  
};


#endif
