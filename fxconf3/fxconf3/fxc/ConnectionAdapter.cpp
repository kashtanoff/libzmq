#pragma once

#include "zmq.h"
#include "fxc.h"

#define CONN_ERRT_SOCK  1
#define CONN_ERRT_SEND  2
#define CONN_ERRT_RECV  3
#define CONN_ERRT_CLOSE 4
#define CONN_ERRT_TERM  5

namespace fxc {

	class ConnectionAdapter {

		public:

			ConnectionAdapter(const char* addr) {
				_context = zmq_ctx_new();
				_socket  = zmq_socket(_context, ZMQ_REQ);
				_connerr = zmq_connect(_socket, addr);

				zmq_setsockopt(_socket, ZMQ_LINGER,        &linger,     sizeof(linger));
				zmq_setsockopt(_socket, ZMQ_SNDTIMEO,      &sndTimeout, sizeof(sndTimeout));
				zmq_setsockopt(_socket, ZMQ_RCVTIMEO,      &rcvTimeout, sizeof(rcvTimeout));
				zmq_setsockopt(_socket, ZMQ_REQ_CORRELATE, &correlate,  sizeof(correlate));
			}
			~ConnectionAdapter() {
				destroyConnection();
			}

			const bool send(std::string message) {
				if (0 != _connerr) {
					_errtype = CONN_ERRT_SOCK;
					fxc::msg << "-> connect error: " << _connerr << "\r\n" << fxc::msg_box;

					destroyConnection();
					return false;
				}

				if (-1 == zmq_send(_socket, message.c_str(), strlen(message.c_str()), 0)) {
					_errtype = CONN_ERRT_SEND;
					_errno   = zmq_errno();
					fxc::msg << "-> send error: " << _errno << " - " << zmq_strerror(_errno) << "\r\n" << fxc::msg_box;

					destroyConnection();
					return false;
				}

				char buffer[10240];
				auto recvSize = zmq_recv(_socket, &buffer, sizeof(buffer), 0);

				if (-1 == recvSize) {
					_errtype = CONN_ERRT_RECV;
					_errno   = zmq_errno();
					fxc::msg << "-> receive error: " << _errno << " - " << zmq_strerror(_errno) << "\r\n" << fxc::msg_box;

					destroyConnection();
					return false;
				}

				_response = std::string(buffer).substr(0, recvSize);
				fxc::msg << "-> received: [" << _response << "]\r\n" << fxc::msg_box;

				destroyConnection();
				return true;
			}
			const int& getErrNo() {
				return _errno;
			}

			const int& getErrType() {
				return _errtype;
			}

			const std::string& getResponse() {
				return _response;
			}

		private:

			const int   linger     = 2000;
			const int   sndTimeout = 2000;
			const int   rcvTimeout = 2000;
			const bool  correlate  = true;

			void* _context;
			void* _socket;
			int   _connerr = 0;
			int   _errtype = 0;
			int   _errno   = 0;

			std::string _response;

			void destroyConnection() {
				if (-1 == zmq_close(_socket)) {
					_errtype = CONN_ERRT_CLOSE;
					_errno = zmq_errno();
					//fxc::msg << "-> close error: " << _errno << " - " << zmq_strerror(_errno) << "\r\n" << fxc::msg_box;
				}

				if (-1 == zmq_ctx_term(_context)) {
					_errtype = CONN_ERRT_TERM;
					_errno = zmq_errno();
					//fxc::msg << "-> terminate error: " << _errno << " - " << zmq_strerror(_errno) << "\r\n" << fxc::msg_box;
				}
			}

	};

}