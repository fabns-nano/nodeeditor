#pragma once

#include <QtGui/QPainter>

#include "Definitions.hpp"

namespace QtNodes {

class BasicGraphicsScene;
class GraphModel;
class NodeGeometry;
class NodeGraphicsObject;
class NodeState;

/// @ Lightweight class incapsulating paint code.
class NodePainter {
 public:
  NodePainter();

 public:
  static void paint(QPainter* painter, NodeGraphicsObject& ngo);

  static void drawNodeRect(QPainter* painter, NodeGraphicsObject& ngo);

  static void drawConnectionPoints(QPainter* painter, NodeGraphicsObject& ngo);
  static void drawFilledConnectionPoints(QPainter* painter,
                                         NodeGraphicsObject& ngo);

  static void drawNodeCaption(QPainter* painter, NodeGraphicsObject& ngo);

  static void drawEntryLabels(QPainter* painter, NodeGraphicsObject const& ngo);

  static void drawResizeRect(QPainter* painter, NodeGraphicsObject const& ngo);

  static void drawModelNickname(QPainter* painter,
                                NodeGraphicsObject const& ngo);

  /**
   * @brief Draws the icon indicating the node's current processing status.
   * @param painter Painter to be used
   * @param geom Node geometry
   * @param model Node model
   */
  static void drawStatusIcon(QPainter* painter, NodeGraphicsObject const& ngo);

  /**
   * @brief Draws the progress value percentage in the processing node.
   * @param painter Painter to be used
   * @param geom Node geometry
   * @param model Node model
   */
  static void drawProgressValue(QPainter* painter,
                                NodeGraphicsObject const& ngo,
                                QString const& nodeProgress);
};
}  // namespace QtNodes
