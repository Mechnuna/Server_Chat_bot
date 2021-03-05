#include <iostream>
#include <uwebsockets/App.h>
#include <map>


//доп функции
//1 добавить имя +
//2 добавить месадж олл

using namespace std;
// какую инфу о пользователе мы храним
struct PerSocketData {
    string name; // имя
    unsigned int user_id; // id пользователя
};
map<unsigned int, string> userNames;
const string MES_TO = "MESSAGE_TO::";
const string NAME = "SET_NAME::";
const string OFF = "OFFLINE::";
const string ON = "ONLINE::";

void updateName(PerSocketData* data)
{
    userNames[data->user_id] = data->name;
}

void deleteName(PerSocketData* data)
{
    userNames.erase(data->user_id);
}

// проверяем имя ли это
bool isName(string message)
{
    return message.find(NAME) == 0;
}

// узнаем имя
string parseName(string message)
{
    return message.substr(NAME.size());
}

// проверяем имя на валидность
bool isNormName(string message)
{
    string us_name = parseName(message);
    if (us_name.find("::") != 4294967295 || us_name.size() > 255)
        return 0;
    return 1;
}

// ONLINE/OFFLINE::19::ALESHA
string status(unsigned int user_id, string status)
{
    string name = userNames[user_id];
    return status + to_string(user_id) + "::" + name;
}


// отрпавляем сообщение
string messageFrom(string user_id, string senderName, string message)
{
    return "MESSAGE_FROM::" + user_id + "::[" + senderName + "] " + message;
}

// проверяем сообщение ли это 
bool isMessageTo(string message)
{
    return message.find(MES_TO) == 0;
}

// узнаем id пользователя
string parseUserId(string message)
{
    string rest = message.substr(MES_TO.size());
    int pos = rest.find("::");
    return rest.substr(0, pos);
}

// узнаем текст сообщения
string parseUserTxt(string message)
{
    string rest = message.substr(MES_TO.size());
    int pos = rest.find("::");
    return rest.substr(pos + 2);
}

int main() {

    unsigned int last_user_id = 10; // последий индефикатор пользователя
    // настраиваем сервер
    uWS::App(). //создаем приложение бзе шифрования
        ws<PerSocketData>("/*", { //для каждого пользователя храним данные в виде PerSocketData
            /* Settings */
            .idleTimeout = 1200, // timeout 
            /* Handlers */
            .open = [&last_user_id](auto*ws) {
                // лямбда функция запускается при открытие соединения
                //0 получить структуру данных
               PerSocketData* user_data = (PerSocketData *)ws->getUserData();
               //1 названить пользователю уник. индефикатор
               user_data->name = "UNNAMED";
               user_data->user_id = last_user_id++;
               updateName(user_data);
               ws->publish("broadcast", "Someone connected");
               cout << "New user connecter, id = " << user_data->user_id << endl;
               // выводим сколько юзеров всего в сети
               cout << "Total users connected:" << userNames.size() << endl; 
               // подписываем пользователя к личному каналу
               ws->subscribe("user#" + to_string(user_data->user_id));
               // подписываем к общему каналу
               ws->subscribe("broadcast");
               // сообщаем новичку кто сейчас онлайн
               for (auto entry : userNames)
               {
                   ws->send(status(entry.first, ON), uWS::OpCode::TEXT);
               }
            },
            .message = [](auto* ws, string_view message, uWS::OpCode opCode) {
                string strMes = string(message);
                PerSocketData* user_data = (PerSocketData*)ws->getUserData();
                string author_id = to_string(user_data->user_id);
                // ws->send(message, opCode, true); для проверки
                // лямбда вызывается при получении сообщения

                // "MESSAGE_TO::10:: Hello "
                if (isMessageTo(strMes))
                {
                    // подготавливаем данные
                    string receiver_id = parseUserId(strMes); // узнаем id
                    if (stoul(receiver_id) < 10 || stoul(receiver_id) > (userNames.size() + 9)) // если такого id не существует
                        ws->send("Error, there is no user with ID = " + receiver_id, uWS::OpCode::TEXT); // выводим ошибку
                    else
                    {
                        string text = parseUserTxt(strMes); // узнаем текст сообщения
                        string outgoingMessage = messageFrom(author_id, user_data->name, text); // собираем сообщение для отправки
                        // отправляем
                        ws->publish("user#" + receiver_id, outgoingMessage, uWS::OpCode::TEXT);
                        ws->send("Message sent", uWS::OpCode::TEXT);
                        cout << "User #" << author_id << " wrote message to " << receiver_id << endl;
                    }
                }

                // "SET_NAME::PETYA"
                if (isName(strMes))
                {
                    if (isNormName(strMes))
                    {
                        updateName(user_data);
                        string new_name = parseName(strMes);
                        user_data->name = new_name;
                        updateName(user_data);
                        ws->publish("broadcast", status(user_data->user_id, ON));
                        cout << "User ~" << author_id << " set their name " << endl;
                        ws->send("Nice to meet you", uWS::OpCode::TEXT);
                    }
                    else
                        ws->send("Your name cannot contain '::' or be more than 255 characters", uWS::OpCode::TEXT);
                }
            },
            .close = [](auto* ws, int /*code*/, string_view /*message*/) {
                // вызывается при отключении от сервера
                PerSocketData* user_data = (PerSocketData*)ws->getUserData();
                ws->publish("broadcast", status(user_data->user_id, OFF));
                deleteName(user_data);
                cout << "Total users connected:" << userNames.size() << endl;
            }
            }).listen(9001, [](auto* listen_socket) {
                if (listen_socket) { // если все ок вывести сообщение
                    std::cout << "Listening on port " << 9001 << std::endl;
                }
                }).run();
}