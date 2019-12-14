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

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QStackedWidget>

#include "atsdb.h"
#include "dbinterfacewidget.h"
#include "dbinterface.h"
#include "dbconnection.h"
#include "logger.h"
#include "stringconv.h"
#include "global.h"

using namespace Utils;


DBInterfaceWidget::DBInterfaceWidget(DBInterface &interface, QWidget* parent, Qt::WindowFlags f)
    : interface_(interface), connection_stack_ (nullptr)
{
    unsigned int frame_width = FRAME_SIZE;
    QFont font_bold;
    font_bold.setBold(true);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(frame_width);

    QVBoxLayout *layout = new QVBoxLayout ();

    QGroupBox *groupBox = new QGroupBox(tr("Database System"));
    QVBoxLayout *grplayout = new QVBoxLayout ();

    connection_stack_ = new QStackedWidget ();

    const std::map<std::string, DBConnection*> &types = interface_.connections();
    for (auto it : types)
    {
        QRadioButton *radio = new QRadioButton(it.first.c_str(), this);
        connect(radio, SIGNAL(pressed()), this, SLOT(databaseTypeSelectSlot()));
        if (interface_.usedConnection() == it.first)
            radio->setChecked (true);
        grplayout->addWidget (radio);

        connection_stack_->addWidget(it.second->widget());
        QObject::connect(it.second->widget(), SIGNAL(databaseOpenedSignal()),
                         this, SLOT(databaseOpenedSlot()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
    }
    groupBox->setLayout(grplayout);
    layout->addWidget(groupBox);

    layout->addStretch();

    layout->addWidget(connection_stack_);

    setLayout (layout);

    if (interface_.usedConnection().size() > 0)
    {
        useConnection (interface_.usedConnection());
    }
}

DBInterfaceWidget::~DBInterfaceWidget()
{
    connection_stack_ = nullptr;
}

void DBInterfaceWidget::databaseTypeSelectSlot ()
{
    QRadioButton *radio = dynamic_cast <QRadioButton *> (QObject::sender());
    useConnection(radio->text().toStdString());
}

void DBInterfaceWidget::useConnection (std::string connection_type)
{
    interface_.useConnection(connection_type);

    assert (connection_stack_);

    connection_stack_->setCurrentWidget(interface_.connectionWidget());
}

void DBInterfaceWidget::databaseOpenedSlot ()
{
    logdbg << "DBInterfaceWidget: databaseOpenedSlot";
    emit databaseOpenedSignal();
}
