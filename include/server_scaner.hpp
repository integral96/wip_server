#pragma once

#include <iostream>
#include <vector>
#include <memory> 
#include <array>
#include <string>
#include <random>
#include <set>
#include <utility>
#include <charconv>
#include <boost/asio.hpp> // подключаем кроссплатформенные библиотеки для сетевой подсистемы
/* буду использовать TCP протокол, пробовал с UDP но сложноватый логический интерфейс, запутался */
#include <boost/uuid/uuid.hpp>  //поключаем билиотеку универсальный уникальный идентификатор.          
#include <boost/uuid/uuid_generators.hpp> //библиотека для генерации имен.
#include <boost/uuid/uuid_io.hpp>   // обеспечивает перегрузку оператора запись объекта типа

namespace SERVER { //задаем пространство сервер

using namespace boost::asio::ip; //подключаем пространство имен буст/асио

template <typename T = char> // создаем шаблонный класс Биометрический сканер(по умолчанию char)
class biometric_scanner
{
    private:
        std::set<std::pair<T, T>> set_compare; //создаем множество пар, где будут сравниваться
                    // поступающие на сканер от клиента биометрические данные в виде массива,
                    // и массивом биометрический эталон.
        std::unique_ptr<T[]> array; //создадим массив биометрический эталон
    public:
        biometric_scanner() { //конструктор по умолчанию 
            this->set_biometric_standard();
        }
        virtual ~biometric_scanner(){} //деструктор

        void constract(const std::string& ar0) //заполняем множество парами данных
        {
            if(ar0.length() > 9 && ar0.length() < 1) throw std::out_of_range("arrays error"); //проверяем на совместимость массива 
            for (size_t i = 0; i < ar0.length(); i++) //с начальными данными, если не выполняется выкидыываем исключение
            {
                set_compare.insert(std::make_pair(array[i], ar0[i])); //заполняем
            }
        }
        std::string get_biometric_result() //выводим результат, 
        {
            std::random_device dre; //так как в задании надо выводить истину или лож случайным образом, 
            std::mt19937 gen(dre());
            std::uniform_int_distribution<int> di(0,1); //задаем генератор случайных целых чисел 0 и 1
            return di(gen) == 0 ? "true" : "false";
        }
        void print_biometric() //распечатываем пару биометрические данные и биометрический эталон
        {
            for(const auto& [x, y] : set_compare) std::cout << "(scan: " << x << ", stand: " << y << "); ";
            std::cout << std::endl;
        }
        void uuids() const //создаем универсальный уникальный идентификатор
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::cout <<  "UUID Biometric scanner: (" <<uuid << ") ";
        }
        
    private:
        void set_biometric_standard() //создаем массив биометрический эталон
        {
            array = std::make_unique<T[]>(9); //так как не было огаворены начальные данные
            for (size_t i = 0; i < 9; i++)    //то я задаю их простым натуральным рядом от 0 до 9(не включительно)
            {
                std::string s = std::to_string(i); //конвертируем в строку
                array[i] = *s.c_str(); //заполняем массив char
            }                          //данная функция может ругаться при изменнение типа Т, я не стал дописывать метод по идеологии SFINAE
        }
};

class sensor //создаем структуру Датчиков
{
    public:
        sensor(){} //коструктор  по умолчанию
        std::string get_sensor_data() //выдаем случайным образом Да или Нет, как обговорено в задании
        {
            std::random_device dre;
            std::mt19937 gen(dre());
            std::uniform_int_distribution<int> di(0,1);
            return di(gen) == 0 ? "Yes" : "No";
        }
        void uuids() const //создаем универсальный уникальный идентификатор для датчиков
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::cout <<  "UUID Sensor: (" <<uuid << ") " << std::endl;
        }
        virtual ~sensor(){}
};
/*создаем класс сессия для класса сервера, он читает данные 
    из сокета и отправляет обратно ответ в секет*/
class session : public std::enable_shared_from_this<session>
/*наследуем класс  который позволяет управляется умным указателем шаред_птр из session
        который определим ниже, для безопасного создания владельцев объектов session*/
{
    private:
        std::array<char, 9> array;
        tcp::socket tcp_socket; // создаем сокет протокола TCP
        
    public:
        session(tcp::socket socket):tcp_socket(std::move(socket)) {} //в коструктор по умолчанию передаем сокет
                                                                     //с помощью мув семантики.
        virtual ~session(){}
    
        void read() //функция где будем использовать метод сокета из буст async_read_some()
        {
            auto self(shared_from_this()); //возвращаем умный указатель, который сссылается на this
            tcp_socket.async_read_some( //считываем:
                boost::asio::buffer(array, 9), //заполняем буфер массивом
                [this, self](const std::error_code error, const size_t length){ //передаем лямбду которая захватывает копирывание
                    if(!error)                                                  //this и указателя, проверяем на ошибки(зависящие от ОП)
                    {
                        auto number = std::string(array.data(), length); //считываем номер из сокета
                        biometric_scanner bio_scnr; //создаем экземпляр класса биометрический сканер
                        bio_scnr.constract(number); //передаем ему номер
                        sensor snsr;                //создаем экземпляр класса Датчик
                        auto result = bio_scnr.get_biometric_result(); //получаем результат от методов биометрический сканер
                        bio_scnr.uuids();           //распечатываем UUID сканера
                        snsr.uuids();               //распечатываем UUID датчика
                        bio_scnr.print_biometric(); //распечатываем пару биометрический сканер
                        std::cout << " Biometric scanner: " << result << ";\t sensor: " << snsr.get_sensor_data() << std::endl;
                        write(result);              //запишем результат ответ для передачи клиенту
                    }
                });
        }
    private:
        void write(std::string_view response) //функция для записи ответа клиенту
        {
            auto self(shared_from_this());
            tcp_socket.async_write_some( //метод записи эти методы действуюют асиннхронно 
                boost::asio::buffer(response.data(), response.length()),
                [this, self](const std::error_code ec, const size_t length){//также создаем лямбду как и выше, 
                    if(!ec)                                                 //только через лямбду пердаем read()
                        read();                                             
                });
        }
};
class server //создаем класс сервер
{
    private:
        tcp::acceptor tcp_acceptor; //создаем ацептор который будет ждать подключение в UDP я такого не нашел и запутался куда передовать подключения.
        tcp::socket tcp_socket;
        
    public:
        server(boost::asio::io_context& context, const int port):tcp_acceptor(context, //передаем контейнер где будут хранится сигналы строки..
            tcp::endpoint(tcp::v4(), port)), //создайте endpoint с использованием номера порта. IP-адрес будет любым.
            tcp_socket(context) //инициализируем сокет
            {
                std::cout << "Server running on port: " << port << std::endl; //выводим на экран порт
                accept(); 
            }
        virtual ~server(){}
    private:
        void accept() //метод создающий ассинхронно через лямбду проверющию на ошибки умный указатеь на класс сесия, вызывая метод read()
        {
            tcp_acceptor.async_accept(tcp_socket, 
            [this](const std::error_code error){
                if(!error)
                    std::make_shared<session>(std::move(tcp_socket))->read();
                accept();
            });
        }
};

void run_server(int port)  //функция запуска сервера
{
    try
        {
            boost::asio::io_context context; //создаем объект для ввода вывода
            server srv(context, port);      //создаем объект класса вервер передаем порт и запускаем.
            context.run();
            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
}

}