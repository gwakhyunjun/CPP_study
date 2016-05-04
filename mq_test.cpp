#include <iostream>
#include <chrono>
#include <thread>

#include "mq.hpp"

using namespace std::chrono_literals;

struct HEAD
{
	int type;
};

struct Done : public HEAD { Done() { type = 0; } };

struct R1 : public HEAD
{
	R1() {
		type = 1;
		a = 1;
	}
	int a;
};

struct R2 : public HEAD
{
	R2() {
		type = 2;
		a = 2;
		b = 2;
	}
	int a;
	int b;
};

struct R3 : public HEAD
{
	R3() {
		type = 3;
		a = 3;
		b = 3;
		c = 3;
	}
	int a;
	int b;
	int c;
};

struct S1 : public HEAD
{
	S1() {
		type = 4;
		d = 4;
	}
	int d;
};

struct S2 : public HEAD
{
	S2() { 
		type = 5;
		e = 5;
		f = 5;
	}
	int e;
	int f;
};


void RcvR1(std::shared_ptr<mq::message_base> v)
{
	if (MQ_MATCH(R1, v, r)) {		
		std::cout << "RcvR1 a = " << r->contents.a << std::endl;
	}
	else{
		std::cout << "RcvR1 error not match type " << std::endl;
	}	
}

void RcvR2(std::shared_ptr<mq::message_base> v)
{
	if (MQ_MATCH(R2, v, r)) {
		std::cout << "RcvR2 a = " << r->contents.a << " b = " << r->contents.b << std::endl;
	}
	else {
		std::cout << "RcvR2 error not match type " << std::endl;
	}	
}

void RcvR3(std::shared_ptr<mq::message_base> v)
{
	if (MQ_MATCH(R3, v, r)) {
		std::cout << "RcvR3 a = " << r->contents.a << " b = " << r->contents.b << " c = " << r->contents.c << std::endl;
	}
	else {
		std::cout << "RcvR3 error not match type " << std::endl;
	}	
}

void SndS1(std::shared_ptr<mq::message_base> v)
{
	if (MQ_MATCH(S1, v, r)) {
		std::cout << "SndS1 d = " << r->contents.d << std::endl;
	}
	else {
		std::cout << "SndS1 error not match type " << std::endl;
	}	
}

void SndS2(std::shared_ptr<mq::message_base> v)
{
	if (MQ_MATCH(S2, v, r)) {
		std::cout << "SndS2 e = " << r->contents.e << " f = " << r->contents.f << std::endl;
	}
	else {
		std::cout << "SndS2 error not match type " << std::endl;
	}
}

void End(std::shared_ptr<mq::message_base> v)
{
	if (MQ_MATCH(Done, v, r)) {
		std::cout << "end" << std::endl;
	}
	else {
		std::cout << "End error not match type " << std::endl;
	}
}

void(*hanlde[6]) (std::shared_ptr<mq::message_base>) = { End, RcvR1, RcvR2, RcvR3, SndS1, SndS2 };


static void producer_routine_1(mq::queue& q)
{
	for (std::size_t i = 0; i < 6; ++i) {
		if (i % 2 == 0) {
			R1 m;
			q.send(m);
		}
		else {
			R2 m;
			q.send(m);
		}
	}
}

static void producer_routine_2(mq::queue& q)
{
	for (std::size_t i = 0; i < 6; ++i) {
		if (i % 2 == 0) {
			R3 m;
			q.send(m);
		}
		else {
			S1 m;
			q.send(m);
		}
	}
	
	//q.send(Done());
}

static void producer_routine_3(mq::queue& q)
{
	for (std::size_t i = 0; i < 6; ++i) {
		if (i % 2 == 0) {
			S2 m;
			q.send(m);
		}
		else {
			R1 m;
			q.send(m);
		}
	}

	q.send(Done());
}


static void consumer_routine(mq::queue& q)
{
	std::cout << "Hello waiter producer_routine_1 " << std::endl;
	auto start = std::chrono::high_resolution_clock::now();

	for (;;) {
		auto m = q.receive();
		if (m->type == 0)
		{
			std::cout << "done" << std::endl;
			break;
		}
		else
		{
			///< 함수포인터 배열 사용.
			hanlde[m->type](m);
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	std::cout << "Waited producer_routine_1 " << elapsed.count() << " ms\n";
}


int main(int argc, char* argv[])
{
	mq::queue q;

	std::thread p1(producer_routine_1, std::ref(q));
	std::thread p2(producer_routine_2, std::ref(q));
	std::thread p3(producer_routine_3, std::ref(q));
	std::thread c(consumer_routine, std::ref(q));

	p1.join();
	p2.join();
	p3.join();
	c.join();

	return 0;
}