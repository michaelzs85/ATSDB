/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtWidgets/QTabWidget>
#include <QtWidgets/QHBoxLayout>


#include "listboxview.h"
#include "listboxviewdatawidget.h"
#include "listboxviewwidget.h"
#include "listboxviewconfigwidget.h"

/*
 */
ListBoxViewWidget::ListBoxViewWidget(const std::string& class_id, const std::string& instance_id,
                                     Configurable* config_parent, ListBoxView* view, QWidget* parent)
 : ViewWidget( class_id, instance_id, config_parent, view, parent ), data_widget_(nullptr), config_widget_(nullptr)
{
    setAutoFillBackground(true);

    QHBoxLayout *hlayout = new QHBoxLayout;

    data_widget_ = new ListBoxViewDataWidget (getView(), view->getDataSource());
    data_widget_->setAutoFillBackground(true);
    QSizePolicy sp_left(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp_left.setHorizontalStretch(3);
    data_widget_->setSizePolicy(sp_left);
    hlayout->addWidget( data_widget_ );


    config_widget_ = new ListBoxViewConfigWidget (getView());
    config_widget_->setAutoFillBackground(true);
    QSizePolicy sp_right(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp_right.setHorizontalStretch(1);
    config_widget_->setSizePolicy(sp_right);

    hlayout->addWidget( config_widget_ );

    setLayout( hlayout );

    setFocusPolicy( Qt::StrongFocus );

    //connect stuff here
    //connect( config_widget_, SIGNAL(variableChanged()), this, SLOT(variableChangedSlot()) );
}

/*
 */
ListBoxViewWidget::~ListBoxViewWidget()
{
}

/*
 */
void ListBoxViewWidget::updateView()
{
}

/*
 */
void ListBoxViewWidget::toggleConfigWidget()
{
    assert (config_widget_);
    bool vis = config_widget_->isVisible();
    config_widget_->setVisible( !vis );
}

/*
 */
ListBoxViewConfigWidget* ListBoxViewWidget::configWidget()
{
    return config_widget_;
}

