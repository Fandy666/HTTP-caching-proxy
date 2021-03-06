#ifndef _CACHE_H_
#define _CACHE_H_
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <mutex>
#include <locale>
#include "response.h"
#include "request.h"
#include "log.h"
#define CACHE_CAP 100

using namespace std;
class Cache{
public:
    map<string, RESPONSE> mymap;
    mutex cache_mutex;
    void valid_req(REQUEST &req, RESPONSE &res){
    string new_header(req.content);
    string add_mess;
    if(res.Etag != ""){
      add_mess = "If-None-Match: " + res.Etag + "\r\n";  //?? add \r 
    }
    if(res.if_modified != ""){
      add_mess.append("If-Modified-Since: " + res.if_modified + "\r\n");
    }
    size_t first_end = new_header.find_first_of("\r\n");
    new_header.insert(first_end+2, add_mess);
  }

  int change_req(REQUEST &req, RESPONSE &res, Log &proxy_log, int ID){
    if(res.cache_control == ""){
      proxy_log.print_infile("in cache, valid", ID);
      return 0;          
    }else if(res.cache_control == "no-store"){
      return 1;
    }else if(res.cache_control == "no-cache"){
      valid_req(req, res);
      return 1;
    }else if(res.cache_control == "must-revalidate"){
      proxy_log.print_infile("in cache, requires validation", ID);
      valid_req(req,res);
      return 1;
    }else if(res.cache_control.find_first_not_of("0123456789") == string::npos){
      time_t current_time;
      current_time = time(NULL);
      if(res.expire_time < current_time){
         string message = "in cache, but expired at " + string(asctime(gmtime(&res.expire_time)));
         size_t found4 = message.find("\n");
 	       message = message.substr(0, found4);
         proxy_log.print_infile(message, ID);
         valid_req(req,res);
	       return 1;
      }else if(res.expire!=""){
         string message = "in cache, but expired at " + res.expire;
         proxy_log.print_infile(message, ID);
         valid_req(req,res);
      }else{
          proxy_log.print_infile("in cache, valid", ID);
	        return 0; //expire?
      }
    }else{
      proxy_log.print_infile("in cache, valid", ID);
      return 0;
    }
  }
  
  void pause_res(RESPONSE &res){
      string temp;
    if (res.content_len > 2048){
      temp = string(res.content.begin(), res.content.begin() + 2048);
    }else{
      temp = string(res.content.data());
    }
    //cache control
    int control = temp.find("Cache-Control:");
    if(control != string::npos){
      res.cache_control = temp.substr(control+15);
      //cout<<"full control "<<res.cache_control<<endl;
      int len_end=res.cache_control.find("\r\n");
      //cout<<len_end<<endl;
      res.cache_control = res.cache_control.substr(0,len_end+2);
      //cout<<"cccc"<<res.cache_control<<endl;
    }else{
      res.cache_control = "";
    }
    //last modified
    int last_modified = temp.find("Last-Modified:");
    if(last_modified != string::npos){
      res.if_modified = temp.substr(last_modified+15);
      int len_end1=res.if_modified.find_first_of("\r\n");
      res.if_modified = res.if_modified.substr(0,len_end1);
    }else{
      res.if_modified = "";
    }
    //Etag
    int etag = temp.find("ETag:");
    if(etag != string::npos){
      res.Etag = temp.substr(etag+6);
      int len_end2=res.Etag.find_first_of("\r\n");
      res.Etag = res.Etag.substr(0,len_end2);
    }else{
      res.Etag = "";
    }
    //date
    int date = temp.find("Date:");
    if(date != string::npos){
      res.date = temp.substr(date+6);
      int len_end3=res.date.find_first_of("\r\n");
      res.date = res.date.substr(0,len_end3);
    }else{
      res.date = "";
    }
    //expire
    int expire1 = temp.find("Expires:");
    if(expire1 != string::npos){
      res.expire = temp.substr(expire1+9);
      int len_end4=res.expire.find_first_of("\r\n");
      res.expire = res.expire.substr(0,len_end4);
    }else{
      res.expire = "";
    }


  }
  void cache_control(RESPONSE &res){
    if(res.cache_control.length()==0){
      return;
    }
    //no store
    //cout<<"c c: "<<res.cache_control<<endl;
    if(res.cache_control.find("no-store") != string::npos || res.cache_control.find("private") != string::npos){  
      res.cache_control = "no-store";
      //cout<<"no-store !!!"<<endl;
    }else if(res.cache_control.find("no-cache") != string::npos){
      res.cache_control = "no-cache";
    }else if(res.cache_control.find("must-revalidate") != string::npos){
      res.cache_control = "must-revalidate";
    }else if(res.cache_control.find("max-age=") != string::npos){
      int ma = res.cache_control.find("max-age=");
      res.cache_control = res.cache_control.substr(ma + 8);
      //cout<<"max: "<<res.cache_control<<endl;
      //
      locale loc;
      string temp2;
      if (isdigit(res.cache_control[0], loc)){
        stringstream(res.cache_control) >> temp2;
      }
      res.cache_control = temp2;
     // int ma_end1=res.cache_control.find_first_of("/r/n");
      //res.cache_control = res.cache_control.substr(0, ma_end1);
      //cout<<"max age: "<<res.cache_control<<endl;
      time_t current_time;
      current_time = time(NULL);
      //cout<<"current time"<<current_time<<endl; //test
      res.expire_time = (time_t)atoll(res.cache_control.c_str()) + current_time;
      //cout << "res.expire_time" << res.expire_time <<endl; // test
    }else if(res.expire != ""){
      //cout<<"expire_time: "<<res.expire<<endl;
      char* ex_time = (char*)res.expire.c_str();
      struct tm tm;
      strftime(ex_time, sizeof(ex_time), "%a, %d %B %Y %H:%M:%S %Z", &tm);
      res.expire_time = mktime(&tm);
      //cout<<"expire_time2: "<<res.expire<<endl;
    }else{}
  }
  
  void save_cache(REQUEST &req, RESPONSE &res, Log &proxy_log, int ID){
    cache_mutex.lock();
    if(mymap.size() >= CACHE_CAP){
      srand(time(0));
      int position = rand()%CACHE_CAP;
      map<string, RESPONSE>::iterator m = mymap.begin();
      for(int j = 0; j<position; j++){
	m++;
      }
      mymap.erase(m);
    }
    pause_res(res);
    cache_control(res);
    if(res.cache_control == "no-store" || req.cache_control == "no-store"){
      proxy_log.print_infile("not cacheable because Cache-Control: no-store", ID);
      cache_mutex.unlock();
      return;
    }else if(res.cache_control == "must-revalidate" || res.cache_control == "no-cache"){
      proxy_log.print_infile("cached, but requires re-validation", ID);
    }else if(res.cache_control.find_first_not_of("0123456789") == string::npos && res.cache_control != ""){
        // cout<<"ex CC: "<<res.cache_control<<endl;
         string message = "cached, expires at " + string(asctime(gmtime(&res.expire_time)));
         size_t found5 = message.find("\n");
 	       message = message.substr(0, found5);
         proxy_log.print_infile(message, ID);
    }else if(res.expire != ""){
         string message = "cached, expires at " + res.expire;
         proxy_log.print_infile(message, ID);
    }else{
         string message = "cached.";
         proxy_log.print_infile(message, ID);
    }
    map<string, RESPONSE>::iterator it;
    it = mymap.find(req.part_url);
    if(it == mymap.end()){
      mymap.insert(make_pair(req.part_url, res));
    }else{
      it->second = res;
    }
    cache_mutex.unlock();
  }
  int check_cache(REQUEST &req, RESPONSE &res, Log &proxy_log, int ID){
    cache_mutex.lock();
    map<string, RESPONSE>::iterator it;
    it = mymap.find(req.part_url);
    if(it == mymap.end()){  //not in cache
      proxy_log.print_infile("not in cache", ID);
      cache_mutex.unlock();
    return 0;
    }
    //cout<<"find url "<<req.part_url<<endl;
    res=it->second;
    if(change_req(req, res, proxy_log, ID)){
      cache_mutex.unlock();
      return 0;
    }
    cache_mutex.unlock();
    return 1;
  }







};


#endif
