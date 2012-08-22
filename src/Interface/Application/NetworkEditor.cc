/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#include <sstream>
#include <fstream>
#include <QtGui>
#include <iostream>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/Node.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/ModuleWidget.h>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/Utility.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/Logger.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
//#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>

#include <boost/bind.hpp>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Domain::Networks;

boost::shared_ptr<Logger> Logger::instance_;

NetworkEditor::NetworkEditor(boost::shared_ptr<CurrentModuleSelection> moduleSelectionGetter, QWidget* parent) : QGraphicsView(parent),
  moduleSelectionGetter_(moduleSelectionGetter),
  moduleDumpAction_(0)
{
  scene_ = new QGraphicsScene(0, 0, 1000, 1000);
  scene_->setBackgroundBrush(Qt::darkGray);
  PortWidget::TheScene = scene_;

  setScene(scene_);
  setDragMode(QGraphicsView::RubberBandDrag);
  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  setContextMenuPolicy(Qt::ActionsContextMenu);

  minZ_ = 0;
  maxZ_ = 0;
  seqNumber_ = 0;

  createActions();

  connect(scene_, SIGNAL(selectionChanged()), this, SLOT(updateActions()));

  updateActions();
}

void NetworkEditor::setNetworkEditorController(boost::shared_ptr<NetworkEditorControllerGuiProxy> controller)
{
  if (controller_ == controller)
    return;

  if (controller_) 
  {
    disconnect(controller_.get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Domain::Networks::ModuleHandle)), 
      this, SLOT(addModule(const std::string&, SCIRun::Domain::Networks::ModuleHandle)));

    disconnect(this, SIGNAL(connectionDeleted(const SCIRun::Domain::Networks::ConnectionId&)), 
      controller_.get(), SLOT(removeConnection(const SCIRun::Domain::Networks::ConnectionId&)));
  }
  
  controller_ = controller;
  
  if (controller_) 
  {
    connect(controller_.get(), SIGNAL(moduleAdded(const std::string&, SCIRun::Domain::Networks::ModuleHandle)), 
      this, SLOT(addModule(const std::string&, SCIRun::Domain::Networks::ModuleHandle)));
    
    connect(this, SIGNAL(connectionDeleted(const SCIRun::Domain::Networks::ConnectionId&)), 
      controller_.get(), SLOT(removeConnection(const SCIRun::Domain::Networks::ConnectionId&)));
  }
}

void NetworkEditor::addModule(const std::string& name, SCIRun::Domain::Networks::ModuleHandle module)
{
  ModuleWidget* moduleWidget = new ModuleWidget(QString::fromStdString(name), module);
  setupModule(moduleWidget);
  Q_EMIT modified();
}

void NetworkEditor::needConnection(const SCIRun::Domain::Networks::ConnectionDescription& cd)
{
  controller_->addConnection(cd);
  Q_EMIT modified();
}

void NetworkEditor::setupModule(ModuleWidget* module)
{
  ModuleProxyWidget* proxy = new ModuleProxyWidget(module);
  connect(module, SIGNAL(removeModule(const std::string&)), controller_.get(), SLOT(removeModule(const std::string&)));
  connect(module, SIGNAL(removeModule(const std::string&)), this, SIGNAL(modified()));
  connect(module, SIGNAL(needConnection(const SCIRun::Domain::Networks::ConnectionDescription&)), 
    this, SLOT(needConnection(const SCIRun::Domain::Networks::ConnectionDescription&)));
  connect(controller_.get(), SIGNAL(connectionAdded(const SCIRun::Domain::Networks::ConnectionDescription&)), 
    module, SIGNAL(connectionAdded(const SCIRun::Domain::Networks::ConnectionDescription&)));
  connect(module, SIGNAL(connectionDeleted(const SCIRun::Domain::Networks::ConnectionId&)), 
    this, SIGNAL(connectionDeleted(const SCIRun::Domain::Networks::ConnectionId&)));
  connect(module, SIGNAL(connectionDeleted(const SCIRun::Domain::Networks::ConnectionId&)), this, SIGNAL(modified()));
  module->getModule()->get_state()->connect_state_changed(boost::bind(&NetworkEditor::modified, this));

  proxy->setZValue(maxZ_);
  proxy->setVisible(true);
  proxy->setSelected(true);
  proxy->setPos(lastModulePosition_);
  proxy->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
  connect(scene_, SIGNAL(selectionChanged()), proxy, SLOT(highlightIfSelected()));
  connect(proxy, SIGNAL(selected()), this, SLOT(bringToFront()));
  proxy->createPortPositionProviders();

  scene_->addItem(proxy);
  ++seqNumber_;

  scene_->clearSelection();
  proxy->setSelected(true);
  bringToFront();

  Logger::Instance()->log("Module added.");
}

void NetworkEditor::bringToFront()
{
  ++maxZ_;
  setZValue(maxZ_);
}

void NetworkEditor::sendToBack()
{
  --minZ_;
  setZValue(minZ_);
}

void NetworkEditor::setZValue(int z)
{
  ModuleProxyWidget* node = selectedModuleProxy();
  if (node)
    node->setZValue(z);
}

ModuleProxyWidget* getModuleProxy(QGraphicsItem* item)
{
  return dynamic_cast<ModuleProxyWidget*>(item);
}

ModuleWidget* getModule(QGraphicsItem* item)
{
  ModuleProxyWidget* proxy = getModuleProxy(item);
  if (proxy)
    return static_cast<ModuleWidget*>(proxy->widget());
  return 0;
}

//TODO copy/paste
ModuleWidget* NetworkEditor::selectedModule() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
  {
    return getModule(items.first());
  }
  return 0;
}

ModuleProxyWidget* NetworkEditor::selectedModuleProxy() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
  {
    return getModuleProxy(items.first());
  }
  return 0;
}

ConnectionLine* NetworkEditor::selectedLink() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 1)
    return dynamic_cast<ConnectionLine*>(items.first());
  return 0;
}

NetworkEditor::ModulePair NetworkEditor::selectedModulePair() const
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  if (items.count() == 2)
  {
    ModuleWidget* first = getModule(items.first());
    ModuleWidget* second = getModule(items.last());
    if (first && second)
      return ModulePair(first, second);
  }
  return ModulePair();
}

void NetworkEditor::del()
{
  QList<QGraphicsItem*> items = scene_->selectedItems();
  QMutableListIterator<QGraphicsItem*> i(items);
  while (i.hasNext())
  {
    ConnectionLine* link = dynamic_cast<ConnectionLine*>(i.next());
    if (link)
    {
      delete link;
      i.remove();
    }
  }
  qDeleteAll(items);
  Q_EMIT modified();
}

void NetworkEditor::properties()
{
  ModuleWidget* node = selectedModule();
  ConnectionLine* link = selectedLink();

  if (node)
  {
    //PropertiesDialog dialog(node, this);
    //dialog.exec();
  }
  else if (link)
  {
    //QColor color = QColorDialog::getColor(link->color(), this);
    //if (color.isValid())
    //  link->setColor(color);
  }
}

void NetworkEditor::cut()
{
  //Module* node = selectedModule();
  //if (!node)
  //  return;

  //copy();
  //delete node;
}

void NetworkEditor::copy()
{
  //Module* node = selectedModule();
  //if (!node)
  //  return;

  //QString str = QString("Module %1 %2 %3 %4")
  //              .arg(node->textColor().name())
  //              .arg(node->outlineColor().name())
  //              .arg(node->backgroundColor().name())
  //              .arg(node->text());
  //QApplication::clipboard()->setText(str);
}

void NetworkEditor::paste()
{
  //QString str = QApplication::clipboard()->text();
  //QStringList parts = str.split(" ");
  //if (parts.count() >= 5 && parts.first() == "Node")
  //{
  //  Module* node = new Module;
  //  node->setText(QStringList(parts.mid(4)).join(" "));
  //  node->setTextColor(QColor(parts[1]));
  //  node->setOutlineColor(QColor(parts[2]));
  //  node->setBackgroundColor(QColor(parts[3]));
  //  setupNode(node);
  //}
}

void NetworkEditor::updateActions()
{
  const bool hasSelection = !scene_->selectedItems().isEmpty();
  const bool isNode = (selectedModule() != 0);
  const bool isLink = (selectedLink() != 0);
  const bool isNodePair = (selectedModulePair() != ModulePair());

  //cutAction_->setEnabled(isNode);
  //copyAction_->setEnabled(isNode);
  //addLinkAction_->setEnabled(isNodePair);
  deleteAction_->setEnabled(hasSelection);
  bringToFrontAction_->setEnabled(isNode);
  sendToBackAction_->setEnabled(isNode);
  propertiesAction_->setEnabled(isNode || isLink);

  Q_FOREACH (QAction* action, actions())
    removeAction(action);

  //foreach (QAction* action, editToolBar_->actions())
  //{
  //  if (action->isEnabled())
  //    view_->addAction(action);
  //}
}

void NetworkEditor::createActions()
{
  //exitAction_ = new QAction(tr("E&xit"), this);
  //exitAction_->setShortcut(tr("Ctrl+Q"));
  //connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));

  //addNodeAction_ = new QAction(tr("Add &Module"), this);
  //addNodeAction_->setIcon(QIcon(":/images/node.png"));
  //addNodeAction_->setShortcut(tr("Ctrl+N"));
  //connect(addNodeAction_, SIGNAL(triggered()), this, SLOT(addModule()));

  //addLinkAction_ = new QAction(tr("Add &Connection"), this);
  //addLinkAction_->setIcon(QIcon(":/images/link.png"));
  //addLinkAction_->setShortcut(tr("Ctrl+L"));
  //connect(addLinkAction_, SIGNAL(triggered()), this, SLOT(addLink()));

  deleteAction_ = new QAction(tr("&Delete"), this);
  deleteAction_->setIcon(QIcon(":/images/delete.png"));
  deleteAction_->setShortcut(tr("Del"));
  connect(deleteAction_, SIGNAL(triggered()), this, SLOT(del()));

  //cutAction_ = new QAction(tr("Cu&t"), this);
  //cutAction_->setIcon(QIcon(":/images/cut.png"));
  //cutAction_->setShortcut(tr("Ctrl+X"));
  //connect(cutAction_, SIGNAL(triggered()), this, SLOT(cut()));

  //copyAction_ = new QAction(tr("&Copy"), this);
  //copyAction_->setIcon(QIcon(":/images/copy.png"));
  //copyAction_->setShortcut(tr("Ctrl+C"));
  //connect(copyAction_, SIGNAL(triggered()), this, SLOT(copy()));

  //pasteAction_ = new QAction(tr("&Paste"), this);
  //pasteAction_->setIcon(QIcon(":/images/paste.png"));
  //pasteAction_->setShortcut(tr("Ctrl+V"));
  //connect(pasteAction_, SIGNAL(triggered()), this, SLOT(paste()));

  bringToFrontAction_ = new QAction(tr("Bring to &Front"), this);
  bringToFrontAction_->setIcon(QIcon(":/images/bringtofront.png"));
  connect(bringToFrontAction_, SIGNAL(triggered()),
    this, SLOT(bringToFront()));

  sendToBackAction_ = new QAction(tr("&Send to Back"), this);
  sendToBackAction_->setIcon(QIcon(":/images/sendtoback.png"));
  connect(sendToBackAction_, SIGNAL(triggered()),
    this, SLOT(sendToBack()));

  propertiesAction_ = new QAction(tr("P&roperties..."), this);
  connect(propertiesAction_, SIGNAL(triggered()),
    this, SLOT(properties()));
}

void NetworkEditor::setModuleDumpAction(QAction* action)
{
  moduleDumpAction_ = action; 
  if (moduleDumpAction_)
    connect(moduleDumpAction_, SIGNAL(triggered()), this, SLOT(dumpModulePositions()));
}

void NetworkEditor::addActions(QWidget* widget)
{
  //widget->addAction(addNodeAction_);
  //widget->addAction(addLinkAction_);
  widget->addAction(bringToFrontAction_);
  widget->addAction(sendToBackAction_);
  //widget->addAction(cutAction_);
  //widget->addAction(copyAction_);
  //widget->addAction(pasteAction_);
  widget->addAction(deleteAction_);
}

void NetworkEditor::dropEvent(QDropEvent* event)
{
  //TODO: mime check here to ensure this only gets called for drags from treewidget
  if (moduleSelectionGetter_->isModule())
  {
    lastModulePosition_ = mapToScene(event->pos());
    controller_->addModule(moduleSelectionGetter_->text().toStdString());
    Q_EMIT modified();
  }
}

void NetworkEditor::dragEnterEvent(QDragEnterEvent* event)
{
  //???
  event->acceptProposedAction();
}
  
void NetworkEditor::dragMoveEvent(QDragMoveEvent* event)
{
}

SCIRun::Domain::Networks::ModulePositionsHandle NetworkEditor::dumpModulePositions()
{
  ModulePositionsHandle positions(new ModulePositions);
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (ModuleProxyWidget* w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      positions->modulePositions[w->getModuleWidget()->getModuleId()] = std::make_pair(item->scenePos().x(), item->scenePos().y());
    }
  }

  return positions;
}

void NetworkEditor::executeAll()
{
  controller_->executeAll(*this);
  //TODO: not sure about this right now.
  //Q_EMIT modified();
}

ExecutableObject* NetworkEditor::lookupExecutable(const std::string& id) const
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (ModuleProxyWidget* w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      if (id == w->getModuleWidget()->getModuleId())
        return w->getModuleWidget();
    }
  }
  return 0;
}

void NetworkEditor::clear()
{
  scene_->clear();
  //TODO: this (unwritten) method does not need to be called here.  the dtors of all the module widgets get called when the scene_ is cleared, which triggered removal from the underlying network.
  // we'll need a similar hook when programming the scripting interface (moduleWidgets<->modules).
  //controller_->clear();
  Q_EMIT modified();
}

void NetworkEditor::moveModules(const ModulePositions& modulePositions)
{
  Q_FOREACH(QGraphicsItem* item, scene_->items())
  {
    if (ModuleProxyWidget* w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      ModulePositions::Data::const_iterator posIter = modulePositions.modulePositions.find(w->getModuleWidget()->getModuleId());
      if (posIter != modulePositions.modulePositions.end())
        w->setPos(posIter->second.first, posIter->second.second);
    }
  }
}
