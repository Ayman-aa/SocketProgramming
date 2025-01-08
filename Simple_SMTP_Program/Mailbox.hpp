#pragma once

#include <vector>
#include <mutex>
#include "MailMessage.hpp"

class Mailbox
{
	public:
		Mailbox(const std::string &username, const std::string &password);
		bool authenticate(const std::string &pwd);
		void addMessage(const MailMessage &msg);
		std::vector<MailMessage> getMessages();
		std::string getUsername() const;
	
	private:
		std::string username;
		std::string password;
		std::vector<MailMessage> messages;
		std::mutex mailboxMutex;
};
