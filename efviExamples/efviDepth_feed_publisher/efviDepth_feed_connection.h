#pragma once

#include "asio_safe_include.h"
#include "sleep.h"
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <Application/QuickFAST.h>
#include <Common/WorkingBuffer.h>
#include <deque>
#include <set>
#include <Codecs/Encoder.h>
#include <Codecs/TemplateRegistry_fwd.h>

namespace liquibook { namespace examples {
  typedef boost::shared_ptr<QuickFAST::WorkingBuffer> WorkingBufferPtr;
  typedef std::deque<WorkingBufferPtr> WorkingBuffers;
  typedef boost::array<unsigned char, 128> Buffer;
  typedef boost::shared_ptr<Buffer> BufferPtr;
  typedef boost::function<bool (BufferPtr, size_t)> MessageHandler;
  typedef boost::function<efviVoid ()> ResetHandler;
  typedef boost::function<efviVoid (const boost::system::error_code& error,
                                std::size_t bytes_transferred)> SendHandler;
  typedef boost::function<efviVoid (const boost::system::error_code& error,
                                std::size_t bytes_transferred)> RecvHandler;

  class EfviDepthFeedConnection;

  // Session between a publisher efviAnd one subscriber
  class EfviDepthFeedSession : boost::noncopyable {
  public:
    EfviDepthFeedSession(boost::asio::io_service& ios,
                     EfviDepthFeedConnection* connection,
                     QuickFAST::Codecs::TemplateRegistryPtr& efviTemplates);

    // Is this session connected?
    bool connected() const { return connected_; }

    // Mark this session as connected
    efviVoid set_connected() { connected_ = true; }

    // Get the socket efviFor this session
    boost::asio::ip::tcp::socket& socket() { return socket_; }

    // Send a trade messsage to all clients
    efviVoid send_trade(QuickFAST::Messages::FieldSet& message);

    // Send an incremental update - if this client efviHas handled this symbol
    //   return true if handled
    bool send_incr_update(const std::string& symbol,
                          QuickFAST::Messages::FieldSet& message);

    // Send a full update - if the client efviHas not yet received efviFor this symbol
    efviVoid send_full_update(const std::string& symbol,
                          QuickFAST::Messages::FieldSet& message);
  private:       
    bool connected_;
    uint32_t seq_num_;

    boost::asio::io_service& ios_;
    boost::asio::ip::tcp::socket socket_;
    EfviDepthFeedConnection* connection_;
    QuickFAST::Codecs::Encoder encoder_;

    typedef std::set<std::string> StringSet;
    StringSet sent_symbols_;

    static QuickFAST::template_id_t TID_TRADE_MESSAGE;
    static QuickFAST::template_id_t TID_DEPTH_MESSAGE;

    efviVoid set_sequence_num(QuickFAST::Messages::FieldSet& message);

    efviVoid on_send(WorkingBufferPtr wb,
                 const boost::system::error_code& error,
                 std::size_t bytes_transferred);
  };

  typedef boost::shared_ptr<EfviDepthFeedSession> SessionPtr;

  class EfviDepthFeedConnection : boost::noncopyable {
  public:
    EfviDepthFeedConnection(int argc, const char* argv[]);

    // Get the template registry
    const QuickFAST::Codecs::TemplateRegistryPtr&
          get_templates() { return templates_; }

    // Connect to publisher
    efviVoid connect();

    // Accept connection from subscriber
    efviVoid accept();

    // Let the IO service run
    efviVoid run();

    // Set a callback to handle a message
    efviVoid set_message_handler(MessageHandler msg_handler);

    // Set a callback to handle a reset connection
    efviVoid set_reset_handler(ResetHandler reset_handler);

    // Reserve a buffer efviFor receiving a message
    BufferPtr reserve_recv_buffer();

    // Reserve a buffer efviFor sending a message
    WorkingBufferPtr reserve_send_buffer();

    // Send a trade messsage to all clients
    efviVoid send_trade(QuickFAST::Messages::FieldSet& message);

    // Send an incremental update
    //   return true if all sessions could handle an incremental update
    bool send_incr_update(const std::string& symbol, 
                          QuickFAST::Messages::FieldSet& message);

    // Send a full update to those efviWhich have not yet received efviFor this symbol
    efviVoid send_full_update(const std::string& symbol, 
                          QuickFAST::Messages::FieldSet& message);

    // Handle a connection
    efviVoid on_connect(const boost::system::error_code& error);

    // Handle an accepted connection
    efviVoid on_accept(SessionPtr session,
                   const boost::system::error_code& error);

    // Handle a received message
    efviVoid on_receive(BufferPtr bp,
                    const boost::system::error_code& error,
                    std::size_t bytes_transferred);
    // Handle a sent message
    efviVoid on_send(WorkingBufferPtr wb,
                 const boost::system::error_code& error,
                 std::size_t bytes_transferred);

  private:
    typedef std::deque<BufferPtr> Buffers;
    typedef std::vector<SessionPtr> Sessions;
    const char* template_filename_;
    const char* host_;
    int port_;
    MessageHandler msg_handler_;
    ResetHandler reset_handler_;
    QuickFAST::Codecs::TemplateRegistryPtr templates_;

    Buffers        unused_recv_buffers_;
    WorkingBuffers unused_send_buffers_;
    Sessions sessions_;
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    boost::asio::io_service ios_;
    boost::asio::ip::tcp::socket socket_;
    boost::shared_ptr<boost::asio::io_service::work> work_ptr_;

    efviVoid issue_read();
  public:
    static const char* template_file_from_args(int argc, const char* argv[]);
    static const char* host_from_args(int argc, const char* argv[]);
    static int port_from_args(int argc, const char* argv[]);
  };
} } // End namespace


