#pragma once
#include <vector>
#include <memory>
#include "SocketConnector.h"

class SocketRegistry
{
  public:
    SocketRegistry() = default;
    ~SocketRegistry() = default;
    void add(std::shared_ptr<ServerConnector>& conn)
    {
      conns_.push_back(conn);
    }
    void forEach(std::function<void(std::shared_ptr<ServerConnector>&)> func)
    {
      std::for_each(conns_.begin(), conns_.end(), func);
    }
    static SocketRegistry& inst()
    {
      static SocketRegistry instance;
      return instance;
    }
  private:
    std::vector<std::shared_ptr<ServerConnector>> conns_;
};