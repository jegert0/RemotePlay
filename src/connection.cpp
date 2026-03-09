/*
make the updatestates function so it updates the emulated controllers while iterating through all the end points
or maybe modify the asnyc recieve so it automatically updates the associated emulated controller
*/

#include "connection.hpp"

using boost::asio::ip::udp;


Udp_Server::Udp_Server( boost::asio::io_context& io, std::string port, PVIGEM_CLIENT * vc )
    : socket_( io, udp::endpoint( udp::v4(), static_cast<unsigned short>( std::stoi( port ) ) ) ),
    vigem_client_(vc), running_(true)
{
    start_receive();
}

Udp_Server::~Udp_Server()
{
    //delete the allocated emulated controllers
    for ( auto& pair : controllers_ )
    {
        delete( pair.second );
    }
}

/*
    Server listens for any incoming information and translate the data to controller states
    If a new connection is detected it creates a new emulatedController assicated to that endpoint
*/
void Udp_Server::start_receive()
{
    //auto self = shared_from_this();
    socket_.async_receive_from( boost::asio::buffer( recv_buffer_ ), remote_endpoint_, 
    [this]( boost::system::error_code ec, std::size_t bytes_recvd)
    {   
        if ( !ec && bytes_recvd == sizeof(controllerState) && this->isRunning() )
        {
            if ( controllers_.find(remote_endpoint_) == controllers_.end() ) // if last connection then notify
            {
                std::cout << "New client: " << remote_endpoint_.address() << " : " << remote_endpoint_.port() << std::endl;
                // create new emulated controller
                controllers_[remote_endpoint_] = new emulatedController( vigem_client_ );
            }
        
            controllerState temp_state;
            std::memcpy( &temp_state, recv_buffer_.data(), sizeof(controllerState) );

            controllers_[remote_endpoint_]->updateState( temp_state );
            
            //echo back
            // socket_.async_send_to( boost::asio::buffer( recv_buffer_ ), remote_endpoint_,
            // [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/){}
            // );
            start_receive();
        }
        
    });
}

int Udp_Server::getNumClients() { return (int) controllers_.size(); }

Udp_Client::Udp_Client( boost::asio::io_context& io, const std::string& host, const std::string& port)
    : socket_(io, udp::v4() ), running_(true)
{
    udp::resolver resolver(io);
    endpoint_ = *resolver.resolve( udp::v4(), host, port).begin();
    controllerState st = {0,0,0,0,0,0,0}; //send empty state at the start
    send_state(st);
}

void Udp_Client::send_state(const controllerState& state)
{
    if ( !this->isRunning() || !socket_.is_open() ) return; // if client is not running or server is closed do nothing

    // Allocate on heap so buffer stays valid until async send completes
    auto buffer = std::make_shared<std::array<char, sizeof(controllerState)>>();

    // Copy state into buffer
    std::memcpy(buffer->data(), &state, sizeof(controllerState));

    socket_.async_send_to( boost::asio::buffer( *buffer ), endpoint_, 
    [this, state] (boost::system::error_code ec, std::size_t /*bytes_sent*/)
    {
        //send_state(state); //start another async send state after the completion of the last
        if ( ec )
        {
            std::cerr << "Send failed: " << ec.message() << std::endl;
        }
    } );

}

bool Udp_Server::isRunning() { return this->running_; }
bool Udp_Client::isRunning() { return this->running_; }
void Udp_Server::turnOff() 
{ 
    this->running_ = false; 
    boost::system::error_code ec;
    this->socket_.close( ec );
}
void Udp_Client::turnOff()
{ 
    this->running_ = false; 
    boost::system::error_code ec;
    this->socket_.close( ec );
}

