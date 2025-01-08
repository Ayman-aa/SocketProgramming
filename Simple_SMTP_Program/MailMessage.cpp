#include "MailMessage.hpp"

MailMessage::MailMessage(const std::string &from, const std::string &to, const std::string &subject, const std::string &content)
	: from(from), to(to), subject(subject), content(content)
{
	timestamp = std::time(NULL);
}

std::string MailMessage::getFrom() const {return from;}
std::string MailMessage::getTo() const {return to;}
std::string MailMessage::getSubject() const {return subject;}
std::string MailMessage::getContent() const {return content;}
time_t MailMessage::getTimestamp() const {return timestamp;}
