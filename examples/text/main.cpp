#include <nodes/DataFlowGraphModel>
#include <nodes/FlowScene>
#include <nodes/DataModelRegistry>
#include <nodes/FlowView>
#include <nodes/NodeData>

#include <QtWidgets/QApplication>

#include <nodes/DataModelRegistry>

#include "TextSourceDataModel.hpp"
#include "TextDisplayDataModel.hpp"

using QtNodes::DataFlowGraphModel;
using QtNodes::FlowScene;
using QtNodes::DataModelRegistry;
using QtNodes::FlowView;

static std::shared_ptr<DataModelRegistry>
registerDataModels()
{
  auto ret = std::make_shared<DataModelRegistry>();

  ret->registerModel<TextSourceDataModel>();
  ret->registerModel<TextDisplayDataModel>();

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

  view.setWindowTitle("Node-based flow editor");
  view.resize(800, 600);
  view.show();

  return app.exec();
}
