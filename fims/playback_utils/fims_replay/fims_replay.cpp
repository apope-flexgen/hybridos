#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <fims/libfims.h>

fims f;

struct Message
{
    std::string method;
    std::string uri;
    std::string body;
    std::string replyto;
    std::string process_name;
    std::string username;
    double timestamp;
};

std::vector<Message> processMessages(const std::string& messageText)
{
    std::vector<Message> messages;
    std::regex messageBlockRegex(R"(\n\s*\n)");
    std::regex methodPattern(R"(Method:\s*(\w+))");
    std::regex uriPattern(R"(Uri:\s*([\w/]+))");
    std::regex valuePattern(R"(Body:\s*([^\n]+))");
    std::regex replytoPattern(R"(ReplyTo:\s*([^\n]+))");
    std::regex processNamePattern(R"(Process Name:\s*([^\n]+))");
    std::regex usernamePattern(R"(Username:\s*([^\n]+))");
    std::regex timestampPattern(R"(Timestamp:\s*([^\n]+))");

    std::sregex_token_iterator iter(messageText.begin(), messageText.end(), messageBlockRegex, -1);
    std::sregex_token_iterator end;

    for (; iter != end; ++iter)
    {
        std::string messageBlock = *iter;

        std::smatch match;
        Message message;

        if (std::regex_search(messageBlock, match, methodPattern))
        {
            message.method = match[1];
        }
        if (std::regex_search(messageBlock, match, uriPattern))
        {
            message.uri = match[1];
        }
        if (std::regex_search(messageBlock, match, valuePattern))
        {
            message.body = match[1];
        }
        if (std::regex_search(messageBlock, match, replytoPattern))
        {
            message.replyto = match[1];
        }
        if (std::regex_search(messageBlock, match, processNamePattern))
        {
            message.process_name = match[1];
        }
        if (std::regex_search(messageBlock, match, usernamePattern))
        {
            message.username = match[1];
        }
        if (std::regex_search(messageBlock, match, timestampPattern))
        {
            std::string timestampStr = match[1];
            struct tm tm = {};
            int milliseconds = 0;

            if (sscanf(timestampStr.c_str(), "%d-%d-%d %d:%d:%d.%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour,
                       &tm.tm_min, &tm.tm_sec, &milliseconds) != 7)
            {
                throw std::runtime_error("Failed to parse timestamp");
            }

            tm.tm_year -= 1900;
            tm.tm_mon -= 1;

            std::time_t time = std::mktime(&tm);
            message.timestamp = time + milliseconds / 1000000.0;
        }

        messages.push_back(message);
    }

    return messages;
}

void sendMessages(const std::vector<Message>& messages)
{
    if (messages.empty())
    {
        return;
    }

    for (size_t i = 0; i < messages.size(); ++i)
    {
        if (i > 0)
        {
            double delay = messages[i].timestamp - messages[i - 1].timestamp;  // delay in seconds
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(delay * 1000000)));
        }

        // bool Send(const char *method,
        //           const char *uri,
        //           const char *replyto,
        //           const char *body,
        //           const char *username = (const char *)nullptr)
        f.Send(messages[i].method.c_str(), messages[i].uri.c_str(), "", messages[i].body.c_str(), "");
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <message_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file)
    {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string messageText = buffer.str();

    f.Connect("fims_replay");

    std::vector<Message> messages = processMessages(messageText);
    sendMessages(messages);

    f.Close();
    return 0;
}
