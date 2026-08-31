#pragma once

class EmptyString : public std::exception
{
private:
	std::string message;
public:
	explicit EmptyString(std::string message) { this->message = message; }
	const char* what() const noexcept  override { return this->message.c_str(); }
};

class EmptyStruct : public std::exception
{
private:
	std::string message;
public:
	explicit EmptyStruct(std::string message) { this->message = message; }
	const char* what() const noexcept  override { return this->message.c_str(); }
};