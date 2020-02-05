#pragma once

#include <boost/asio.hpp> // подключаем кроссплатформенные библиотеки для сетевой подсистемы


#include <array>
#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <list>
#include <map>

using namespace boost;

template<typename Callback>
struct Session
{
    asio::ip::tcp::socket m_sock;
    asio::ip::tcp::endpoint m_ep;
    std::string m_request;

    asio::streambuf m_response_buf;
    std::string m_response;

    system::error_code m_ec;

    unsigned int m_id;
    Callback m_callback;

    bool m_was_canceled;
    std::mutex m_canceled_guard;
    Session(asio::io_service& ios, const std::string& raw_ip_address, unsigned short port_num, const std::string& request, unsigned int id,  Callback callback):
        m_sock(ios), m_ep(asio::ip::address::from_string(raw_ip_address), port_num), m_request(request), m_id(id), m_callback(callback), m_was_canceled(false) {} 
};
template<typename Callback, typename T = int>
class AsyncTCPClient : public boost::asio::noncopyable
{
private:
    asio::io_service m_ios;
    std::map<T, std::shared_ptr<Session<Callback> > > m_active_session;
    std::mutex m_active_session_guard;
    std::unique_ptr<boost::asio::io_service::work> m_work;
    std::list<std::unique_ptr<std::thread>> m_threads;
private:
    void onRequestComplete(std::shared_ptr<Session<Callback> > session){
        boost::system::error_code ignored_ec;
        session->m_sock.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
        std::unique_lock<std::mutex> lock(m_active_session_guard);

        auto it = m_active_session.find(session->m_id);
        if (it != m_active_session.end()) m_active_session.erase(it);

        lock.unlock();
        
        boost::system::error_code ec;
        if (!(session->m_ec) && session->m_was_canceled ) ec = asio::error::operation_aborted;
        else ec = session->m_ec;

        session->m_callback(session->m_id, session->m_response, ec);
    }
public:
    AsyncTCPClient(size_t num_of_threads){
        m_work.reset(new boost::asio::io_service::work(m_ios));
        for (size_t i = 1; i <= num_of_threads; i++)
        {
            std::unique_ptr<std::thread> th(new std::thread([this](){
                m_ios.run();
            }));
            m_threads.push_back(std::move(th));
        }
    }
    ~AsyncTCPClient(){}
    void emulateLongComputation(unsigned int duration_sec, const std::string& raw_ip_address, unsigned short port_num, Callback callback, unsigned int request_id)
    {
        std::string request = "EMULATE_LONG_CALC_OP " + std::to_string(duration_sec) + "\n";
        auto session = std::make_shared<Session<Callback> >(m_ios, raw_ip_address, port_num, request, request_id, callback);
        session->m_sock.open(session->m_ep.protocol());
        std::unique_lock<std::mutex> lock(m_active_session_guard);
        m_active_session[request_id] = session;
        lock.unlock();
        session->m_sock.async_connect(session->m_ep, [this, session](const system::error_code& ec){
            if(ec){
                session->m_ec = ec;
                onRequestComplete(session);
                return;
            }

            std::unique_lock<std::mutex> cancel_lock(session->m_canceled_guard);
            if (session->m_was_canceled)
            {
                onRequestComplete(session);
                return;
            }
            asio::async_write(session->m_sock, asio::buffer(session->m_request), [this, session](const boost::system::error_code& ec, size_t bytes_transferred){
                if (ec)
                {
                    session->m_ec = ec;
                    onRequestComplete(session);
                    return;
                }
                std::unique_lock<std::mutex> cancel_lock(session->m_canceled_guard);
                if (session->m_was_canceled)
                {
                    onRequestComplete(session);
                    return;
                }
                asio::async_read_until(session->m_sock, session->m_response_buf, '\n', [this, session](const boost::system::error_code& ec, size_t bytes_transferred){
                    if(ec) session->m_ec = ec;
                    else
                    {
                        std::istream strm(&session->m_response_buf);
                        std::getline(strm, session->m_response);
                    }
                    onRequestComplete(session);
                });
            });
        });
    }
    void cancelRequest(unsigned int request_id){
            std::unique_lock<std::mutex> lock(m_active_session_guard);
            auto it = m_active_session.find(request_id);
            if (it != m_active_session.end())
            {
                std::unique_lock<std::mutex> cancel_lock(it->second->m_canceled_guard);
                it->second->m_was_canceled = true;
                it->second->m_sock.cancel();
            }
    }
    void close(){
        m_work.reset(nullptr);
        for(const auto& thread : m_threads)
        thread->join();
    }
};