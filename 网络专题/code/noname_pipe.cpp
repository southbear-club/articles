/**
 * @file noname_pipe.cpp
 * @author wotsen (astralrovers@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2020-10-25
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <time.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

typedef int pipe_t;

/**
 * @brief pipe管道
 * @details 半双工，只能用于父子进程，一端只能读，一端只能写
 */
class Pipe
{
public:
	Pipe();
	~Pipe();

	// 读取
	int read(void* data, const size_t len, const time_t wait=0);
	// 写入
	int write(const void* data, const size_t len, const time_t wait=0);
	// 关闭全部
	void close(void);

	// 关闭读端
	void close_r(void);
	// 关闭写端
	void close_w(void);

	// 设置管道容量
	bool set_size(const size_t &size);

	/**
	 * @brief 设置阻塞
	 * 
	 * @param en true-阻塞，false-非阻塞
	 * @return true 成功
	 * @return false 失败
	 */
	bool set_block(const bool en);

	// 获取读端描述符
	pipe_t read_fd(void);
	// 获取写端描述符
	pipe_t write_fd(void);
private:
	pipe_t pipe_[2];
};

bool set_fd_block(const int fd, const bool en) {
	int flag = fcntl(fd, F_GETFL, 0);
	if (flag < 0) { return false; }

	if (en) { ///< 阻塞
		flag &= ~O_NONBLOCK;
	} else { ///< 非阻塞
		flag |= O_NONBLOCK;
	}

	return fcntl(fd, F_SETFL, flag) == 0;
}

Pipe::Pipe() {
	pipe_[0] = -1;
	pipe_[1] = -1;

    // 创建管道
	if (pipe(pipe_) < 0) {
		throw std::runtime_error("open pipe error");
	}
}

Pipe::~Pipe(void) {
	close();
}

void Pipe::close_r(void)
{
	if (pipe_[0] >= 0) {
        // 关闭读端
		::close(pipe_[0]);
	}

	pipe_[0] = -1;
}

void Pipe::close_w(void)
{
	if (pipe_[1] >= 0) {
        // 关闭写端
		::close(pipe_[1]);
	}

	pipe_[1] = -1;
}

void Pipe::close(void) {
	close_w();
	close_r();
}

bool Pipe::set_size(const size_t &size) {
	if (pipe_[0] < 0 && pipe_[1] < 0) { return false; }

    // 只对一个有效的描述符进行操作
    if (pipe_[0] >= 0) {
	    return 0 == fcntl(pipe_[0], F_SETPIPE_SZ, size);
    } else if (pipe_[1] >= 0) {
	    return 0 == fcntl(pipe_[1], F_SETPIPE_SZ, size);
    } else {
        return false;
    }
}

bool Pipe::set_block(const bool en) {
    // 只对一个有效的描述符进行操作
	if (pipe_[0] >= 0) {
		if (set_fd_block(pipe_[0], en)) {
			return true;
		}
		return false;
	}

	if (pipe_[1] >= 0) {
		if (set_fd_block(pipe_[1], en)) {
			return true;
		}
		return false;
	}

	return false;
}

pipe_t Pipe::read_fd(void) {
	return pipe_[0];
}

pipe_t Pipe::write_fd(void) {
	return pipe_[1];
}

int Pipe::write(const void* data, const size_t len, const time_t wait) {
	if (pipe_[1] < 0) { return -1; }

	return ::write(pipe_[1], data, len);
}

int Pipe::read(void* data, const size_t len, const time_t wait) {
	if (pipe_[0] < 0) { return -1; }

	return ::read(pipe_[0], data, len);
}

int main(void) {
    Pipe p;

	char buf[512] = "hello world";

    if (fork() > 0) {
        p.close_r();
	    printf("write len %d\n", p.write(buf, strlen(buf)));
    } else {
        p.close_w();
	    memset(buf, 0, sizeof(buf));
	    p.read(buf, 512);
	    printf("read : %s\n", buf);
    }

    printf("proccess exit.\n");

    return 0;
}