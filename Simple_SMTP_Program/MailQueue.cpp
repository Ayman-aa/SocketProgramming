#include "MailQueue.hpp"

void MailQueue::enqueue(const MailMessage &msg)
{
	std::lock_guard<std::mutex> lock(queueMutex);
	messageQueue.push(msg);
}

bool MailQueue::dequeue(MailMessage &msg)
{
	std::lock_guard<std::mutex> lock(queueMutex);

	if(messageQueue.empty())
		return false;

	msg = messageQueue.front();
	messageQueue.pop();
	return true;
}

bool MailQueue::isEmpty() const
{
	std::lock_guard<std::mutex> lock(queueMutex);
	return messageQueue.empty();
}
