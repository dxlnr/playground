#include <iostream>

#include "ipc.h"
#include "impl_msgq.h"
#include "impl_zmq.h"

bool messaging_use_zmq()
{
  if (std::getenv("ZMQ")) { return true; }
  return false;
}

Context * Context::create()
{
  Context * c;
  if (messaging_use_zmq()) {
    c = new ZMQContext();
  } else {
    c = new MSGQContext();
  }
  return c;
}

SubSocket * SubSocket::create()
{
  SubSocket * s;
  if (messaging_use_zmq()) {
    s = new ZMQSubSocket();
  } else {
    s = new MSGQSubSocket();
  }
  return s;
}

SubSocket * SubSocket::create(Context * context, std::string endpoint, std::string address, bool conflate, bool check_endpoint, int buf_size)
{
  SubSocket *s = SubSocket::create();
  int r = s->connect(context, endpoint, address, conflate, check_endpoint, buf_size);

  if (r == 0) {
    return s;
  } else {
    std::cerr << "Error, failed to connect SubSocket to " << endpoint << ": " << strerror(errno) << std::endl;

    delete s;
    return nullptr;
  }
}

PubSocket * PubSocket::create()
{
  PubSocket * s;
  if (messaging_use_zmq()) {
    s = new ZMQPubSocket();
  } else {
    s = new MSGQPubSocket();
  }
  return s;
}

PubSocket * PubSocket::create(Context * context, std::string endpoint, bool check_endpoint, int buf_size)
{
  PubSocket *s = PubSocket::create();
  int r = s->connect(context, endpoint, check_endpoint, buf_size);

  if (r == 0) {
    return s;
  } else {
    std::cerr << "Error, failed to bind PubSocket to " << endpoint << ": " << strerror(errno) << std::endl;

    delete s;
    return nullptr;
  }
}

Poller * Poller::create()
{
  Poller * p;
  if (messaging_use_zmq()) {
    p = new ZMQPoller();
  } else {
    p = new MSGQPoller();
  }
  return p;
}

Poller * Poller::create(std::vector<SubSocket*> sockets)
{
  Poller * p = Poller::create();

  for (auto s : sockets) {
    p->registerSocket(s);
  }
  return p;
}
