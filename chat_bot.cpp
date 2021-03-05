#include "date_chat_bot.h"

using namespace std;

//переводим в строчные буквы
string to_lower(string txt)
{
    transform(txt.begin(), txt.end(), txt.begin(), ::tolower);
    return txt;
}

//функция для вводы вопроса пользователя
string user_ask()
{
    string question;
    cout << "[USER]: ";
    getline(cin, question);
    question = to_lower(question);
    return question;
}

//функция для вызова ответа
void bot_say(string txt)
{
    cout << "[BOT]: " << txt << endl;
}

int main()
{

    string question; //вопрос пользователя
    int num_answers = 0; // кол-во вопросов
    bot_say("Hello, My name is ChatBot");
    while (question != "exit")
    {
        question = user_ask();
        for (auto entry : database)
        {
            regex patern = regex(".*" + entry.first + ".*");
            if (regex_match(question, patern))
            {
                ++num_answers;
                bot_say(entry.second);
            }
        }
        if (num_answers == 5)
            bot_say("Oiiiii, I very talkative today");
        if (num_answers == 0)
            bot_say("Oiiiii, I don't knooow");
    }
    bot_say("okey, bye");

    /* 495 345-02-02
   regex ("[0-9]{3} [0-9]{3}-[0-9]{2}-[0-9]{2}") как найти номер с помощью regex */
}