#include <multi_progress.hpp>
#include <progress_bar.hpp>
#include <class_client.hpp>
#include <iostream>
#include <thread>

struct Handler
{
    void operator()(unsigned int request_id, const std::string& response, const system::error_code& ec) {
        if (ec)
        {
            std::cout << "Request # " << request_id << " has completed. Response: " << response << std::endl;
        }
        else if (ec == asio::error::operation_aborted)  
        {
            std::cout << "Request # " << request_id << " has been cancelled by the user." << std::endl;
        }
        else
        {
            std::cout << "Request # " << request_id << " failed! Error code = " << ec.value() << ". Error message = " << ec.message() << std::endl;
        }
    }
};



void client(const std::string& host, const int port) //функция получает хостнаме и порт и запускает клиента
{
    try
    {
        ProgressBar bar1, bar2, bar3;
    //     MultiProgress<ProgressBar, 3> bars(bar1, bar2, bar3);
    // // Job for the first bar
    //     auto job1 = [&bars]() {
    //         for (size_t i = 0; i <= 100; ++i) {
    //         bars.update<0>(i);
    //         std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //         }
    //     };

    // // Job for the second bar
    //     auto job2 = [&bars]() {
    //         for (size_t i = 0; i <= 100; ++i) {
    //         bars.update<1>(i);
    //         std::this_thread::sleep_for(std::chrono::milliseconds(200));
    //         }
    //     };

    // // Job for the third bar
    //     auto job3 = [&bars]() {
    //         for (size_t i = 0; i <= 100; ++i) {
    //         bars.update<2>(i);
    //         std::this_thread::sleep_for(std::chrono::milliseconds(60));
    //         }
    //     };
        // std::thread first_job(job1);
        // std::thread second_job(job2);
        // std::thread third_job(job3);

        // first_job.join();
        // second_job.join();
        // third_job.join();
        bar1.set_bar_width(50);
        bar1.fill_bar_progress_with("■");
        bar1.fill_bar_remainder_with(" ");

        bar2.set_bar_width(50);
        bar2.fill_bar_progress_with("■");
        bar2.fill_bar_remainder_with(" ");

        bar3.set_bar_width(50);
        bar3.fill_bar_progress_with("■");
        bar3.fill_bar_remainder_with(" ");

        Handler handler;
        AsyncTCPClient<Handler> client(2);
        
        for (size_t i = 1; i <= 100; ++i) {
            bar1.update(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        client.emulateLongComputation(10, host, port, handler, 1);
        
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        for (size_t i = 1; i <= 100; ++i) {
            bar2.update(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(60));
            }
        client.emulateLongComputation(11, host, port, handler, 2);
        client.cancelRequest(1);
        
        // std::this_thread::sleep_for(std::chrono::seconds(6));
        for (size_t i = 1; i <= 100; ++i) {
            bar3.update(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(70));
            }
        client.emulateLongComputation(12, host, port, handler, 3);
        
        // std::this_thread::sleep_for(std::chrono::seconds(15));
        client.close();
        
        /////////////////////////////
        // asio::io_context context;                  //создаем объект для ввода вывода
        // asio::ip::tcp::socket tcp_socket(context); // создаем сокет протокола TCP
        // asio::ip::tcp::resolver resolver(context); //объект отменяет все асинхронные операции, ожидающие разрешения.
        // connect(tcp_socket, resolver.resolve({host.data(), std::to_string(port)})); //Конектимся по хостнейму и порту
        // while (true)
        // {
        //     std::cout << "biometric number or string [1-9]: "; //предлогаем ввести номера или строку не более 9 байт.
        //     std::string line;
        //     std::cin >> line;                               //считываем
        //     if(std::cin.fail() || line.length() < 1 || line.length() > 9) break; //проверяем на условия задачи

        //     // auto request = line;
        //     tcp_socket.write_some(asio::buffer(line, line.length()));  //записываем в буфер и отправляем

        //     std::array<char, 9> reply;
        //     auto reply_length = tcp_socket.read_some(asio::buffer(reply, reply.size())); //получаем ответ от сервера

        //     std::cout << "biometric scaner reply is: ";
        //     std::cout.write(reply.data(), reply_length); //распечатываем ответ из сервера
        //     std::cout << std::endl;
        // }
    }
    catch(const boost::system::system_error& e) //если что ловим исключение
    {
        std::cerr << "Error occured! Error code = " << e.code() << ". Message: " << e.what() << '\n';
        // return e.code().value();
    }
    
}

int main(int argc, char const *argv[])
{
    // auto host = std::string(argv[1]);
    // int port = std::stoi( argv[2] );
    // ProgressBar bar;
    // bar.set_bar_width(50);
    // bar.fill_bar_progress_with("■");
    // bar.fill_bar_remainder_with(" ");

    // for (size_t i = 1; i <= 100; ++i) {
    //     bar.update(i);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // }
    client(std::string(argv[1]), std::stoi(argv[2]));
    return 0;
}