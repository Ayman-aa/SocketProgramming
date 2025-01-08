#include "Mailbox.hpp"

Mailbox::Mailbox(const std::string &username, const std::string &password): username(username), password(password){}

bool Mailbox::authenticate(const std::string &pwd)
{
	return password ==  pwd;
}

void Mailbox::addMessage(const MailMessage &msg)
{
	std::lock_guard<std::mutex> lock(mailboxMutex);
	messages.push_back(msg);
}

std::vector<MailMessage> Mailbox::getMessages()
{
	std::lock_guard<std::mutex> lock(mailboxMutex);
	return messages;
}

std::string Mailbox::getUsername() const {return username;}
