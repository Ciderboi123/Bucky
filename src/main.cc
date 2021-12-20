#include "include.hh"
#include "spdlog/spdlog.h"

#ifndef ASIO_STANDALONE
namespace asio = boost::asio;
#endif
using json = nlohmann::json;
namespace dpp = discordpp;

std::string getToken();

std::istream &safeGetline(std::istream &is, std::string &t);

void filter(std::string &target, const std::string &pattern);

int main()
{
    spdlog::set_pattern("[ %T ] - (%^%L%$) » %v");

    dpp::log::filter = dpp::log::error;
    // dpp::log::out = &spdlog::err_handler;

    spdlog::info("Starting Bot");

    std::string token = getToken();
    if (token.empty())
    {
        std::cerr << "No Token" << std::endl;
        exit(1);
    }

    // Create Bot object
    auto bot = std::make_shared<DppBot>();
    bot->debugUnhandled = true;
    bot->intents = dpp::intents::NONE | dpp::intents::GUILD_MESSAGES;
    bot->prefix = ".";

    discordpp::User self;
    bot->handlers.insert(
        {"READY", [&self](dpp::Ready ready)
         { 
             self = *ready.user;
             spdlog::info("Bot Online !");
         }});

    
    
    // Create Asio context, this handles async stuff.
    auto aioc = std::make_shared<asio::io_context>();

    // Set the bot up
    bot->initBot(9, token, aioc);

    // Run the bot!
    bot->run();

    return 0;
}

std::string getToken()
{
    std::string token;

    /*
                                                                    First
       attempt to read the token from the BOT_TOKEN environment variable.
    */
    char const *env = std::getenv("BOT_TOKEN");
    if (env != nullptr)
    {
        token = std::string(env);
    }
    else
    {
        /*/
         * Read token from token file.
         * Tokens are required to communicate with Discord, and hardcoding
        tokens is a bad idea.
         * If your bot is open source, make sure it's ignore by git in your
        .gitignore file.
        /*/
        std::ifstream tokenFile("token.dat");
        if (!tokenFile)
        {
            return "";
        }
        safeGetline(tokenFile, token);
        tokenFile.close();
    }
    return token;
}

/*/
 * Source: https://stackoverflow.com/a/6089413/1526048
/*/
std::istream &safeGetline(std::istream &is, std::string &t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    for (;;)
    {
        int c = sb->sbumpc();
        switch (c)
        {
        case '\n':
            return is;
        case '\r':
            if (sb->sgetc() == '\n')
            {
                sb->sbumpc();
            }
            return is;
        case std::streambuf::traits_type::eof():
            // Also handle the case when the last line has no line ending
            if (t.empty())
            {
                is.setstate(std::ios::eofbit);
            }
            return is;
        default:
            t += (char)c;
        }
    }
}
