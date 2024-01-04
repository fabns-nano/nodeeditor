#pragma once

#include <QtWidgets/QGraphicsView>

#include "Export.hpp"

namespace QtNodes
{

class BasicGraphicsScene;


/**
 * @brief A central view able to render objects from `BasicGraphicsScene`.
 */
class NODE_EDITOR_PUBLIC GraphicsView
  : public QGraphicsView
{
  Q_OBJECT
public:
  struct ScaleRange
  {
    double minimum = 0;
    double maximum = 0;
  };

public:
  GraphicsView(QWidget* parent = Q_NULLPTR);
  GraphicsView(BasicGraphicsScene* scene, QWidget* parent = Q_NULLPTR);

  GraphicsView(const GraphicsView &) = delete;
  GraphicsView
  operator=(const GraphicsView &) = delete;

  QAction*
  clearSelectionAction() const;

  QAction*
  deleteSelectionAction() const;

  /**
   * @brief Returns the "copy selection" action.
   */
  QAction* copySelectionAction() const;

  /**
   * @brief Copies the selected scene items to the clipboard.
   */
  void copySelectionToClipboard();

  /**
   * @brief Cuts the selected scene items to the clipboard.
   */
  void cutSelectionToClipboard();

  /**
   * @brief Pastes the clipboard items on the scene.
   */
  void pasteFromClipboard();

  /**
   * @brief Handles a "load group from file" action, setting where the group should
   * be positioned and calling the appropriate flow scene function.
   */
  void handleLoadGroup();


  /**
   * @brief Returns the "cut selection" action.
   */
  QAction* cutSelectionAction() const;

  /**
   * @brief Returns the "paste from clipboard" action.
   */
  QAction* pasteClipboardAction() const;

  /**
   * @brief Returns the "create group from selection" action.
   */
  QAction* createGroupFromSelectionAction() const;

  /**
   * @brief _createGroupFromSelectionAction Action that triggers the creation of a new group from the
   * selected items.
   */
  QAction* _createGroupFromSelectionAction;

  /**
   * @brief Returns the "load group from file" action.
   */
  QAction* loadGroupAction() const;

  void
  setScene(BasicGraphicsScene* scene);

  void
  centerScene();

  /// @brief max=0/min=0 indicates infinite zoom in/out
  void
  setScaleRange(double minimum = 0, double maximum = 0);

  void
  setScaleRange(ScaleRange range);

  double
  getScale() const;

public Q_SLOTS:
  void
  scaleUp();

  void
  scaleDown();

  void
  setupScale(double scale);

  void
  onDeleteSelectedObjects();

  void
  onDuplicateSelectedObjects();

  void
  onCopySelectedObjects();

  void
  onPasteObjects();

Q_SIGNALS:
  void
  scaleChanged(double scale);

protected:
  void
  contextMenuEvent(QContextMenuEvent* event) override;

  void
  wheelEvent(QWheelEvent* event) override;

  void
  keyPressEvent(QKeyEvent* event) override;

  void
  keyReleaseEvent(QKeyEvent* event) override;

  void
  mousePressEvent(QMouseEvent* event) override;

  void
  mouseMoveEvent(QMouseEvent* event) override;

  void
  drawBackground(QPainter* painter, const QRectF & r) override;

  void
  showEvent(QShowEvent* event) override;

protected:
  BasicGraphicsScene*
  nodeScene();

  /// Computes scene position for pasting the copied/duplicated node groups.
  QPointF
  scenePastePosition();

private:
  QAction* _clearSelectionAction = nullptr;
  QAction* _deleteSelectionAction = nullptr;
  QAction* _duplicateSelectionAction = nullptr;
  QAction* _copySelectionAction = nullptr;
  QAction* _pasteAction = nullptr;

  /**
   * @brief _copySelectionAction Action that triggers copying the selected scene items to the clipboard.
   */
  QAction* _copySelectionAction;

  /**
   * @brief _cutSelectionAction Action that triggers cutting the selected scene items to the clipboard.
   */
  QAction* _cutSelectionAction;

  /**
   * @brief _pasteClipboardAction Action that triggers pasting of the stored clipboard on the scene.
   */
  QAction* _pasteClipboardAction;

  /**
   * @brief _loadGroupAction Action to load a node group from a .group file.
   */
  QAction* _loadGroupAction;


  QPointF _clickPos;
  ScaleRange _scaleRange;
};
}
