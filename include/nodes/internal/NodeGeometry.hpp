#pragma once

#include <QIcon>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtGui/QFontMetrics>
#include <QtGui/QTransform>

#include "Definitions.hpp"
#include "Export.hpp"

namespace QtNodes {

class AbstractGraphModel;
class NodeGraphicsObject;

enum class NodeProcessingStatus;
/**
 * @brief The NodeGeometry class holds the aspects of a node's graphical object
 * geometry in the FlowScene, such as the position of each port within a node.
 * Each connection is associated with a unique geometry object.
 */

/**
 * A helper-class for manipulating the node's geometry.
 * It is designed to be constructed on stack and used in-place.
 * The class is in essense a wrapper around the AbstractGraphModel.
 */
class NODE_EDITOR_PUBLIC NodeGeometry {
 public:
  NodeGeometry(NodeGraphicsObject const& ngo);

 public:
  unsigned int entryHeight() const;

  unsigned int verticalSpacing() const;

 public:
  QRectF boundingRect() const;

  QSize size() const;

  /// Updates size unconditionally
  QSize recalculateSize() const;

  /// Updates size if the QFontMetrics is changed
  QSize recalculateSizeIfFontChanged(QFont const& font) const;

  QPointF portNodePosition(PortType const portType,
                           PortIndex const index) const;

  QPointF portScenePosition(PortType const portType,
                            PortIndex const index,
                            QTransform const& t) const;

  PortIndex checkHitScenePoint(PortType portType,
                               QPointF point,
                               QTransform const& t = QTransform()) const;

  QRect resizeRect() const;

  /// Returns the position of a widget on the Node surface
  QPointF widgetPosition() const;

  /// Returns the maximum height a widget can be without causing the node to
  /// grow.
  int equivalentWidgetHeight() const;

  // unsigned int validationHeight() const;

  // unsigned int validationWidth() const;

  // static
  // QPointF calculateNodePositionBetweenNodePorts(PortIndex targetPortIndex,
  // PortType targetPort, Node * targetNode, PortIndex sourcePortIndex, PortType
  // sourcePort, Node * sourceNode, Node & newNode);

  /**
   * @brief Updates the space reserved for the status icon based on the
   * model's current status.
   */
  void updateStatusIconSize() const;

  /**
   * @brief Returns the size (width and height) of the icon that indicates
   * the node's current processing status.
   */
  QSize statusIconSize() const;

  /**
   * @brief Returns the dimensions of the icon that indicates the node's
   * current processing status.
   */
  QRect statusIconRect() const;

  /**
   * @brief Returns the icon associated with the model's current processing
   * status.
   */
  const QIcon& processingStatusIcon() const;

 private:
  unsigned int captionHeight() const;

  unsigned int captionWidth() const;

  unsigned int nicknameHeight() const;

  unsigned int nicknameWidth() const;

  unsigned int portWidth(PortType portType) const;

 private:
  NodeGraphicsObject const& _ngo;
  AbstractGraphModel& _graphModel;

  // some variables are mutable because
  // we need to change drawing metrics
  // corresponding to fontMetrics
  // but this doesn't change constness of Node

  mutable unsigned int _defaultInPortWidth;
  mutable unsigned int _defaultOutPortWidth;
  mutable unsigned int _entryHeight;
  unsigned int _verticalSpacing;
  mutable QFontMetrics _fontMetrics;
  mutable QFontMetrics _boldFontMetrics;
  mutable QSize _statusIconSize;

  /**
   * @brief Flag indicating whether the processing status icon should be part
   * of the node's geometry.
   */
  mutable bool _statusIconActive;

  //  std::unique_ptr<NodeDataModel> const& _dataModel;

  /**
   * @brief Processing status icons
   */
  const QIcon _statusUpdated{"://status_icons/updated.svg"};
  const QIcon _statusProcessing{"://status_icons/processing.svg"};
  const QIcon _statusPending{"://status_icons/pending.svg"};
  const QIcon _statusInvalid{"://status_icons/failed.svg"};
  const QIcon _statusEmpty{"://status_icons/empty.svg"};
  const QIcon _statusPartial{"://status_icons/partial.svg"};

  unsigned int _entryWidth;  // TODO: where was is used?

  unsigned int _nSources;  // TODO: where was it used?
  unsigned int _nSinks;

  QPointF _draggingPos;
};
}  // namespace QtNodes
