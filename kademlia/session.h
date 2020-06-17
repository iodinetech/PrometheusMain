#include <memory>
#include <boost/asio.hpp>
#include <string>
#include <cstdlib>
#include <iostream>
#include <tools/Logger.h>
#include "common.h"

#ifndef _SESSION_H_
#define _SESSION_H_
namespace Prometheus
{

class Session : public std::enable_shared_from_this<Session> {
public:
  typedef bi::tcp TCP;
  typedef boost::system::error_code Error; 
  typedef std::shared_ptr<Session> Pointer;
  typedef File_info::Size_type Size_type;
  
  static void print_asio_error(const Error& error) { std::cerr << error.message() << "\n";}
  
  static Pointer create(ba::io_service& io) { return Pointer(new Session(io));}
  
  TCP::socket& socket() { return socket_; }
  
  ~Session() 
  {
    if (fp_) fclose(fp_);
    clock_ = clock() - clock_;
    Size_type bytes_writen = total_bytes_writen_;
    if (clock_ == 0) clock_ = 1;
    double speed = bytes_writen * (CLOCKS_PER_SEC / 1024.0 / 1024.0) / clock_ ;
    std::cout << "cost time: " << clock_ / (double) CLOCKS_PER_SEC << " s  " 
       << "bytes_writen: " << bytes_writen << " bytes\n"
       << "speed: " <<  speed << " MB/s\n\n"; 
  }
  
  void start()
  {
    clock_ = clock();
    socket_.async_receive(
      ba::buffer(reinterpret_cast<char*>(&file_info_), sizeof(file_info_)),
      std::bind(&Session::handle_header, shared_from_this(),std::placeholders::_1,std::placeholders::_2)); 
  }
  
private:
  Session(ba::io_service& io) : socket_(io), fp_(NULL), total_bytes_writen_(0) { }
  
  void handle_header(const Error& error,size_t len) 
  {
    if (error) return print_asio_error(error);
    size_t filename_size = file_info_.filename_size;
    if (filename_size > k_buffer_size) {
      std::cerr << "Path name is too long!\n";
      return;
    }
    //得用async_read, 不能用async_read_some，防止路径名超长时，一次接收不完
    ba::async_read(socket_, ba::buffer(buffer_, file_info_.filename_size),
      std::bind(&Session::handle_file, shared_from_this(), std::placeholders::_1,std::placeholders::_2)); 
  }
  
  void handle_file(const Error& error, size_t len)
  {
    if (error) return print_asio_error(error);
    const char *basename = buffer_ + file_info_.filename_size - 1;
    while (basename >= buffer_ && (*basename != '\\' && *basename != '/')) --basename;
    ++basename;
    
    std::cout << "Open file: " << basename << " (" << buffer_ << ")\n";
    
    fp_ = fopen(basename, "wb");
    if (fp_ == NULL) {
      std::cerr << "Failed to open file to write\n";
      return;
    }
    receive_file_content();
  }
  
  void receive_file_content()
  {
    socket_.async_receive(ba::buffer(buffer_, k_buffer_size), 
        std::bind(&Session::handle_write, shared_from_this(), std::placeholders::_1,
        std::placeholders::_2)); 
  }
  
  void handle_write(const Error& error, size_t bytes_transferred)
  {
    if (error) {
      if (error != boost::asio::error::eof) return print_asio_error(error);
      Size_type filesize = file_info_.filesize;
      if (total_bytes_writen_ != filesize) 
          std::cerr <<  "Filesize not match! " << total_bytes_writen_ 
            << "/" << filesize << "\n";
      return;     
    }  
    total_bytes_writen_ += fwrite(buffer_, 1, bytes_transferred, fp_);
    receive_file_content();
  }
  
  clock_t clock_;
  bi::tcp::socket socket_;
  FILE *fp_;
  File_info file_info_;
  Size_type total_bytes_writen_;
  static const unsigned k_buffer_size = 1024 * 32;
  char buffer_[k_buffer_size];
};

} // namespace Prometheus
#endif