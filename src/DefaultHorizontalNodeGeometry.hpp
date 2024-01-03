#pragma once

#include "AbstractNodeGeometry.hpp"

#include <QtGui/QFontMetrics>

namespace QtNodes {

class AbstractGraphModel;
class BasicGraphicsScene;

class NODE_EDITOR_PUBLIC DefaultHorizontalNodeGeometry
    : public AbstractNodeGeometry {
 public:
  DefaultHorizontalNodeGeometry(AbstractGraphModel& graphModel);

 public:
  QRectF boundingRect(NodeId const nodeId) const override;

  QSize size(NodeId const nodeId) const override;

  void recomputeSize(NodeId const nodeId) const override;

  QPointF portPosition(NodeId const nodeId,
                       PortType const portType,
                       PortIndex const index) const override;

  QPointF portTextPosition(NodeId const nodeId,
                           PortType const portType,
                           PortIndex const PortIndex) const override;
  QPointF captionPosition(NodeId const nodeId) const override;

  QRectF captionRect(NodeId const nodeId) const override;

  QPointF widgetPosition(NodeId const nodeId) const override;

  QRect resizeHandleRect(NodeId const nodeId) const override;

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
  QRectF portTextRect(NodeId const nodeId,
                      PortType const portType,
                      PortIndex const portIndex) const;

  /// Finds max number of ports and multiplies by (a port height + interval)
  unsigned int maxVerticalPortsExtent(NodeId const nodeId) const;

  unsigned int maxPortsTextAdvance(NodeId const nodeId,
                                   PortType const portType) const;

 private:
  // Some variables are mutable because we need to change drawing
  // metrics corresponding to fontMetrics but this doesn't change
  // constness of the Node.

  mutable unsigned int _portSize;
  unsigned int _portSpasing;
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
