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

#include <iostream>
#include <QtGui>
#include <Interface/Application/Module.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/Port.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/PositionProvider.h>
#include <Interface/Application/Logger.h>

using namespace SCIRun::Gui;

QPointF ProxyWidgetPosition::currentPosition() const
{
  return widget_->pos() + offset_;
}

Module::Module(const QString& name, QWidget* parent /* = 0 */)
  : QFrame(parent)
{
  setupUi(this);
  titleLabel_->setText(name);
  progressBar_->setMaximum(100);
  progressBar_->setMinimum(0);
  progressBar_->setValue(0);
  
  addAllHardCodedPorts(name);
}

void Module::addAllHardCodedPorts(const QString& name)
{
  outputPortLayout_ = new QHBoxLayout;
  outputPortLayout_->setSpacing(3);
  
  QHBoxLayout* outputRowLayout = new QHBoxLayout;
  outputRowLayout->setAlignment(Qt::AlignLeft);
  outputRowLayout->addLayout(outputPortLayout_);
  verticalLayout_2->insertLayout(-1, outputRowLayout);

  inputPortLayout_ = new QHBoxLayout;
  inputPortLayout_->setSpacing(3);
  QHBoxLayout* inputRowLayout = new QHBoxLayout;
  inputRowLayout->setAlignment(Qt::AlignLeft);
  inputRowLayout->addLayout(inputPortLayout_);
  verticalLayout_2->insertLayout(0, inputRowLayout);

  //TODO: extract into factory
  if (name.contains("ComputeSVD"))
  {
    addPort(new OutputPort("Output1", Qt::blue, this));
    addPort(new OutputPort("Output2", Qt::blue, this));
    addPort(new OutputPort("Output2", Qt::blue, this));
    addPort(new InputPort("Input1", Qt::blue, this));
    optionsButton_->setVisible(false);
  }
  else if (name.contains("ReadMatrix"))
  {
    addPort(new OutputPort("Output1", Qt::blue, this));
    addPort(new OutputPort("Output1", Qt::darkGreen, this));
    addPort(new InputPort("Input1", Qt::darkGreen, this));
  }
  else if (name.contains("WriteMatrix"))
  {
    addPort(new InputPort("Input1", Qt::blue, this));
    addPort(new InputPort("Input1", Qt::darkGreen, this));
  }
}

void Module::addPort(OutputPort* port)
{
  outputPortLayout_->addWidget(port);
  ports_.push_back(port);
}

void Module::addPort(InputPort* port)
{
  inputPortLayout_->addWidget(port);
  ports_.push_back(port);
}

Module::~Module()
{
  foreach (Port* p, ports_)
    p->deleteConnections();
  Logger::Instance->log("Module deleted.");
}

void Module::trackConnections()
{
  foreach (Port* p, ports_)
    p->trackConnections();
}

QPointF Module::inputPortPosition() const 
{
  if (positionProvider_)
    return positionProvider_->currentPosition() + QPointF(20,10);
  return pos();
}

QPointF Module::outputPortPosition() const 
{
  if (positionProvider_)
    return positionProvider_->currentPosition() + QPointF(20, height() - 10);
  return pos();
}

double Module::percentComplete() const
{
  return progressBar_->value() / 100.0;
}

void Module::setPercentComplete(double p)
{
  if (0 <= p && p <= 1)
  {
    progressBar_->setValue(100 * p);
  }
}

void Module::incrementProgressFake()
{
  setPercentComplete(percentComplete() + 0.1);
}
