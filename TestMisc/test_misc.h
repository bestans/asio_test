#pragma once
#include <memory>

void TestThreadPool();
void TestEchoServerMultiple(int argc, char* argv[]);
void TestThreadPoolOne2One(int argc, char* argv[]);

//asio example
void test_priority_scheduler();
int test_timer();
void test_async_1();
void test_async();
void test_pipline();
void test_dataqueue();