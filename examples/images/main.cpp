#include <nodes/DataFlowGraphModel>
#include <nodes/FlowScene>
#include <nodes/DataModelRegistry>
#include <nodes/FlowView>
#include <nodes/NodeData>

#include <QtWidgets/QApplication>
#include <QtGui/QScreen>

#include "ImageShowModel.hpp"
#include "ImageLoaderModel.hpp"

using QtNodes::ConnectionStyle;
using QtNodes::DataFlowGraphModel;
using QtNodes::FlowScene;
using QtNodes::DataModelRegistry;
using QtNodes::FlowView;


static std::shared_ptr<DataModelRegistry>
registerDataModels()
{
  auto ret = std::make_shared<DataModelRegistry>();
  ret->registerModel<ImageShowModel>();

  ret->registerModel<ImageLoaderModel>();

  return ret;
}


int
main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  std::shared_ptr<DataModelRegistry> registry = registerDataModels();

  DataFlowGraphModel dataFlowGraphModel(registry);

  FlowScene scene(dataFlowGraphModel);

  FlowView view(&scene);

  view.setWindowTitle("Data Flow: Resizable Images");
  view.resize(800, 600);
  // Center window.
  view.move(QApplication::primaryScreen()->availableGeometry().center() - view.rect().center());
  view.show();

  return app.exec();
}

