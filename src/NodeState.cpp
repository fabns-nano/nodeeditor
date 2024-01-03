#include "NodeState.hpp"

#include "ConnectionGraphicsObject.hpp"
#include "NodeGraphicsObject.hpp"

namespace QtNodes {

NodeState::NodeState(NodeGraphicsObject& ngo)
    : _ngo(ngo),
      _hovered(false),
      _resizing(false),
      _connectionForReaction(nullptr) {
  Q_UNUSED(_ngo);
}

void NodeState::setResizing(bool resizing) {
  _resizing = resizing;
}

bool NodeState::resizing() const {
  return _resizing;
}

ConnectionGraphicsObject const* NodeState::connectionForReaction() const {
  return _connectionForReaction;
}

void NodeState::storeConnectionForReaction(
    ConnectionGraphicsObject const* cgo) {
  _connectionForReaction = cgo;
}

void NodeState::resetConnectionForReaction() {
  _connectionForReaction = nullptr;
}

}  // namespace QtNodes

// void
// NodeState::
// insertPort(const PortType& portType,
//            const size_t index)
// {
//   auto& ports = getEntries(portType);
//   ports.emplace(std::next(ports.begin(), index));
//   updateConnectionIndices(portType, index + 1);
// }

// void
// NodeState::
// erasePort(const PortType portType,
//           const size_t index)
// {
//   auto& ports = getEntries(portType);
//   auto erased_port_map_it = std::next(ports.begin(), index);

//   // erases port connections
//   for (auto& entry : *erased_port_map_it)
//   {
//     eraseConnection(portType, index, entry.first);
//   }

//   // erases port
//   ports.erase(erased_port_map_it);

//   // reassigns subsequent ports
//   updateConnectionIndices(portType, index);
// }

// void
// NodeState::
// updateConnectionIndices(const PortType portType,
//                         const size_t index)
// {
//   auto& ports = getEntries(portType);
//   for (size_t i = index; i < ports.size(); i++)
//   {
//     for (const auto& entry : ports[i])
//     {
//       entry.second->setPortIndex(portType, i);
//     }
//   }
// }
