#pragma once
#include <string>
#include <ctime>

class MailMessage
{
	public:
		MailMessage(const std::string &from, const std::string &to,
				const std::string &subject, const std::string &content);
	
		std::string getFrom() const;
		std::string getTo() const;
		std::string getSubject() const;
		std::string getContent() const;
		time_t getTimestamp() const;
	
	private:
		std::string from;
		std::string to;
		std::string subject;
		std::string content;
		time_t timestamp;
};
