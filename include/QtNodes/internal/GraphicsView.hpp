#pragma once

#include <QtWidgets/QGraphicsView>

#include "Export.hpp"

namespace QtNodes
{

class BasicGraphicsScene;

class NODE_EDITOR_PUBLIC GraphicsView
  : public QGraphicsView
{
  Q_OBJECT
public:
  GraphicsView(QWidget *parent = Q_NULLPTR);
  GraphicsView(BasicGraphicsScene *scene, QWidget *parent = Q_NULLPTR);

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
  setScene(BasicGraphicsScene *scene);

  void
  centerScene();

public Q_SLOTS:
  void
  scaleUp();

  void
  scaleDown();

  void
  onDeleteSelectedObjects();

  void
  onDuplicateSelectedObjects();

protected:
  void
  contextMenuEvent(QContextMenuEvent *event) override;

  void
  wheelEvent(QWheelEvent *event) override;

  void
  keyPressEvent(QKeyEvent *event) override;

  void
  keyReleaseEvent(QKeyEvent *event) override;

  void
  mousePressEvent(QMouseEvent *event) override;

  void
  mouseMoveEvent(QMouseEvent *event) override;

  void
  drawBackground(QPainter* painter, const QRectF & r) override;

  void
  showEvent(QShowEvent *event) override;

protected:
  BasicGraphicsScene *
  nodeScene();

private:
  QAction* _clearSelectionAction;
  QAction* _deleteSelectionAction;
  QAction* _duplicateSelectionAction;

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
};
}
