#pragma once

#include <vector>
#include <unordered_map>

#include <QtCore/QPointF>
#include <QtCore/QUuid>

#include "Export.hpp"

#include "Definitions.hpp"
#include "NodeData.hpp"

namespace QtNodes
{

class ConnectionGraphicsObject;
class NodeGraphicsObject;

/// Stores bool for hovering connections and resizing flag.
class NODE_EDITOR_PUBLIC NodeState
{
public:

  NodeState(NodeGraphicsObject & ngo);

public:

  bool
  hovered() const
  { return _hovered; }

  void
  setHovered(bool hovered = true)
  { _hovered = hovered; }

  void
  setResizing(bool resizing);

  bool
  resizing() const;

  ConnectionGraphicsObject const *
  connectionForReaction() const;

  void
  storeConnectionForReaction(ConnectionGraphicsObject const * cgo);

  void
  resetConnectionForReaction();

  /**
   * @brief Adds a new input/output port at the desired index.
   * @param portType Type of port (input or output)
   * @param index Index at which the new port will be placed
   */
  void
  insertPort(const PortType& portType,
             const size_t index);

  /**
   * @brief Removes the input/output port at the given index.
   * @param portType Type of port (input or output)
   * @param index Index at which the port will be removed
   */
  void
  erasePort(const PortType portType,
            const size_t index);

  /**
   * @brief Updates the indices of all connections of this
   * node's ports, starting at the given index. Useful when
   * inserting or removing ports from a node.
   * @param portType Type of port (input or output)
   * @param index Starting index to update
   */
  void
  updateConnectionIndices(const PortType portType,
                          const size_t index);

private:

  NodeGraphicsObject & _ngo;

  bool _hovered;

  bool _resizing;

  ConnectionGraphicsObject const * _connectionForReaction;
};
}
