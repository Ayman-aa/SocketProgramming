#pragma once
#include <queue>
#include <mutex>
#include "MailMessage.hpp"

class MailQueue
{
	public:
		void enqueue(const MailMessage &msg);
		bool dequeue(MailMessage &msg);
		bool isEmpty() const;
	
	private:
		std::queue<MailMessage> messageQueue;
		mutable std::mutex queueMutex;
};
