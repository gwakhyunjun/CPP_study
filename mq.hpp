#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

// test
namespace mq {
	struct message_base
	{	
		int type;		
		virtual ~message_base() {}
	};

	template<typename Message>
	struct wraped_message : message_base
	{
		Message contents;

		explicit wraped_message(Message const& x) :contents(x) { type = x.type; }
	};

	class queue
	{
		std::mutex m_;
		std::condition_variable c_;
		std::queue<std::shared_ptr<message_base>> q_;

	public:
		queue() : m_(), c_(), q_() {}

		template<typename T>
		void send(T const& msg)
		{
			std::lock_guard<std::mutex> lk(m_);
			q_.push(std::make_shared<wraped_message<T> >(msg));
			c_.notify_all();
		}

		std::shared_ptr<message_base> receive()
		{
			std::unique_lock<std::mutex> lk(m_);
			c_.wait(lk, [&] {return !q_.empty();});
			auto res = q_.front();
			q_.pop();
			return res;
		}
	};
}

#define MQ_MATCH(_message_type_, _message_, _ret_) \
	mq::wraped_message<_message_type_>* _ret_ = \
		dynamic_cast<mq::wraped_message<_message_type_>*>(_message_.get())